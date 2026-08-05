#ifndef FSYMULTICASTBEACON_H
#define FSYMULTICASTBEACON_H

#include <QObject>

class QUdpSocket;
class QTimer;

/** FSY App 组播发现：236.2.3.6:2468，与 NeiJi / FsyMulticastDiscovery 一致 */
class FsyMulticastBeacon : public QObject
{
    Q_OBJECT
public:
    explicit FsyMulticastBeacon(QObject *parent = nullptr);
    ~FsyMulticastBeacon() override;

    void start(int intervalMs = 1000);
    void stop();

private slots:
    void sendBeacon();

private:
    QString resolveIPv4() const;
    QString buildPayload(const QString &ip) const;
    void updateMulticastInterface(const QString &ip);

    QUdpSocket *m_socket;
    QTimer *m_timer;
    QString m_lastIp;
};

#endif // FSYMULTICASTBEACON_H
