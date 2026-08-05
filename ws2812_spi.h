#ifndef WS2812_SPI_H
#define WS2812_SPI_H

#include <QObject>
#include <QByteArray>
#include <stdint.h>

/** WS2812B via Linux spidev (MOSI → DIN). Default 31 LEDs, /dev/spidev0.0, 6.4 MHz. */
class Ws2812Spi : public QObject
{
public:
    explicit Ws2812Spi(QObject *parent = nullptr);
    ~Ws2812Spi();

    bool init(const char *devicePath = "/dev/spidev0.0",
              int ledCount = 31,
              uint32_t speedHz = 6400000);

    int ledCount() const { return m_ledCount; }
    bool isOpen() const;

    void setLed(int index, uint8_t r, uint8_t g, uint8_t b);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    void clear();

    /** 将缓冲区发到灯带（GRB 顺序，含复位低电平尾） */
    bool show();

private:
    void appendWs2812Byte(uint8_t v, QByteArray *out);
    void rebuildSpiBuffer();

    int m_fd;
    int m_ledCount;
    uint32_t m_speedHz;
    uint8_t m_zeroSym;
    uint8_t m_oneSym;
    QByteArray m_grb;
    QByteArray m_spi;
};

#endif
