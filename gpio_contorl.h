#ifndef GPIO_CONTORL_H
#define GPIO_CONTORL_H



#include <QObject>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_LINUX
//extern int gpio_export(unsigned gpio);
//extern int gpio_unexport(unsigned gpio);
//extern int gpio_value(unsigned gpio, unsigned value);
//extern int gpio_dir_in(unsigned gpio);
//extern int gpio_dir_out(unsigned gpio);
//extern int gpio_dir(unsigned gpio, unsigned dir);
#endif
extern int set_gpio_value(int index, unsigned value);
class GPIO_CONTORL : public QThread
{
    Q_OBJECT
public:
    GPIO_CONTORL();
    void run() override;
    QTimer *testmode;

     QTimer *ledmodetime;
    int icout;
    int icouthv;
    int  checkpower();
    int  checkbat();
    int  checkhv();
    int  icountpwm;
    bool increasing = true;  //
    int  HVCHAGE(unsigned value);
    int gammaTable[256]; // 伽马校正表
    int fdback;
    unsigned long args[3]={0};
signals:
    void startkeypressed();
    void endkeypressed();
private slots:
    void dotimeout();
    void doledtimeout();

};

#endif // GPIO_CONTORL_H
