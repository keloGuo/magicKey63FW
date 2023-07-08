#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "keyboardscan.pio.h"
#include "hardware/dma.h"
#include "keyboardScan.h"

//一个DMA 传输数据，完成后触发计数DMA，
//计数DMA 完成后触发控制dma，每次写NULL，地址后后移4字节，末尾1个4字节是数据传输地址，
//控制DMA，控制数据传输的DMA，数据传输地址，写到对应的寄存器，并开始传输。如果地址被NULL覆盖，则自动停止

#define PIO_BUFF_COUNT  2      // 2组缓存（乒乓） 必须2
#define PIO_BUFF_LEN    20      // 每组 20 ，4次全键盘扫描 
unsigned short dmaBuff[PIO_BUFF_LEN * PIO_BUFF_COUNT] = {0};

typedef struct
{
    uint32_t dataCountFlag;                                 //NULL,计数的时候把这个往后面搬                                      
    uint32_t FillData[4 - ((PIO_BUFF_COUNT) % 4)];          //占位子的，为了让dmaBuffAddr 16字节对齐。（需要用到dma的ring功能）  
    uint32_t dataCountBuff[PIO_BUFF_COUNT-1];               //数据计数buff，非NULL的时候，没有数据，NULL的时候有数据。           
    unsigned short *dmaBuffAddr[PIO_BUFF_COUNT];		    //data DMA的写地址，如果计数来不及清空，就会写到这里，这里被写NULL，DMA自动停止， 
}  sDmaData;

sDmaData dataCount __attribute__((aligned(16))) = {(uint32_t)NULL,{0},{1},{dmaBuff,dmaBuff+PIO_BUFF_LEN}}; //__attribute__((aligned(16))) 确保整个结构体是16字节对齐

//int a ;

static int data_chan = 0;
static int ctrl_chan = 0;
static int count_chan = 0;

unsigned char keyboardScanDmaInit(PIO pio, uint sm, unsigned short *capture_buf)
{
    ctrl_chan = dma_claim_unused_channel(true);                                             //dma 控制通道                     
    data_chan = dma_claim_unused_channel(true);                                             //dma 数据通道
    count_chan = dma_claim_unused_channel(true);                                            //dma 数据通道
    
    dma_channel_config   c = dma_channel_get_default_config(data_chan);                     //数据通道DMA
    channel_config_set_read_increment(&c, false);                                           //读地址不用变，PIO FIFO 地址
    channel_config_set_write_increment(&c, true);                                           //写地址自加
    channel_config_set_transfer_data_size(&c,DMA_SIZE_16);                                  //每次2个字节
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));                              //设置触发
    channel_config_set_chain_to(&c, count_chan);                                            //传输完成触发计数通道DMA
    channel_config_set_irq_quiet(&c, true);                                                 //写入NULL的时候，停止。
    // Destination pointer // Source pointer // Number of transfers // Start immediately
    dma_channel_configure(data_chan, &c,capture_buf,&pio->rxf[sm],PIO_BUFF_LEN,false);      //计数DMA

    c = dma_channel_get_default_config(count_chan);                                         //计数通道
    channel_config_set_read_increment(&c, false);                                           //读地址不用变
    channel_config_set_write_increment(&c, true);                                           //写地址自加
    channel_config_set_transfer_data_size(&c,DMA_SIZE_32);                                  //每次4个字节
    channel_config_set_chain_to(&c, ctrl_chan);                                             //传输完成触发控制通道DMA
    // Destination pointer // Source pointer // Number of transfers // Start immediately
    dma_channel_configure(count_chan, &c,dataCount.dataCountBuff,&dataCount.dataCountFlag,1,false);      //计数DMA

    c = dma_channel_get_default_config(ctrl_chan);           
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);  
    channel_config_set_ring(&c, false, 3);
    // Initial write address // Initial read address // Halt after each control block // Don't start yet
    dma_channel_configure(ctrl_chan, &c, &dma_hw->ch[data_chan].al2_write_addr_trig, dataCount.dmaBuffAddr,1,false); //只修改了地址。

    dma_start_channel_mask(1u << ctrl_chan);

    return 0;
}


//开始扫描键盘，配置PIO，DMA
void keyboardScanStart(uint sm, uint pin, unsigned short *capture_buf)
{    
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &keyboardScan_program);
    keyboardScan_program_init(pio, sm, offset, pin);
    //pio_sm_set_clkdiv_int_frac(pio,sm,31,0x40);
    pio_sm_set_clkdiv_int_frac(pio,sm,6250,0); // 100us 一行。0.5ms一个键盘 4个全键盘处理一次， 500 * 80 40000hz ，整个键盘的扫描频率是500hz
    keyboardScanDmaInit(pio,sm,capture_buf);
    pio_sm_set_enabled(pio, sm, true);
}
//500k 每秒字节，可以用DMA ，但是CPU处理不过来啊

unsigned char keyboardScanBuffErroHandle(void)
{
    if((void *)(dma_channel_hw_addr(data_chan)->write_addr) != NULL) return 0;  //肯定是停了

    printf("temp keyboardScanBuffErroHandle stop\r\n");

    dataCount.dmaBuffAddr[0] = dmaBuff;
    dataCount.dmaBuffAddr[1] = dmaBuff+PIO_BUFF_LEN;
    dataCount.dataCountBuff[0] = 1;

    dma_channel_set_write_addr(count_chan,dataCount.dataCountBuff,false); //这里还需要重新启动 pio，不然位置对不上
    dma_start_channel_mask(1u << ctrl_chan);    

    return 1;
}



//4M 4000000
/*
大前提：
    1. 只有松开需要滤波，按下的时候，有就触发。
    2. 滤波数据延后一个周期。

    旧值（按下0，松开1） old
    新值（按下0，松开1）new
    4个缓存里面的值，全与，是1的，有效松开  filter //滤波值里面找松开的
    这里的1 才是真的1

    temp = filter & new; //新值是松开的，滤波值也是送开的，结果就是松开的

    有改变的值 change = temp ^  old
    changeUp = temp & change;
    changeDown = (~temp) & change;
    old = temp;


    新值备份到缓存里，用于软件滤波，
*/
//键盘扫描数据处理，每次传一整个键盘的扫描数据进来
//传入的数据是，空闲是1，按下是0，
//找出变换的值
static unsigned short keyboardScanDataOld[5] = {0xffff,0xffff,0xffff,0xffff,0xffff};          //之前的状态
//缓存最近4次的数据，用DMA操作，用ring功能，是自动环形缓冲。数组长度是20就够，但是ring功能需要地址对齐
static unsigned short keyboardScanDataTemp[32]  __attribute__((aligned(32))) = {0xffff}; 

//初始化键盘扫描
unsigned char keyboardScanInit(void)
{
    //printf("keyboardScanInit =%x %x %x %x %x\r\n", &dataCount.dataCountFlag,&(dataCount.FillData[0]),&(dataCount.FillData[1]),&(dataCount.dmaBuffAddr[0]),&(dataCount.dmaBuffAddr[1]));
    keyboardScanStart(0, 7,dmaBuff);
    memset((char *)keyboardScanDataTemp , 0xff,64);
    return 0;
}

void KeyMapPrintf(unsigned short *data)
{
    for(int i=0;i<5;i++)
    {
        printf("%04x ",data[i]);
    }
    printf("\r\n");
}

unsigned short *getkeyMatrixData(void)
{
    return keyboardScanDataOld;
}
unsigned char encoderScan(unsigned short a,unsigned short b);
unsigned char encoderKeyScan(unsigned char newStete);

unsigned char keyboardScanDataHandleOne(unsigned short *data)
{
//    KeyMapPrintf(data);
    for(int i =0; i < 5; i++)
    {
        unsigned short filter = keyboardScanDataTemp[i] & keyboardScanDataTemp[i + 8] & keyboardScanDataTemp[i + 16]  & keyboardScanDataTemp[i + 24]; //新的松开数据，软件滤波结果
        unsigned short temp = filter & data[i]; //新的值是松开，滤波值也是松开的，才是松开的
//        unsigned short change = temp ^ keyboardScanDataOld[i];  //其实我们可能不用关心是那个有变化，只要有变化就把新值同步给PC就就好，                 
        keyboardScanDataOld[i] = temp;      //更新旧值
        
        // for(int j = 0;j<16;j++) //debug 用的
        // {
        //     if(change & (0x0001 << j)) 
        //     {
        //         printf("key %02d %02d %c\r\n",i,j ,temp & (0x0001 << j) ? 'u' : 'd');
        //     }
        // }
    } 

    //(keyboardScanDataOld[1] & 0x4000) == 0  下 这里需要优先处理按键。
    encoderKeyScan((keyboardScanDataOld[1] & 0x4000) != 0);   //处理编码器 按键
    encoderScan(keyboardScanDataOld[0]&0x4000,keyboardScanDataOld[2]&0x4000);                   //处理编码器 旋钮 ab相
    {
        static unsigned char count = 0;
        memcpy((char *)(keyboardScanDataTemp + (count *8)),(char *)data,10);
        count ++;
        count = count % 4;
    }

    return 0;
}

unsigned char dmaHandle(void) 
{
    // unsigned int writeAddrTemp =  dma_channel_hw_addr(data_chan)->write_addr;
    // printf("writeAddrTemp = %x %x %x %x %d \r\n",writeAddrTemp, dataCount.dataCountBuff[0],dataCount.dmaBuffAddr[0],dataCount.dmaBuffAddr[1],dma_channel_is_busy(data_chan));
    if(keyboardScanBuffErroHandle()) return 0; //buff 不够用了

    if(dataCount.dataCountBuff[0] == (uint32_t)NULL) 
    {
        dma_channel_set_write_addr(count_chan,dataCount.dataCountBuff,false);
        dataCount.dataCountBuff[0] = 1;
        keyboardScanDataHandleOne(((dma_channel_hw_addr(data_chan)->write_addr) > (uint32_t)(dmaBuff + PIO_BUFF_LEN))?(dmaBuff + PIO_BUFF_LEN):(dmaBuff));

    }
    return 0;
}

