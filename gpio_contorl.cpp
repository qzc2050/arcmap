#include "gpio_contorl.h"
#include "mainwindow.h"
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





    gpio_export(35);
    gpio_dir_out(35);

    gpio_value(38,1);



#endif
    testmode = new QTimer();
    connect(testmode,SIGNAL(timeout()),this,SLOT(dotimeout()));
    testmode->start(1000);
    icout = 0;


}

void GPIO_CONTORL::dotimeout()
{
    icout++;


    if(GDebugMoni)
    HVCHAGE((icout & 1));

    #ifdef Q_OS_LINUX
    gpio_value(35,(icout & 1));
    #endif

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

void GPIO_CONTORL::run()
{
    #ifdef Q_OS_LINUX
    gpio_edge(230,1);
    gpio_edge(231,1);
    #endif


    while(1)
    {


        #ifdef Q_OS_LINUX
        int gpio_fd1, gpio_fd2;
        struct pollfd fd_set[2];
        char* gpio_path1 = "/sys/class/gpio/gpio231/value"; // ph7 开始
        char* gpio_path2 = "/sys/class/gpio/gpio230/value"; // ph6 结束

        // 打开GPIO文件描述符
        gpio_fd1 = open(gpio_path1, O_RDONLY);
        gpio_fd2 = open(gpio_path2, O_RDONLY);

        if (gpio_fd1 == -1 || gpio_fd2 == -1) {
            perror("Error opening GPIO");

        }

        // 配置pollfd结构体数组
        fd_set[0].fd = gpio_fd1;
        fd_set[0].events = POLLPRI; // 监听可读事件（即IO状态变化）
        fd_set[1].fd = gpio_fd2;
        fd_set[1].events = POLLPRI; // 监听可读事件（即IO状态变化）

        while(1) {
            int rc = poll(fd_set, 2, -1); // 阻塞等待IO状态变化
            if (rc == -1) {
                perror("Error in poll");
                QThread::msleep(100);

            }

            // 检查IO状态变化的事件
            for (int i = 0; i < 2; i++) {
                if (fd_set[i].revents & POLLPRI) {
                    lseek(fd_set[i].fd, 0, SEEK_SET); // 定位到文件开头
                    char buf[2];
                    read(fd_set[i].fd, buf, sizeof(buf));


                    if(i==0)
                    {
                        printf("GPIO%d startkeypressed()!\n", i + 1);
                        emit startkeypressed();
                    }

                    if(i==1)
                    {
                        emit endkeypressed();
                        printf("GPIO% endkeypressed()!\n", i + 1);
                    }
                }
            }
            QThread::msleep(100);
        }

        // 关闭文件描述符
        close(gpio_fd1);
        close(gpio_fd2);

        //229

//        int fd;
//        int izhankongbi =0;
//        int ifan=0;
//        char* gpio_path1 = "/sys/class/gpio/gpio229/value"; // ph7 开始

//        fd = open(gpio_path1, O_WRONLY);
//        while(1)
//        {
//            if(ifan%2==0)
//            {
//                 write(fd, "0", 2);
//            }
//            else
//            {
//                write(fd, "1", 2);
//            }
//            ifan++;
//            QThread::usleep(2);
//        }











        #endif

        QThread::msleep(100);
    }

}
