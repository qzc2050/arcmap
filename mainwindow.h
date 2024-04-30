#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ddc2256.h"
#include <QMainWindow>
#include "tcpnetserver.h"
#include "iicthread.h"
#include <modbusslave.h>
#include "fbtft.h"
#include "gpio_contorl.h"
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
}PsystemStatus;



extern PsystemStatus GpsystemStatus;
extern int GDebugMoni;
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

    FbTFT *mytft;
    bool binit;
//    xxDS2783     *whyds2783;

//    pcf8563      *mypcf8563;
    GPIO_CONTORL *mygpio;

    void initproinfo();
    RDSModbusSlave *modbusdevice;
     bool Beth0;
     void getLocalIpWithQt();
public slots:
    void doscantimer();
    void shownets();
    void onThreadFinished();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
