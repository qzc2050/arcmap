#include "ads118.h"
#include "myiic.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QApplication>
#include "mainwindow.h"
#define MAX_BYTES 10

#define TEMP_MAX  226

//温度参考电阻
#define TEMPERATURE_REFERENCE_RES (10.0f)    //10K


//热敏电阻阻值表   型号：CMFB 103F3435 （10K+—1%）
//5℃-50℃电阻值表，一个数据间隔一℃
const double resistor_vale_table[] =
{
    22.165, 21.253, 20.384, 19.555, 18.764, 18.010, 17.290, 16.602, 15.946, 15.319,

    14.720, 14.148, 13.601, 13.078, 12.578, 12.099, 11.642, 11.204, 10.785, 10.384,

    10.000, 9.632, 9.280, 8.943, 8.619, 8.309, 8.012, 7.727, 7.453, 7.191,

    6.939, 6.698, 6.466, 6.243, 6.029, 5.824, 5.627, 5.437, 5.255, 5.080,

    4.911, 4.749, 4.593, 4.443, 4.299, 4.160,
};


// 温度查表 5-50度   0.2°C精度 226位   10k电阻分压   3.3V供电  3.3V部参考电压
const uint16_t temp_voltage[TEMP_MAX] =
{
    18191,/*5.0°*/ 18143,/*5.2°*/ 18095,/*5.4°*/ 18047,/*5.6°*/ 17999,/*5.8°*/
    17952,/*6.0°*/ 17903,/*6.2°*/ 17855,/*6.4°*/ 17806,/*6.6°*/ 17758,/*6.8°*/
    17710,/*7.0°*/ 17661,/*7.2°*/ 17612,/*7.4°*/ 17563,/*7.6°*/ 17514,/*7.8°*/
    17466,/*8.0°*/ 17417,/*8.2°*/ 17368,/*8.4°*/ 17319,/*8.6°*/ 17270,/*8.8°*/
    17221,/*9.0°*/ 17171,/*9.2°*/ 17122,/*9.4°*/ 17072,/*9.6°*/ 17023,/*9.8°*/
    16974,/*10.0°*/ 16924,/*10.2°*/ 16874,/*10.4°*/ 16824,/*10.6°*/ 16774,/*10.8°*/
    16725,/*11.0°*/ 16675,/*11.2°*/ 16625,/*11.4°*/ 16575,/*11.6°*/ 16525,/*11.8°*/
    16475,/*12.0°*/ 16424,/*12.2°*/ 16374,/*12.4°*/ 16324,/*12.6°*/ 16274,/*12.8°*/
    16224,/*13.0°*/ 16173,/*13.2°*/ 16123,/*13.4°*/ 16072,/*13.6°*/ 16022,/*13.8°*/
    15972,/*14.0°*/ 15921,/*14.2°*/ 15870,/*14.4°*/ 15820,/*14.6°*/ 15769,/*14.8°*/
    15719,/*15.0°*/ 15668,/*15.2°*/ 15617,/*15.4°*/ 15567,/*15.6°*/ 15516,/*15.8°*/
    15466,/*16.0°*/ 15415,/*16.2°*/ 15364,/*16.4°*/ 15314,/*16.6°*/ 15263,/*16.8°*/
    15213,/*17.0°*/ 15162,/*17.2°*/ 15111,/*17.4°*/ 15061,/*17.6°*/ 15010,/*17.8°*/
    14960,/*18.0°*/ 14909,/*18.2°*/ 14858,/*18.4°*/ 14807,/*18.6°*/ 14756,/*18.8°*/
    14706,/*19.0°*/ 14655,/*19.2°*/ 14604,/*19.4°*/ 14554,/*19.6°*/ 14503,/*19.8°*/
    14453,/*20.0°*/ 14402,/*20.2°*/ 14352,/*20.4°*/ 14301,/*20.6°*/ 14251,/*20.8°*/
    14201,/*21.0°*/ 14150,/*21.2°*/ 14100,/*21.4°*/ 14049,/*21.6°*/ 13999,/*21.8°*/
    13949,/*22.0°*/ 13898,/*22.2°*/ 13848,/*22.4°*/ 13798,/*22.6°*/ 13748,/*22.8°*/
    13698,/*23.0°*/ 13648,/*23.2°*/ 13598,/*23.4°*/ 13548,/*23.6°*/ 13498,/*23.8°*/
    13448,/*24.0°*/ 13398,/*24.2°*/ 13348,/*24.4°*/ 13298,/*24.6°*/ 13248,/*24.8°*/
    13199,/*25.0°*/ 13149,/*25.2°*/ 13100,/*25.4°*/ 13050,/*25.6°*/ 13001,/*25.8°*/
    12952,/*26.0°*/ 12902,/*26.2°*/ 12853,/*26.4°*/ 12804,/*26.6°*/ 12755,/*26.8°*/
    12706,/*27.0°*/ 12657,/*27.2°*/ 12608,/*27.4°*/ 12560,/*27.6°*/ 12511,/*27.8°*/
    12463,/*28.0°*/ 12414,/*28.2°*/ 12365,/*28.4°*/ 12317,/*28.6°*/ 12268,/*28.8°*/
    12220,/*29.0°*/ 12172,/*29.2°*/ 12124,/*29.4°*/ 12076,/*29.6°*/ 12028,/*29.8°*/
    11980,/*30.0°*/ 11932,/*30.2°*/ 11884,/*30.4°*/ 11837,/*30.6°*/ 11789,/*30.8°*/
    11742,/*31.0°*/ 11695,/*31.2°*/ 11648,/*31.4°*/ 11601,/*31.6°*/ 11554,/*31.8°*/
    11507,/*32.0°*/ 11460,/*32.2°*/ 11413,/*32.4°*/ 11366,/*32.6°*/ 11319,/*32.8°*/
    11273,/*33.0°*/ 11226,/*33.2°*/ 11180,/*33.4°*/ 11134,/*33.6°*/ 11088,/*33.8°*/
    11042,/*34.0°*/ 10996,/*34.2°*/ 10950,/*34.4°*/ 10905,/*34.6°*/ 10859,/*34.8°*/
    10814,/*35.0°*/ 10769,/*35.2°*/ 10724,/*35.4°*/ 10679,/*35.6°*/ 10634,/*35.8°*/
    10589,/*36.0°*/ 10544,/*36.2°*/ 10499,/*36.4°*/ 10455,/*36.6°*/ 10410,/*36.8°*/
    10366,/*37.0°*/ 10322,/*37.2°*/ 10278,/*37.4°*/ 10234,/*37.6°*/ 10190,/*37.8°*/
    10146,/*38.0°*/ 10102,/*38.2°*/ 10059,/*38.4°*/ 10015,/*38.6°*/ 9972,/*38.8°*/
    9929,/*39.0°*/ 9886,/*39.2°*/ 9843,/*39.4°*/ 9801,/*39.6°*/ 9758,/*39.8°*/
    9716,/*40.0°*/ 9673,/*40.2°*/ 9631,/*40.4°*/ 9589,/*40.6°*/ 9547,/*40.8°*/
    9505,/*41.0°*/ 9463,/*41.2°*/ 9421,/*41.4°*/ 9380,/*41.6°*/ 9338,/*41.8°*/
    9297,/*42.0°*/ 9256,/*42.2°*/ 9215,/*42.4°*/ 9174,/*42.6°*/ 9133,/*42.8°*/
    9093,/*43.0°*/ 9053,/*43.2°*/ 9013,/*43.4°*/ 8973,/*43.6°*/ 8933,/*43.8°*/
    8893,/*44.0°*/ 8853,/*44.2°*/ 8813,/*44.4°*/ 8773,/*44.6°*/ 8733,/*44.8°*/
    8694,/*45.0°*/ 8655,/*45.2°*/ 8616,/*45.4°*/ 8577,/*45.6°*/ 8538,/*45.8°*/
    8500,/*46.0°*/ 8461,/*46.2°*/ 8423,/*46.4°*/ 8384,/*46.6°*/ 8346,/*46.8°*/
    8308,/*47.0°*/ 8270,/*47.2°*/ 8233,/*47.4°*/ 8195,/*47.6°*/ 8158,/*47.8°*/
    8121,/*48.0°*/ 8084,/*48.2°*/ 8047,/*48.4°*/ 8010,/*48.6°*/ 7973,/*48.8°*/
    7936,/*49.0°*/ 7899,/*49.2°*/ 7863,/*49.4°*/ 7827,/*49.6°*/ 7791,/*49.8°*/
    7755,/*50.0°*/
};


//参考电压
#define ADS1115_REFERENCE_VOL (3.3f)    //3.3V
//ad最大测量正电压
#define ADS1115_MAX_MEASURED_POSITIVE_VOL (4.096f)    //单位V（0-4.096）
//ad最大测量正电压对应ad值
#define ADS1115_MAX_MEASURED_POSITIVE_AD_VALUE (0x7FFF)    //（0-7FFF）

//读写命令+地址
#define ADS1115_WRITE_ADDRESS        0x90
#define ADS1115_READ_ADDRESS         0x91

//以下两个预处理作为配置寄存器预处理要更改其配在这里更改
//也可以添加新的预处理对不同通道的采集或者选取不同的采样速率等
#define CONFIG_REG_H     ADS1115_REG_CONFIG_OS_START|\
                         ADS1115_REG_CONFIG_PGA_4|\
                         ADS1115_REG_CONFIG_MODE_SINGLE


#define CONFIG_REG_L    ADS1115_REG_CONFIG_DR_128|\
                        ADS1115_REG_CONFIG_COMP_MODE_TRADITIONAL|\
                        ADS1115_REG_CONFIG_COMP_POL_LOW|\
                        ADS1115_REG_CONFIG_COMP_LAT_NONLATCH|\
                        ADS1115_REG_CONFIG_COMP_QUE_DIS


/***************/
//配置寄存器说明

//地址指针寄存器（即寄存器地址）
#define ADS1015_REG_POINTER_CONVERT     (0x00)  //转换结果寄存器
#define ADS1015_REG_POINTER_CONFIG      (0x01)  //工作状态等配置寄存器
#define ADS1015_REG_POINTER_LOWTHRESH   (0x02)   //阀值比较寄存器高字节寄存器
#define ADS1015_REG_POINTER_HITHRESH    (0x03)  //阀值比较寄存器低字节寄存器


//config register（工作状态等配置寄存器）
/*CRH[15:8](R/W)
   BIT      15      14      13      12      11      10      9       8
   NAME     OS      MUX2    MUX1    MUX0    PGA2    PGA1    PGA0    MODE
CRL[7:0] (R/W)
   BIT      7       6       5       4       3       2       1       0
   NAME    DR0     DR1     DR0   COM_MODE COM_POL COM_LAT COM_QUE1 COM_QUE0


   -----------------------------------------------------------------------------------
 * 15    | OS             |  运行状态会单词转换开始
 *       |                | 写时:
 *       |                | 0   : 无效
 *       |                | 1   : 开始单次转换处于掉电状态时
 *       |                | 读时:
 *       |                | 0   : 正在转换
 *       |                | 1   : 未执行转换
 * -----------------------------------------------------------------------------------
 * 14:12 | MUX [2:0]      | 输入复用多路配置
 *       |                | 000 : AINP = AIN0 and AINN = AIN1 (default)
 *       |                | 001 : AINP = AIN0 and AINN = AIN3
 *       |                | 010 : AINP = AIN1 and AINN = AIN3
 *       |                | 011 : AINP = AIN2 and AINN = AIN3
 *       |                | 100 : AINP = AIN0 and AINN = GND
 *       |                | 101 : AINP = AIN1 and AINN = GND
 *       |                | 110 : AINP = AIN2 and AINN = GND
 *       |                | 111 : AINP = AIN3 and AINN = GND
 * -----------------------------------------------------------------------------------
 * 11:9  | PGA [2:0]      | 可编程增益放大器配置(FSR  full scale range)
 *       |                | 000 : FSR = В±6.144 V
 *       |                | 001 : FSR = В±4.096 V
 *       |                | 010 : FSR = В±2.048 V (默认)
 *       |                | 011 : FSR = В±1.024 V
 *       |                | 100 : FSR = В±0.512 V
 *       |                | 101 : FSR = В±0.256 V
 *       |                | 110 : FSR = В±0.256 V
 *       |                | 111 : FSR = В±0.256 V
 * -----------------------------------------------------------------------------------
 * 8     | MODE           | 工作模式
 *       |                | 0   : 连续转换
 *       |                | 1   : 单次转换
 * -----------------------------------------------------------------------------------
 * 7:5   | DR [2:0]       | 采样频率 (采样速率不宜过高，当采样率大于128SPS时，分辨率在不同量程下均有所下降)
 *       |                | 000 : 8 SPS
 *       |                | 001 : 16 SPS
 *       |                | 010 : 32 SPS
 *       |                | 011 : 64 SPS
 *       |                | 100 : 128 SPS (默认)
 *       |                | 101 : 250 SPS
 *       |                | 110 : 475 SPS
 *       |                | 111 : 860 SPS
 * -----------------------------------------------------------------------------------
 * 4     | COMP_MODE      | 比较器模式
 *       |                | 0   : 传统比较器 (default)
 *       |                | 1   : 窗口比较器
 * -----------------------------------------------------------------------------------
 * 3     | COMP_POL       | Comparator polarity
 *       |                | 0   : 低电平有效 (default)
 *       |                | 1   : 高电平有效
 * -----------------------------------------------------------------------------------
 * 2     | COMP_LAT       | Latching comparator
 *       |                | 0   : 非锁存比较器. (default)
 *       |                | 1   : 锁存比较器.
 * -----------------------------------------------------------------------------------
 * 1:0   | COMP_QUE [1:0] | Comparator queue and disable
 *       |                | 00  : Assert after one conversion
 *       |                | 01  : Assert after two conversions
 *       |                | 10  : Assert after four conversions
 *       |                | 11  : 禁用比较器并将ALERT/RDY设置为高阻抗 (default)
 * -----------------------------------------------------------------------------------
*/


// 单次转换开始
#define ADS1115_REG_CONFIG_OS_START                     (0x1U << 7)//设备单词转换开启 高字节的最高位
#define ADS1115_REG_CONFIG_OS_NULL                      (0x0U << 7)

//输入引脚选择和输入方式选择
#define ADS1115_REG_CONFIG_MUX_Diff_01                  (0x0U << 4)  // 差分输入0引脚和1引脚（默认）
#define ADS1115_REG_CONFIG_MUX_Diff_03                  (0x1U << 4)  // 差分输入0引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_Diff_13                  (0x2U << 4)  // 差分输入1引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_Diff_23                  (0x3U << 4)  // 差分输入2引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_SINGLE_0                 (0x4U << 4)  //单端输入 0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1                 (0x5U << 4)  //单端输入 1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2                 (0x6U << 4)  //单端输入 2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3                 (0x7U << 4)  //单端输入 3

//量程选择
#define ADS1115_REG_CONFIG_PGA_6                        (0x0U << 1) // +/- 6.1144
#define ADS1115_REG_CONFIG_PGA_4                        (0x1U << 1) // +/- 4.096
#define ADS1115_REG_CONFIG_PGA_2                        (0x2U << 1) // +/- 2.048（默认）
#define ADS1115_REG_CONFIG_PGA_1                        (0x3U << 1) // +/- 1.024
#define ADS1115_REG_CONFIG_PGA_05                       (0x4U << 1) // +/- 0.512
#define ADS1115_REG_CONFIG_PGA_02                       (0x5U << 1) // +/- 0.256

//运行方式

#define ADS1115_REG_CONFIG_MODE_SINGLE              (0x1U << 0)  //  单次
#define ADS1115_REG_CONFIG_MODE_CONTIN              (0x0U << 0)  //连续转换（默认）

//转换速率

#define ADS1115_REG_CONFIG_DR_8                     (0x0U << 5)
#define ADS1115_REG_CONFIG_DR_16                    (0x1U << 5)
#define ADS1115_REG_CONFIG_DR_32                    (0x2U << 5)
#define ADS1115_REG_CONFIG_DR_64                    (0x3U << 5)
#define ADS1115_REG_CONFIG_DR_128                   (0x4U << 5)//（默认）
#define ADS1115_REG_CONFIG_DR_250                   (0x5U << 5)
#define ADS1115_REG_CONFIG_DR_475                   (0x6U << 5)
#define ADS1115_REG_CONFIG_DR_860                   (0x7U << 5)

//比较器模式
#define ADS1115_REG_CONFIG_COMP_MODE_TRADITIONAL    (0x0U << 4)//（默认）
#define ADS1115_REG_CONFIG_COMP_MODE_WINDOW         (0x1U << 4)


#define ADS1115_REG_CONFIG_COMP_POL_LOW         (0x0U << 3)//（默认）
#define ADS1115_REG_CONFIG_COMP_POL_HIG         (0x1U << 3)


#define ADS1115_REG_CONFIG_COMP_LAT_NONLATCH        (0x0U << 2) //（默认）
#define ADS1115_REG_CONFIG_COMP_LAT_LATCH           (0x1U << 2)


#define ADS1115_REG_CONFIG_COMP_QUE_ONE         (0x0U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_TWO         (0x1U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_THR         (0x2U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_DIS         (0x3U << 0) //(默认)

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
       // printf("----_writedata:%x------\n",_buf[1+i]);
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

    gDeviceStatus = 1;
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
        gDeviceStatus = 0;
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
        return -1;
    }
    #endif
    gDeviceStatus = 1;
    return 0;
}

ADS118::ADS118(QObject *parent) : QObject(parent)
{
    temprate0=0;
    temprate1=0;
    temprate2=0;
    temprate3=0;

    icout=0;
    icout0=1;
    icout1=1;
    icout2=1;
    icout3=1;



    status=0;//0 表示ok，1 iic通信不正常
    currentch=0;
    devaddr = ADS1115_READ_ADDRESS/2;
    icout=0;
    adstimer = new QTimer();

    //adstimer->setInterval(1000);

    //非工作状态
    adstimer->start(1000);
    connect(adstimer,SIGNAL(timeout()),this,SLOT(doadstimer()));

}

void ADS118::ads1115init()
{
    ads1115_config_channel(0);
}

void ADS118::setid(int fd)
{
    devfd = fd;
}

void ADS118::doadstimer()
{

    status = gDeviceStatus;
    GpsystemStatus.temp = GpsystemStatus.temp1;//(GpsystemStatus.temp1+GpsystemStatus.temp2+GpsystemStatus.temp3+GpsystemStatus.temp4)/4;
    GpsystemStatus.temptatus = status;
    switch (icout%4)
    {
        case 0:
            GpsystemStatus.temp1 = ads1115_read_ad_vale(0);


        break;

        case 1:
            //GpsystemStatus.temp2 = 0;


        break;

        case 2:
            //GpsystemStatus.temp3 = 0;


        break;

        case 3:
            //GpsystemStatus.temp4 = 0;
        break;

    }




    icout++;
}


unsigned short int ADS118::ads1115_read_two_byte(unsigned char ReadAddr)
{
    uint16_t value=0;
    uint8_t rxbuffer[10];

    i2c_read(devfd,devaddr, ReadAddr,rxbuffer,2);

   // alt_avalon_i2c_master_tx_rx(i2c_dev, txbuffer, 1, rxbuffer, 2,ALT_AVALON_I2C_NO_INTERRUPTS);
    value = (int16_t)rxbuffer[0]<<8|rxbuffer[1];

    return *(uint16_t *)&value;
}



void  ADS118::ads1115_write_two_byte(unsigned char WriteAddr,unsigned short int DataToWrite)
{
    uint8_t txbuffer[10];
    uint8_t rxbuffer[10];


    txbuffer[0]= (uint8_t)((DataToWrite & 0xff00) >> 8);
    txbuffer[1]= (uint8_t)(DataToWrite & 0x00ff);
    i2c_write(devfd,devaddr, WriteAddr,txbuffer,2);
}

//配置通道 单次模式。
/*******************Config寄存器*********************/
void ADS118::ads1115_config_channel(uint8_t channel)
{

    uint8_t reg_addr = ADS1015_REG_POINTER_CONFIG;    // 指向配置寄存器
    uint8_t config_h_byte;
    uint8_t config_l_byte;
    uint16_t configdata;
    currentch = channel;

    // switch(channel) //通道选择
    // {
    //     case 0: config_h_byte = 0xC4; break;
    //     case 1: config_h_byte = 0xD4; break;
    //     case 2: config_h_byte = 0xE4; break;
    //     case 3: config_h_byte = 0xF4; break;
    // }

    switch(channel) //通道选择
    {
        case 0: config_h_byte = (CONFIG_REG_H | ADS1115_REG_CONFIG_MUX_SINGLE_0); break;
        case 1: config_h_byte = (CONFIG_REG_H | ADS1115_REG_CONFIG_MUX_SINGLE_1); break;
        case 2: config_h_byte = (CONFIG_REG_H | ADS1115_REG_CONFIG_MUX_SINGLE_2); break;
        case 3: config_h_byte = (CONFIG_REG_H | ADS1115_REG_CONFIG_MUX_SINGLE_3); break;
    }



    //禁用比较器并将ALERT / RDY引脚设置为高阻抗（默认）
    config_l_byte = CONFIG_REG_L; // 配置字低字节

    configdata = (((uint16_t)(config_h_byte)))<<8|CONFIG_REG_L;

    ads1115_write_two_byte(reg_addr,configdata);





//    //…
//    voltage = getVoltageFromDigit(data);


//    /*iic1_start();

//    iic1_send_byte(ADS1115_WRITE_ADDRESS);	//发从机地址
//    iic1_wait_ack();
//    iic1_send_byte(reg_addr);	//发送配置寄存器地址
//    iic1_wait_ack();
//    iic1_send_byte(config_h_byte);	//写数据
//    iic1_wait_ack();
//    iic1_send_byte(config_l_byte);
//    iic1_wait_ack();

//    iic1_stop();*/
//    QTimer::singleShot(200, this, SLOT(ads1115_read_ad_vale()));

    // QTimer::singleShot(100, [/* your code here */](){ /* callback function */ });
    // QTimer::singleShot(100,this,ads1115_read_ad_vale());
}


/**************获取通道转换结果***************/
float  ADS118::ads1115_read_ad_vale(uint8_t channel)
{


    float fvol;
    float fres;
    ads1115_config_channel(channel);
    currentch = channel;
#if 1
    QElapsedTimer t;
    t.start();
    while(t.elapsed() < 10)
    {
        QApplication::processEvents();
    }
#endif
    t.restart();
    // 读取配置,若一直在转换则循环读取
    uint16_t readConfig=1;
    do {
        readConfig = ads1115_read_two_byte(ADS1015_REG_POINTER_CONFIG);

        QApplication::processEvents();
        if(t.elapsed()>20)
        {
            break;
        }

    }while((readConfig & 0x8000) == 0);

    uint16_t vol = 0;
    uint16_t lev = 0;
    uint16_t ret = 0;
    int i;
    vol = ads1115_read_two_byte(ADS1015_REG_POINTER_CONVERT);

    fvol = (float (vol)*ADS1115_MAX_MEASURED_POSITIVE_VOL)/ADS1115_MAX_MEASURED_POSITIVE_AD_VALUE;
    qDebug()<<"ads1115_read_ad_vale channel "<< channel<<"value" <<fvol;
   // qDebug()<<"ads1115_read_vol_vale"<<fvol;

    //3.3 v时是开路状态


    // /******/
     if(vol < temp_voltage[100])   //对应25度开始查表
     {
        for( i = 100; i < TEMP_MAX; ++i)
        {
            if(vol > temp_voltage[i]) break;
            lev = i;
        }
     }
     else
     {
        for(i = 100; i > 0; --i)
        {
            if(vol < temp_voltage[i]) break;
            lev = i - 1;
        }
     }

     ret = 250 + 2 * (lev - 100);

     fres = ret/10.0;

    // qDebug()<<"temp *10"<<ret;



    switch (currentch)
    {
        case 0:


            temprate0 = ret;
        break;

        case 1:
            temprate1 = ret;

            break;

        case 2:
            temprate2 = ret;

            break;

        case 3:
             temprate3 = ret;

            break;
        default:
            break;

    }
    return fres;


}










