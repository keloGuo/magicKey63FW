  
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "keyboardscan.pio.h"
#include "keyBounceDiag.pio.h"
#include "hardware/dma.h"
#include "keyboardScan.h"

void userPrintf(const char* format, ...);
void debugEvent(const char *tag, int value);
void debugEventText(const char *tag, const char *text, int value);
void ws2812RestoreHardware(void);

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

#define KEY_SCAN_ROW_PIN_BASE        7u
#define KEY_SCAN_COL_PIN_BASE        13u
#define KEY_SCAN_ROW_COUNT           5u
#define KEY_SCAN_COL_COUNT           16u
#define KEY_BOUNCE_DIAG_PIO          pio0
#define KEY_BOUNCE_DIAG_SM           0u
#define KEY_BOUNCE_DIAG_STABLE_US    1000000u
#define KEY_BOUNCE_DIAG_RELEASE_STABLE_US 300000u
#define KEY_BOUNCE_DIAG_CLKDIV       1.0f
#define KEY_BOUNCE_DIAG_ROW4_IGNORE_MASK 0x8d60u
#define KEY_BOUNCE_DIAG_DRIVE_ALL_ROWS 0u


//8字节最新结果
static unsigned short dataBuff[8] __attribute__((aligned(16))) = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static unsigned char tempBuff[256] __attribute__((aligned(256)))= {0}; //上次生效数据
static unsigned int encoderData[2] __attribute__((aligned(8))) = {0,0};

unsigned short *getDataBuff(void)
{   
    return dataBuff;
}

static int Dma0Chan = -1, Dma1Chan = -1, Dma2Chan = -1, Dma3Chan = -1, Dma4Chan = -1, Dma5Chan = -1, Dma6Chan = -1;//
static unsigned char keyboardScanDmaClaimed = 0;
static unsigned char keyboardScanPio0Loaded = 0;
static unsigned char keyboardScanPio1Loaded = 0;
static unsigned char keyBounceDiagPioLoaded = 0;
static uint keyBounceDiagOffset = 0;
static unsigned char keyBounceDiagPioMode = 0;

static volatile key_bounce_diag_result_t keyBounceDiagResult = {0};
static unsigned short keyBounceDiagBaseline[5] = {0};
static unsigned int keyBounceDiagLastPollUs = 0;
static unsigned int keyBounceDiagColPin = 0xffffffffu;
static unsigned int keyBounceDiagLastTraceUs = 0;
static unsigned int keyBounceDiagReleaseStartUs = 0;
static unsigned char keyBounceDiagReleasePhase = 0;
static unsigned char keyBounceDiagSelectCol = 0xff;
static volatile unsigned char keyBounceDiagCancelPending = 0;
static unsigned char keyBounceDiagDoneContinue = 0;

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
    if(!keyboardScanDmaClaimed)
    {
        debugEventText("scan", "dma_claim", 0);
        Dma0Chan = dma_claim_unused_channel(true);
        Dma1Chan = dma_claim_unused_channel(true);
        Dma2Chan = dma_claim_unused_channel(true);
        Dma3Chan = dma_claim_unused_channel(true);
        Dma4Chan = dma_claim_unused_channel(true);
        Dma5Chan = dma_claim_unused_channel(true);
        Dma6Chan = dma_claim_unused_channel(true);
        keyboardScanDmaClaimed = 1;
        debugEvent("scan_dma_ch0", Dma0Chan);
        debugEvent("scan_dma_ch6", Dma6Chan);
    }

    
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
    debugEventText("scan", "dma_start", 0);
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

static void keyboardScanLoadPrograms(void)
{
    if(!keyboardScanPio0Loaded)
    {
        debugEventText("scan", "pio0_load", 0);
        offset = pio_add_program(SCAN_PIO_A, &keyboardScan_program);
        offset2 = pio_add_program(SCAN_PIO_B, &keyboardScanPinSet_program);
        offset3 = pio_add_program(XOR_PIO, &xor_program);
        keyboardScanPio0Loaded = 1;
        debugEvent("scan_pio0_off", (int)offset);
    }

    if(!keyboardScanPio1Loaded)
    {
        debugEventText("scan", "pio1_load", 0);
        offset4 = pio_add_program(DATA_SEPARATION_PIO, &DataSeparation_program);
        offset5 = pio_add_program(ENCODER_A_PIO, &encoderA_program);
        keyboardScanPio1Loaded = 1;
        debugEvent("scan_pio1_off", (int)offset4);
    }
}

static void keyboardScanUnloadPio0Programs(void)
{
    if(!keyboardScanPio0Loaded) return;
    debugEventText("scan", "pio0_unload", 0);
    pio_remove_program(SCAN_PIO_A, &keyboardScan_program, offset);
    pio_remove_program(SCAN_PIO_B, &keyboardScanPinSet_program, offset2);
    pio_remove_program(XOR_PIO, &xor_program, offset3);
    keyboardScanPio0Loaded = 0;
}

static void keyboardScanStopHardware(void)
{
    debugEventText("scan", "stop_hw", 0);
    if(keyboardScanDmaClaimed)
    {
        dma_channel_set_irq1_enabled(Dma6Chan, false);
        dma_channel_abort(Dma0Chan);
        dma_channel_abort(Dma1Chan);
        dma_channel_abort(Dma2Chan);
        dma_channel_abort(Dma3Chan);
        dma_channel_abort(Dma4Chan);
        dma_channel_abort(Dma5Chan);
        dma_channel_abort(Dma6Chan);
    }

    pio_sm_set_enabled(SCAN_PIO_A, SCAN_PIO_SM_A, false);
    pio_sm_set_enabled(SCAN_PIO_B, SCAN_PIO_SM_B, false);
    pio_sm_set_enabled(XOR_PIO, XOR_PIO_SM, false);
    pio_sm_set_enabled(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, false);
    pio_sm_set_enabled(ENCODER_A_PIO, ENCODER_A_PIO_SM, false);
}

void keyboardScanStart(void)
{
    debugEventText("scan", "start", 0);
    keyboardScanLoadPrograms();

    keyboardScan_program_init(SCAN_PIO_A, SCAN_PIO_SM_A, offset, 13);
    
    keyboardScanPinSet_program_init(SCAN_PIO_B, SCAN_PIO_SM_B, offset2, 7); 

    xor_program_init(XOR_PIO, XOR_PIO_SM, offset3);

    DataSeparation_program_init(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, offset4);

    encoderA_program_init(ENCODER_A_PIO, ENCODER_A_PIO_SM, offset5);

    keyboardScanDmaInit( );
    pio_sm_set_enabled(SCAN_PIO_A, SCAN_PIO_SM_A, true); // 0 0 
    pio_sm_set_enabled(SCAN_PIO_B, SCAN_PIO_SM_B, true); // 0 1
    pio_sm_set_enabled(XOR_PIO, XOR_PIO_SM, true);       // 0 2
    pio_sm_set_enabled(DATA_SEPARATION_PIO, DATA_SEPARATION_PIO_SM, true);      //1 0      
    pio_sm_set_enabled(ENCODER_A_PIO, ENCODER_A_PIO_SM, true);                  //1 1                                                                                                                                                                  
    debugEventText("scan", "sm_enable", 0);
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

static unsigned short keyBounceDiagRowMask(unsigned char row, unsigned short value)
{
    if(row == 4) value &= (unsigned short)(~KEY_BOUNCE_DIAG_ROW4_IGNORE_MASK);
    return value;
}

static void keyBounceDiagDriveSelectedRow(unsigned char row)
{
    for(unsigned int i = 0; i < KEY_SCAN_ROW_COUNT; i++)
    {
        unsigned int pin = KEY_SCAN_ROW_PIN_BASE + i;
        pio_gpio_init(KEY_BOUNCE_DIAG_PIO, pin);
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
#if KEY_BOUNCE_DIAG_DRIVE_ALL_ROWS
        gpio_put(pin, 1);
#else
        gpio_put(pin, i == row ? 1 : 0);
#endif
    }
}

static void keyBounceDiagRestoreRowsLow(void)
{
    for(unsigned int i = 0; i < KEY_SCAN_ROW_COUNT; i++)
    {
        unsigned int pin = KEY_SCAN_ROW_PIN_BASE + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }
}

static unsigned int keyBounceDiagStableTarget(void)
{
    unsigned int sys = clock_get_hz(clk_sys);
    unsigned int loopCount = sys / 3u;
    if(loopCount == 0) loopCount = 1;
    return 0xffffffffu - loopCount;
}

static unsigned int keyBounceDiagTicksToUs(unsigned int ticks)
{
    unsigned int sys = clock_get_hz(clk_sys);
    if(sys == 0) return 0;
    unsigned long long cycles = (unsigned long long)ticks * 3ull;
    return (unsigned int)((cycles * 1000000ull) / sys);
}

static unsigned int keyBounceDiagIntervalTotalUs(volatile unsigned int *intervals, unsigned int count)
{
    unsigned int capped = count;
    unsigned long long total = 0;
    if(capped > KEY_BOUNCE_DIAG_INTERVAL_MAX) capped = KEY_BOUNCE_DIAG_INTERVAL_MAX;
    for(unsigned int i = 0; i < capped; i++) total += intervals[i];
    if(total > 0xffffffffull) return 0xffffffffu;
    return (unsigned int)total;
}

static unsigned int keyBounceDiagReadColsForRow(unsigned char row)
{
    unsigned int mask = 0;
    for(unsigned int i = 0; i < KEY_SCAN_COL_COUNT; i++)
    {
        if(gpio_get(KEY_SCAN_COL_PIN_BASE + i)) mask |= (1u << i);
    }
    if(row == 4) mask &= ~KEY_BOUNCE_DIAG_ROW4_IGNORE_MASK;
    return mask;
}

static unsigned char keyBounceDiagFirstSetCol(unsigned int mask)
{
    for(unsigned char i = 0; i < KEY_SCAN_COL_COUNT; i++)
    {
        if(mask & (1u << i)) return i;
    }
    return 0xff;
}

static void keyBounceDiagStopPioOnly(void)
{
    if(!keyBounceDiagPioLoaded) return;
    pio_sm_set_enabled(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM, false);
    pio_sm_clear_fifos(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM);
    pio_sm_restart(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM);
    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 0);
    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 1);
    if(keyBounceDiagPioMode == 2)
    {
        pio_remove_program(KEY_BOUNCE_DIAG_PIO, &key_bounce_diag_release_active_high_program, keyBounceDiagOffset);
    }
    else
    {
        pio_remove_program(KEY_BOUNCE_DIAG_PIO, &key_bounce_diag_press_active_high_program, keyBounceDiagOffset);
    }
    keyBounceDiagPioLoaded = 0;
    keyBounceDiagPioMode = 0;
}

static void keyBounceDiagLoadPioForCol(unsigned char col, unsigned char releaseMode)
{
    unsigned int colPin = KEY_SCAN_COL_PIN_BASE + col;
    keyBounceDiagColPin = colPin;
    gpio_init(colPin);
    gpio_set_dir(colPin, GPIO_IN);
    gpio_pull_down(colPin);
    debugEvent("bounce_pin", ((unsigned int)colPin << 8) | gpio_get(colPin));

    keyBounceDiagOffset = releaseMode ?
        pio_add_program(KEY_BOUNCE_DIAG_PIO, &key_bounce_diag_release_active_high_program) :
        pio_add_program(KEY_BOUNCE_DIAG_PIO, &key_bounce_diag_press_active_high_program);
    debugEvent("bounce_pio_off", (int)keyBounceDiagOffset);
    if(releaseMode)
    {
        key_bounce_diag_release_active_high_program_init(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM, keyBounceDiagOffset, colPin, KEY_BOUNCE_DIAG_CLKDIV);
    }
    else
    {
        key_bounce_diag_press_active_high_program_init(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM, keyBounceDiagOffset, colPin, KEY_BOUNCE_DIAG_CLKDIV);
    }
    pio_sm_clear_fifos(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM);
    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 0);
    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 1);
    pio_sm_put_blocking(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM, keyBounceDiagStableTarget());
    keyBounceDiagPioLoaded = 1;
    keyBounceDiagPioMode = releaseMode ? 2 : 1;
    pio_sm_set_enabled(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM, true);
    debugEventText("bounce", releaseMode ? "release_enabled" : "press_enabled", 0);
    keyBounceDiagLastTraceUs = to_us_since_boot(get_absolute_time());
}

static void keyBounceDiagLogResult(void)
{
    unsigned int logged = keyBounceDiagResult.edge_count;
    if(logged > KEY_BOUNCE_DIAG_INTERVAL_MAX) logged = KEY_BOUNCE_DIAG_INTERVAL_MAX;
    unsigned int diagCol = 0xffu;
    if(keyBounceDiagColPin != 0xffffffffu && keyBounceDiagColPin >= KEY_SCAN_COL_PIN_BASE)
    {
        diagCol = keyBounceDiagColPin - KEY_SCAN_COL_PIN_BASE;
    }

    debugEvent("bounce_result0",
               ((unsigned int)keyBounceDiagResult.row << 24) |
               ((unsigned int)keyBounceDiagResult.col << 16) |
               ((diagCol & 0xffu) << 8) |
               (keyBounceDiagResult.edge_count & 0xffu));
    debugEvent("bounce_result1", (int)keyBounceDiagResult.edge_count);
    debugEvent("bounce_result2", (int)keyBounceDiagResult.release_edge_count);
    debugEvent("bounce_press_total", (int)keyBounceDiagResult.press_total_us);
    debugEvent("bounce_release_total", (int)keyBounceDiagResult.release_total_us);

    for(unsigned int i = 0; i < logged; i += 4)
    {
        unsigned int a = keyBounceDiagResult.intervals[i];
        unsigned int b = (i + 1u < logged) ? keyBounceDiagResult.intervals[i + 1u] : 0u;
        unsigned int c = (i + 2u < logged) ? keyBounceDiagResult.intervals[i + 2u] : 0u;
        unsigned int d = (i + 3u < logged) ? keyBounceDiagResult.intervals[i + 3u] : 0u;
        debugEvent("bounce_us_idx", (int)i);
        debugEvent("bounce_us_a", (int)a);
        debugEvent("bounce_us_b", (int)b);
        debugEvent("bounce_us_c", (int)c);
        debugEvent("bounce_us_d", (int)d);
    }

    logged = keyBounceDiagResult.release_edge_count;
    if(logged > KEY_BOUNCE_DIAG_INTERVAL_MAX) logged = KEY_BOUNCE_DIAG_INTERVAL_MAX;
    for(unsigned int i = 0; i < logged; i += 4)
    {
        unsigned int a = keyBounceDiagResult.release_intervals[i];
        unsigned int b = (i + 1u < logged) ? keyBounceDiagResult.release_intervals[i + 1u] : 0u;
        unsigned int c = (i + 2u < logged) ? keyBounceDiagResult.release_intervals[i + 2u] : 0u;
        unsigned int d = (i + 3u < logged) ? keyBounceDiagResult.release_intervals[i + 3u] : 0u;
        debugEvent("release_us_idx", (int)i);
        debugEvent("release_us_a", (int)a);
        debugEvent("release_us_b", (int)b);
        debugEvent("release_us_c", (int)c);
        debugEvent("release_us_d", (int)d);
    }
}

static void keyBounceDiagResumeScan(void)
{
    debugEventText("bounce", "resume_scan", 0);
    keyboardScanStopHardware();
    if(keyBounceDiagPioLoaded)
    {
        keyBounceDiagStopPioOnly();
        debugEventText("bounce", "pio_removed", 0);
    }

    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 0);
    pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 1);
    keyBounceDiagRestoreRowsLow();
    keyboardScanStart();
    ws2812RestoreHardware();
}

static void keyBounceDiagStartCapture(unsigned char row, unsigned char col)
{
    debugEvent("bounce_start", ((unsigned int)row << 8) | col);
    keyBounceDiagReleaseStartUs = 0;
    keyBounceDiagColPin = 0xffffffffu;
    keyboardScanStopHardware();
    keyboardScanUnloadPio0Programs();
    memset((void *)dataBuff, 0, sizeof(dataBuff));
    keyBounceDiagDriveSelectedRow(row);

    for(unsigned int i = 0; i < KEY_SCAN_COL_COUNT; i++)
    {
        unsigned int pin = KEY_SCAN_COL_PIN_BASE + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    unsigned int cols = keyBounceDiagReadColsForRow(row);
    debugEvent("bounce_cols_start", (int)cols);
    if(cols != 0)
    {
        unsigned char actualCol = keyBounceDiagFirstSetCol(cols);
        if(actualCol != 0xff)
        {
            debugEvent("bounce_map", ((unsigned int)col << 8) | actualCol);
            col = actualCol;
            keyBounceDiagResult.col = actualCol;
        }
    }
    keyBounceDiagRestoreRowsLow();
    keyboardScanStart();
    debugEventText("bounce", "wait_release", 0);
}

static void keyBounceDiagArmCapture(void)
{
    debugEvent("bounce_arm", ((unsigned int)keyBounceDiagResult.row << 8) | keyBounceDiagResult.col);
    keyboardScanStopHardware();
    keyboardScanUnloadPio0Programs();
    memset((void *)dataBuff, 0, sizeof(dataBuff));
    keyBounceDiagDriveSelectedRow(keyBounceDiagResult.row);

    for(unsigned int i = 0; i < KEY_SCAN_COL_COUNT; i++)
    {
        unsigned int pin = KEY_SCAN_COL_PIN_BASE + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    keyBounceDiagResult.state = KEY_BOUNCE_DIAG_WAIT_PRESS;
    keyBounceDiagLoadPioForCol(keyBounceDiagResult.col, 0);
}

static void keyBounceDiagSelectPoll(void)
{
    unsigned short *raw = getDataBuff();
    for(unsigned char row = 0; row < KEY_SCAN_ROW_COUNT; row++)
    {
        unsigned short now = keyBounceDiagRowMask(row, raw[row]);
        unsigned short base = keyBounceDiagRowMask(row, keyBounceDiagBaseline[row]);
        unsigned short changedDown = now & (unsigned short)(~base);
        if(changedDown == 0) continue;

        for(unsigned char col = 0; col < KEY_SCAN_COL_COUNT; col++)
        {
            if((changedDown & (1u << col)) == 0) continue;
            keyBounceDiagResult.row = row;
            keyBounceDiagResult.col = col;
            keyBounceDiagSelectCol = col;
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_WAIT_RELEASE;
            debugEvent("bounce_select", ((unsigned int)row << 8) | col);
            keyBounceDiagStartCapture(row, col);
            keyBounceDiagLastPollUs = to_us_since_boot(get_absolute_time());
            return;
        }
    }
}

static void keyBounceDiagDonePoll(void)
{
    unsigned short *raw = getDataBuff();
    for(unsigned char row = 0; row < KEY_SCAN_ROW_COUNT; row++)
    {
        unsigned short now = keyBounceDiagRowMask(row, raw[row]);
        unsigned short base = keyBounceDiagRowMask(row, keyBounceDiagBaseline[row]);
        unsigned short changedDown = now & (unsigned short)(~base);
        if(changedDown == 0) continue;

        for(unsigned char col = 0; col < KEY_SCAN_COL_COUNT; col++)
        {
            if((changedDown & (1u << col)) == 0) continue;
            debugEvent("bounce_next", ((unsigned int)row << 8) | col);
            memset((void *)&keyBounceDiagResult, 0, sizeof(keyBounceDiagResult));
            keyBounceDiagResult.row = row;
            keyBounceDiagResult.col = col;
            keyBounceDiagSelectCol = col;
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_WAIT_RELEASE;
            keyBounceDiagDoneContinue = 0;
            keyBounceDiagStartCapture(row, col);
            keyBounceDiagLastPollUs = to_us_since_boot(get_absolute_time());
            return;
        }
    }
}

void keyBounceDiagBegin(void)
{
    unsigned short *raw = getDataBuff();
    debugEventText("bounce", "begin", 0);
    keyBounceDiagCancelPending = 0;
    memset((void *)&keyBounceDiagResult, 0, sizeof(keyBounceDiagResult));
    keyBounceDiagResult.state = KEY_BOUNCE_DIAG_SELECT;
    keyBounceDiagResult.row = 0xff;
    keyBounceDiagResult.col = 0xff;
    keyBounceDiagColPin = 0xffffffffu;
    keyBounceDiagLastTraceUs = to_us_since_boot(get_absolute_time());
    keyBounceDiagReleaseStartUs = 0;
    keyBounceDiagReleasePhase = 0;
    keyBounceDiagSelectCol = 0xff;
    keyBounceDiagDoneContinue = 0;
    memcpy(keyBounceDiagBaseline, raw, sizeof(keyBounceDiagBaseline));
}

void keyBounceDiagCancel(void)
{
    debugEvent("bounce_cancel_req", (int)keyBounceDiagResult.state);
    keyBounceDiagCancelPending = 1;
}

void keyBounceDiagPoll(void)
{
    if(keyBounceDiagCancelPending)
    {
        keyBounceDiagCancelPending = 0;
        debugEvent("bounce_cancel_do", (int)keyBounceDiagResult.state);
        keyBounceDiagResumeScan();
        keyBounceDiagResult.state = KEY_BOUNCE_DIAG_IDLE;
        keyBounceDiagColPin = 0xffffffffu;
        keyBounceDiagReleaseStartUs = 0;
        keyBounceDiagReleasePhase = 0;
        keyBounceDiagSelectCol = 0xff;
        keyBounceDiagDoneContinue = 0;
        return;
    }

    key_bounce_diag_state_t state = keyBounceDiagResult.state;
    if(state == KEY_BOUNCE_DIAG_DONE && keyBounceDiagDoneContinue)
    {
        keyBounceDiagDonePoll();
        return;
    }
    if(state == KEY_BOUNCE_DIAG_SELECT)
    {
        keyBounceDiagSelectPoll();
        return;
    }
    if(state == KEY_BOUNCE_DIAG_WAIT_RELEASE)
    {
        unsigned int nowUs = to_us_since_boot(get_absolute_time());
        unsigned short *raw = getDataBuff();
        unsigned char released = 0;
        if(keyBounceDiagResult.row < KEY_SCAN_ROW_COUNT && keyBounceDiagSelectCol < KEY_SCAN_COL_COUNT)
        {
            unsigned short rowValue = keyBounceDiagRowMask(keyBounceDiagResult.row, raw[keyBounceDiagResult.row]);
            released = ((rowValue & (1u << keyBounceDiagSelectCol)) == 0) ? 1 : 0;
        }
        if((unsigned int)(nowUs - keyBounceDiagLastTraceUs) >= 300000u)
        {
            keyBounceDiagLastTraceUs = nowUs;
            debugEvent("bounce_release_raw", released);
        }

        if(released)
        {
            if(keyBounceDiagReleaseStartUs == 0)
            {
                keyBounceDiagReleaseStartUs = nowUs;
                debugEventText("bounce", "release_zero", 0);
            }
            else if((unsigned int)(nowUs - keyBounceDiagReleaseStartUs) >= KEY_BOUNCE_DIAG_RELEASE_STABLE_US)
            {
                debugEventText("bounce", "armed", 0);
                keyBounceDiagArmCapture();
                keyBounceDiagLastPollUs = nowUs;
            }
        }
        else
        {
            if(keyBounceDiagReleaseStartUs != 0) debugEventText("bounce", "release_reset", 0);
            keyBounceDiagReleaseStartUs = 0;
        }
        return;
    }
    if(state != KEY_BOUNCE_DIAG_WAIT_PRESS && state != KEY_BOUNCE_DIAG_CAPTURE &&
       state != KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE && state != KEY_BOUNCE_DIAG_RELEASE_CAPTURE) return;
    if(!keyBounceDiagPioLoaded) return;

    unsigned int nowUs = to_us_since_boot(get_absolute_time());
    if(keyBounceDiagColPin != 0xffffffffu && (unsigned int)(nowUs - keyBounceDiagLastTraceUs) >= 1000000u)
    {
        keyBounceDiagLastTraceUs = nowUs;
        unsigned int cols = keyBounceDiagReadColsForRow(keyBounceDiagResult.row);
        debugEvent("bounce_cols", (int)cols);
        debugEvent("bounce_trace", ((unsigned int)state << 24) |
                                  ((unsigned int)(gpio_get(keyBounceDiagColPin) & 1u) << 16) |
                                  (keyBounceDiagResult.edge_count & 0xffffu));
        if(state == KEY_BOUNCE_DIAG_WAIT_PRESS && cols != 0)
        {
            unsigned char actualCol = keyBounceDiagFirstSetCol(cols);
            if(actualCol != 0xff && actualCol != keyBounceDiagResult.col)
            {
                debugEvent("bounce_rebind", ((unsigned int)keyBounceDiagResult.col << 8) | actualCol);
                keyBounceDiagStopPioOnly();
                keyBounceDiagResult.col = actualCol;
                keyBounceDiagLoadPioForCol(actualCol, 0);
            }
        }
    }

    while(!pio_sm_is_rx_fifo_empty(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM))
    {
        unsigned int x = pio_sm_get(KEY_BOUNCE_DIAG_PIO, KEY_BOUNCE_DIAG_SM);
        unsigned int ticks = 0xffffffffu - x;
        if(keyBounceDiagPioMode == 2)
        {
            unsigned int edge = keyBounceDiagResult.release_edge_count;
            if(edge < KEY_BOUNCE_DIAG_INTERVAL_MAX) keyBounceDiagResult.release_intervals[edge] = keyBounceDiagTicksToUs(ticks);
            keyBounceDiagResult.release_edge_count = edge + 1;
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_RELEASE_CAPTURE;
            keyBounceDiagLastPollUs = to_us_since_boot(get_absolute_time());
            if(keyBounceDiagResult.release_edge_count <= KEY_BOUNCE_DIAG_INTERVAL_MAX) debugEvent("release_edge", (int)keyBounceDiagResult.release_edge_count);
        }
        else
        {
            unsigned int edge = keyBounceDiagResult.edge_count;
            if(edge < KEY_BOUNCE_DIAG_INTERVAL_MAX) keyBounceDiagResult.intervals[edge] = keyBounceDiagTicksToUs(ticks);
            keyBounceDiagResult.edge_count = edge + 1;
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_CAPTURE;
            keyBounceDiagLastPollUs = to_us_since_boot(get_absolute_time());
            if(keyBounceDiagResult.edge_count <= KEY_BOUNCE_DIAG_INTERVAL_MAX) debugEvent("bounce_edge", (int)keyBounceDiagResult.edge_count);
        }
    }

    if(pio_interrupt_get(KEY_BOUNCE_DIAG_PIO, 0))
    {
        pio_interrupt_clear(KEY_BOUNCE_DIAG_PIO, 0);
        if(keyBounceDiagPioMode == 1)
        {
            keyBounceDiagReleasePhase = 1;
            keyBounceDiagResult.press_total_us = keyBounceDiagIntervalTotalUs(keyBounceDiagResult.intervals, keyBounceDiagResult.edge_count);
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE;
            debugEvent("bounce_irq_press", (int)keyBounceDiagResult.edge_count);
            keyBounceDiagStopPioOnly();
            keyBounceDiagLoadPioForCol(keyBounceDiagResult.col, 1);
        }
        else if(keyBounceDiagPioMode == 2)
        {
            debugEvent("bounce_done", (int)keyBounceDiagResult.release_edge_count);
            keyBounceDiagResult.stable_us = KEY_BOUNCE_DIAG_STABLE_US;
            keyBounceDiagResult.release_total_us = keyBounceDiagIntervalTotalUs(keyBounceDiagResult.release_intervals, keyBounceDiagResult.release_edge_count);
            keyBounceDiagResult.state = KEY_BOUNCE_DIAG_DONE;
            keyBounceDiagLogResult();
            keyBounceDiagResumeScan();
            memcpy(keyBounceDiagBaseline, getDataBuff(), sizeof(keyBounceDiagBaseline));
            keyBounceDiagDoneContinue = 1;
        }
    }
}

key_bounce_diag_result_t keyBounceDiagGetResult(void)
{
    return keyBounceDiagResult;
}

const char *keyBounceDiagStateText(key_bounce_diag_state_t state)
{
    switch(state)
    {
        case KEY_BOUNCE_DIAG_IDLE: return "READY";
        case KEY_BOUNCE_DIAG_SELECT: return "SELECT";
        case KEY_BOUNCE_DIAG_WAIT_RELEASE: return "RELEASE";
        case KEY_BOUNCE_DIAG_WAIT_PRESS: return "PRESS";
        case KEY_BOUNCE_DIAG_CAPTURE: return "CAPTURE";
        case KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE: return "RELKEY";
        case KEY_BOUNCE_DIAG_RELEASE_CAPTURE: return "RELCAP";
        case KEY_BOUNCE_DIAG_DONE: return "DONE";
        case KEY_BOUNCE_DIAG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
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
