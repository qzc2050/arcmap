#include "modbusslave.h"
#include <QModbusTcpServer>
#include <QByteArray>
#include <QDebug>
#include <QUrl>
#include <QModbusResponse>
#include <mainwindow.h>
#include <QSettings>
//#include "collect_save.h"
//#include <QModbusResponsePdu>

/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      modbus initialization
 * @param      IP/PORT/debugflag
 * @version    v1
 * @return     null
 * @date       2021/10/6
 **************************************************************/
bool RDSModbusSlave::initModbus(std::string Host_Ip = "127.0.0.1", int port = 502, bool debugging = true)
{
//    ctx = modbus_new_tcp(Host_Ip.c_str(), port);
//    modbus_set_debug(ctx, debugging);
//    if (ctx == NULL)
//    {
//        fprintf(stderr, "There was an error allocating the modbus\n");
//        throw - 1;
//    }
//    m_modbusSocket = modbus_tcp_listen(ctx, 3);
//    /*设置线圈, 离散输入, 输入寄存器, 保持寄存器个数(数组元素个数))*/
//    mapping = modbus_mapping_new(m_numBits, m_numInputBits, m_numInputRegisters, m_numRegisters);
//    if (mapping == NULL)
//    {
//        fprintf(stderr, "Unable to assign mapping：%s\n", modbus_strerror(errno));
//        modbus_free(ctx);
//        m_initialized = false;
//        return false;
//    }
//    m_initialized = true;
    return true;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      Constructor
 * @version    v1
 * @return     null
 * @date       2021/10/6
 **************************************************************/
RDSModbusSlave::RDSModbusSlave()
{
    const QUrl url("http://0.0.0.0:502");
    this->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
    this->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());

    this->setServerAddress(2);

    if(!this->connectDevice())
    {
        qDebug()<<"RDSModbusSlave init fail;";
    }
    else
    {
        qDebug()<<"RDSModbusSlave init success;";
    }



    connect(this, &QModbusServer::errorOccurred,
            this, &RDSModbusSlave::handleDeviceError);


}

QModbusResponse RDSModbusSlave::processRequest(const QModbusPdu &request)
{
    qDebug()<<request.functionCode();
    // 根据请求的功能码进行逻辑处理
    QModbusResponse response;
    switch (request.functionCode()) {
        case QModbusPdu::ReadHoldingRegisters:
            // 处理读取保持寄存器的请求
            response =  handleReadHoldingRegisters(request);
            break;
        case QModbusPdu::WriteMultipleRegisters:
            response = handleWriteHoldingRegisters(request);

            break;
        case QModbusPdu::WriteSingleRegister:
            // 处理写入单个寄存器的请求
            break;
        case QModbusPdu::ReadInputRegisters:
            // 处理读取输入寄存器的请求
            response =  handleReadInputRegisters(request); // 调用处理ReadInputRegisters功能码的函数
            break;
        case QModbusPdu::ReadCoils:
            // 处理读取线圈的请求
            break;
        // 处理其他功能码的请求
        case QModbusPdu::WriteSingleCoil:

            response =  handleWriteSingleCoil(request); // 调用处理ReadInputRegisters功能码的函数

            break;
        // ...

        default:
            // 处理未被显式处理的功能码
            break;
    }


    // 创建并返回相应的QModbusResponse对象


    return response;
}
QString toHexadecimal(const QByteArray &byteArray)
{
    QString str;
    for(int i = 0; i< byteArray.length(); i++){
        QString byteStr = QString::number(static_cast<uchar>(byteArray[i]), 16);
        if(byteStr.length() == 1) str += "0" + byteStr;
        else str += byteStr;
    }
    return str;
}

QModbusResponse RDSModbusSlave::handleWriteSingleCoil(const QModbusPdu &request)
{
    QByteArray requestData = request.data();

    // 读取起始地址和寄存器数量
    uint16_t x = (unsigned char)requestData.at(0);
    uint16_t y = (unsigned char)requestData.at(1);
    quint16 startAddress = x * 256 + y;
    x = requestData.at(2);
    y = requestData.at(3);
    quint16 registerCount = x * 256 + y;

    if(startAddress<16)
    {
//        if(registerCount==0xff00)
//            //set_gpio_value(startAddress,1);
//        else
//            //set_gpio_value(startAddress,0);
    }
    else if(startAddress==1000)
    {
        system("sync");
        system("sync");
        system("sync");

        system("sleep 3");
        system("reboot");
    }
    else if(startAddress==1001)
    {
        if(registerCount==0xff00)
            GDebugMoni =1;
        else
            GDebugMoni =0;
    }

    qDebug() << toHexadecimal(requestData);
    QModbusResponse response(request);
    QByteArray responsedata;

    responsedata[0] = requestData.at(0);
    responsedata[1] = requestData.at(1);
    responsedata[2] = requestData.at(2);
    responsedata[3] = requestData.at(3);
    response.setData(responsedata);
    return  response;
}

QModbusResponse RDSModbusSlave::handleWriteHoldingRegisters(const QModbusPdu &request)
{
    QByteArray requestData = request.data();

    qDebug() << "handleWriteHoldingRegisters"<<toHexadecimal(requestData);

    // 读取起始地址和寄存器数量
    uint16_t x = (unsigned char)requestData.at(0);
    uint16_t y = (unsigned char)requestData.at(1);
    quint16 startAddress = x * 256 + y;
    x = requestData.at(2);
    y = requestData.at(3);
    quint16 registerCount = x * 256 + y;
    quint16 byteCount = (unsigned char)requestData.at(4);

    QModbusResponse response(request);
    QByteArray responsedata;

    qDebug() << startAddress << registerCount << byteCount;


    if(startAddress < 500)
    {
        int iistartAddress = startAddress - 500;
        qDebug() << startAddress << iistartAddress << registerCount;
        
        //有线IP地址，wifi名字，wifi的密码
        // typedef struct
        // {
        //     char ip[16];//有线IP地址
        //     char ssid[32]; //wifi名字
        //     char password[32];//wifi的密码
        
        // }CONFIG_TYPE_NET;

        if((startAddress==100)&&(registerCount<9))//ip地址类似于192.168.1.1这种QString
        {
            //设置有线IP地址，最长16个字节
            //先判断是不是有效ip地址
            char tempip[16];
            memset(tempip,0,16);
            for(int i=0;i<registerCount*2;i++)
            {
                tempip[i] = requestData.at(5+i);    
            }
            QString ipString = QString(tempip);
            QHostAddress ipAddress(ipString);
            if(ipAddress.isNull())
            {
                //如果ipAddress是null，说明ipString不是合法的IP地址 
                //返回异常响应
                qDebug() << "ipString is not a valid IP address";   
                responsedata.resize(3);            // 异常响应的数据大小为 3    
                responsedata[0] = 0x83;            // Modbus 异常响应功能码
                responsedata[1] = 0x02;            // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
                responsedata[2] = 0x04;            // 附加异常数据（根据协议定义）
            }
            else
            {
                //更新  GconfigNet的ip
                memset(GconfigNet.ip,0,16);
                for(int i=0;i<registerCount*2;i++)
                {
                    GconfigNet.ip[i] = requestData.at(5+i);
                }
                //如果ipAddress不是null，说明ipString是合法的IP地址
                //QSetting里面设置IP地址，groupname是eth0,key是ipeth0
                //把ipString保存到QSetting里面  
                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("eth0");
                settings->setValue("ipeth0",ipString);
                settings->endGroup();
                settings->sync();
                delete  settings;  
                //返回正确响应

                responsedata[0] = requestData.at(0);
                responsedata[1] = requestData.at(1);
                responsedata[2] = requestData.at(2);
                responsedata[3] = requestData.at(3);
            
                 //更新设备ip地址
                char cmdbuf[1024];
                memset ( cmdbuf, 0,1024 );
                QString ips = QString(GconfigNet.ip);
                sprintf ( cmdbuf, "ifconfig eth0 %s netmask 255.255.254.0",ips.toLatin1().data());
                system ( cmdbuf );

            }                
            


        }
        else if((startAddress==108)&&(registerCount<17))//先判断是否是108,最长16个寄存器，32个字节
        {
            //设置wifi名字，不定长的字节
            //ssid没法判断有没有效，所以不用判断直接保存
            memset(GconfigNet.ssid,0,32);
            for(int i=0;i<registerCount*2;i++)
            {
                GconfigNet.ssid[i] = requestData.at(5+i);   
            }
            //更新设备的wifi名字 wifigroup 是wlan，key是wifiname
            QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
            settings->beginGroup("wlan");
            settings->setValue("wifiname",QString(GconfigNet.ssid));
            settings->endGroup();
            settings->sync();
            delete  settings;
            //写/etc/wpa_supplicant.conf
            FILE* fp = fopen ( "/etc/wpa_supplicant.conf", "wb" );
            if(fp == NULL)
            {
                //pline() << "None Encryption";

                //return false;
            }
            fprintf ( fp, "ctrl_interface=/var/run/wpa_supplicant\nctrl_interface_group=0\nap_scan=1\n\n" );

            fprintf ( fp, "network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tpriority=2\n}\n", GconfigNet.ssid,GconfigNet.password);


            fclose(fp);
            //返回正确响应

            responsedata[0] = requestData.at(0);
            responsedata[1] = requestData.at(1);
            responsedata[2] = requestData.at(2);
            responsedata[3] = requestData.at(3);
        }
        else if((startAddress==124)&&(registerCount<17))//wifi的密码
        {
            //设置wifi密码，不定长的字节
            //ssid没法判断有没有效，所以不用判断直接保存
            memset(GconfigNet.password,0,32);        
            for(int i=0;i<registerCount*2;i++)
            {
                GconfigNet.password[i] = requestData.at(5+i);       
            }
            //更新设备的wifi密码 wifigroup 是wlan，key是wifipassword
            QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
            settings->beginGroup("wlan");
            settings->setValue("wifipassword",QString(GconfigNet.password));        
            settings->endGroup();
            settings->sync();
            delete  settings;
            //写/etc/wpa_supplicant.conf
            //写/etc/wpa_supplicant.conf
            FILE* fp = fopen ( "/etc/wpa_supplicant.conf", "wb" );
            if(fp == NULL)
            {
                //pline() << "None Encryption";

                //return false;
            }
            fprintf ( fp, "ctrl_interface=/var/run/wpa_supplicant\nctrl_interface_group=0\nap_scan=1\n\n" );

            fprintf ( fp, "network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tpriority=2\n}\n", GconfigNet.ssid,GconfigNet.password);


            fclose(fp);
            //返回正确响应

            responsedata[0] = requestData.at(0);
            responsedata[1] = requestData.at(1);
            responsedata[2] = requestData.at(2);
            responsedata[3] = requestData.at(3);
        }
    }
    else if (startAddress < 1000)
    {
        int iistartAddress = startAddress - 500;
        qDebug() << startAddress << iistartAddress << registerCount;
        if((iistartAddress==20)&&registerCount<11)
        {
            //sernum
            memset(GproductInfo.productsernum,0,20);
            for(int i=0;i<registerCount*2;i++)
            {
                 GproductInfo.productsernum[i] = requestData.at(5+i);
            }

            QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);

            settings->beginGroup("GproductInfo");
            settings->setValue("productsernum",QString(GproductInfo.productsernum));



            settings->sync();
            settings->endGroup();
            delete  settings;
        }
        responsedata[0] = requestData.at(0);
        responsedata[1] = requestData.at(1);
        responsedata[2] = requestData.at(2);
        responsedata[3] = requestData.at(3);
    }
    else if (startAddress < 2000)
    {
        // 取浮点，赋值给 Gfloat 数组
        int iistartAddress = startAddress - 1000;

        qDebug() << startAddress << iistartAddress << registerCount;


        if (((iistartAddress + registerCount) * 2) > 1024)
        {
            // 返回异常
            qDebug() << (iistartAddress + registerCount) * 2;
            responsedata.resize(3);            // 异常响应的数据大小为 3
            responsedata[0] = 0x83;            // Modbus 异常响应功能码
            responsedata[1] = 0x02;            // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;            // 附加异常数据（根据协议定义）
        }
        else if (registerCount % 2 == 1 || byteCount % 4 != 0)
        {
            // 字节数至少是 4 的倍数，一个寄存器占两个字节
            qDebug() << "Invalid registerCount or byteCount";
        }
        else
        {
            int iistartindex = iistartAddress/2;
            for (int i = 0; i < byteCount / 4; i++)
            {
//                // 将请求数据中的浮点数值赋值给 Gfloat 数组的相应元素
//                char bytes[4];// = reinterpret_cast<const char*>(&value);//
//                bytes[0] = requestData[5+i*4+1];
//                bytes[1] = requestData[5+i*4+0];
//                bytes[2] = requestData[5+i*4+3];
//                bytes[3] = requestData[5+i*4+2];

//                float xvalue;
//                memcpy(&xvalue, bytes, sizeof(float));




//                Gfloatjiaozhun[iistartindex + i] = xvalue;
//                qDebug()<<Gfloatjiaozhun[iistartindex + i];
            }


            for(int i=0;i<256;i++)
            {
//                if(Gfloatjiaozhun[i]<0.5)
//                {
//                    Gfloatjiaozhun[i]=1;
//                }
//                if(Gfloatjiaozhun[i]>1.5)
//                {
//                    Gfloatjiaozhun[i]=1;
//                }
            }
            if((iistartAddress+registerCount)==512)
            {
//                // 将 float 数组转换为 QByteArray
//                qDebug()<<"update argyizhi----------------";
//                QSqlQuery query;
//                QByteArray byteArray(reinterpret_cast<const char*>(Gfloatjiaozhun), sizeof(Gfloatjiaozhun));

//                // 准备 SQL 查询字符串，用于删除表中的旧数据
//                QString tableName = "argyizhi";
//                QString deleteQueryString = "DELETE FROM " + tableName;

//                // 执行删除旧数据的查询
//                if (query.exec(deleteQueryString)) {
//                    // 删除成功，现在可以插入新数据

//                    // 准备 SQL 查询字符串，用于插入新数据
//                    QString columnName = "data_yizhi";
//                    QString insertQueryString = "INSERT INTO " + tableName + " (" + columnName + ") VALUES (:data)";
//                    query.prepare(insertQueryString);
//                    query.bindValue(":data", byteArray);

//                    // 执行插入新数据的查询
//                    if (query.exec()) {
//                        // 插入成功
//                    } else {
//                        // 插入失败，处理错误
//                        qDebug() << "Error inserting new data: ";
//                    }
//                } else {
//                    // 删除失败，处理错误
//                    qDebug() << "Error deleting old data: " ;
//                }
            }
            responsedata[0] = requestData.at(0);
            responsedata[1] = requestData.at(1);
            responsedata[2] = requestData.at(2);
            responsedata[3] = requestData.at(3);
        }
    }

    // 设置响应数据
    response.setData(responsedata);

    // 设置响应的功能码（与请求中的功能码一致）
    response.setFunctionCode(request.functionCode());

    // 返回响应对象
    return response;
}



QModbusResponse RDSModbusSlave::handleReadHoldingRegisters(const QModbusPdu &request)
{
    QByteArray requestData = request.data();

    qDebug() << "handleReadHoldingRegisters"<<toHexadecimal(requestData);
    // 读取起始地址和寄存器数量
    uint16_t x = (unsigned char )requestData.at(0);
    uint16_t y = (unsigned char )requestData.at(1);
    quint16 startAddress = x*256+y;
    x = requestData.at(2);
    y = requestData.at(3);

    quint16 registerCount = x*256+y;

    QModbusResponse response(request);
    QByteArray responsedata;


    qDebug()<<startAddress<<registerCount;
    if(startAddress<100)
    {
        //
    }
    else if(startAddress<500)
    {
        //100到500是ip wifi名 和密码 GconfigNet
        char* netConfigChar = reinterpret_cast<char*>(&GconfigNet);
        unsigned int iistartAddress = startAddress-100;
        qDebug()<<iistartAddress<<startAddress<<registerCount<<sizeof (CONFIG_TYPE_NET);
         if(registerCount<((sizeof(CONFIG_TYPE_NET)/2)+1))
         {
            responsedata.resize(registerCount*2+1);
            responsedata[0] =  registerCount*2;
            for(int i=0;i<registerCount*2;i++)
            {
                responsedata[i+1] = netConfigChar[iistartAddress+i];
            }
         }
         else
         {
             //返回异常
             responsedata.resize(3);  // 异常响应的数据大小为3
             responsedata[0] = 0x83;  // Modbus 异常响应功能码  
             responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
             responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）
         }

    }
    else if(startAddress<1000)
    {

        unsigned int iistartAddress = startAddress-500;
        qDebug()<<iistartAddress<<startAddress<<registerCount<<sizeof (CONFIG_TYPE_ABOUT);


        if(registerCount==sizeof(CONFIG_TYPE_ABOUT)/2)
        {


            char* aboutConfigChar = reinterpret_cast<char*>(&GproductInfo);
            responsedata.resize(registerCount*2+1);
            responsedata[0] =  registerCount*2;
            for(int i= 0;i<(registerCount);i++)//一个寄存器两个字节，一个浮点是个字节
            {
                 responsedata[1+i*2] =  aboutConfigChar[iistartAddress*2+i*2];
                 responsedata[1+i*2+1] = aboutConfigChar[iistartAddress*2+i*2+1];
            }

        }
        else
        {
            //返回异常
             qDebug()<<(iistartAddress+registerCount)*2;
            responsedata.resize(3);  // 异常响应的数据大小为3
            responsedata[0] = 0x83;  // Modbus 异常响应功能码
            responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）
        }
        qDebug()<<toHexadecimal(responsedata);
        response.setData(responsedata);


    }
    else if(startAddress<2000)
    {

        int iistartAddress = startAddress-1000;


        qDebug()<<startAddress<<iistartAddress<<registerCount;

        if(((iistartAddress+registerCount)*2)>1024)
        {
            //返回异常
             qDebug()<<(iistartAddress+registerCount)*2;
            responsedata.resize(3);  // 异常响应的数据大小为3
            responsedata[0] = 0x83;  // Modbus 异常响应功能码
            responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）

        }
        else if(registerCount%2==1)
        {
            //字节数至少是4的倍数，1个寄存器两个字节
            qDebug()<<"error registerCount";
        }
        else
        {
            //QDataStream stream(&byteArraysystem, QIODevice::WriteOnly);
           // stream.writeRawData(reinterpret_cast<const char*>(&Gbendi), 512);
            responsedata.resize(registerCount*2+1);
            responsedata[0] =  registerCount*2;
             int startindex = iistartAddress/2;
             qDebug()<<startindex;


            for(int i= 0;i<(registerCount/2);i++)//一个寄存器两个字节，一个浮点是个字节
            {


//                float value = Gfloatjiaozhun[startindex+i];  // 获取浮点数值

//                // 将浮点数值转换为字节表示，并存储到字节数组中
//                const char* bytes = reinterpret_cast<const char*>(&value);//
//                responsedata[1+i*4] =   bytes[3];
//                responsedata[1+i*4+1] = bytes[2];
//                responsedata[1+i*4+2] = bytes[1];
//                responsedata[1+i*4+3] = bytes[0];

            }
        }


        qDebug()<<toHexadecimal(responsedata);
        response.setData(responsedata);

    }
    return response;
}

QModbusResponse RDSModbusSlave::handleReadInputRegisters(const QModbusPdu &request)
{
    // 从请求中获取读取的输入寄存器地址和数量



    QByteArray byteArraysystem;


    QByteArray requestData = request.data();

     qDebug()<<toHexadecimal(requestData);
    // 读取起始地址和寄存器数量
    uint16_t x = (unsigned char )requestData.at(0);
    uint16_t y = (unsigned char )requestData.at(1);
    quint16 startAddress = x*256+y;
    x = requestData.at(2);
    y = requestData.at(3);

    quint16 registerCount = x*256+y;


   // qDebug()<<toHexadecimal(byteArraysystem);


    QModbusResponse response(request);
    QByteArray responsedata;



    if(startAddress<1000)
    {
        // PsystemStatus
        QDataStream stream(&byteArraysystem, QIODevice::WriteOnly);
        stream.writeRawData(reinterpret_cast<const char*>(&GpsystemStatus), sizeof(PsystemStatus));
        if(((startAddress+registerCount)*2)>byteArraysystem.size())
        {
            //返回异常
            responsedata.resize(3);  // 异常响应的数据大小为3
            responsedata[0] = 0x83;  // Modbus 异常响应功能码
            responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）

        }
        else
        {
            responsedata[0] =  registerCount*2;
            for(int i= 0;i<(registerCount);i++)
            {
                responsedata[1+i*2] = byteArraysystem[startAddress+2*i];
                responsedata[1+i*2+1] = byteArraysystem[startAddress+2*i+1];
            }
        }


       qDebug()<<toHexadecimal(responsedata);
        response.setData(responsedata);
    }
    else if(startAddress<2000)
    {
        //本底数据
        int iistartAddress = startAddress-1000;


        qDebug()<<iistartAddress<<registerCount;

        if(((iistartAddress+registerCount)*2)>512)
        {
            //返回异常
            qDebug()<<(iistartAddress+registerCount)*2;
            responsedata.resize(3);  // 异常响应的数据大小为3
            responsedata[0] = 0x83;  // Modbus 异常响应功能码
            responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）

        }
        else
        {
//            //QDataStream stream(&byteArraysystem, QIODevice::WriteOnly);
//           // stream.writeRawData(reinterpret_cast<const char*>(&Gbendi), 512);
//            responsedata[0] =  registerCount*2;
//            for(int i= 0;i<(registerCount);i++)
//            {
//                responsedata[1+i*2] = (Gbendi[iistartAddress+i]&0xff00)>>8;  //byteArraysystem[iistartAddress*2+2*i];
//                responsedata[1+i*2+1] = Gbendi[iistartAddress+i]&0xff;  //byteArraysystem[iistartAddress*2+2*i+1];
//            }
        }


       qDebug()<<toHexadecimal(responsedata);
        response.setData(responsedata);
    }
    else if(startAddress<3000)
    {
        //本底数据
        int iistartAddress = startAddress-2000;


        qDebug()<<iistartAddress<<registerCount;

        if(((iistartAddress+registerCount)*2)>512)
        {
            //返回异常
             qDebug()<<(iistartAddress+registerCount)*2;
            responsedata.resize(3);  // 异常响应的数据大小为3
            responsedata[0] = 0x83;  // Modbus 异常响应功能码
            responsedata[1] = 0x02;  // Modbus 异常码（这里使用自定义的异常码，具体根据协议定义进行选择）
            responsedata[2] = 0x04;  // 附加异常数据（根据协议定义）

        }
        else
        {
            //QDataStream stream(&byteArraysystem, QIODevice::WriteOnly);
           // stream.writeRawData(reinterpret_cast<const char*>(&Gbendi), 512);
            responsedata[0] =  registerCount*2;
            for(int i= 0;i<(registerCount);i++)
            {
                responsedata[1+i*2] = (ddc2556_data[0].adc_vale[iistartAddress+i]&0xff00)>>8;  //byteArraysystem[iistartAddress*2+2*i];
                responsedata[1+i*2+1] = ddc2556_data[0].adc_vale[iistartAddress+i]&0xff;  //byteArraysystem[iistartAddress*2+2*i+1];
            }
        }


       qDebug()<<toHexadecimal(responsedata);
        response.setData(responsedata);
    }



    return response;
}



void RDSModbusSlave::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !modbusDevice)
        return;
    qDebug()<<modbusDevice->errorString();

    //statusBar()->showMessage(modbusDevice->errorString(), 5000);
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      Destructor
 * @version    v1
 * @return     null
 * @date       2021/10/6
 **************************************************************/
RDSModbusSlave::~RDSModbusSlave()
{
//    modbus_mapping_free(mapping);
//    modbus_close(ctx);
//    modbus_free(ctx);
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      loadFromConfigFile
 * @version    v1
 * @return     null
 * @date       2021/10/18
 **************************************************************/
void RDSModbusSlave::loadFromConfigFile()
{
    return;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      run
 * @version    v1
 * @return     null
 * @date       2021/10/18
 **************************************************************/

/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      modbus_set_slave_id
 * @param      id
 * @version    v1
 * @return     null
 * @date       2021/10/19
 **************************************************************/
bool RDSModbusSlave::modbus_set_slave_id(int id)
{

    this->setServerAddress(id);
    return true;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      setRegisterValue(设置保存寄存器的值，类型为uint16_t)
 * @param      registerStartaddress(保存寄存器的起始地址)
 * @param      Value(写入到保存寄存器里的值)
 * @version    v1
 * @return     null
 * @date       2021/10/6
 **************************************************************/
bool RDSModbusSlave::setRegisterValue(int registerStartaddress, uint16_t Value)
{
    if (registerStartaddress > (m_numRegisters - 1))
    {
        return false;
    }
    slavemutex.lock();
    //mapping->tab_registers[registerStartaddress] = Value;
    slavemutex.unlock();
    return true;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      getRegisterValue(获取保存寄存器的值)
 * @param      registerStartaddress(保存寄存器的起始地址)
 * @version    v1
 * @return     null
 * @date       2021/10/6
 **************************************************************/
uint16_t RDSModbusSlave::getRegisterValue(int registerStartaddress)
{
    if (!m_initialized)
    {
        return -1;
    }
    //return mapping->tab_registers[registerStartaddress];
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      setTab_Input_Bits(设置输入寄存器某一位的值)
 * @param      NumBit(输入寄存器的起始地址)
 * @param      Value(输入寄存器的值)
 * @version    v1
 * @return     null
 * @date       2021/10/8
 **************************************************************/
bool RDSModbusSlave::setTab_Input_Bits(int NumBit, uint8_t Value)
{
    if (NumBit > (m_numInputBits - 1))
    {
        return false;
    }
    slavemutex.lock();
    //mapping->tab_input_bits[NumBit] = Value;
    slavemutex.unlock();
    return true;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      getTab_Input_Bits(获取输入寄存器某一位的值)
 * @param      NumBit(输入寄存器相应的bit位)
 * @version    v1
 * @return     null
 * @date       2021/10/8
 **************************************************************/
uint8_t RDSModbusSlave::getTab_Input_Bits(int NumBit)
{
    if (!m_initialized)
    {
        return -1;
    }
    return 0;// mapping->tab_input_bits[NumBit];
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      setRegisterFloatValue(设置浮点值)
 * @param      (Value：浮点值，registerStartaddress寄存器起始地址)
 * @version    v1
 * @return     null
 * @date       2021/10/8
 **************************************************************/
bool RDSModbusSlave::setRegisterFloatValue(float Value, int registerStartaddress)
{
    if (registerStartaddress > (m_numRegisters - 2))
    {
        return false;
    }
    /*小端模式*/
    slavemutex.lock();
    //modbus_set_float_badc(Value, &mapping->tab_registers[registerStartaddress]);
    slavemutex.unlock();
    return true;
}
/***************************************************************
 * @file       RDSModbusSlave.cpp
 * @author     seer-txj
 * @brief      获取寄存器里的浮点数
 * @param      registerStartaddress寄存器起始地址
 * @version    v1
 * @return     两个uint16_t拼接而成的浮点值
 * @date       2021/10/6
 **************************************************************/
float RDSModbusSlave::getRegisterFloatValue(int registerStartaddress)
{
    if (!m_initialized)
    {
        return -1.0f;
    }
    return  0;//modbus_get_float_badc(&mapping->tab_registers[registerStartaddress]);
}
