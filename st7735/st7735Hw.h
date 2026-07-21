

#define LCD_RST_PIN  4
#define LCD_DC_PIN   6
#define LCD_BL_PIN   7

#define LCD_CS_PIN   5
#define LCD_CLK_PIN  2
#define LCD_MOSI_PIN 3

void st7735RstCtrl(unsigned char v);
void st7735DcCtrl(unsigned char v);
void st7735BlCtrl(unsigned char v);

unsigned char spiInit(void);

unsigned char st7735HwInit(void);

void spiSendDataDma(unsigned char *data,unsigned int len,int *overFlag);
int getSpiDmaChan(void);
