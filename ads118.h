#ifndef ADS118_H
#define ADS118_H

#include <QObject>
#include<QTimer>







//_ADS1115_C_EXT_ INT16U ads1115_read_ad_vale(INT8U channel);


//#endif




class ADS118 : public QObject
{
    Q_OBJECT
public:
    explicit ADS118(QObject *parent = nullptr);
    void ads1115init();
    void setid(int fd);
    int icout;

    QTimer *adstimer;
    int devfd;
    int devaddr;
//    uint16_t ads1115_read_ad_vale();

    void  ads1115_write_two_byte(unsigned char WriteAddr,unsigned short int DataToWrite);
    unsigned short int ads1115_read_two_byte(unsigned char ReadAddr);
    int currentch;

    float temprate0;
    int icout0;
    float temprate1;
    int icout1;
    float temprate2;
    int icout2;
    float temprate3;
    int icout3;
    int status;//0 表示ok，1 iic通信不正常


signals:
public slots:
    void ads1115_config_channel(uint8_t channel);
    float ads1115_read_ad_vale(uint8_t channel);
    void doadstimer();

private slots:


};

#endif // ADS118_H
