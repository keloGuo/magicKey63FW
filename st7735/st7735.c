#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "st7735drive.h"
#include "st7735Hw.h"

#define ST7735_TYPE


unsigned char st7735Int(void)
{
	st7735DriveInit();
    return 0;
}

void LcdPush2(unsigned int x,unsigned int x1,unsigned int y,unsigned int y1,unsigned short *data,int *p)
{
	st7735WindowUpdate(x,y,x1,y1,(uint8_t *)data,p);
}
