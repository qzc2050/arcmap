#include "gpio_contorl.h"
#include "mainwindow.h"
#include <QDebug>
#include <cmath> // 包含数学库，用于正弦波计算
#define	DISP_LCD_SET_BRIGHTNESS  0x102
#define DISP_LCD_GET_BRIGHTNESS  0x103
#define	DISP_LCD_BACKLIGHT_ENABLE   0x104
#define	DISP_LCD_BACKLIGHT_DISABLE  0x105

#define BRIGHTNESS_MAX 255     //最大亮度

#ifdef Q_OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/spi/spidev.h>


#define GPIO_DIR_IN	0
#define GPIO_DIR_OUT	1

#define GPIO_DDI  34



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

static int gpio_edge(unsigned gpio, unsigned edge)
{
    int fd, len;
    char buf[60];

    len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/edge");
        return fd;
    }

    if (edge == 1)
       write(fd, "rising", 6);
    else
        write(fd, "falling", 7);

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

static int readGPIOValue(int gpioNumber) {
    char gpioPath[64];
    sprintf(gpioPath, "/sys/class/gpio/gpio%d/value", gpioNumber);

    int fd = open(gpioPath, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open GPIO value file");
        exit(EXIT_FAILURE);
    }

    char valueStr[2];
    if (read(fd, valueStr, sizeof(valueStr)) == -1) {
        perror("Failed to read GPIO value");
        exit(EXIT_FAILURE);
    }

    int value = atoi(valueStr);

    close(fd);

    return value;
}
#endif
//PB7 PB6
int myio_setlight(int ilight)
{
   int brightness = ilight;
#ifdef Q_OS_LINUX
    int fd;
    unsigned long args[3]={0};
    /*打开disp设备文件*/
    fd = open("/dev/disp", O_RDWR, 0);
    if(fd < 0)
    {
        printf("open /dev/disp failed.\n");
        return -1;
    }
//    /*打印旧的背光值*/
//    args[0] = 0;     //选择fb0(lcd0)
//    //printf("the old lcd%d brightness is %d\n",\
//    args[0],ioctl(fd,DISP_LCD_GET_BRIGHTNESS,args));





//    if(brightness < 0 || brightness > BRIGHTNESS_MAX)
//    {
//        printf("ERROR: the range is 0 to %d\n",BRIGHTNESS_MAX);
//        return -1;
//    }

    /*参数数组填参*/
    args[0] = 0;
    args[1] = brightness; 		//背光值(本人配置里最高为255)
    args[2] = 0;     	      //暂时用不到的参数
    ioctl(fd,DISP_LCD_SET_BRIGHTNESS,args);
    //printf("the new lcd%d brightness is %d\n",\
    args[0],ioctl(fd,DISP_LCD_GET_BRIGHTNESS,args));
    close(fd);
#endif
    return brightness;
}
unsigned Ggpioser[16] = {35,39};
int set_gpio_value(int index, unsigned value)
{

    int res;
    if(index>1)
    {
        return 0;
    }
    unsigned gpio = Ggpioser[index];
    if(gpio==39)
    {
        int fd;


        #ifdef Q_OS_LINUX

        fd = open("/sys/class/gpio_sw/PB7/data", O_WRONLY);
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
    }
    else
    {
        #ifdef Q_OS_LINUX
        res=gpio_value(gpio,value);
        return res;
        #endif
    }






    return 1;
}

int GPIO_CONTORL::HVCHAGE(unsigned value)
{
    int fd, len;
    char buf[60];

    #ifdef Q_OS_LINUX

    fd = open("/sys/class/gpio_sw/PB7/data", O_WRONLY);
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



GPIO_CONTORL::GPIO_CONTORL()
{

#ifdef Q_OS_LINUX






    //    HV_EN         PPB7







      //    HV_TEST		PB6





    gpio_export(38);
    gpio_dir_in(38);

    //gpio_value(38,0);


    /*打开disp设备文件*/
    fdback = open("/dev/disp", O_RDWR, 0);



#endif
    testmode = new QTimer();
    connect(testmode,SIGNAL(timeout()),this,SLOT(dotimeout()));
    testmode->start(500);
    icout = 0;
    icouthv = 0;
    icountpwm = 1;


    // 生成伽马校正表，并立即反转极性
    // 生成伽马校正表，使用对数函数
//    for (int i = 0; i < 256; ++i) {
//        // 通过对数函数调整，然后翻转
//        double logValue = std::log1p(i); // 使用log1p(i) = log(1 + i)，避免i = 0时的无定义情况
//        gammaTable[i] = int(255.0 * logValue / std::log1p(255));
//        if(gammaTable[i]<100)
//        {
//            gammaTable[i] = 100;
//        }
//        printf("%d, ",gammaTable[i]);
//    }
    //printf("n/");
    ledmodetime = new QTimer();

    connect(ledmodetime,SIGNAL(timeout()),this,SLOT(doledtimeout()));
   // ledmodetime->start(10);


    HVCHAGE(1);
    myio_setlight(0);
}

void GPIO_CONTORL::doledtimeout()
{
    //qDebug()<<GpsystemStatus.wifistatus;
    //if(GpsystemStatus.wifistatus)



}

void GPIO_CONTORL::dotimeout()
{
    icout++;
    if(icout>0)
    {
        if(icout%4==0)
        {
            icouthv++;
            checkhv();
        }
    }




    if(GDebugMoni)
    HVCHAGE((icouthv & 1));


//        if(GpsystemStatus.bendi==1)
//        {
//            if(icout & 1)
//            {
//                #ifdef Q_OS_LINUX




//                    /*参数数组填参*/
//                    args[0] = 0;
//                    args[1] = 0; 		//背光值(本人配置里最高为255)
//                    args[2] = 0;     	      //暂时用不到的参数
//                    ioctl(fdback,DISP_LCD_SET_BRIGHTNESS,args);

//                #endif
//            }
//            else
//            {
//                    #ifdef Q_OS_LINUX




//                        /*参数数组填参*/
//                        args[0] = 0;
//                        args[1] = 255; 		//背光值(本人配置里最高为255)
//                        args[2] = 0;     	      //暂时用不到的参数
//                        ioctl(fdback,DISP_LCD_SET_BRIGHTNESS,args);

//                    #endif

//            }
//        }
//        else
//        {
//            if(GpsystemStatus.eth0==0)
//            {

//                #ifdef Q_OS_LINUX




//                    /*参数数组填参*/
//                    args[0] = 0;
//                    args[1] = 120; 		//背光值(本人配置里最高为255)
//                    args[2] = 0;     	      //暂时用不到的参数
//                    ioctl(fdback,DISP_LCD_SET_BRIGHTNESS,args);

//                #endif

//            }

//        }



}


int  GPIO_CONTORL::checkpower()
{
    int res = -1;
     #ifdef Q_OS_LINUX
    res =  readGPIOValue(45);
     qDebug()<<"checkpower()"<<res;
    #endif
    return res;
}

int  GPIO_CONTORL::checkbat()
{
    int res = 0;
     #ifdef Q_OS_LINUX
    res =  readGPIOValue(243);
    #endif
    qDebug()<<"checkbat()"<<res;
    return res;
}
int  GPIO_CONTORL::checkhv()
{
    int res = 0;
     #ifdef Q_OS_LINUX
    res =  readGPIOValue(38);
    #endif
    // qDebug()<<"checkhv()"<<res;
    return res;
}
void GPIO_CONTORL::run()
{


    QThread::msleep(5000);

    gammaTable[0] = 255;
    gammaTable[1] = 255;

    gammaTable[2] = 254;
    gammaTable[3] = 254;

    gammaTable[4] = 253;
    gammaTable[5] = 253;

    gammaTable[6] = 252;
    gammaTable[7] = 252;

    gammaTable[8] = 251;
    gammaTable[9] = 251;

    gammaTable[10] = 250;
    gammaTable[11] = 250;

    gammaTable[12] = 249;
    gammaTable[13] = 248;

    gammaTable[14] = 247;
    gammaTable[15] = 246;


    gammaTable[16] = 245;
    gammaTable[17] = 244;


    gammaTable[18] = 243;
    gammaTable[19] = 242;



    gammaTable[20] = 240;


    gammaTable[21] = 238;

    gammaTable[22] = 236;
    gammaTable[23] = 234;

    gammaTable[24] = 230;
    gammaTable[25] = 228;

    gammaTable[26] = 226;
    gammaTable[27] = 224;

    gammaTable[28] = 220;
    gammaTable[29] = 218;

    gammaTable[30] = 214;
    gammaTable[31] = 210;

    gammaTable[32] = 205;
    gammaTable[33] = 200;

    gammaTable[34] = 190;
    gammaTable[35] = 180;


    gammaTable[36] = 170;
    gammaTable[37] = 160;


    gammaTable[38] = 150;
    gammaTable[39] = 140;

    gammaTable[40] = 130;



    while(1)
    {

//        if((GpsystemStatus.eth0==1)&&(GpsystemStatus.bendi==0))
//        {
//            if (increasing) {
//                if (icountpwm < 40) {
//                    icountpwm++;
//                } else {
//                    increasing = false;
//                }
//            } else {
//                if (icountpwm > 0) {
//                    icountpwm--;
//                } else {
//                    increasing = true;
//                }
//            }


//            //myio_setlight(gammaTable[icountpwm]);
//            #ifdef Q_OS_LINUX




//                /*参数数组填参*/
//                args[0] = 0;
//                args[1] = gammaTable[icountpwm]; 		//背光值(本人配置里最高为255)
//                args[2] = 0;     	      //暂时用不到的参数
//                ioctl(fdback,DISP_LCD_SET_BRIGHTNESS,args);

//            #endif

//        }

        QThread::msleep(100);
    }


}
