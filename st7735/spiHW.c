
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "st7735Hw.h"

static unsigned char spiIoInit(void)
{
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_CS_PIN, GPIO_FUNC_SPI);
    return 0;
}
static int spi_chan = 0xff;
static int spiOver_chan = 0xff;
static unsigned int spiOverflag = 0;
static unsigned char spiDmaInit(void)
{
    spi_chan = dma_claim_unused_channel(true); 
    spiOver_chan = dma_claim_unused_channel(true); 

    dma_channel_config c = dma_channel_get_default_config(spi_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
    channel_config_set_chain_to(&c, spiOver_chan);                                            ///spi 数据发送完成后触发

    dma_channel_configure(spi_chan, &c,
                          &spi_get_hw(spi0)->dr, // write address
                          NULL, // read address
                          128, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    c = dma_channel_get_default_config(spiOver_chan);                                        
    channel_config_set_read_increment(&c, false);                                           //读地址不用变
    channel_config_set_write_increment(&c, true);                                           //写地址自加
    channel_config_set_transfer_data_size(&c,DMA_SIZE_32);                                  //每次4个字节
    // Destination pointer // Source pointer // Number of transfers // Start immediately
    dma_channel_configure(spiOver_chan, &c,NULL,&spiOverflag,2,false);                        //写地址，开始写入，先不管。读地址一直是0，

    return 0;
}

unsigned char spiInit(void)
{
    spiIoInit();
    spi_init(spi0, 60000*1000);
    spiDmaInit();
    return 0;
}

int *tempxxx = NULL;
void spiSendDataDma(unsigned char *data,unsigned int len, int *overFlag)
{
    tempxxx = overFlag;
    dma_channel_set_write_addr(spiOver_chan,overFlag,false);
    dma_channel_transfer_from_buffer_now(spi_chan,data,len);
}

int getSpiDmaChan(void)
{
    return spi_chan;
}

