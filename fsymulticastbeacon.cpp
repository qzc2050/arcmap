#include "fsymulticastbeacon.h"

#include "mainwindow.h"
#include "tcpnetserver.h"

#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QTimer>

namespace {
constexpr char kMulticastGroup[] = "236.2.3.6";
constexpr quint16 kMulticastPort = 2468;
constexpr int kControlPort = 5001;
constexpr int kDataPort = 5000;
constexpr int kProtoType = 2;
constexpr int kReserved = 0;

QString ipv4OnInterface(const QString &ifName)
{
    const QNetworkInterface iface = QNetworkInterface::interfaceFromName(ifName);
    if (!iface.isValid())
        return QString();

    for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
            return entry.ip().toString();
    }
    return QString();
}

QString interfaceNameForIp(const QString &ip)
{
    if (ip.isEmpty())
        return QString();

    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol
                && entry.ip().toString() == ip) {
                return iface.name();
            }
        }
    }
    return QString();
}
} // namespace

FsyMulticastBeacon::FsyMulticastBeacon(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &FsyMulticastBeacon::sendBeacon);
}

FsyMulticastBeacon::~FsyMulticastBeacon()
{
    stop();
}

void FsyMulticastBeacon::start(int intervalMs)
{
    m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    if (!m_timer->isActive())
        m_timer->start(intervalMs);
    sendBeacon();
}

void FsyMulticastBeacon::stop()
{
    m_timer->stop();
}

QString FsyMulticastBeacon::resolveIPv4() const
{
    if (GpsystemStatus.eth0) {
        const QString ethIp = ipv4OnInterface(QStringLiteral("eth0"));
        if (!ethIp.isEmpty())
            return ethIp;
    }

    if (GpsystemStatus.wifistatus) {
        const QString wlanIp = ipv4OnInterface(QStringLiteral("wlan0"));
        if (!wlanIp.isEmpty())
            return wlanIp;
    }

    const QString ethIp = ipv4OnInterface(QStringLiteral("eth0"));
    if (!ethIp.isEmpty())
        return ethIp;

    return ipv4OnInterface(QStringLiteral("wlan0"));
}

QString FsyMulticastBeacon::buildPayload(const QString &ip) const
{
    const QString model = QString::fromUtf8(GproductInfo.producttype).trimmed();
    const QString serial = QString::fromUtf8(GproductInfo.productsernum).trimmed();

    return QStringLiteral("%1,%2,%3,%4,%5,%6,%7,%8")
        .arg(model.isEmpty() ? QStringLiteral("ArcMapII") : model)
        .arg(serial.isEmpty() ? QStringLiteral("0000000000") : serial)
        .arg(ip)
        .arg(kControlPort)
        .arg(kDataPort)
        .arg(NET_CMD_DEVICE_TYPE)
        .arg(kProtoType)
        .arg(kReserved);
}

void FsyMulticastBeacon::updateMulticastInterface(const QString &ip)
{
    const QString ifName = interfaceNameForIp(ip);
    if (ifName.isEmpty())
        return;

    const QNetworkInterface iface = QNetworkInterface::interfaceFromName(ifName);
    if (!iface.isValid())
        return;

    m_socket->setMulticastInterface(iface);
}

void FsyMulticastBeacon::sendBeacon()
{
    const QString ip = resolveIPv4();
    if (ip.isEmpty() || ip == QStringLiteral("0.0.0.0")) {
        return;
    }

    if (ip != m_lastIp) {
        m_lastIp = ip;
        updateMulticastInterface(ip);
        qDebug() << "FSY multicast beacon ip:" << ip;
    }

    const QByteArray payload = buildPayload(ip).toUtf8();
    const qint64 sent = m_socket->writeDatagram(
        payload,
        QHostAddress(QString::fromLatin1(kMulticastGroup)),
        kMulticastPort);

    if (sent != payload.size()) {
        qWarning() << "FSY multicast send failed:" << m_socket->errorString();
    }
}
