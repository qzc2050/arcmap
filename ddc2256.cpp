#include "ddc2256.h"

#include "mainwindow.h"
#include <QCoreApplication>
#include "QElapsedTimer"
#include "QTime"
#ifdef Q_OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define GPIO_DIR_IN	0
#define GPIO_DIR_OUT	1

#define GPIO_DDI  41

//pi15
#define GPIO_CS1  271

//pi16
#define GPIO_CS2  272


//pi13
#define GPIO_CS3  269

//pi14
#define GPIO_CS4  270

static int gpio_export(unsigned gpio)
{
    int fd, len;
    char buf[11];

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

static int gpio_unexport(unsigned gpio)
{
    int fd, len;
    char buf[11];

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);
    return 0;
}

static int gpio_dir(unsigned gpio, unsigned dir)
{
    int fd, len;
    char buf[60];

    len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

    if (dir == GPIO_DIR_OUT)
        write(fd, "out", 4);
    else
        write(fd, "in", 3);

    close(fd);
    return 0;
}

static int gpio_dir_out(unsigned gpio)
{
    return gpio_dir(gpio, GPIO_DIR_OUT);
}

static int gpio_dir_in(unsigned gpio)
{
    return gpio_dir(gpio, GPIO_DIR_IN);
}

static int gpio_value(unsigned gpio, unsigned value)
{
    int fd, len;
    char buf[60];

    len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/value");
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

#endif

struct ddc2556_data_type_ ddc2556_data[9];
struct ddc2556_data_type_ ddc2556_datacp[9];
volatile uint32_t ddc2556_data_upload_timer = -1;//上传数据倒计时
uint32_t ddc2556_data_wait_timer;	//
volatile uint8_t ddc2556_data_upload_flag = 0;
//static INT32U ddc2556_integr_cap_table[] = {10000, 5000, 2500, 1250, 6250, 15000, 7500, 3750};
volatile uint8_t ddc2556_conv_state = 0;
//271 272

static int gDeviceStatus;
void ddc2256::delayMs(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//检查ddc2556_data[p_chip*2+ic].adc_vale[i + 128] 256个数据是否大于1000，如果大于1000本底异常，173 测试通道除外
#define  BENDI_MOSTVALUE 500
void ddc2256::check_bendi()
{
    //chip 0 到7 ，通道0到255
    int i;
    int p_chip = 0;
    int icoutling=0;
    GpsystemStatus.bendi = 0;
    qDebug()<<"check bendi";
    for (p_chip = 0; p_chip < 4; p_chip++)
    {
		for (i = 0; i < 256; i++)
		{
			if (ddc2556_data[p_chip].adc_vale[i] > BENDI_MOSTVALUE)
			{
				if (i != 173)
				{
                    //本底异常
					GpsystemStatus.bendi = 1;
                    //qDebug()<<"why the challes > is `````````````````````````````````````````"<<i<<ddc2556_data[p_chip].adc_vale[i];
				}
			}
            if(ddc2556_data[p_chip].adc_vale[i]<50)
            {
                if (i != 173)
                {
                    //本底异常
                    GpsystemStatus.bendi = 1;
                    //qDebug()<<"why the challes < is ````````````````````````````````````"<<i<<ddc2556_data[p_chip].adc_vale[i];
                }
            }
		}
	}
}
ddc2256::ddc2256(QObject *parent) : QThread(parent)
{

    // 打开SPI设备文件
    devfd=-1;
    status=0;
    #ifdef Q_OS_LINUX
    devfd = open("/dev/spidev1.0", O_RDWR);
    if (devfd < 0) {
        perror("Failed to open SPI device");

    }


    // 配置SPI传输参数
//    int mode = SPI_MODE_3;  // CPOL=1, CPHA=1
//    ioctl(fd, SPI_IOC_WR_MODE, &mode);  // 设置SPI模式

    int mode = SPI_MODE_3;    // SPI工作模式
     //mode &= ~SPI_CS_HIGH;  //0x04


    int bits = 8;             // 每个字节的位数
    int speed = 1000000;      // 传输速度（4MHz）

    ioctl(devfd, SPI_IOC_WR_MODE, &mode);       // 设置SPI模式
    ioctl(devfd, SPI_IOC_WR_BITS_PER_WORD, &bits);   // 设置每个字节的位数
    ioctl(devfd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);    // 设置传输速度


    // 获取当前模式
    if (ioctl(devfd, SPI_IOC_RD_MODE, &mode) < 0) {
        perror("Failed to get SPI mode");

    }
    printf("the mode is %02x\n",mode);


    // 将 NSS 设置为软件控制模式
    mode &= ~SPI_CS_HIGH;  // 清除自动控制标志位

    if (ioctl(devfd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");

    }


    // 获取当前模式
    if (ioctl(devfd, SPI_IOC_RD_MODE, &mode) < 0) {
        perror("Failed to get SPI mode");

    }
    printf("the mode22222 is %02x\n",mode);

    //DDC_ID1          PB2/PWM0                 1*32+2 = 34
    gpio_export(GPIO_DDI);
    gpio_dir(GPIO_DDI,GPIO_DIR_OUT);


    gpio_export(GPIO_CS1);
    gpio_dir(GPIO_CS1,GPIO_DIR_OUT);


    gpio_export(GPIO_CS2);
    gpio_dir(GPIO_CS2,GPIO_DIR_OUT);


    gpio_export(GPIO_CS3);
    gpio_dir(GPIO_CS3,GPIO_DIR_OUT);


    gpio_export(GPIO_CS4);
    gpio_dir(GPIO_CS4,GPIO_DIR_OUT);

    SPI1_read_write_byte(0Xff);

    gpio_value(GPIO_DDI, 1);
    gpio_value(GPIO_CS1, 1);
    gpio_value(GPIO_CS2, 1);
    gpio_value(GPIO_CS3, 1);
    gpio_value(GPIO_CS4, 1);
    #endif

     tmer100ms = new QTimer();
     //tmer100ms->start(100);
     connect(tmer100ms,SIGNAL(timeout()),this,SLOT(dotmer100ms()));

     tmer500ms = new QTimer();

     connect(tmer500ms,SIGNAL(timeout()),this,SLOT(dotmer50ms()));


     //tmer100ms->start(100);
     //tmer500ms->start(30);

     uint32_t fpga_state = ddc2256_read_fpga_state(0);

     uint32_t fpga_state2 = ddc2256_read_fpga_state(1);

     uint32_t fpga_state3 = ddc2256_read_fpga_state(2);

     uint32_t fpga_state4 = ddc2256_read_fpga_state(3);
     printf("chip 0 fpga_state %d\r\nchip 1 fpga_state %d\r\nchip 2 fpga_state %d\r\nchip 3 fpga_state %d\r\n",fpga_state, fpga_state2,fpga_state3, fpga_state4);

}
void ddc2256::run()
{
    QThread::msleep(500);//等待其他初始化
    QElapsedTimer timer; // 使用QElapsedTimer来计时
    int iicount = 0;
    GpsystemStatus.bendi = 0;
    while (1)
    {
        timer.start(); // 开始计时
        iicount++;
        #ifdef Q_OS_LINUX
            gpio_value(GPIO_DDI, (iicount & 1));
        #endif



        int res0 = 1, res1 = 1, res2 = 1, res3 = 1,res4 = 1, res5 = 1, res6 = 1, res7 = 1;
        bool collected = false; // 用于标记是否已经收集到所有必要的数据
        bool send = false;

        QThread::msleep(10);
        while(timer.elapsed() < 99) // 循环直到超过100ms
        {




            if(res3 != 0) res3 = read_ddc2256_adc_data(1, 1);
            //QThread::msleep(1);
            if(res1 != 0) res1 = read_ddc2256_adc_data(0, 1);
            //QThread::msleep(1);
            if(res2 != 0) res2 = read_ddc2256_adc_data(1, 0);
            //QThread::msleep(1);
            if(res0 != 0) res0 = read_ddc2256_adc_data(0, 0);



             if(res7 != 0) res7 = read_ddc2256_adc_data(3, 1);

            //QThread::msleep(1);
            if(res5 != 0) res5 = read_ddc2256_adc_data(2, 1);
            //QThread::msleep(1);
            if(res6 != 0) res6 = read_ddc2256_adc_data(3, 0);
            //QThread::msleep(1);


            if(res4 != 0) res4 = read_ddc2256_adc_data(2, 0);


            //if((0 == res0) || (0 == res1) || (0 == res2) || (0 == res3))
            if((0 == res0) && (0 == res1) && (0 == res2) && (0 == res3)&&(0 == res4) && (0 == res5) && (0 == res6) && (0 == res7))
            {
                collected = true;
               // break; // 如果四个地址的数据都已采集，则跳出循环
                if(!send)
                {
                    emit onepictureok(); // 发送信号

                    send = true;
                }
            }
            //QThread::msleep(10);
            QThread::msleep(1);
        }
        //检查ddc2556_data[p_chip*2+ic].adc_vale[i + 128] 256个数据是否大于1000，如果大于1000本底异常，173 测试通道除外
        if(100<GpsystemStatus.chip0)
        {
            //qDebug()<<GpsystemStatus.chip0;
            if(GpsystemStatus.chip0<200)
            {
                //check_bendi();
            }
        }





        if(!collected) // 如果不是所有数据都采集到
        {

            if((0 == res0) || (0 == res1) || (0 == res2) || (0 == res3)||(0 == res4) || (0 == res5) || (0 == res6) || (0 == res7))
            {
                int i=0;
                if(0 != res0)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[0].adc_vale[i] = 0;
                    }
                }
                if(0 != res1)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[1].adc_vale[i] = 0;
                    }
                }
                if(0 != res2)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[2].adc_vale[i] = 0;
                    }
                }
                if(0 != res3)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[3].adc_vale[i] = 0;
                    }
                }
                if(0 != res4)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[4].adc_vale[i] = 0;
                    }
                }
                if(0 != res5)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[5].adc_vale[i] = 0;
                    }
                }
                if(0 != res6)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[6].adc_vale[i] = 0;
                    }
                }
                if(0 != res7)
                {
                    for (i = 0; i < 256; i++)
                    {
                        ddc2556_data[7].adc_vale[i] = 0;
                    }
                }
                for (int i = 0; i < 9; i++) {
                    memcpy(&ddc2556_datacp[i], &ddc2556_data[i], sizeof(struct ddc2556_data_type_));
                }
                emit onepictureok(); // 发送信号
            }

        }


    }
}
void ddc2256::dotmer100ms()
{
    icount++;
    #ifdef Q_OS_LINUX
        gpio_value(GPIO_DDI, (icount & 1));
    #endif

    tmer500ms->start(20);

}



void ddc2256::dotmer50ms()
{


    tmer500ms->stop();

    for( int i=0;i<7;i++)
    {
        for(int j=0;j<256;j++)
        {
            ddc2556_data[i].adc_vale[j]=0;

        }
        //ddc2556_data[i].adc_vale[i]
    }
    int res=1,res0=1,res1=1,res2=2,res3=1;
    //if()
    res0 = read_ddc2256_adc_data(0,0);
//    delayMs(1);
    res1 = read_ddc2256_adc_data(0,1);
    res2 = read_ddc2256_adc_data(1,0);
    res3 = read_ddc2256_adc_data(1,1);
    //delayMs(1);


    //res2 = read_ddc2256_adc_data(1,0);
    res = res0*res1*res2*res3;
    //if((0==res0)&&(0==res1)&&(0==res2)&&(0==res3))
    if((0==res0)||(0==res1)||(0==res2)||(0==res3))
    {
        emit  onepictureok();
    }
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
uint8_t ddc2256::SPI1_read_write_byte(uint8_t byte)
{

    unsigned char tx_buffer[1]={0};
    unsigned char rx_buffer[1]={0};

    #ifdef Q_OS_LINUX
    struct spi_ioc_transfer transfer;

    memset(&transfer,0,sizeof(struct spi_ioc_transfer));
    tx_buffer[0] = byte;
    transfer.tx_buf = (unsigned long)tx_buffer;
    transfer.rx_buf  =(unsigned long) rx_buffer;
    transfer.len = 1;

    int ret = ioctl(devfd, SPI_IOC_MESSAGE(1), &transfer);


    if (ret < 1)
    {
        //perror("ioctl()");
        //fprintf(stderr,"ioctl returned %d\n",ret);
    }
    #endif

    //printf("%02x,rx_buffer[0]\n",rx_buffer[0]);
    return rx_buffer[0];



}
/************************************************************
 * 功能: ddc2256读寄存器多个字节
 * 形参: p_chip:选择与那个芯片通信 reg_s_addr：寄存器起始地址	buf:接收缓冲区 len长度
 * 返回: 无
 * 说明: p_chip = DDC2556_CHIP_1--DDC2556_CHIP_7
 *************************************************************/
void  ddc2256::ddc2256_read_reg_multi_byte(uint8_t p_chip, uint16_t reg_s_addr, uint8_t *buf, uint16_t len)
{
    int i;
    //int ilen = ;
    reg_s_addr &= 0x7fff;  //最高位清0 表示读

//    SPI1_read_write_byte(reg_s_addr >> 8);	//写地址/
//    SPI1_read_write_byte(reg_s_addr);

    unsigned char * tx_buffer = (unsigned char *)malloc(len+2);

    unsigned char * rx_buffer = (unsigned char *)malloc(len+2);
    memset(tx_buffer,0,len+2);

    memset(rx_buffer,0,len+2);

    tx_buffer[0] = reg_s_addr >> 8;
    tx_buffer[1] = reg_s_addr &0xff;
    tx_buffer[3] = 0x11;




    #ifdef Q_OS_LINUX
    struct spi_ioc_transfer transfer;

    switch (p_chip) //片选拉低
    {
        case DDC2256_CHIP_1:
        gpio_value(GPIO_CS1, 0);
            break;
        case DDC2256_CHIP_2:
        gpio_value(GPIO_CS2, 0);
            break;
        case DDC2256_CHIP_3:
        gpio_value(GPIO_CS3, 0);
            break;
        case DDC2256_CHIP_4:
        gpio_value(GPIO_CS4, 0);
            break;
    }
    delayMs(1);
//     int ret = write(devfd, tx_buffer , 2); // 发送数据
//     if (ret < 0)
//     {
//         perror("write()"); // 处理可能的错误

//     }
//     ret = read(devfd, buf , len); // 读数据
//     if (ret < 0)
//     {
//         perror("read()"); // 处理可能的错误

//     }

//    memset(&transfer,0,sizeof(struct spi_ioc_transfer));

//    delayMs(1);

//    struct spi_ioc_transfer xfer[2];

//    int status;



//    memset(xfer, 0, sizeof(xfer));

//    xfer[0].tx_buf = (unsigned long)tx_buffer;

//    xfer[0].len = 2;

//    xfer[1].rx_buf = (unsigned long) rx_buffer;

//    xfer[1].len = len;

//    int ret = ioctl(devfd, SPI_IOC_MESSAGE(2), xfer);
//    for(i=0;i<len;i++)
//    {
//            buf[i] = rx_buffer[i];
//            //printf("%02x ",rx_buffer[2+i]);
//    }

    memset(&transfer,0,sizeof(struct spi_ioc_transfer));
    transfer.tx_buf = (unsigned long)tx_buffer;
    transfer.rx_buf  =(unsigned long) rx_buffer;
    transfer.len = len+2;
    transfer.delay_usecs = 1000;//设置通信的时间延迟
    transfer.speed_hz = 4000000;//通信的比特率
    transfer.bits_per_word = 8;//一个字节占用多少比特
    transfer.tx_nbits = 1;
    transfer.rx_nbits = 1;

    int ret = ioctl(devfd, SPI_IOC_MESSAGE(1), &transfer);

    for(i=0;i<len;i++)
    {
        buf[i] = rx_buffer[2+i];
        //printf("%02x ",rx_buffer[2+i]);
    }

    //printf("\n",rx_buffer[2+i]);

    if (ret < 1)
    {
        status = 0;
        //perror("ioctl()");
        //fprintf(stderr,"ioctl returned %d\n",ret);
    }
    else
    {
        status = 1;
    }

    //delayMs(1);
    switch (p_chip) //片选
    {
        case DDC2256_CHIP_1:
        gpio_value(GPIO_CS1, 1);
            break;
        case DDC2256_CHIP_2:
        gpio_value(GPIO_CS2, 1);
            break;
        case DDC2256_CHIP_3:
        gpio_value(GPIO_CS3, 1);
            break;
        case DDC2256_CHIP_4:
        gpio_value(GPIO_CS4, 1);
    }
//    delayMs(1);
    #endif



    free(tx_buffer);
    free(rx_buffer);
//    delayMs(1);


}


/************************************************************
 * 功能: ddc2256写寄存器多个字节
 * 形参: p_chip:选择与那个芯片通信 reg_s_addr：寄存器起始地址	buf:发送缓冲区 len长度
 * 返回: 无
 * 说明: p_chip = DDC2556_CHIP_1--DDC2556_CHIP_7
 *************************************************************/
void ddc2256::ddc2256_write_reg_multi_byte(uint8_t p_chip, uint16_t reg_s_addr, uint8_t *buf, uint16_t len)
{
//    uint16_t i;
//    reg_s_addr |= 0x8000;  //最高位置1 表示写

//    SPI1_read_write_byte(reg_s_addr >> 8);	//写地址
//    SPI1_read_write_byte(reg_s_addr);

//    for (i = 0; i < len; i++)
//    {
//        SPI1_read_write_byte(buf[i]);
//    }



    int i;
    reg_s_addr |= 0x8000;  //最高位置1 表示写
    unsigned char * tx_buffer = (unsigned char *)malloc(len+2);
    tx_buffer[0] = reg_s_addr >> 8;
    tx_buffer[1] = reg_s_addr &0xff;


    for (i = 0; i < len; i++)
    {
        tx_buffer[2+i] = buf[i];
    }

    #ifdef Q_OS_LINUX
    struct spi_ioc_transfer transfer;

    memset(&transfer,0,sizeof(struct spi_ioc_transfer));
    switch (p_chip) //片选
    {
        case DDC2256_CHIP_1:
        gpio_value(GPIO_CS1, 0);
            break;
        case DDC2256_CHIP_2:
        gpio_value(GPIO_CS2, 0);
            break;
        case DDC2256_CHIP_3:
        gpio_value(GPIO_CS3, 0);
            break;
        case DDC2256_CHIP_4:
        gpio_value(GPIO_CS4, 0);
            break;

    }
    transfer.tx_buf = (unsigned long)tx_buffer;
    transfer.len = len+2;

    int ret = ioctl(devfd, SPI_IOC_MESSAGE(1), &transfer);


    if (ret < 1)
    {
        perror("ioctl()");
        fprintf(stderr,"ioctl returned %d\n",ret);
    }


    switch (p_chip) //片选
    {
        case DDC2256_CHIP_1:
        gpio_value(GPIO_CS1, 1);
            break;
        case DDC2256_CHIP_2:
        gpio_value(GPIO_CS2, 1);
            break;
        case DDC2256_CHIP_3:
        gpio_value(GPIO_CS3, 1);
            break;
        case DDC2256_CHIP_4:
        gpio_value(GPIO_CS4, 1);
    }
    #endif


    free(tx_buffer);
}


/************************************************************
 * 功能:  读取2256通道数据
 * 形参:
 * 返回: 0 数据更新读取成功 1：数据还没更新
 * 说明: 无
 *************************************************************/


//volatile INT32S ddc2556_data_upload_timer;
//volatile INT8U ddc2556_data_upload_flag;

//volatile INT8U ddc2556_conv_state;

//INT32U ddc2556_data_wait_timer;















/*************************************************************
 功能：ad采集任务
 形参：
 返回：
 详解：
 编写：
**************************************************************/
// void ddc2256_scan_task(void)
// {
// 	switch (task_table[DDC2256_SCAN_TASK].event)
// 	{
// 		case DDC2256_SCAN_EVENT:
// 			if (read_ddc2256_adc_data() == 0)
// 			{
// 				if (sys_run_mode)
// 				{
// 					network_send_image_api_old_protocol();
// 					network_send_pressure_info_api_old_protocol(100);
// 				}
// 				else
// 				{
// 					network_send_image_api();
// 				}

// 				collect_save_data_handle();	//处理数据累加
// 			}
// 			break;

// 		default:
// 			break;
// 	}
// }





/************************************************************
 * 功能:  读取2256通道数据
 * 形参:  p_chip: 0 --> 选择测试夹具板子上的DDC2256
 *                1 --> 选择外接板子上的DDC2256
 * 返回: 0 数据更新读取成功 1：数据还没更新
 * 说明: 无
 *************************************************************/
uint8_t ddc2256:: read_ddc2256_adc_data(uint8_t p_chip, uint8_t ic)
{
    uint8_t data_discard = 0;
    static uint32_t p_collect_sum[32];
    uint8_t pdata_h[3]={0};
//	uint8_t pdata_l[3];
    uint16_t i, j;
    static uint8_t vaild_cnt = 0; //记录那些采集板已经读到数据
    uint8_t *p_adc_vale;
    static uint8_t first = 0;
    uint8_t data_buf[3 * 262];
//	static uint32_t t_collect_sum;



//    uint32_t xid=0;
//    xid = ddc2256_read_fpga_id(p_chip);
//    printf("chip %d id %d\r\n",p_chip, xid);
//    return 2;
    //开机读总数
    if (first < 100)
    //if (1)
    {
        //读取更新次数
        uint32_t id  = ddc2256_read_fpga_id(p_chip);
        //printf("p_chip %d id is  %d\r\n",p_chip,id);

        uint32_t fpga_state= ddc2256_read_fpga_state(p_chip);
        //printf("chip %d fpga_state %d\r\n",p_chip, fpga_state);


        p_collect_sum[p_chip*2+ic] = ddc2256_read_num(p_chip,ic);

       // printf("ddc2256_read_num chip%02u ic %d:%d\r\n", p_chip,ic, p_collect_sum[p_chip*2+ic]);
        //printf("%d, %d, %d, sum_1: %d, sum_2: %d\r\n",pdata_h[0],pdata_h[1],pdata_h[2],((uint32_t)pdata_h[0] << 16) + ((uint32_t)pdata_h[1] << 8),p_collect_sum[p_chip*2+ic]);
        if ((p_collect_sum[p_chip*2+ic] != 0) && (p_collect_sum[p_chip*2+ic] != ddc2556_data[p_chip*2+ic].collect_sum))
        {
            ddc2556_data[p_chip*2+ic].collect_sum = p_collect_sum[p_chip*2+ic]; //保存采集总数
            first++;
        }
        return 2;
    }

    //读取更新次数
    p_collect_sum[p_chip*2+ic] = ddc2256_read_num(p_chip,ic);

    //return 2;

    //printf("chip%02u ic %d:%d\r\n", p_chip,ic, p_collect_sum[p_chip*2+ic]);

  //  uint32_t fpga_state= ddc2256_read_fpga_state(p_chip);


    //printf("chip %d fpga_state %d\r\n",p_chip, fpga_state);


    if ((p_collect_sum[p_chip*2+ic] != 0) && (p_collect_sum[p_chip*2+ic] != ddc2556_data[p_chip*2+ic].collect_sum))
    {
        // printf("num:%d\r\n", p_collect_sum);
        if (p_collect_sum[p_chip*2+ic] != (ddc2556_data[p_chip*2+ic].collect_sum+1))
        {
           printf("***xwrong,ch%02u no %d: cur %lu pre %lu**\r\n\r\n", p_chip,ic,	p_collect_sum[p_chip*2+ic], ddc2556_data[p_chip*2+ic].collect_sum );
        }
        else
        {
           //

        }

        //读取帧头
        ddc2256_read_head(p_chip, ic, data_buf);   //

        if ((data_buf[0] == 0x12) && (data_buf[1] == 0x34) && (data_buf[2] == 0x56) && (data_buf[3] == 0x78) && (data_buf[4] == 0x9a) && (data_buf[5] == 0xbc))
        {
            ddc2556_data[p_chip*2+ic].collect_sum = p_collect_sum[p_chip*2+ic]; //保存采集总数


            ddc2256_read_data(p_chip, ic, data_buf);   //262 = （2个24位的报头+128个24位的存储数据+1个24位的报尾）* 2   【乘以2：指前半段128个数据+后半段128个数据】

            p_adc_vale = &(data_buf[6]);
//			t_collect_sum = ddc2556_data.collect_sum;

            for (i = 0; i < 128; i++)
            {
            //	ddc2556_data[j].adc_vale[i] = ((INT16U)p_adc_vale[1] << 8) + (INT16U)p_adc_vale[2];
                ddc2556_data[p_chip*2+ic].adc_vale[i] = ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                p_adc_vale += 3;
            }

            p_adc_vale += 9;//跳过中间的尾和头

            for (i = 0; i < 128; i++)
            {
                //ddc2556_data[j].adc_vale[i + 128] =  ((INT16U)p_adc_vale[1] << 8) + (INT16U)p_adc_vale[2];//pdata_l[1] * 256 + pdata_l[2];//((INT16U)pdata_l[1]) << 8 + pdata_l[2];
                ddc2556_data[p_chip*2+ic].adc_vale[i + 128] =  ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                p_adc_vale += 3;
            }


/*
            for (i = 0; i < 126; i++)
            {
                if((i == 48) || ((i+data_discard) == 112))
                {
                    data_discard++;
                    p_adc_vale += 3;
                }
                //lv_acq_board.Acq_Board_IN_VAL[i] = ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                ddc2556_data[p_chip*2+ic].adc_vale[i] = ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                p_adc_vale += 3;
            }

            data_discard = 0;
            p_adc_vale += 9; //跳过中间的尾和头

            for (i = 0; i < 126; i++)
            {
                if(((i+128) == 175) || ((i+128+data_discard) == 239))
                {
                    data_discard++;
                    p_adc_vale += 3;
                }

                //lv_acq_board.Acq_Board_IN_VAL[i + 126] = ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                ddc2556_data[p_chip*2+ic].adc_vale[i + 126] = ((uint16_t)p_adc_vale[0] << 8) + (uint16_t)p_adc_vale[1];
                p_adc_vale += 3;
            }
            */
//			for(i = 0;i < 252;i++)
//			{
//				printf("%d: %d\r\n",i+1,lv_acq_board.Acq_Board_IN_VAL[i]);
//			}
//            printf("yese p_chip %d ic %d\r\n\r\n",p_chip,ic);
            //p_chip=0 ic=0,对应GpsystemStatus.chip0=1
            //p_chip=1 ic=0,对应GpsystemStatus.chip1=1
            //p_chip=0 ic=1,对应GpsystemStatus.chip2=1
            //p_chip=1 ic=1,对应GpsystemStatus.chip3=1
            if (p_chip==0&&ic==0)
            {
                GpsystemStatus.chip0+=1;
            }
            else if(p_chip==0&&ic==1)
            {
                GpsystemStatus.chip1+=1;
            }
            else if(p_chip==1&&ic==0)
            {
                GpsystemStatus.chip2+=1;
            }
            else if(p_chip==1&&ic==1)
            {
                GpsystemStatus.chip3+=1;
            }
            else if (p_chip==2&&ic==0)
            {
                GpsystemStatus.chip4+=1;
            }
            else if(p_chip==2&&ic==1)
            {
                GpsystemStatus.chip5+=1;
            }
            else if(p_chip==3&&ic==0)
            {
                GpsystemStatus.chip6+=1;
            }
            else if(p_chip==3&&ic==1)
            {
                GpsystemStatus.chip7+=1;
            }
            return 0;
        }
        else
        {

            printf("chip %d ic  %02u headh err =  0x%02x%02x%02x\r\n", p_chip,ic, data_buf[0], data_buf[1], data_buf[2]);
            //printf("channeld %02u headl =  0x%02x%02x%02x\r\n", p_chip,ic, data_buf[3], data_buf[4], data_buf[5]);
           // printf("ddc head err \r\n");
            return 2;
        }
    }
    else
    {
        printf("***ywrong,ch%02u no %d: cur %lu pre %lu**\r\n\r\n", p_chip,ic,	p_collect_sum[p_chip*2+ic], ddc2556_data[p_chip*2+ic].collect_sum );

    }

            return 3;
}

/************************************************************
 * 功能: 设置积分时间
 * 形参: p_time ：积分时间 （单位：ms）
 * 返回: 无
 * 说明: 无
 *************************************************************/
void ddc2256::ddc2256_set_integral_time(uint16_t p_time)
{
    uint8_t j;
    uint8_t pdata[3];

    pdata[0] = 0;
    pdata[1] = p_time >> 8; //按大端高位在前发
    pdata[2] = p_time;

    for (j = 0; j <= 1; j++)
    {
        //读取更新次数
        ddc2256_write_reg_multi_byte(j, 0x0002, pdata, 3);
    }
}

/************************************************************
 * 功能: 设置积分电容
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明: DDC积分电容设置  0-100pC   1--50pC   2--25pC   3-12.5pC
                        4--6.25pC    5--150pC    6--75pC     7--37.5pC
 *************************************************************/
void ddc2256::ddc2256_set_integral_cap(uint8_t p_cap)
{
    uint8_t j;
    uint8_t pdata[3];

    pdata[0] = 0;
    pdata[1] = 0; //按大端高位在前发
    pdata[2] = p_cap;

    for (j = DDC2256_CHIP_1; j <= DDC2256_CHIP_1; j++)
    {
        //读取更新次数
        ddc2256_write_reg_multi_byte(j, 0x0001, pdata, 3);
    }
}

/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明:
 *************************************************************/
void ddc2256::ddc2256_set_add_up_time(uint8_t cnt)
{
    uint8_t j;

    uint8_t pdata[3];

    pdata[0] = 0;
    pdata[1] = 0; //按大端高位在前发
    pdata[2] = cnt;

    for (j = DDC2256_CHIP_1; j <= DDC2256_CHIP_1; j++)
    {
        //读取更新次数
        ddc2256_write_reg_multi_byte(j, 0x0003, pdata, 3);
    }
}

/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明:
 *************************************************************/
uint32_t ddc2256::ddc2256_read_fpga_id(uint8_t ch)
{
    uint8_t pdata[3] = {0,0,0};
    ddc2256_read_reg_multi_byte(ch, 0x0003, pdata, 3);
    return(uint32_t)(pdata[0]<<16|pdata[1]<<8|pdata[2]);
}


/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明:
 *************************************************************/
uint32_t ddc2256::ddc2256_read_fpga_state(uint8_t ch)
{
    uint8_t pdata[3] = {0,0,0};
    ddc2256_read_reg_multi_byte(ch, 0x0002, pdata, 3);
    return(uint32_t)(pdata[0]<<16|pdata[1]<<8|pdata[2]);
}

/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明:
 *************************************************************/
uint32_t ddc2256::ddc2256_read_num(uint8_t ch,uint8_t ic)
{
    uint8_t pdata[3] = {0,0,0};
    uint32_t p_collect_sum;
    //读取更新次数
    if(ic==0)ddc2256_read_reg_multi_byte(ch, 0x0007, pdata, 3);
    else if(ic==1)ddc2256_read_reg_multi_byte(ch, 0x0008, pdata, 3);
    //大端转小端,检测每个采集芯片的数据是否有更新
    p_collect_sum = ((uint32_t)pdata[0] << 16) + ((uint32_t)pdata[1] << 8) + (uint32_t)pdata[2];
    return p_collect_sum;
}
/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明: 0 A  1 B
 *************************************************************/
uint32_t ddc2256::ddc2256_read_head(uint8_t ch,uint8_t ic,uint8_t *head)
{
    //读取更新次数
    if(ic==0)ddc2256_read_reg_multi_byte(ch, 0x1000, head, 6);
    else if(ic==1)ddc2256_read_reg_multi_byte(ch, 0x2000, head, 6);
}

/************************************************************
 * 功能: 设置累加次数
 * 形参: p_cap ：积分电容
 * 返回: 无
 * 说明:
 *************************************************************/
uint32_t ddc2256::ddc2256_read_data(uint8_t ch,uint8_t ic,uint8_t *data)
{
    //读取更新次数    ddc2256_read_reg_multi_byte
    if(ic == 0)		ddc2256_read_reg_multi_byte(ch, 0x1000, data, 3 * 262);
    else if(ic==1)	ddc2256_read_reg_multi_byte(ch, 0x2000, data, 3 * 262);
}













