#include "ws2812_spi.h"

#include <QDebug>
#include <cstring>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif

static const int kResetZeroBytes = 320;

Ws2812Spi::Ws2812Spi(QObject *parent)
    : QObject(parent)
    , m_fd(-1)
    , m_ledCount(0)
    , m_speedHz(6400000)
    , m_zeroSym(0xC0)
    , m_oneSym(0xF8)
{
}

Ws2812Spi::~Ws2812Spi()
{
#ifdef Q_OS_LINUX
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
#endif
}

bool Ws2812Spi::isOpen() const
{
#ifdef Q_OS_LINUX
    return m_fd >= 0;
#else
    return false;
#endif
}

void Ws2812Spi::appendWs2812Byte(uint8_t v, QByteArray *out)
{
    for (int bit = 7; bit >= 0; --bit) {
        if (v & (1u << bit))
            out->append(static_cast<char>(m_oneSym));
        else
            out->append(static_cast<char>(m_zeroSym));
    }
}

void Ws2812Spi::rebuildSpiBuffer()
{
    m_spi.resize(0);
    m_spi.reserve(m_ledCount * 24 + kResetZeroBytes * 2);

    m_spi.append(QByteArray(kResetZeroBytes, char(0)));

    for (int i = 0; i < m_ledCount; ++i) {
        const int base = i * 3;
        appendWs2812Byte(static_cast<uint8_t>(m_grb[base + 0]), &m_spi);
        appendWs2812Byte(static_cast<uint8_t>(m_grb[base + 1]), &m_spi);
        appendWs2812Byte(static_cast<uint8_t>(m_grb[base + 2]), &m_spi);
    }

    m_spi.append(QByteArray(kResetZeroBytes, char(0)));
}

bool Ws2812Spi::init(const char *devicePath, int ledCount, uint32_t speedHz)
{
#ifdef Q_OS_LINUX
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }

    if (ledCount <= 0 || ledCount > 1024) {
        qWarning() << "Ws2812Spi: invalid ledCount" << ledCount;
        return false;
    }

    m_ledCount = ledCount;
    m_speedHz = speedHz;
    m_grb.resize(m_ledCount * 3);
    m_grb.fill(char(0));

    if (speedHz >= 7500000) {
        m_zeroSym = 0xC0;
        m_oneSym = 0xFC;
    } else {
        m_zeroSym = 0xC0;
        m_oneSym = 0xF8;
    }

    m_fd = open(devicePath, O_RDWR);
    if (m_fd < 0) {
        perror("Ws2812Spi: open spidev");
        return false;
    }

    int mode = SPI_MODE_0;
    int bits = 8;
    if (ioctl(m_fd, SPI_IOC_WR_MODE, &mode) < 0
        || ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0
        || ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speedHz) < 0) {
        perror("Ws2812Spi: ioctl SPI config");
        close(m_fd);
        m_fd = -1;
        return false;
    }

    mode &= ~SPI_CS_HIGH;
    ioctl(m_fd, SPI_IOC_WR_MODE, &mode);

    rebuildSpiBuffer();
    return true;
#else
    Q_UNUSED(devicePath);
    Q_UNUSED(ledCount);
    Q_UNUSED(speedHz);
    qWarning() << "Ws2812Spi: Linux only";
    return false;
#endif
}

void Ws2812Spi::setLed(int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < 0 || index >= m_ledCount)
        return;
    const int base = index * 3;
    m_grb[base + 0] = static_cast<char>(g);
    m_grb[base + 1] = static_cast<char>(r);
    m_grb[base + 2] = static_cast<char>(b);
}

void Ws2812Spi::fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < m_ledCount; ++i)
        setLed(i, r, g, b);
}

void Ws2812Spi::clear()
{
    m_grb.fill(char(0));
}

bool Ws2812Spi::show()
{
#ifdef Q_OS_LINUX
    if (m_fd < 0)
        return false;

    rebuildSpiBuffer();

    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = reinterpret_cast<unsigned long>(m_spi.data());
    tr.rx_buf = 0;
    tr.len = static_cast<uint32_t>(m_spi.size());
    tr.speed_hz = m_speedHz;
    tr.bits_per_word = 8;
    tr.delay_usecs = 0;

    const int ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Ws2812Spi: SPI_IOC_MESSAGE");
        return false;
    }
    return true;
#else
    return false;
#endif
}
