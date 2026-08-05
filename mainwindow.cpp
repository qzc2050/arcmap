#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include "fsymulticastbeacon.h"
#include <QSettings>
#include <QScreen>
#include <QNetworkInterface>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <cstdio>
PsystemStatus GpsystemStatus;
CONFIG_TYPE_NET  GconfigNet;
CONFIG_TYPE_ABOUT GproductInfo;

int GDebugMoni=0;
int Goldmode=0;
CONFIG_TYPE_SETING GSeting;
uint32_t Giswork=0;//是否在工作

void applyEth0IpDelayed(const QString &ip, int delayMs)
{
    const QString ipCopy = ip;
    QTimer::singleShot(delayMs, [ipCopy]() {
        char cmdbuf[1024];
        memset(cmdbuf, 0, sizeof(cmdbuf));
        snprintf(cmdbuf, sizeof(cmdbuf), "ifconfig eth0 %s netmask 255.255.254.0",
                 ipCopy.toLatin1().constData());
        qDebug() << "apply eth0 ip:" << ipCopy;
        system(cmdbuf);
    });
}

void syncNetServicesFromIni()
{
    QSettings settings("set.ini", QSettings::IniFormat);
    settings.beginGroup("netservices");
    GpsystemStatus.netservices = settings.value("enable", 1).toInt();
    settings.endGroup();
}

void syncWifiEnableFromIni()
{
    QSettings settings("set.ini", QSettings::IniFormat);
    settings.beginGroup("wlan0");
    GpsystemStatus.wifienable = settings.value("enable", 1).toInt() ? 1 : 0;
    settings.endGroup();
}

void applyNetServicesChmod(bool enable)
{
    static const char *services[] = {
        "S50telnet",
        "S70vsftpd",
        "S50lighttpd",
        "S50dropbear",
        "S50sshd",
    };
    const char *mode = enable ? "+x" : "-x";

    for (unsigned i = 0; i < sizeof(services) / sizeof(services[0]); ++i) {
        char cmdbuf[128];
        snprintf(cmdbuf, sizeof(cmdbuf),
                 "[ -f /etc/init.d/%s ] && chmod %s /etc/init.d/%s",
                 services[i], mode, services[i]);
        system(cmdbuf);
    }
    system("sync");
    qDebug() << "applyNetServicesChmod:" << (enable ? "enable" : "disable");
}

static void updateWifiSignalFromProc(bool associated)
{
    GpsystemStatus.wifirssi = -127;

    if (!associated)
        return;

#ifdef Q_OS_LINUX
    QFile file("/proc/net/wireless");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    file.readLine();
    file.readLine();
    while (!file.atEnd())
    {
        const QByteArray line = file.readLine().trimmed();
        if (!line.startsWith("wlan0:"))
            continue;

        int status = 0;
        float link = 0, level = 0, noise = 0;
        if (sscanf(line.constData(), "wlan0: %x %f %f %f",
                   &status, &link, &level, &noise) < 4)
            break;

        if (noise <= -200.0f)
            break;

        GpsystemStatus.wifirssi = (qint16)(level + (level >= 0 ? 0.5f : -0.5f));
        break;
    }
    file.close();
#endif
}

/*
 * WS2812 面板灯位（灯号从 1 起，软件 index = 灯号 - 1；共 31 颗）：
 *
 *        1                    31
 *        2                    30
 *        3                    29
 *        4                    28     ← 左右括弧
 *        5                    27
 *        6                    26
 *        7                    25
 *        8                    24
 *        9                    23
 *       10                    22
 *          15 14 13 12 11           ← 嘴巴（logo），左→右
 *        16 17 18 19 20 21         ← 下巴/底排（下射灯），左→右，6 颗
 *
 *   1～10 ：左边括弧，上→下
 *   11～15：嘴巴（logo）；左→右为 15,14,13,12,11
 *   16～21：下巴（下射灯）；左→右为 16,17,18,19,20,21
 *   22～31：右边括弧，下→上（底 22 … 顶 31）
 */
static const int kWs2812LedCount = 31;
static const int kWs2812SpeedHz = 6400000;
static const int kWs2812FlowIntervalMs = 50;
static const int kWs2812MaxBrightness = 80;
static const int kWs2812FlashHalfCycles = 6; // 亮灭各 3 次 = 闪烁 3 次
// 档位映射：1000～30000 → 0～80% 档；30000～65000 → 80%～100% 档
static const uint16_t kWs2812BeamThreshold = 1000;
static const uint32_t kWs2812BeamMapMid = 30000;
static const uint32_t kWs2812BeamMapMax = 65000;
static const int kWs2812BeamHoldMs = 2000; // 锁定中心点后至少跟 2s
static const int kWs2812BeamAnimCycleMs = 1000; // 1秒走完 0→目标档→0 一个来回
static const int kWs2812BeamBelowHoldMs = 2000; // 锁定点连续低于阈值这么久才解锁重找
static const int kWs2812BeamChipCount = 8;
static const int kWs2812BeamTopN = 10;
static const int kWs2812BeamPickRank = 5; // 用前10大中的第5大作为中心点

/*
 * 灯效用 ADC 来源（二选一，改这一处即可）：
 *   >0  ：强制固定值（调试灯效，不读电离室）
 *   =0  ：用电离室动态锁定点
 * 看充放效果时设 15000；正式运行改回 0。
 */
static uint16_t GWs2812BeamForceAdc = 0; // >0 固定调试值；=0 用电离室（正式）

// 灯位分区（index = 灯号 - 1），见 ws2812_led_layout.txt
static const int kLedLeftFirst = 0;    // 1～10 左括弧
static const int kLedLeftCount = 10;
static const int kLedLogoFirst = 10;   // 11～15 logo
static const int kLedLogoCount = 5;
static const int kLedChinFirst = 15;   // 16～21 下巴/射灯，index 15～20
static const int kLedChinCount = 6;
static const int kLedRightFirst = 21;  // 22～31 右括弧（底→顶）
static const int kLedRightCount = 10;

// ADC → 括弧档位(0～10)：低段占 80%，高段占 20%
static int ws2812AdcToLevel(uint16_t adc)
{
    if (adc <= kWs2812BeamThreshold)
        return 0;

    // 用千分比避免浮点：0～800 对应低段，800～1000 对应高段
    int permille = 0;
    if (adc <= kWs2812BeamMapMid) {
        permille = static_cast<int>(
                    (static_cast<quint64>(adc - kWs2812BeamThreshold) * 800)
                    / (kWs2812BeamMapMid - kWs2812BeamThreshold));
    } else if (adc < kWs2812BeamMapMax) {
        permille = 800 + static_cast<int>(
                    (static_cast<quint64>(adc - kWs2812BeamMapMid) * 200)
                    / (kWs2812BeamMapMax - kWs2812BeamMapMid));
    } else {
        permille = 1000;
    }

    int level = (permille * kLedLeftCount + 500) / 1000; // 四舍五入
    if (level < 1)
        level = 1;
    if (level > kLedLeftCount)
        level = kLedLeftCount;
    return level;
}

struct Ws2812BeamTopEntry {
    uint16_t value;
    int chip;
    int ch;
};

// 只读扫描：找前10大，输出第5大所在 chip/通道/数值（不改 ddc2556）
static bool ws2812Find5thOfTop10(int *chipOut, int *chOut, uint16_t *valueOut)
{
    Ws2812BeamTopEntry top[kWs2812BeamTopN];
    for (int i = 0; i < kWs2812BeamTopN; ++i) {
        top[i].value = 0;
        top[i].chip = -1;
        top[i].ch = -1;
    }

    for (int chip = 0; chip < kWs2812BeamChipCount; ++chip) {
        for (int ch = 0; ch < 256; ++ch) {
            const uint16_t v = ddc2556_data[chip].adc_vale[ch];
            if (v <= top[kWs2812BeamTopN - 1].value)
                continue;
            int i = kWs2812BeamTopN - 1;
            while (i > 0 && v > top[i - 1].value) {
                top[i] = top[i - 1];
                --i;
            }
            top[i].value = v;
            top[i].chip = chip;
            top[i].ch = ch;
        }
    }

    const Ws2812BeamTopEntry &pick = top[kWs2812BeamPickRank - 1];
    if (pick.chip < 0)
        return false;
    *chipOut = pick.chip;
    *chOut = pick.ch;
    *valueOut = pick.value;
    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ws2812(nullptr)
    , m_ws2812Timer(nullptr)
    , m_ws2812Step(0)
    , m_ws2812FlashRemain(0)
    , m_ws2812BeamSmooth(0)
    , m_beamCenterLocked(false)
    , m_beamLockChip(-1)
    , m_beamLockCh(-1)
{
    ui->setupUi(this);



//    mytft = new FbTFT(this);
//    mytft->LCD_Init();
//    connect(mytft, &FbTFT::finished, this, &MainWindow::onThreadFinished);



    Beth0 = false;

    GpsystemStatus.wifirssi = -127;

    mygpio = new GPIO_CONTORL();

    mygpio->start(QThread::IdlePriority);

    binit = false;

    tcpnetser = new TcpNetServer();
    tcpnetser->start(5001);
    connect(tcpnetser, SIGNAL(hostClientConnected()), this, SLOT(onHostTcpConnected()));
    connect(tcpnetser, SIGNAL(hostClientDisconnected()), this, SLOT(onHostTcpDisconnected()));

    tcpnetser2 = new TcpNetServer();
    tcpnetser2->start(5000);

    myiicthread = new iicThread();

    myiicthread->start(QThread::IdlePriority);



    myddc2256 = NULL;




    myddc2256 = new ddc2256();
   //




    myscantimer = new QTimer(this);





    initproinfo();
    ipinit();
    connect(myscantimer,SIGNAL(timeout()),this,SLOT(doscantimer()));

    connect(myddc2256,SIGNAL(onepictureok()),tcpnetser2,SLOT(sendDataToClientsLocally()));
    myscantimer->start(1000);
    modbusdevice = new RDSModbusSlave();
    fsyBeacon = new FsyMulticastBeacon(this);
    fsyBeacon->start(1000);
    myddc2256->start();
    startWs2812Flow();

}
void MainWindow::ipinit()
{
    QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);

    settings->beginGroup("wlan0");
    if(settings->contains("enable"))
    {
        if(settings->value("enable").toInt()==1)
        {
            GpsystemStatus.wifienable = 1;
        }
        else
        {
            GpsystemStatus.wifienable = 0;
            system("ifconfig wlan0 down");
        }
    }
    else
    {
        GpsystemStatus.wifienable = 1;
        settings->setValue("enable", 1);
    }







    settings->endGroup();
    delete  settings;

}
void MainWindow::onThreadFinished()
{



//    mytft->setImage();
//    mytft->start(QThread::LowestPriority);

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

    // wifi名字和密码（主键 ssid/password；兼容旧键 wifiname/wifipassword）
    settings->beginGroup("wlan0");
    if(settings->contains("ssid"))
    {
        strcpy(GconfigNet.ssid, settings->value("ssid").toString().toStdString().c_str());  //ssid
        qDebug()<<"ssid"<<QString(GconfigNet.ssid);
    }
    else if(settings->contains("wifiname"))
    {
        strcpy(GconfigNet.ssid, settings->value("wifiname").toString().toStdString().c_str());
        settings->setValue("ssid", QString(GconfigNet.ssid));
        qDebug()<<"ssid(from wifiname)"<<QString(GconfigNet.ssid);
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
    else if(settings->contains("wifipassword"))
    {
        strcpy(GconfigNet.password, settings->value("wifipassword").toString().toStdString().c_str());
        settings->setValue("password", QString(GconfigNet.password));
        qDebug()<<"password(from wifipassword)"<<QString(GconfigNet.password);
    }
    else
    {
        strcpy(GconfigNet.password, "raydose888");
        qDebug()<<"password"<<QString(GconfigNet.password);
        settings->setValue("password","raydose888");
    }
    settings->endGroup();
    // /etc 常为只读根/叠加层，断电后 wpa 可能丢；用持久化的 set.ini 每次启动重建
    writeWpaSupplicantConf(GconfigNet.ssid, GconfigNet.password);
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

    settings->beginGroup("GconfigSetting");
    if(settings->contains("jifendianrong"))
    {
        GSeting.jifendianrong = settings->value("jifendianrong").toInt();  //jifendianrong
        qDebug()<<"jifendianrong"<<GSeting.jifendianrong;
    }
    else
    {
        GSeting.jifendianrong = 0;
        qDebug()<<"jifendianrong"<<GSeting.jifendianrong;
        settings->setValue("jifendianrong",0);
    }
    if(settings->contains("beiyong"))
    {
        GSeting.beiyong = settings->value("beiyong").toInt();  //beiyong
        qDebug()<<"beiyong"<<GSeting.beiyong;
    }
    else
    {
        GSeting.beiyong = 0;
        qDebug()<<"beiyong"<<GSeting.beiyong;
        settings->setValue("beiyong",0);
    }

    if(settings->contains("batty"))
    {
        GSeting.batty = settings->value("batty").toInt();  //batty
        qDebug()<<"batty"<<GSeting.batty;
    }
    else
    {
        GSeting.batty = 0;
        qDebug()<<"batty"<<GSeting.batty;
        settings->setValue("batty",0);
    }
    settings->endGroup();

    syncNetServicesFromIni();
    syncWifiEnableFromIni();

    // 软件版本见 version.h（Modbus 关于信息 / TCP cmd 11 可读）
    QString hardversion = QStringLiteral(ARCMAP_HW_VERSION_DEFAULT);

    settings->beginGroup("GproductInfo");
    if (settings->contains("hardwareversion"))
    {
        hardversion = settings->value("hardwareversion").toString();
    }
    else
    {
        settings->setValue("hardwareversion", QStringLiteral(ARCMAP_HW_VERSION_DEFAULT));
    }
    settings->endGroup();

    settings->sync();

    memset(&GproductInfo, 0, sizeof(CONFIG_TYPE_ABOUT));
    strcpy(GproductInfo.productname, "ArcMapII");
    strcpy(GproductInfo.producttype, "ArcMapII");
    strcpy(GproductInfo.softver, ARCMAP_VERSION_STRING);
    strcpy(GproductInfo.hardver, hardversion.toStdString().c_str());
   // strcpy(GproductInfo.productsernum, "1703MC0501");

    strncpy(GproductInfo.productsernum, productsernum.toUtf8().constData(), sizeof(GproductInfo.productsernum) - 1);
    GproductInfo.productsernum[sizeof(GproductInfo.productsernum) - 1] = '\0';  // 添加终止的 null 字符

    qDebug()<<productsernum<<QString(GproductInfo.productsernum);

    settings->sync();
    delete  settings;


}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startWs2812Flow()
{
    m_ws2812 = new Ws2812Spi(this);
    if (!m_ws2812->init("/dev/spidev0.0", kWs2812LedCount, kWs2812SpeedHz)) {
        qWarning() << "WS2812B init failed on /dev/spidev0.0";
        return;
    }

    m_ws2812Timer = new QTimer(this);
    connect(m_ws2812Timer, SIGNAL(timeout()), this, SLOT(updateWs2812Flow()));
    m_ws2812Timer->start(kWs2812FlowIntervalMs);
    updateWs2812Flow();
}

void MainWindow::triggerWs2812Flash3()
{
    m_ws2812FlashRemain = kWs2812FlashHalfCycles;
}

void MainWindow::onHostTcpConnected()
{
    qDebug() << "TCP5001 host connected, flash LEDs x3";
    triggerWs2812Flash3();
}

void MainWindow::onHostTcpDisconnected()
{
    qDebug() << "TCP5001 host disconnected, flash LEDs x3";
    triggerWs2812Flash3();
}

void MainWindow::paintWs2812Breath()
{
    const int period = kWs2812MaxBrightness * 2;
    int brightness = m_ws2812Step % period;
    if (brightness > kWs2812MaxBrightness)
        brightness = period - brightness;
    m_ws2812->fill(0, 0, static_cast<uint8_t>(brightness));
    m_ws2812Step++;
}

void MainWindow::paintWs2812Solid()
{
    m_ws2812->fill(0, 0, kWs2812MaxBrightness);
    m_ws2812Step = 0;
}

void MainWindow::paintWs2812Flash()
{
    // 偶数半周期亮，奇数半周期灭；约 200ms 切换一次（4×50ms）
    if ((m_ws2812FlashRemain % 2) == 0)
        m_ws2812->fill(0, 0, kWs2812MaxBrightness);
    else
        m_ws2812->clear();
    m_ws2812Step++;
    if ((m_ws2812Step % 4) == 0 && m_ws2812FlashRemain > 0)
        m_ws2812FlashRemain--;
}

// 括弧按物理高度上色：pos 1=最底(轻) … 10=最顶(最严重/最红)
static void ws2812BeamHeightColor(int pos, uint8_t maxB, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (pos < 1)
        pos = 1;
    if (pos > 10)
        pos = 10;
    // 蓝→紫→红：底下偏蓝，越往上越红
    *r = static_cast<uint8_t>(maxB * (pos - 1) / 9);
    *g = 0;
    *b = static_cast<uint8_t>(maxB * (10 - pos) / 9);
}

static uint8_t ws2812BreathLevel(int step)
{
    const int period = kWs2812MaxBrightness * 2;
    int brightness = step % period;
    if (brightness > kWs2812MaxBrightness)
        brightness = period - brightness;
    return static_cast<uint8_t>(brightness);
}

void MainWindow::paintWs2812LogoChinBlue(uint8_t brightness)
{
    static const int kLogoOrder[kLedLogoCount] = {14, 13, 12, 11, 10};
    for (int i = 0; i < kLedLogoCount; ++i)
        m_ws2812->setLed(kLogoOrder[i], 0, 0, brightness);
    for (int i = 0; i < kLedChinCount; ++i)
        m_ws2812->setLed(kLedChinFirst + i, 0, 0, brightness);
}

void MainWindow::paintWs2812BeamLevel(int level)
{
    m_ws2812->clear();

    if (level < 0)
        level = 0;
    if (level > kLedLeftCount)
        level = kLedLeftCount;

    // 从底往上亮 level 颗；颜色按固定灯位：底(10/22)最轻蓝，顶(1/31)最严重红
    if (level > 0) {
        for (int i = kLedLeftCount - level; i < kLedLeftCount; ++i) {
            const int pos = kLedLeftCount - i; // 灯10→pos1 … 灯1→pos10
            uint8_t r, g, b;
            ws2812BeamHeightColor(pos, static_cast<uint8_t>(kWs2812MaxBrightness), &r, &g, &b);
            m_ws2812->setLed(kLedLeftFirst + i, r, g, b);
        }
        for (int i = 0; i < level; ++i) {
            const int pos = i + 1; // 灯22→pos1 … 灯31→pos10
            uint8_t r, g, b;
            ws2812BeamHeightColor(pos, static_cast<uint8_t>(kWs2812MaxBrightness), &r, &g, &b);
            m_ws2812->setLed(kLedRightFirst + i, r, g, b);
        }
    }

    const uint8_t blueLevel = (GpsystemStatus.eth0 == 1)
            ? static_cast<uint8_t>(kWs2812MaxBrightness)
            : ws2812BreathLevel(m_ws2812Step);
    paintWs2812LogoChinBlue(blueLevel);

    m_ws2812Step++;
}

void MainWindow::paintWs2812Beam(uint16_t adc0)
{
    paintWs2812BeamLevel(ws2812AdcToLevel(adc0));
}

void MainWindow::updateWs2812Flow()
{
    if (!m_ws2812 || !m_ws2812->isOpen())
        return;

    // 优先级：TCP 闪 3 次 > 出束括弧渐变 > 未插网线呼吸 > 有网线常亮
    if (m_ws2812FlashRemain > 0) {
        paintWs2812Flash();
    } else {
        // ---------- 灯效目标 ADC：强制固定 或 电离室 ----------
        uint16_t beamAdc = 0;
        if (GWs2812BeamForceAdc > 0) {
            // 调试：全局固定值，不读电离室
            beamAdc = GWs2812BeamForceAdc;
            if (!m_beamLockTimer.isValid())
                m_beamLockTimer.start();
        } else {
            // 正式：动态区域 + 中心锁定（只读 ddc2556，不写回）
            if (m_beamCenterLocked
                    && m_beamLockChip >= 0 && m_beamLockChip < kWs2812BeamChipCount
                    && m_beamLockCh >= 0 && m_beamLockCh < 256) {
                beamAdc = ddc2556_data[m_beamLockChip].adc_vale[m_beamLockCh];

                if (beamAdc > kWs2812BeamThreshold) {
                    m_beamBelowTimer.invalidate();
                } else if (!m_beamBelowTimer.isValid()) {
                    m_beamBelowTimer.start();
                }

                const bool minHoldDone = m_beamLockTimer.isValid()
                        && m_beamLockTimer.elapsed() >= kWs2812BeamHoldMs;
                const bool belowLongEnough = m_beamBelowTimer.isValid()
                        && m_beamBelowTimer.elapsed() >= kWs2812BeamBelowHoldMs;
                if (minHoldDone && belowLongEnough) {
                    m_beamCenterLocked = false;
                    m_beamLockChip = -1;
                    m_beamLockCh = -1;
                    m_beamLockTimer.invalidate();
                    m_beamBelowTimer.invalidate();
                }
            }

            if (!m_beamCenterLocked) {
                int chip = -1;
                int ch = -1;
                uint16_t v = 0;
                if (ws2812Find5thOfTop10(&chip, &ch, &v) && v > kWs2812BeamThreshold) {
                    m_beamCenterLocked = true;
                    m_beamLockChip = chip;
                    m_beamLockCh = ch;
                    m_beamLockTimer.start();
                    m_beamBelowTimer.invalidate();
                    beamAdc = v;
                } else {
                    beamAdc = 0;
                }
            }
        }

        m_ws2812BeamSmooth = static_cast<uint16_t>(
                    (static_cast<uint32_t>(m_ws2812BeamSmooth) * 3
                     + static_cast<uint32_t>(beamAdc)) / 4);

        const bool showBeamAnim = (GWs2812BeamForceAdc > 0) || m_beamCenterLocked;
        if (showBeamAnim) {
            const uint16_t targetAdc = (GWs2812BeamForceAdc > 0)
                    ? GWs2812BeamForceAdc
                    : m_ws2812BeamSmooth;
            const int targetLevel = ws2812AdcToLevel(targetAdc);

            // 1秒三角波：按档位 0→目标→0 逐颗点亮/熄灭（充电条效果）
            int phaseMs = 0;
            if (m_beamLockTimer.isValid())
                phaseMs = static_cast<int>(m_beamLockTimer.elapsed() % kWs2812BeamAnimCycleMs);
            const int halfMs = kWs2812BeamAnimCycleMs / 2;
            int litLevel = 0;
            if (targetLevel > 0) {
                if (phaseMs <= halfMs) {
                    litLevel = (phaseMs * targetLevel + halfMs / 2) / halfMs;
                } else {
                    litLevel = ((kWs2812BeamAnimCycleMs - phaseMs) * targetLevel + halfMs / 2) / halfMs;
                }
                if (litLevel > targetLevel)
                    litLevel = targetLevel;
            }
            paintWs2812BeamLevel(litLevel);
        } else if (GpsystemStatus.eth0 != 1) {
            paintWs2812Breath();
        } else {
            paintWs2812Solid();
        }
    }

    m_ws2812->show();
}

void MainWindow::doscantimer()
{
//    if (Giswork == 1) {
//        // 如果Giswork为1，则周期为1000ms
//        myscantimer->setInterval(1000);
//    } else {
//        // 如果Giswork为0，则周期为500ms
//        myscantimer->setInterval(500);
//    }


//    if(!binit)
//    {
//        onThreadFinished();
//        binit= true;
//    }
//    return;

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


                GpsystemStatus.eth0 = 0;
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

                  GpsystemStatus.eth0 = 1;
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
    if(Beth0)
    {
        GpsystemStatus.eth0 = 1;
    }
    else
    {
        GpsystemStatus.eth0 = 0;
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
            GpsystemStatus.wifistatus = 0;
            updateWifiSignalFromProc(false);
        }
        else
        {
            updateWifiSignalFromProc(true);

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
                if(!Beth0)
                {
                    QString stext = QString(tr(":/wifi4.png"));

                    QPixmap picno(stext);

                    ui->label_wifi->setPixmap(picno);
                    GpsystemStatus.wifistatus = 1;
                }
                else {
                    //无线网络断开
                    GpsystemStatus.wifistatus = 0;

                    ui->label_wifi->setPixmap(QPixmap());

                }

			}
			else
			{
                //无线网络断开
                

                GpsystemStatus.wifistatus = 0;

                ui->label_wifi->setPixmap(QPixmap());

            }


        }
    }
    else
    {
        GpsystemStatus.wifistatus = 0;
        updateWifiSignalFromProc(false);

        QString stext = QString(tr(""));

        QPixmap picno(stext);

        ui->label_wifi->setPixmap(picno);
    }






#endif
    //GpsystemStatus.wifistatus = 1;
}
void MainWindow::getLocalIpWithQt()
{


    ui->label_ip->setText(" ");

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
                    {

                        ui->label_ip->setText(ipAddress);

                    }

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
