#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QScreen>
#include <QElapsedTimer>
#include <QNetworkInterface>
PsystemStatus GpsystemStatus;
CONFIG_TYPE_NET  GconfigNet;
CONFIG_TYPE_ABOUT GproductInfo;
#include <QFile>
int GDebugMoni=0;

CONFIG_TYPE_SETING GSeting;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    mytft = new FbTFT(this);
    mytft->LCD_Init();
    connect(mytft, &FbTFT::finished, this, &MainWindow::onThreadFinished);



    mygpio = new GPIO_CONTORL();

    binit = false;

    tcpnetser = new TcpNetServer();
    tcpnetser->start(5001);

    tcpnetser2 = new TcpNetServer();
    tcpnetser2->start(5000);

    myiicthread = new iicThread();

    myiicthread->start(QThread::IdlePriority);



    myddc2256 = NULL;




    myddc2256 = new ddc2256();
   //




    myscantimer = new QTimer(this);





    initproinfo();
    connect(myscantimer,SIGNAL(timeout()),this,SLOT(doscantimer()));

    connect(myddc2256,SIGNAL(onepictureok()),tcpnetser2,SLOT(sendDataToClientsLocally()));
    myscantimer->start(1000);
    modbusdevice = new RDSModbusSlave();
    myddc2256->start();

}
void MainWindow::onThreadFinished()
{

    mytft->setImage();
    mytft->start(QThread::IdlePriority);

}
void MainWindow::initproinfo()
{


    QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);

    QString productsernum;
    settings->beginGroup("GproductInfo");
    if(settings->contains("productsernum"))
    {
        productsernum = settings->value("productsernum").toString();


        //strcpy(GproductInfo.productsernum, settings->value("productsernum").toString().toStdString().c_str());

    }
    else
    {
        settings->setValue("productsernum","1234567890");
        productsernum = "1234567890";
        //strcpy(GproductInfo.productsernum, "123456");
    }
    settings->endGroup();

   //初始化GconfigNet根据ini文件 eth0 ipeth0
    settings->beginGroup("eth0");
    if(settings->contains("ipeth0"))
    {
        //GconfigNet.ip是char
        strcpy(GconfigNet.ip, settings->value("ipeth0").toString().toStdString().c_str());
        qDebug()<<"ipeth0"<<QString(GconfigNet.ip);
    }
    else
    {
        //默认192.168.2.212
        strcpy(GconfigNet.ip, "192.168.2.212");
        qDebug()<<"ipeth0"<<QString(GconfigNet.ip);
        settings->setValue("ipeth0","192.168.2.212");
    }
    settings->endGroup();

    // wifi名字和密码
    settings->beginGroup("wlan0");
    if(settings->contains("ssid"))
    {
        strcpy(GconfigNet.ssid, settings->value("ssid").toString().toStdString().c_str());  //ssid
        qDebug()<<"ssid"<<QString(GconfigNet.ssid);
    }
    else
    {
        strcpy(GconfigNet.ssid, "RayDose");
        qDebug()<<"ssid"<<QString(GconfigNet.ssid);
        settings->setValue("ssid","RayDose");
    }   
    if(settings->contains("password"))
    {
        strcpy(GconfigNet.password, settings->value("password").toString().toStdString().c_str());  //password  
        qDebug()<<"password"<<QString(GconfigNet.password);
    }
    else
    {
        strcpy(GconfigNet.password, "raydose888");
        qDebug()<<"password"<<QString(GconfigNet.password);
        settings->setValue("password","raydose888");
    }
    settings->endGroup();
    //更新cx cy cz group是cangle，key 分别是cx cy cz
    settings->beginGroup("cangle");
    if(settings->contains("cx"))
    {
        GpsystemStatus.cx = settings->value("cx").toFloat();  //cx
        qDebug()<<"cx"<<GpsystemStatus.cx;
    }
    else
    {
        GpsystemStatus.cx = 0;
        qDebug()<<"cx"<<GpsystemStatus.cx;
        settings->setValue("cx",0);
    }
    if(settings->contains("cy"))
    {
        GpsystemStatus.cy = settings->value("cy").toFloat();  //cy
        qDebug()<<"cy"<<GpsystemStatus.cy;
    }
    else
    {
        GpsystemStatus.cy = 0;
        qDebug()<<"cy"<<GpsystemStatus.cy;
        settings->setValue("cy",0);
    }
    if(settings->contains("cz"))
    {
        GpsystemStatus.cz = settings->value("cz").toFloat();  //cz
        qDebug()<<"cz"<<GpsystemStatus.cz;
    }
    else
    {
        GpsystemStatus.cz = 0;
        qDebug()<<"cz"<<GpsystemStatus.cz;
        settings->setValue("cz",0);
    }
    settings->endGroup();




    settings->sync();
    delete  settings;

    memset(&GproductInfo, 0, sizeof(CONFIG_TYPE_ABOUT));
    strcpy(GproductInfo.productname, "SRTMap");
    strcpy(GproductInfo.producttype, "SRTMap");
    strcpy(GproductInfo.hardver, "1.0");
    strcpy(GproductInfo.softver, "0.9");
   // strcpy(GproductInfo.productsernum, "1703MC0501");

    strncpy(GproductInfo.productsernum, productsernum.toUtf8().constData(), sizeof(GproductInfo.productsernum) - 1);
    GproductInfo.productsernum[sizeof(GproductInfo.productsernum) - 1] = '\0';  // 添加终止的 null 字符

    qDebug()<<productsernum<<QString(GproductInfo.productsernum);


}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::doscantimer()
{


    QFile filex("/sys/class/thermal/thermal_zone0/temp");


   //显示的值是绝对值加上标定值
   float sxx = GpsystemStatus.x+GpsystemStatus.cx;

   if(-0.099<sxx&&sxx<0.099)
   {
       sxx=0;
   }

   QString sx;

   sx.append(QString::number(sxx,'f',1));
   sx.append("°");
   ui->label_xang->clear();
   ui->label_xang->setText(sx);







   //显示的值是绝对值加上标定值
   float syy = GpsystemStatus.y+GpsystemStatus.cy;
   if(-0.099<syy&&syy<0.099)
   {
       syy=0;
   }
   QString sy;

   sy.append(QString::number(syy,'f',1));
   sy.append("°");




   ui->label_yang->setText(sy);

   QString stemp = QString::number(GpsystemStatus.temp, 'f', 1).append("℃");
   ui->label_temp->setText(stemp);

    switch (icount)
    {


        case 0:


            break;
        case 1:


            break;

        case 2:


        if(1)
        {
            QString sfpress = QString::number(GpsystemStatus.press, 'f', 1).append("Kpa");
            ui->label_press->setText(sfpress);
        }


         break;
        case 3:
            #ifdef Q_OS_LINUX

                if (filex.open(QIODevice::ReadOnly))
                {
                    QString tempString = filex.readAll();
                    filex.close();

                    bool ok;
                    double temperature = tempString.toDouble(&ok);
                    if (ok)
                    {
                        GpsystemStatus.tempcpu = temperature;
                        //qDebug()<<"cpu tem"<<GpsystemStatus.tempcpu;
                        //return temperature / 1000.0; // 转换为摄氏度
                    }
                }
            #endif
           break;
    default:

        break;

    }

    icount++;

    if(icount%5==0)
    {
         icount = 0;




    }
    shownets();

    getLocalIpWithQt();
    if(!binit)
    {
        onThreadFinished();
        binit= true;
    }
}
void MainWindow::shownets()
{
#ifdef Q_OS_LINUX

    //#ifdef xxs
    bool eth0status;
    QString interfaceName = "eth0"; // 替换为你要判断的网络接口名称

    QNetworkInterface interface1 = QNetworkInterface::interfaceFromName(interfaceName);
    if (interface1.isValid() && interface1.flags().testFlag(QNetworkInterface::IsUp))
    {
        eth0status = true;

    }
    else
    {
        eth0status = false;

    }

    bool wifistatus;
    QString interfaceName2 = "wlan0"; // 替换为你要判断的网络接口名称

    QNetworkInterface interface2 = QNetworkInterface::interfaceFromName(interfaceName2);
    if (interface2.isValid() && interface2.flags().testFlag(QNetworkInterface::IsUp))
    {
        wifistatus = true;

    }
    else
    {
        wifistatus = false;

    }


    char cmdbuf[256];
    char cmdresult[256];

    memset ( cmdbuf,0, 256 );
    memset ( cmdresult,0, 256 );
    FILE* pp;



    if (eth0status)
    {
        sprintf ( cmdbuf, "cat /sys/class/net/eth0/carrier" );
        pp = popen ( cmdbuf, "r" ); //建立管道
        fgets ( cmdresult, sizeof ( cmdresult ), pp ); //""
        pclose ( pp );



        if ( strstr ( cmdresult, "0" ) )
        {
            //有线网络断开
            //qDebug()<<"yes lianjie c";
            QString stext = QString(tr(""));

            QPixmap picno(stext);

            ui->label_eth0->setPixmap(picno);
            //有线网络断开
            if(Beth0)
            {
                //上一次是连接的
                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("eth0");
                if(settings->contains("dhcp"))
                {
                    if(settings->value("dhcp").toInt()==1)
                    {





                    }
                    else
                    {
                        char cmdbuf[1024];
                        memset ( cmdbuf, 0,1024 );


                         QString ips = settings->value("ipeth0").toString();
                         QString ipmask = settings->value("masketh0").toString();

                         QHostAddress ipAddr(ips);
                         if (ipAddr.isNull())
                         {
                             ips.clear();
                             ips.append("192.168.2.212");
                         }
                         QHostAddress subnetMask(ipmask);

                         // 判断IP地址是否有效
                         if (ipAddr.isNull())
                         {
                             ipmask.clear();
                             ipmask.append("255.255.254.0");
                         }

                        //ifconfig eth0 192.168.2.211 netmask 255.255.254.0
                         sprintf ( cmdbuf, "ifconfig eth0 %s netmask 255.255.254.0",
                                  ips.toLatin1().data());
                         system ( cmdbuf );

                    }
                }
                else
                {
                    system("ifconfig eth0 192.168.2.212 netmask 255.255.254.0");
                }

                settings->endGroup();
                delete settings;

                Beth0 = false;
            }
        }
        else
        {
            //有线网络连接
            if(!Beth0)
            {
                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("eth0");
                if(settings->contains("dhcp"))
                {
                    if(settings->value("dhcp").toInt()==1)
                    {





                    }
                    else
                    {
                        char cmdbuf[1024];
                        memset ( cmdbuf, 0,1024 );


                         QString ips = settings->value("ipeth0").toString();
                         QString ipmask = settings->value("masketh0").toString();

                         QHostAddress ipAddr(ips);
                         if (ipAddr.isNull())
                         {
                             ips.clear();
                             ips.append("192.168.2.212");
                         }
                         QHostAddress subnetMask(ipmask);

                         // 判断IP地址是否有效
                         if (ipAddr.isNull())
                         {
                             ipmask.clear();
                             ipmask.append("255.255.255");
                         }

                        //ifconfig eth0 192.168.2.211 netmask 255.255.254.0
                         sprintf ( cmdbuf, "ifconfig eth0 %s netmask 255.255.255.0",
                                  ips.toLatin1().data());
                         system ( cmdbuf );

                    }
                }
                else
                {
                    system("ifconfig eth0 192.168.2.212 netmask 255.255.255.0");
                }

                settings->endGroup();
                delete settings;

                Beth0 = true;
            }

            //qDebug()<<"yes nolianjie";
            QString stext = QString(tr(":/eth0yes.png"));

            QPixmap picno(stext);

            ui->label_eth0->setPixmap(picno);
        }
    }
    else
    {
        QString stext = QString(tr(""));

        //qDebug()<<"is no readable";
        QPixmap picno(stext);

        ui->label_eth0->setPixmap(picno);
    }





    if(wifistatus)
    {
        //要判断是否连接

        memset ( cmdbuf,0, 256 );
        memset ( cmdresult,0, 256 );
        sprintf ( cmdbuf, "cat /sys/class/net/wlan0/carrier" );
        pp = popen ( cmdbuf, "r" ); //建立管道
        fgets ( cmdresult, sizeof ( cmdresult ), pp ); //""
        pclose ( pp );

        if ( strstr ( cmdresult, "0" ) )
        {
            //无线网络断开
            //qDebug()<<"yes lianjie c";
            QString stext = QString(tr(""));

            QPixmap picno(stext);

            ui->label_wifi->setPixmap(picno);
        }
        else
        {
            //无线网络连接
            //判断wlan0的ip地址是否有效
            //先获取wlan0的ip地址

            memset ( cmdbuf,0, 256 );
            memset ( cmdresult,0, 256 );
            sprintf ( cmdbuf, "ifconfig wlan0 | grep 'inet addr' | awk '{print $2}'" );
            pp = popen ( cmdbuf, "r" ); //建立管道
            fgets ( cmdresult, sizeof ( cmdresult ), pp ); 
            pclose ( pp );
            //判断是否有ip地址
            if (strstr(cmdresult, "192.168"))
            {
				//有ip地址
				//qDebug()<<"yes nolianjie";
                QString stext = QString(tr(":/wifi4.png"));

				QPixmap picno(stext);

				ui->label_wifi->setPixmap(picno);
			}
			else
			{
                //无线网络断开
                


                ui->label_wifi->setPixmap(QPixmap());

            }


        }
    }
    else
    {
        QString stext = QString(tr(""));

        QPixmap picno(stext);

        ui->label_wifi->setPixmap(picno);
    }






#endif
}
void MainWindow::getLocalIpWithQt()
{


    ui->label_ip->setText("noooooooo");

    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    //ui->com_Interface->clear();
    for(const QNetworkInterface& xinterface : interfaces)
    {
        //qDebug()<<xinterface.humanReadableName()<< xinterface.name();
        QHostAddress gateway;
        if(xinterface.name()=="wlan0")// 显示所有网络接口名称
        {

            const QList<QNetworkAddressEntry> entrys = xinterface.addressEntries();

            for(auto entery : entrys)
            {
                QString strType;
                switch (entery.ip().protocol())       // 判断IP地址类型
                {
                case QAbstractSocket::IPv4Protocol:

                    strType = "--------IPv4地址--------";
                    //qDebug()<<entery.ip().toString();
                    if(!Beth0)
                    ui->label_ip->setText(entery.ip().toString());


                    break;
                case QAbstractSocket::IPv6Protocol:
                    strType = "--------IPv6地址--------";
                    //qDebug()<<entery.ip().toString();
                    break;
                case QAbstractSocket::AnyIPProtocol:
                    strType = "--------IPv4或IPv6地址--------";
                    //qDebug()<<entery.ip().toString();
                    break;
                case QAbstractSocket::UnknownNetworkLayerProtocol:
                    strType = "--------未知地址--------";
                    //qDebug()<<entery.ip().toString();
                    break;
                }

            }
        }
        else if(xinterface.name()=="eth0")//"tunnel_5"
        {
            const QList<QNetworkAddressEntry> entrys = xinterface.addressEntries();

            foreach (QNetworkAddressEntry entry, entrys)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    QString ipAddress = entry.ip().toString();

                    if(Beth0)
                    ui->label_ip->setText(ipAddress);

                    //ui->legateeth->setText(entry.broadcast().toString());
                    if(entry.ip().toString().contains("192.168"))
                    {
                        break;
                    }
                    //qDebug() << "eth0 IP address: " << ipAddress;
                }
            }


        }


    }




}
