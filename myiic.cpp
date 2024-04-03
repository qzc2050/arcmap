#include "myiic.h"
#include<QDebug>
#define MAX_BYTES 16
#define DEVICE_NAME "/dev/i2c-2"







myIIc::myIIc()
{
    fd = -1;
    fdxx = 999;
    qDebug()<<"heris ywl";

    #ifdef Q_OS_LINUX
    fd =open("/dev/i2c-2", O_RDWR);
    if(fd<0)
    {
        qDebug()<<"heris ywl3333333333333";
        printf("open failed\n");
    }
    else {
        qDebug()<<"heris ywl44444444444443"<<fd;
        printf("open sucess\n");
    }
    #endif
qDebug()<<"heris ywl2222222222222222";

}

int myIIc::getid()
{
    qDebug()<<fd;
    qDebug()<<fdxx;
    return fd;
}


