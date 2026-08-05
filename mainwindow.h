#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ddc2256.h"
#include <QMainWindow>
#include <QTimer>
#include "tcpnetserver.h"
#include "iicthread.h"
#include <modbusslave.h>
//#include "fbtft.h"
#include "gpio_contorl.h"
#include "ws2812_spi.h"
#include <QElapsedTimer>

class FsyMulticastBeacon;

typedef struct
{
    char productname[20];
    char producttype[20];
    char productsernum[20];
    char hardver[20];
    char softver[20];

}CONFIG_TYPE_ABOUT ;

//有线IP地址，wifi名字，wifi的密码
typedef struct
{
    char ip[16];//有线IP地址
    char ssid[32]; //wifi名字
    char password[32];//wifi的密码
  
}CONFIG_TYPE_NET;

typedef struct
{
    int jifendianrong;
    int beiyong;//
    int batty;
}CONFIG_TYPE_SETING ;

extern CONFIG_TYPE_NET  GconfigNet;

extern CONFIG_TYPE_SETING GSeting;

extern CONFIG_TYPE_ABOUT GproductInfo;
extern uint32_t Giswork;
extern int Goldmode;
typedef struct systemstatus
{
    int   battstatus;   //电池状态 0 没有电池，1充电， 2正常放电
    int   pressstatus;  //压力传感器bmp280是否在线
    int   temptatus;    //温度传感器ads115是否在线
    int   ddc2256tatus; //采集板连接状态
    float press;        //压强
    float temp;         //温度，四个温度的平均值
    float temp1;        //ads115通道1的值
    float x;        //x角度
    float y;        //y角度
    float z;        //z角度
    float tempbord;     //板子温度
    float tempcpu;      //cpu温度
    float battvol;      //充放电电压
    float battcur;      //放电电流
    float battEnerg;    //电池电量 %;      //ads115通道4的值
    float cx;           //x角度标定
    float cy;           //y角度标定
    float cz;           //z角度标定
    int   chip0;        //chip0 状态
    int   chip1;        //chip1 状态
    int   chip2;        //chip2 状态
    int   chip3;        //chip3 状态
    int   bendi;        //本底状态
    int   wifistatus; //wifi 实时连接状态（carrier/IP）
    int   eth0      ;  //有线状态
    int   oldprotol ; //旧协议模式
    int   HVTEST    ; //高压测试
    // 以下三个字段与 srtmap15 协议对齐（HVTEST 之后固定顺序）
    int   netservices; //网络管理服务 telnet/ftp/http/ssh，1=启用 0=禁用
    int   wifienable;  // [wlan0] enable，WiFi 开关配置（线圈1004），1=启用 0=禁用
    qint16 wifirssi;   // WiFi RSSI(dBm)，1个Modbus寄存器(2字节)；未关联/无效=-127
    // ArcMap-II 扩展：8 路电离室状态（协议共有字段之后追加）
    int   chip4;        //chip4 状态
    int   chip5;        //chip5 状态
    int   chip6;        //chip6 状态
    int   chip7;        //chip7 状态
}PsystemStatus;



extern PsystemStatus GpsystemStatus;
extern int GDebugMoni;

void syncNetServicesFromIni();
void syncWifiEnableFromIni();
void applyNetServicesChmod(bool enable);
void applyEth0IpDelayed(const QString &ip, int delayMs = 1500);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    ddc2256 *myddc2256;
    TcpNetServer *tcpnetser;
    TcpNetServer *tcpnetser2;
    QTimer *myscantimer;
    iicThread *myiicthread;
    int icount;
    int iicfd;

    //FbTFT *mytft;
    bool binit;
//    xxDS2783     *whyds2783;

//    pcf8563      *mypcf8563;
    GPIO_CONTORL *mygpio;

    void initproinfo();
    RDSModbusSlave *modbusdevice;
    FsyMulticastBeacon *fsyBeacon;
     bool Beth0;
     void getLocalIpWithQt();
     void ipinit();
public slots:
    void doscantimer();
    void shownets();
    void onThreadFinished();
    void updateWs2812Flow();
    void onHostTcpConnected();
    void onHostTcpDisconnected();

private:
    void startWs2812Flow();
    void triggerWs2812Flash3();
    void paintWs2812Breath();
    void paintWs2812Solid();
    void paintWs2812Flash();
    void paintWs2812Beam(uint16_t adc0);
    void paintWs2812BeamLevel(int level);
    void paintWs2812LogoChinBlue(uint8_t brightness);

    Ui::MainWindow *ui;
    Ws2812Spi *m_ws2812;
    QTimer *m_ws2812Timer;
    int m_ws2812Step;
    int m_ws2812FlashRemain; // >0 时执行全灯闪烁剩余半周期数（6=闪3次）
    uint16_t m_ws2812BeamSmooth; // 仅灯效用：(pre+now)/2，不改 ddc2256 原值
    bool m_beamCenterLocked;     // 已锁定出束中心点
    int m_beamLockChip;          // 锁定的 chip 0～7
    int m_beamLockCh;            // 锁定的通道 IN
    QElapsedTimer m_beamLockTimer;   // 锁定起始（至少跟 2s）
    QElapsedTimer m_beamBelowTimer;  // 锁定点持续低于阈值的计时
};
#endif // MAINWINDOW_H
