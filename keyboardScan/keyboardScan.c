  
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "keyboardscan.pio.h"
#include "hardware/dma.h"

void userPrintf(const char* format, ...);

#define SCAN_PIO_A                  pio0
#define SCAN_PIO_SM_A               0

#define SCAN_PIO_B                  pio0
#define SCAN_PIO_SM_B               1

#define XOR_PIO                     pio0
#define XOR_PIO_SM                  2

#define DATA_SEPARATION_PIO         pio1
#define DATA_SEPARATION_PIO_SM      0

#define ENCODER_A_PIO               pio1
#define ENCODER_A_PIO_SM            1


//8字节最新结果
static unsigned short dataBuff[8] __attribute__((aligned(16))) = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static unsigned char tempBuff[256] __attribute__((aligned(256)))= {0}; //上次生效数据
static unsigned int encoderData[2] __attribute__((aligned(8))) = {0,0};

unsigned short *getDataBuff(void)
{   
    return dataBuff;
}

static int Dma0Chan = 0, Dma1Chan =0,Dma2Chan =0,Dma3Chan =0,Dma4Chan =0,Dma5Chan=0,Dma6Chan=0;//

unsigned short encoderAData = 0;
void encoderCallback(int tx,unsigned char keyT);
void dma_handler6() { 
    dma_hw->ints1 = 0xffffffff;
    static unsigned char encoderAbusy =0,encoderBbusy=0;
    if((encoderData[0]  & 0x0f) == 0b0111 && encoderAbusy == 0)//左边编码器逆时针
    {
        encoderAData = 0x040;
        encoderAbusy = 1;
        // userPrintf("encoderAData =   %x\r\n",encoderAData);
        
    }
    else if((encoderData[0]  & 0x0f) == 0b1011 && encoderAbusy == 0)//左边编码器顺时针
    {   
        encoderAData = 0x020;
        encoderAbusy = 1;
        // userPrintf("encoderAData =   %x\r\n",encoderAData);
    }
    else if((encoderData[0]  & 0x0f) == 0b0000 )
    {
        encoderAbusy = 0;
    }

    if((encoderData[1]  & 0x0f) == 0b0111 && encoderBbusy == 0)//右边编码器逆时针
    {
        encoderCallback(1,3);
        encoderBbusy = 1;
        // userPrintf("encoderBData =   %x\r\n",encoderData[1]);
    }
    else if((encoderData[1]  & 0x0f) == 0b1011 && encoderBbusy == 0)//右边编码器顺时针
    {
        encoderBbusy = 1;
        encoderCallback(-1,3);
        // userPrintf("encoderBData =   %x\r\n",encoderData[1]);
    }
    else if((encoderData[1]  & 0x0f) == 0b0000 )
    {
        encoderBbusy = 0;
    }

    dma_start_channel_mask(1u << Dma5Chan);//
}

void keyboardScanDmaInit(void)
{
    Dma0Chan = dma_claim_unused_channel(true);       
    Dma1Chan = dma_claim_unused_channel(true);                       
    Dma2Chan = dma_claim_unused_channel(true);                                            //
    
    dma_channel_config   c = dma_channel_get_default_config(Dma0Chan);                    //
    channel_config_set_read_increment(&c, true);                                          //
    channel_config_set_write_increment(&c, false);                                        //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_8);                                 //
    channel_config_set_dreq(&c, pio_get_dreq(SCAN_PIO_A, SCAN_PIO_SM_A, true));                       //
    channel_config_set_chain_to(&c, Dma1Chan);    
    channel_config_set_irq_quiet(&c, true);                                               //
    channel_config_set_ring(&c, false,8);   
    dma_channel_configure(Dma0Chan, &c,&SCAN_PIO_A->txf[SCAN_PIO_SM_A],tempBuff,2,false);                    //

    
    c = dma_channel_get_default_config(Dma1Chan);                                         // 
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, false);                                        //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_8);                                 //
    channel_config_set_dreq(&c, pio_get_dreq(SCAN_PIO_A, SCAN_PIO_SM_A, false));                             // 
    channel_config_set_chain_to(&c, Dma2Chan);    
    channel_config_set_irq_quiet(&c, true);                                               //
    dma_channel_configure(Dma1Chan, &c,&XOR_PIO->txf[XOR_PIO_SM],&SCAN_PIO_A->rxf[SCAN_PIO_SM_A],3,false);               //
    
//    01 00 00  01 00 00  01 00 00  01 00 00  01 00 00  01 00 00  01 00 00  01 00 00   01 00 00  01 00 00  01 00 00  00 00 00  01 00 00  01 00 00  01 00 00  01 00 00

    c = dma_channel_get_default_config(Dma2Chan);                                         //
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, false);                                        //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_8);                                 //
    channel_config_set_dreq(&c, pio_get_dreq(XOR_PIO,XOR_PIO_SM, false));                            //
    channel_config_set_chain_to(&c, Dma0Chan);    
    channel_config_set_irq_quiet(&c, true);                                               //
    dma_channel_configure(Dma2Chan, &c,&DATA_SEPARATION_PIO->txf[DATA_SEPARATION_PIO_SM],&XOR_PIO->rxf[XOR_PIO_SM],2,false);              // 
//01 00  01 00  01 00  01 00  01 00  01 00  01 00  01 00   01 00  01 00  01 00  00 00  01 00  01 00  01 00  01 00 
 
    Dma3Chan = dma_claim_unused_channel(true);                                            //
    Dma4Chan = dma_claim_unused_channel(true);                                            //

    c = dma_channel_get_default_config(Dma3Chan);                                         //
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, true);                                         //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_8);                                 //
    channel_config_set_dreq(&c, pio_get_dreq(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, false));             //
//    channel_config_set_irq_quiet(&c, true);                                             //
    channel_config_set_chain_to(&c, Dma4Chan);    
    channel_config_set_ring(&c, true,8);  
    dma_channel_configure(Dma3Chan, &c,tempBuff,&DATA_SEPARATION_PIO->rxf[DATA_SEPARATION_PIO_SM],32,false);    // 

    c = dma_channel_get_default_config(Dma4Chan);                                         //
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, true);                                         //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_16);                                //
    channel_config_set_dreq(&c, pio_get_dreq(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, false));               //
    channel_config_set_irq_quiet(&c, true);                                               //
    channel_config_set_chain_to(&c, Dma3Chan);    
    channel_config_set_ring(&c, true,4);  
    dma_channel_configure(Dma4Chan, &c,dataBuff,&DATA_SEPARATION_PIO->rxf[DATA_SEPARATION_PIO_SM],1,false);      //

    Dma5Chan = dma_claim_unused_channel(true);                                            //
    Dma6Chan = dma_claim_unused_channel(true);                                            //

    c = dma_channel_get_default_config(Dma5Chan);                                         //
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, false);                                        //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_16);                                //
    channel_config_set_dreq(&c, pio_get_dreq(ENCODER_A_PIO, ENCODER_A_PIO_SM, true));     // 需要数据就给 
    channel_config_set_irq_quiet(&c, true);                                               //
    channel_config_set_chain_to(&c, Dma6Chan);    
    dma_channel_configure(Dma5Chan, &c,&ENCODER_A_PIO->txf[ENCODER_A_PIO_SM],&dataBuff[4],1,false);                     // 复制数据到TX

    c = dma_channel_get_default_config(Dma6Chan);                                         //
    channel_config_set_read_increment(&c, false);                                         //
    channel_config_set_write_increment(&c, true);                                         //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_32);                                                        //
    channel_config_set_dreq(&c, pio_get_dreq(ENCODER_A_PIO, ENCODER_A_PIO_SM, false));                            //
    //channel_config_set_irq_quiet(&c, true);                                                                     //
    //channel_config_set_chain_to(&c, Dma5Chan);
    channel_config_set_ring(&c, true,3);   
    dma_channel_configure(Dma6Chan, &c,encoderData,&ENCODER_A_PIO->rxf[ENCODER_A_PIO_SM],2,false);                     //&ENCODER_B_PIO->txf[ENCODER_B_PIO_SM]

    dma_channel_set_irq1_enabled(Dma6Chan, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler6);
    irq_set_enabled(DMA_IRQ_1, true);

    dma_start_channel_mask(1u << Dma0Chan | 1u << Dma3Chan | 1u << Dma5Chan);//
}

//这里16个输入
void keyboardScan_program_init(PIO pio, uint sm, uint offset, uint pin) {
    for(int i =0;i<16;i++)
    {
        pio_gpio_init(pio, pin+i);
        gpio_pull_down(pin+i);
    }

    pio_sm_set_consecutive_pindirs(pio, sm, pin, 16, false);
    pio_sm_config c = keyboardScan_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin);

    sm_config_set_out_shift(&c,true,false,32);
    sm_config_set_in_shift(&c,false,false,32);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio,sm,512,0); 
}

//5个输出
void keyboardScanPinSet_program_init(PIO pio, uint sm, uint offset, uint pin) {
    for(int i =0;i<5;i++)
    {
        pio_gpio_init(pio, pin+i);
    }
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 5, true);
    pio_sm_config c = keyboardScanPinSet_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);

    sm_config_set_out_shift(&c,true,false,32);
    sm_config_set_in_shift(&c,true,false,32);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio,sm,1,0); 
}

//异或输出
void xor_program_init(PIO pio, uint sm, uint offset) {
    pio_sm_config c = xor_program_get_default_config(offset);
    sm_config_set_out_shift(&c,true,true,8);
    sm_config_set_in_shift(&c,false,true,1);
    pio_sm_init(pio, sm, offset+2, &c);
    pio_sm_set_clkdiv_int_frac(pio,sm,1,0); 
}

//输出格式处理
void DataSeparation_program_init(PIO pio, uint sm, uint offset) {
    pio_sm_config c = DataSeparation_program_get_default_config(offset);
    sm_config_set_out_shift(&c,true,false,8);
    sm_config_set_in_shift(&c,false,true,8);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio,sm,1,0); 
}

//编码器预处理
void encoderA_program_init(PIO pio, uint sm, uint offset) {
    pio_sm_config c = encoderA_program_get_default_config(offset);
    sm_config_set_out_shift(&c,true,false,32);
    sm_config_set_in_shift(&c,false,false,32);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio,sm,1,0); 
}

uint offset,offset2,offset3,offset4,offset5;

void keyboardScanStart(void)
{
    offset = pio_add_program(SCAN_PIO_A, &keyboardScan_program);
    keyboardScan_program_init(SCAN_PIO_A, SCAN_PIO_SM_A, offset, 13);
    
    offset2 = pio_add_program(SCAN_PIO_B, &keyboardScanPinSet_program);
    keyboardScanPinSet_program_init(SCAN_PIO_B, SCAN_PIO_SM_B, offset2, 7); 

    offset3 = pio_add_program(XOR_PIO, &xor_program);
    xor_program_init(XOR_PIO, XOR_PIO_SM, offset3);

    offset4 = pio_add_program(DATA_SEPARATION_PIO, &DataSeparation_program);
    DataSeparation_program_init(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, offset4);

    offset5 = pio_add_program(ENCODER_A_PIO, &encoderA_program);
    encoderA_program_init(ENCODER_A_PIO, ENCODER_A_PIO_SM, offset5);

    keyboardScanDmaInit( );
    pio_sm_set_enabled(SCAN_PIO_A, SCAN_PIO_SM_A, true); // 0 0 
    pio_sm_set_enabled(SCAN_PIO_B, SCAN_PIO_SM_B, true); // 0 1
    pio_sm_set_enabled(XOR_PIO, XOR_PIO_SM, true);       // 0 2
    pio_sm_set_enabled(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, true);      //1 0      
    pio_sm_set_enabled(ENCODER_A_PIO, ENCODER_A_PIO_SM, true);                  //1 1                                                                                                                                                                  
}


//初始化键盘扫描
unsigned char keyboardScanInit(void)
{ 
    for (int i = 0; i < 256; i++) 
    {
        tempBuff[i]= 0;
    }
    
    keyboardScanStart();
    return 0;
}

unsigned char TestPrintf(void)
{
    static unsigned short tempOld[8] = {0};
    if(memcmp(tempOld,dataBuff,16)) 
    {  
        userPrintf("%04x %04x %04x %04x %04x\n",dataBuff[0],dataBuff[1],dataBuff[2],dataBuff[3],dataBuff[4]);
        memcpy(tempOld,dataBuff,16);
    }
    return 0;
}

bool encoderKeyCleck(repeating_timer_t *rt)
{
    static unsigned char timeCount = 0;

    if((dataBuff[4]&0x8000))
    {
        timeCount ++;

        if(timeCount == 100)
        {
            encoderCallback(0,2);//触发长按，
        }
    }
    else
    {
        // if(timeCount != 0) userPrintf("timeCount =   %x\r\n",timeCount);
        if((timeCount > 0) && (timeCount < 50))
        { 
            encoderCallback(0,1);//触发短按，
        }
         timeCount = 0;
    }
    return 1;
}

struct repeating_timer encoderKeyCleckTimer;
unsigned char encoderKeyCleckInit(void)
{
    add_repeating_timer_ms(10,encoderKeyCleck,0,&encoderKeyCleckTimer);
    return 0;
}
