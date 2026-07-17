
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "st7735Hw.h"

#define ST7735A   1
unsigned char st7735Reset(void)
{
    st7735RstCtrl(1); 
    sleep_ms(200);
    st7735RstCtrl(0); 
    sleep_ms(200);
    st7735RstCtrl(1); 
    sleep_ms(200);
    return 0;
}

void st7735SpiWritByte(unsigned char data)
{
    spi_write_blocking(spi0, &data, 1);
}

void st7735SpiWritNbyte(unsigned char pData[], unsigned int Len)
{
    spi_write_blocking(spi0, pData, Len);
}

static void ST7735SendCommand(unsigned char Reg)
{
    st7735DcCtrl(0);
    st7735SpiWritByte(Reg);
}

static void St7735SendData8Bit(unsigned char  Data)
{
    st7735DcCtrl(1);
    st7735SpiWritByte(Data);
}

void St7735SetWindows(unsigned short Xstart, unsigned short Ystart, unsigned short Xend, unsigned short Yend)
{
#if ST7735A == 0	
	Xstart = Xstart + 1;
	Xend = Xend + 1;
	Ystart = Ystart + 26;
	Yend = Yend+26;

#else

	Xstart = Xstart;
	Xend = Xend;
	Ystart = Ystart + 24;
	Yend = Yend+24;
#endif
	ST7735SendCommand(0x2a);
	St7735SendData8Bit(0);
	St7735SendData8Bit(Xstart);

	St7735SendData8Bit(0);
	St7735SendData8Bit(Xend);

	ST7735SendCommand(0x2b);
	St7735SendData8Bit(0);
	St7735SendData8Bit(Ystart);

    St7735SendData8Bit(0);
	St7735SendData8Bit(Yend);

	ST7735SendCommand(0x2C);
}

void st7735RegInit(void)
{
	st7735Reset();

    ST7735SendCommand(0X11);
    sleep_ms(120);
#if ST7735A == 0	
    ST7735SendCommand(0X21);
    ST7735SendCommand(0X21);
#endif
    ST7735SendCommand(0Xb1);
    St7735SendData8Bit(0x05);
    St7735SendData8Bit(0x3a);
    St7735SendData8Bit(0x3a);

    ST7735SendCommand(0Xb2);
    St7735SendData8Bit(0x05);
    St7735SendData8Bit(0x3a);
    St7735SendData8Bit(0x3a);

    ST7735SendCommand(0Xb3);
    St7735SendData8Bit(0x05);
    St7735SendData8Bit(0x3a);
    St7735SendData8Bit(0x3a);
    St7735SendData8Bit(0x05);
    St7735SendData8Bit(0x3a);
    St7735SendData8Bit(0x3a);

    ST7735SendCommand(0Xb4);
    St7735SendData8Bit(0x03);

	ST7735SendCommand(0xC0);
	St7735SendData8Bit(0x62);
	St7735SendData8Bit(0x02);
	St7735SendData8Bit(0x04);

    ST7735SendCommand(0xC1);
	St7735SendData8Bit(0xC0);

	ST7735SendCommand(0xC2);
	St7735SendData8Bit(0x0D);
	St7735SendData8Bit(0x00);

	ST7735SendCommand(0xC3);
	St7735SendData8Bit(0x8D);
	St7735SendData8Bit(0x6A);   

	ST7735SendCommand(0xC4);
	St7735SendData8Bit(0x8D); 
	St7735SendData8Bit(0xEE); 

	ST7735SendCommand(0xC5);  /*VCOM*/
	St7735SendData8Bit(0x0E);    

	ST7735SendCommand(0xE0);
	St7735SendData8Bit(0x10);
	St7735SendData8Bit(0x0E);
	St7735SendData8Bit(0x02);
	St7735SendData8Bit(0x03);
	St7735SendData8Bit(0x0E);
	St7735SendData8Bit(0x07);
	St7735SendData8Bit(0x02);
	St7735SendData8Bit(0x07);
	St7735SendData8Bit(0x0A);
	St7735SendData8Bit(0x12);
	St7735SendData8Bit(0x27);
	St7735SendData8Bit(0x37);
	St7735SendData8Bit(0x00);
	St7735SendData8Bit(0x0D);
	St7735SendData8Bit(0x0E);
	St7735SendData8Bit(0x10);

	ST7735SendCommand(0xE1);
	St7735SendData8Bit(0x10);
	St7735SendData8Bit(0x0E);
	St7735SendData8Bit(0x03);
	St7735SendData8Bit(0x03);
	St7735SendData8Bit(0x0F);
	St7735SendData8Bit(0x06);
	St7735SendData8Bit(0x02);
	St7735SendData8Bit(0x08);
	St7735SendData8Bit(0x0A);
	St7735SendData8Bit(0x13);
	St7735SendData8Bit(0x26);
	St7735SendData8Bit(0x36);
	St7735SendData8Bit(0x00);
	St7735SendData8Bit(0x0D);
	St7735SendData8Bit(0x0E);
	St7735SendData8Bit(0x10);

	ST7735SendCommand(0x3A); 
	St7735SendData8Bit(0x05);

	ST7735SendCommand(0x36);
	St7735SendData8Bit(0x68);
	ST7735SendCommand(0x29); 

}

//X 缓存
//Y 计数

/*
. SEND_DATA
// Y = 8				1
// 数据，侧载时钟		 2
// 时钟					3
//判断					4
JMP BACK			    5
*/

//程序1 

//设置CS DC												     5
//取32位到 OSR ，									    	  6

//最低8位复制到X  	//第一个像素的高8位先留着			  		7
// Y = 8													 4
//循环发送8位	 	//第一个像素的低8位先发出去			  		5 6 7 //还可以用Y计数			
//16位放ISR		   	//第二个像素低8位缓存起来，用ISR做缓存，	 8
//X 8位复制OSR	   	//把一个像素的高8位复制到OSR，				9
//循环发送8位	   	//发出去
//把ISR8位复制到OSR	//准备发第二个像素的低8位发出去
//循环发8位			//
//Y 8位复制到OSR	
//循环发8位			//

unsigned char st7735WindowUpdate(unsigned char x,unsigned char y,unsigned char x1,unsigned char y1,unsigned char *data, int *p)
{
	if(dma_channel_is_busy(getSpiDmaChan()) == true) return 0;
	St7735SetWindows(x, y, x1, y1);
	st7735DcCtrl(1);
	//spi_write_blocking(spi0, data, (width + 1) * (hight + 1) *2);
	//dma_channel_transfer_from_buffer_now(DMA_SPI_CHANNEL,data, (x1 - x + 1)* (y1 - y + 1 ) *2);
	spiSendDataDma(data,(x1 - x + 1)* (y1 - y + 1 ) *2,p);
	return 0;
}

unsigned char st7735DriveInit(void)
{
    st7735HwInit();
    st7735RegInit();

    return 0;
}
