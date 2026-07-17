#include "tusb.h"

unsigned char tempFlag = 0;
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    printf("itf %d ,report_id %d report_type %d , bufferlen = %d \n",itf,report_id,report_type,bufsize);
    unsigned char __not_in_flash_func(updateVirtuallyLed)(unsigned char data);
  	if(report_id == 0 && report_type == 2) updateVirtuallyLed(buffer[0]);
    tempFlag = 1;
}
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	return 0;
}

//根据新的矩阵数据，生成report数据，通过USB发送给PC

unsigned short *getDataBuff(void);
unsigned short getKeymapByXY(unsigned char x,unsigned char y);
unsigned char layerChangeKeyHandle(unsigned char k,unsigned char layer);
unsigned char __not_in_flash_func(getEncoderValue)(void);
unsigned char getlayerTempNumber(void);

unsigned int sendBuffMedia = 0;
unsigned int sendBuffMediaNew = 0;

unsigned int sendBuffMouse = 0;
unsigned int sendBuffMouseNew = 0;
extern unsigned short encoderAData;
void userPrintf(const char* format, ...);
bool keyMatrix2ReportData(repeating_timer_t *rt)
{
    if ( !tud_hid_ready() ) return true;
	static unsigned char init = 1;
    unsigned char temp[] = {0x00,0x00,0x00,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned short data[5] =        {0,0,0,0,0};
    static unsigned short keyDataBack[5] = {0,0,0,0,0};
    unsigned short *keyData = getDataBuff();
    unsigned char jmp = 0;
    sendBuffMediaNew = 0;
    sendBuffMouseNew = 0;
    memcpy(data,keyData,10);

    data[4] &= (~0x8160);
    data[4] = data[4]|(encoderAData);
    encoderAData = 0;
    
    for(int i = 0;i< 5;i++)
    {
        for(int j = 0; j<16;j++)
        {
            unsigned short t = getKeymapByXY(i,j);
            
            if((t >> 8) == 0x04) //层切换                                   //是不是层切换按键
            {   
                unsigned char tempLayer = layerChangeKeyHandle(((data[i]) & (0x0001 << j))!=0,(t&0xff));
                if(tempLayer == 1) //切换到临时层，就备份数据，然后进来就把备份的位去掉
                {
                    memcpy(keyDataBack,data,10);
                    // userPrintf("memcpy %04x %04x %04x %04x %04x  \n",keyDataBack[0],keyDataBack[1],keyDataBack[2],keyDataBack[3],keyDataBack[4] );
                    jmp = 1;
                }
                else if(tempLayer == 2) //刚刚从临时层切换回来
                {
                    memset(keyDataBack,0x00,10);
                    // userPrintf("memset %04x %04x %04x %04x %04x  \n",keyDataBack[0],keyDataBack[1],keyDataBack[2],keyDataBack[3],keyDataBack[4] );
                    jmp = 2;
                }
            }
            else if((t >> 8) == 0x02)   //多媒体键                          //多媒体按键
            {
                if((data[i]) & (0x0001 << j))
                {
                    t = t & 0xff;
                    if(t > 32) continue;
                    sendBuffMediaNew = (1 << t);
                }
            }
            else if((t >> 8) == 0x01)   //鼠标
            {
                if((data[i]) & (0x0001 << j))
                {
                    t = t & 0xff;
                    if(t < 3)
                    {
                        sendBuffMouseNew |= (1 << t);
                    } 
                    else
                    {
                       sendBuffMouseNew |=  (t == 3)?(1 << 24):(0xff << 24);
                    }
                }
            }
            else
            {   
                
                if(((keyDataBack[i]) & (0x0001 << j)) == 0)
                {
                    if((data[i]) & (0x0001 << j))
                    {
                        if(t>>8) continue;
                        temp[t/8] |= (1 << (t%8));
                    }
                }
            }
        }
    }
    if((getlayerTempNumber() !=0xff )&& (jmp == 0))   tud_hid_n_report(0,0,temp,16);
    if(jmp == 2 || init) 
    {
        memset(temp,0,16);
        tud_hid_n_report(0,0,temp,16);
    }


    jmp = 0;
    if(sendBuffMediaNew != sendBuffMedia || init)
    {
        tud_hid_n_report(1,0,(unsigned char *)(&sendBuffMediaNew),4);
        sendBuffMedia = sendBuffMediaNew;
    }
    
    if(sendBuffMouseNew != sendBuffMouse || init)
    {
        tud_hid_n_report(2,0,(unsigned char *)(&sendBuffMouseNew),4);
        sendBuffMouse = sendBuffMouseNew;
    }
    
    init = 0;

    return true ;
}

bool keyboardReportLayerTimerHandle (repeating_timer_t *rt)
{
    if ( !tud_hid_ready() ) return true;
    if(getlayerTempNumber()!=0xff) return true;
    tud_hid_n_report(3,0,getDataBuff(),10);//
    return true ;
}
unsigned char getFlashRateInfo(void);
struct repeating_timer keyboardReportTimer;
struct repeating_timer keyboardReportTimerMouse;
struct repeating_timer keyboardReportLayerTimer;
unsigned char keyboardReportInit(void)
{
    unsigned char Interval = getFlashRateInfo() ;
    if(Interval > 20 ) Interval = 20;
    add_repeating_timer_ms(Interval,keyboardReportLayerTimerHandle,0,&keyboardReportLayerTimer); //第一层直接发
    add_repeating_timer_ms(10,keyMatrix2ReportData,0,&keyboardReportTimer);               //临时层

    // unsigned char temp[16] = {0};
    // memset(temp,0,16);
    // tud_hid_n_report(0,0,temp,16);
    // tud_hid_n_report(1,0,temp,4);
    // tud_hid_n_report(2,0,temp,4);
    // tud_hid_n_report(3,0,temp,10);//
    return 0;
}
