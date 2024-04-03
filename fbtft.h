#ifndef FBTFT_H
#define FBTFT_H

#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QImage>
//LCDÖØÒª²ÎÊý¼¯
typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t id;
    uint8_t  dir;
    uint16_t	 wramcmd;
    uint16_t  setxcmd;
    uint16_t  setycmd;
}_lcd_dev;

extern _lcd_dev lcddev;

#define USE_HORIZONTAL 0

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 320
#define LCD_H 480

#else
#define LCD_W 480
#define LCD_H 320
#endif
extern const unsigned char gImage_1[];

class FbTFT : public QThread
{
    Q_OBJECT
public:
    explicit FbTFT(QObject *parent = nullptr);

    void LCD_WR_REG(uint8_t dat);
    void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
    void LCD_Init(void);
    void delay_ms(int msec);
    void LCD_RES_Clr();
    void LCD_RES_Set();

    void    LCD_SCLK_Clr();
    void    LCD_SCLK_Set();

    void  LCD_MOSI_Clr();
    void  LCD_MOSI_Set();

    void  LCD_CS_Clr() ;
    void  LCD_CS_Set();


    void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);

    uint8_t SPI1_read_write_byte(uint8_t byte);
    void  LCD_DC_Clr();
    void  LCD_DC_Set();

    void LCD_BLK_Clr();
    void LCD_BLK_Set();

    int devfd;
    void LCD_Writ_Bus(uint8_t dat);
    void scl300_write_reg_multi_byte( const uint8_t *buf, uint16_t len);


    void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[]);


    void LCD_WR_DATA(uint8_t dat);
    void LCD_WR_DATA8(uint8_t dat);
    void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
    void showtest();
    void LCD_WriteRAM_Prepare(void);
    void Lcd_WriteData_16Bit(uint16_t Data);
    void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);
    void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
    void LCD_direction(uint8_t direction);
    void LCD_RESET(void);
    void LCD_Clear(uint16_t Color);
    void LCD_DrawPoint(uint16_t x,uint16_t y);
    void LCD_UpdateScreen();
    void run() override;
    void setImage();
    QImage m_image;

signals:

public slots:
};

#endif // FBTFT_H
