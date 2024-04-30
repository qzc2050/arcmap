#include "scl300.h"
#include <QTime>
#include <QCoreApplication>
#include <QDebug>
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
#endif
void scl300::delayMs(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


int scl300::CS(unsigned value)
{
    int fd, len;
    char buf[60];

    #ifdef Q_OS_LINUX

    fd = open("/sys/class/gpio_sw/PB12/data", O_WRONLY);
    if (fd < 0) {
        perror("gpio/value");
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    #endif
    return 0;
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */
/**
 @brief 写SCL3300寄存器
 @param cmd -[in] 写入命令
 @return 无
*/
void scl300:: writeRegister(uint32_t cmd)
{
     CS(0);
     delayMs(5);
    uint8_t writeData[4] = {0};
    writeData[0] = (cmd >> 24) & 0xFF;
    writeData[1] = (cmd >> 16) & 0xFF;
    writeData[2] = (cmd >> 8) & 0xFF;
    writeData[3] = (cmd >> 0) & 0xFF;


//    #ifdef Q_OS_LINUX
//    // 假设chunk是包含数据的QByteArray
//    int result = write(devfd, writeData, 4);
//    if (result == -1) {
//        // 错误处理
//         qDebug()<<"writeRegister err";
//    }
//    #endif

    scl300_write_reg_multi_byte(writeData, 4);



     delayMs(5);
     CS(1);
}
/**
 @brief 读SCL3300寄存器
 @param 无
 @return 无
*/
void scl300::readRegister(uint8_t  *s_readData,int len)
{


    CS(0);
    delayMs(5);
//    #ifdef Q_OS_LINUX
//    // 假设chunk是包含数据的QByteArray
//    int result = read(devfd, s_readData, len);
//    if (result == -1) {
//        // 错误处理
//        qDebug()<<"readRegister err";
//    }
//    #endif
    scl300_read_reg_multi_byte(s_readData,  len);



    delayMs(5);
    CS(1);
}

scl300::scl300(QObject *parent) : QObject(parent)
{
    #ifdef Q_OS_LINUX
    devfd = open("/dev/spidev2.0", O_RDWR);
    if (devfd < 0) {
        perror("Failed to open SPI device");

    }


    // 配置SPI传输参数
    //    int mode = SPI_MODE_3;  // CPOL=1, CPHA=1
    //    ioctl(fd, SPI_IOC_WR_MODE, &mode);  // 设置SPI模式

    int mode = SPI_MODE_0;    // SPI工作模式
     //mode &= ~SPI_CS_HIGH;  //0x04


    int bits = 8;             // 每个字节的位数
    int speed = 1000000;      // 传输速度（1MHz）

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

    CS(1);
    //SPI1_read_write_byte(0Xff);


    #endif
    SCL3300_Init();

}

void scl300::SCL3300_Init()
{




    int i;
    uint8_t s_readData[4] = {0};

    delayMs(10);

    //writeRegister(CHANGE_TO_MODE4);                         // 选择倾角模式
    //readRegister(s_readData,4);

    delayMs(5);

    writeRegister(READ_STATUS_SUMMARY);                     // 清除状态
    readRegister(s_readData,4);
    for(i=0;i<4;i++)
    {
        printf("SCL3300_Init %02x \n",s_readData[i]);
    }
delayMs(5);
    writeRegister(READ_STATUS_SUMMARY);
    readRegister(s_readData,4);

    for(i=0;i<4;i++)
    {
        printf("SCL3300_Init %02x \n",s_readData[i]);
    }
delayMs(5);
    writeRegister(READ_STATUS_SUMMARY);
    readRegister(s_readData,4);

    for(i=0;i<4;i++)
    {
        printf("SCL3300_Init %02x \n",s_readData[i]);
    }
delayMs(5);
    writeRegister(ENABLE_ANGLE_OUTPUTS);                    // 使能角度输出
    readRegister(s_readData,4);

    for(i=0;i<4;i++)
    {
        printf("SCL3300_Init %02x \n",s_readData[i]);
    }

}

/**
 @brief 检查SPI通信，返回值第3字节为0xC1则通信正常
 @param 无
 @return 无
*/
void scl300::SCL3300_Test(void)
{
    int i;
    uint8_t s_readData[4] = {0};
    writeRegister(READ_ID);
    readRegister(s_readData,4);
    for(i=0;i<4;i++)
    {
        printf("%02x \n",s_readData[i]);
    }
    // 打印s_readData[2]
}
/**
 @brief 读取角度
 @param direction -[in] 要读取的方向X，Y，Z轴
 @return 角度数
*/
float scl300::SCL3300_ReadAngle(uint8_t direction)
{
    uint8_t s_readData[4] = {0};

    if(direction == ANG_X)
    {
        //这个是x
        writeRegister(READ_ANG_X);
    }
    else if(direction == ANG_Y)
    {
        writeRegister(READ_ANG_Y);
    }
    else if(direction == ANG_Z)
    {
        //这个是y
        writeRegister(READ_ANG_Z);
    }
    readRegister(s_readData,4);
                                                            // SPI返回结构第1字节OP+RS，
    uint16_t angle = (s_readData[1] << 8) + s_readData[2];  // 第2和第3字节为Data
                                                            // 第4字节为CRC
    float fangle = (float)angle / 16384.0 * 90.0;           // 如3976/2^14*90=21.84°

    qDebug()<<direction<<fangle;
    if(fangle>180)
    {
        fangle-=360;
    }
    return fangle;
}


void  scl300::scl300_read_reg_multi_byte(uint8_t *buf, uint16_t len)
{
    int i;

    unsigned char * tx_buffer = (unsigned char *)malloc(len+2);

    unsigned char * rx_buffer = (unsigned char *)malloc(len+2);
    memset(tx_buffer,0,len+2);

    memset(rx_buffer,0,len+2);




    CS(0);

    #ifdef Q_OS_LINUX
    struct spi_ioc_transfer transfer;


    memset(&transfer,0,sizeof(struct spi_ioc_transfer));

    transfer.tx_buf = (unsigned long)tx_buffer;
    transfer.rx_buf  =(unsigned long) rx_buffer;
    transfer.len = len;

    int ret = ioctl(devfd, SPI_IOC_MESSAGE(1), &transfer);

    for(i=0;i<len;i++)
    {
        buf[i] = rx_buffer[i];
        //printf("%02x ",buf[i]);
    }

    printf("\n");

    if (ret < 1)
    {
        status = 0;
        perror("ioctl()");
        fprintf(stderr,"ioctl scl300_read_reg_multi_byte returned %d\n",ret);
    }
    else
    {
        status = 1;
    }


    #endif
    CS(1);


    free(tx_buffer);
    free(rx_buffer);


}
void scl300::scl300_write_reg_multi_byte( uint8_t *buf, uint16_t len)
{

    int i;

    unsigned char * tx_buffer = (unsigned char *)malloc(len+2);



    for (i = 0; i < len; i++)
    {
        tx_buffer[i] = buf[i];
    }

    CS(0);
    #ifdef Q_OS_LINUX
    struct spi_ioc_transfer transfer;

    memset(&transfer,0,sizeof(struct spi_ioc_transfer));

    transfer.tx_buf = (unsigned long)tx_buffer;
    transfer.len = len;

    int ret = ioctl(devfd, SPI_IOC_MESSAGE(1), &transfer);


    if (ret < 1)
    {
        perror("ioctl()");
        fprintf(stderr,"ioctl scl300_write_reg_multi_byte returned %d\n",ret);
    }



    #endif

    CS(1);
    free(tx_buffer);
}
