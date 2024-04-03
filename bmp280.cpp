#include "bmp280.h"

#include "myiic.h"

#define MAX_BYTES 10
static int gDeviceStatus;
static int i2c_write(int fd,unsigned int addr,unsigned int offset,unsigned char *buf,unsigned int len)
{
    #ifdef Q_OS_LINUX
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    int i;
    unsigned char _buf[MAX_BYTES+1];

    if(len>MAX_BYTES)
    {
        return -1;
    }

    if(offset+len>256)
    {
        return -1;
    }

    _buf[0]=offset;
    for(i=0;i<len;i++)
    {
        _buf[1+i]=buf[i];
        printf("----_writedata:%x------\n",_buf[1+i]);
    }

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;

    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1+len;
    i2cmsg.buf   = _buf;

    if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
        perror("ioctl()");
        fprintf(stderr,"ioctl returned %d\n",i);
        gDeviceStatus = 0;
        return -1;
    }
    #endif

    gDeviceStatus=1;
    return 0;
}

static int  i2c_read(int fd,unsigned int addr,unsigned int offset,unsigned char *buf,unsigned int len)
{

    #ifdef Q_OS_LINUX
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    int i;

    if(len>MAX_BYTES)
    {
        return -1;
    }

    if(i2c_write(fd,addr,offset,NULL,0)<0)
    {
        return -1;
    }

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;

    i2cmsg.addr  = addr;
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = len;
    i2cmsg.buf   = buf;


    if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
        perror("ioctl()");
        fprintf(stderr,"ioctl returned %d\n",i);
         gDeviceStatus=0;
        return -1;
    }
    #endif
    gDeviceStatus=1;
    return 0;
}







MYBMP280::MYBMP280(QObject *parent) : QObject(parent)
{
    pbmp280 = (BMP280*)(&yesbmp280);
    status = 0;
}

void MYBMP280::setid(int fd)
{
    devfd = fd;
    devaddr = BMP280_ADDRESS;
}





uint8_t MYBMP280::BMP280_Read_Byte(u8 reg)
{

    /*uint8_t rec_data;
    iic1_start();
    iic1_send_byte(BMP280_ADDRESS<<1|0);
    iic1_wait_ack();
    iic1_send_byte(reg);
    iic1_wait_ack();

    iic1_start();
    iic1_send_byte(BMP280_ADDRESS<<1|1);
    iic1_wait_ack();
    rec_data = iic1_read_byte(0);	//不应答
    iic1_stop();*/
    unsigned char rec_data;
    unsigned char buf[2]={0x00,0x00};

    i2c_read(devfd,devaddr, reg,buf,1);
    rec_data  = buf[0];
    return rec_data;

}

void MYBMP280::BMP280_Write_Byte(u8 reg,u8 data)
{
    /*iic1_start();
    iic1_send_byte(BMP280_ADDRESS<<1);
    iic1_wait_ack();
    iic1_send_byte(reg);
    iic1_wait_ack();

    iic1_send_byte(data);
    iic1_wait_ack();
    iic1_stop();*/
    unsigned char buf[2]={0x00,0x00};
    buf[0] = data;

    i2c_write(devfd,devaddr,reg,buf,1);

}




//设置保持时间和滤波器分频因子
void MYBMP280::BMP280_Set_Standby_FILTER(BMP_CONFIG * BMP_Config)
{
    u8 Regtmp;
    Regtmp = ((BMP_Config->T_SB)<<5)|
             ((BMP_Config->FILTER_COEFFICIENT)<<2)|
             ((BMP_Config->SPI_EN));

    BMP280_Write_Byte(BMP280_CONFIG_REG,Regtmp);
}



//获取BMP当前状态
//status_flag = BMP280_MEASURING ||
//			 	BMP280_IM_UPDATE
uint8_t  MYBMP280::BMP280_GetStatus(uint8_t status_flag)
{
    u8 flag;
    flag = BMP280_Read_Byte(BMP280_STATUS_REG);
    if(flag&status_flag)	return 1;
    else return 0;
}





/**************************传感器值转定点值*************************************/
BMP280_S32_t t_fine;			//用于计算补偿
//我用浮点补偿
#ifdef USE_FIXED_POINT_COMPENSATE
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) *
    ((BMP280_S32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
    BMP280_S64_t var1, var2, p;
    var1 = ((BMP280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
    var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
    var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
    var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
    var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
    if (var1 == 0)
    {
    return 0; // avoid exception caused by division by zero
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
    return (BMP280_U32_t)p;
}


/***********************************CUT*************************************/
#else
/**************************传感器值转定点值*************************************/
// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
double MYBMP280::bmp280_compensate_T_double(BMP280_S32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T)/16384.0 - ((double)(pbmp280->T1))/1024.0) * ((double)pbmp280->T2);
    var2 = ((((double)adc_T)/131072.0 - ((double)pbmp280->T1)/8192.0) *
    (((double)adc_T)/131072.0 - ((double) pbmp280->T1)/8192.0)) * ((double)pbmp280->T3);
    t_fine = (BMP280_S32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double MYBMP280::bmp280_compensate_P_double(BMP280_S32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine/2.0) - 64000.0;
    var2 = var1 * var1 * ((double)pbmp280->P6) / 32768.0;
    var2 = var2 + var1 * ((double)pbmp280->P5) * 2.0;
    var2 = (var2/4.0)+(((double)pbmp280->P4) * 65536.0);
    var1 = (((double)pbmp280->P3) * var1 * var1 / 524288.0 + ((double)pbmp280->P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)pbmp280->P1);
    if (var1 == 0.0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)pbmp280->P9) * p * p / 2147483648.0;
    var2 = p * ((double)pbmp280->P8) / 32768.0;
    p = p + (var1 + var2 + ((double)pbmp280->P7)) / 16.0;
    return p;
}
#endif




// //大气压值-Pa
// long BMP280_Get_Pressure(void)
// {
// 	uint8_t XLsb,Lsb, Msb;
// 	long signed Bit32;
// 	//double pressure;
// 	XLsb = BMP280_Read_Byte(BMP280_PRESSURE_XLSB_REG);
// 	Lsb	 = BMP280_Read_Byte(BMP280_PRESSURE_LSB_REG);
// 	Msb	 = BMP280_Read_Byte(BMP280_PRESSURE_MSB_REG);
// 	Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值组合起来
// 	return Bit32 ;
// }


// //大气压值-Pa
// BMP280_U32_t BMP280_Get_Pressure2(void)
// {
// 	uint8_t XLsb,Lsb, Msb;
// 	long signed Bit32;
// 	BMP280_S32_t pressure;
// 	XLsb = BMP280_Read_Byte(BMP280_PRESSURE_XLSB_REG);
// 	Lsb	 = BMP280_Read_Byte(BMP280_PRESSURE_LSB_REG);
// 	Msb	 = BMP280_Read_Byte(BMP280_PRESSURE_MSB_REG);
// 	Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值,组成一个浮点数
// 	pressure = bmp280_compensate_P_int64(Bit32);
// 	return pressure;
// }



/*******************主要部分*********************/
/****************获取传感器精确值****************/
//大气压值-Pa
double MYBMP280::bmp280_get_pressure(void)
{
    uint8_t XLsb,Lsb, Msb;
    long signed Bit32;
    double pressure;
    XLsb = BMP280_Read_Byte(BMP280_PRESSURE_XLSB_REG);
    Lsb	 = BMP280_Read_Byte(BMP280_PRESSURE_LSB_REG);
    Msb	 = BMP280_Read_Byte(BMP280_PRESSURE_MSB_REG);
    Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值,组成一个浮点数
    pressure = bmp280_compensate_P_double(Bit32);
    status = gDeviceStatus;
    return pressure;
}

//温度值-℃
double MYBMP280::bmp280_get_temperature(void)
{
    uint8_t XLsb,Lsb, Msb;
    long signed Bit32;
    double temperature;
    XLsb = BMP280_Read_Byte(BMP280_TEMPERATURE_XLSB_REG);
    Lsb	 = BMP280_Read_Byte(BMP280_TEMPERATURE_LSB_REG);
    Msb	 = BMP280_Read_Byte(BMP280_TEMPERATURE_MSB_REG);
    Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值,组成一个浮点数
    temperature = bmp280_compensate_T_double(Bit32);
    return temperature;
}
/***************************************END OF LINE*********************************************/


uint8_t MYBMP280::bmp280_read_id(void)
{
    return BMP280_Read_Byte(BMP280_CHIPID_REG);
}


//设置BMP过采样因子 MODE
//BMP280_SLEEP_MODE||BMP280_FORCED_MODE||BMP280_NORMAL_MODE
void MYBMP280::BMP280_Set_TemOversamp(BMP_OVERSAMPLE_MODE * Oversample_Mode)
{
    u8 Regtmp;
    Regtmp = ((Oversample_Mode->T_Osample)<<5)|
             ((Oversample_Mode->P_Osample)<<2)|
             ((Oversample_Mode)->WORKMODE);

    BMP280_Write_Byte(BMP280_CTRLMEAS_REG,Regtmp);
}







void MYBMP280::bmp280_init(void)
{
    u8 Lsb,Msb;

    /********************接下来读出矫正参数*********************/
    //温度传感器的矫正值
    Lsb = BMP280_Read_Byte(BMP280_DIG_T1_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_T1_MSB_REG);
    pbmp280->T1 = (((u16)Msb)<<8) + Lsb;			//高位加低位
    Lsb = BMP280_Read_Byte(BMP280_DIG_T2_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_T2_MSB_REG);
    pbmp280->T2 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_T3_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_T3_MSB_REG);
    pbmp280->T3 = (((u16)Msb)<<8) + Lsb;

    //大气压传感器的矫正值
    Lsb = BMP280_Read_Byte(BMP280_DIG_P1_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P1_MSB_REG);
    pbmp280->P1 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P2_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P2_MSB_REG);
    pbmp280->P2 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P3_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P3_MSB_REG);
    pbmp280->P3 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P4_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P4_MSB_REG);
    pbmp280->P4 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P5_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P5_MSB_REG);
    pbmp280->P5 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P6_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P6_MSB_REG);
    pbmp280->P6 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P7_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P7_MSB_REG);
    pbmp280->P7 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P8_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P8_MSB_REG);
    pbmp280->P8 = (((u16)Msb)<<8) + Lsb;
    Lsb = BMP280_Read_Byte(BMP280_DIG_P9_LSB_REG);
    Msb = BMP280_Read_Byte(BMP280_DIG_P9_MSB_REG);
    pbmp280->P9 = (((u16)Msb)<<8) + Lsb;
    /******************************************************/
    BMP280_Write_Byte(BMP280_RESET_REG,BMP280_RESET_VALUE);	//往复位寄存器写入给定值

    BMP_OVERSAMPLE_MODE			BMP_OVERSAMPLE_MODEStructure;
    BMP_OVERSAMPLE_MODEStructure.P_Osample = BMP280_P_MODE_3;
    BMP_OVERSAMPLE_MODEStructure.T_Osample = BMP280_T_MODE_1;
    BMP_OVERSAMPLE_MODEStructure.WORKMODE  = BMP280_NORMAL_MODE;
    BMP280_Set_TemOversamp(&BMP_OVERSAMPLE_MODEStructure);

    BMP_CONFIG					BMP_CONFIGStructure;
    BMP_CONFIGStructure.T_SB = BMP280_T_SB1;
    BMP_CONFIGStructure.FILTER_COEFFICIENT = BMP280_FILTER_MODE_4;
    BMP_CONFIGStructure.SPI_EN = DISABLE;

    BMP280_Set_Standby_FILTER(&BMP_CONFIGStructure);
}
