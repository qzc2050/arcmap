#ifndef MYIIC_H
#define MYIIC_H

#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>




#ifdef Q_OS_LINUX



#include<stdio.h>
#include<linux/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<string.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#endif






class myIIc : public QObject
{
    Q_OBJECT
public:

    myIIc();
    //myIIc* GetInstance();



   // int i2c_write(int fd,unsigned int addr,unsigned int offset,unsigned char *buf,unsigned int len);

    //int i2c_read(int fd, unsigned int addr,unsigned int offset, unsigned char *buf, unsigned int len);


    int getid();


private:
    //static myIIc *m_myIIc;

    int fd;
    int fdxx;


signals:

};

#endif // MYIIC_H
