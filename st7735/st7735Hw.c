
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "st7735Hw.h"

static unsigned char st7735IoInit(void)
{
    gpio_init(LCD_BL_PIN);
    gpio_set_dir(LCD_BL_PIN, GPIO_OUT);
    gpio_put(LCD_BL_PIN, 1);  

    //复位线
    gpio_init(LCD_RST_PIN);
    gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    gpio_put(LCD_RST_PIN, 1);  

    //DC线
    gpio_init(LCD_DC_PIN);
    gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
    gpio_put(LCD_DC_PIN, 1);  
    return 0;
}

unsigned char st7735HwInit(void)
{
    spiInit();
    st7735IoInit();
    return 0;
}

void st7735DcCtrl(unsigned char v)
{
    gpio_put(LCD_DC_PIN, v);  
}

void st7735RstCtrl(unsigned char v)
{
    gpio_put(LCD_RST_PIN, v);  
}

void st7735BlCtrl(unsigned char v)
{
    gpio_put(LCD_BL_PIN, v);  
}