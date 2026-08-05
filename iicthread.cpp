#include "iicthread.h"
#include <QDebug>
#include <QDateTime>
#include <QTime>
#include "mainwindow.h"
iicThread::iicThread(QObject *parent) : QThread(parent)
{

    whybmp280 = NULL;

    myads115 = NULL;

    myscl300 = NULL;


   //
    myscl300 = new scl300();

    myiic = new myIIc();

    iicfd = myiic->getid();


    inittimer = false;
    qDebug()<<"iicfd"<<iicfd;
    if(iicfd<0)
    {

        qDebug()<<"dS2783_iic->devfd not set";
    }
    else
    {



        whybmp280 = new MYBMP280();
        whybmp280->setid(iicfd);





        myads115 = new ADS118();
        myads115->setid(iicfd);

        /*
         * 系统时间 现在自动对时服务ntp，如果没有网络
         * */

    }





}

void iicThread::run()
{

     QThread::msleep(100);
     if(whybmp280)
     whybmp280->bmp280_init();


    QThread::msleep(100);

    if(whybmp280)
    whybmp280->bmp280_init();
    QThread::msleep(100);
    if(whybmp280)
    whybmp280->bmp280_get_pressure();//先读一次

    int icout=0;
    while (1)
    {
        icout++;
        if(icout%4==0)
        {

        }
        else if(icout%4==1)
        {
            if(icout>8)
            {
                if(whybmp280)
                {
                    float fpress = whybmp280->bmp280_get_pressure()/1000;
                    GpsystemStatus.press = fpress;

                    // qDebug()<<whybmp280->bmp280_get_temperature()<<fpress;
                    GpsystemStatus.pressstatus = whybmp280->status;
                    GpsystemStatus.tempbord = whybmp280->bmp280_get_temperature();
                }
            }

        }
        else if(icout%4==2)
        {
            if(myads115)
            {
                myads115->doadstimer();
            }
        }



        QThread::msleep(150);

        if(myscl300)
        {

           float xx=0,yy=0,zz=0;

           xx =  myscl300->SCL3300_ReadAngle(0)*(-1.0);

           yy =  myscl300->SCL3300_ReadAngle(1);

           zz= myscl300->SCL3300_ReadAngle(2)*(-1.0);








           if(yy>0)
           {
                if(xx>0)
                {
                    xx = (90-xx)+90;
                }
                else
                {
                    xx = (xx+90)*(-1.0)-90;
                }
           }



           GpsystemStatus.x =xx;
           GpsystemStatus.y =zz;
           GpsystemStatus.z =yy;

           // qDebug()<<"angle"<<GpsystemStatus.x<<GpsystemStatus.y<<GpsystemStatus.z;

        }


        QThread::msleep(150);
    }
}
