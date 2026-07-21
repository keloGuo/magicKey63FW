#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "tusb.h"
#include "lfs.h"
#include "macroRecorder.h"

#define MACRO_MAGIC 0x4d414331u
#define MACRO_VERSION 1u
#define MACRO_MAX_ID 255u
#define MACRO_MAX_ACTIONS 64u
#define MACRO_FILE_PREFIX "macro_"
#define MACRO_TICK_MS 10u

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t action_count;
    uint16_t id;
    uint8_t mode;
    uint8_t reserved;
    uint32_t crc32;
} macro_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t value;
    uint16_t param;
} macro_action_t;

typedef struct __attribute__((packed)) {
    macro_entry_t header;
    macro_action_t actions[MACRO_MAX_ACTIONS];
} macro_data_t;

lfs_t *fsInit(void);
unsigned short getKeymapByXY(unsigned char x,unsigned char y);
void ws2812MacroPlayStart(void);
void ws2812MacroPlayStop(void);
void ws2812MacroPlayKey(unsigned char ledIndex, unsigned char pressed);
void debugStage(unsigned char core, unsigned int stage);
void debugEvent(const char *tag, int value);
void debugEventText(const char *tag, const char *text, int value);
static unsigned char macroLedByXY(unsigned char x, unsigned char y);

static macro_data_t macroActive;
static unsigned char macroRunning = 0;
static unsigned char macroSourceId = 0;
static unsigned char macroSourceDown = 0;
static unsigned char macroIndex = 0;
static unsigned int macroDelayMs = 0;
static unsigned char macroKeyboardBits[16] = {0};
static unsigned int macroMediaBits = 0;
static unsigned int macroMouseBits = 0;
static unsigned char macroTapActive = 0;
static unsigned char macroTapType = 0;
static unsigned char macroTapValue = 0;

unsigned char macroPlayerIsRunning(void)
{
    return macroRunning;
}

unsigned char macroPlayerGetId(void)
{
    return macroSourceId;
}

unsigned char macroPlayerGetIndex(void)
{
    unsigned int total = macroActive.header.action_count;
    return macroIndex > total ? total : macroIndex;
}

unsigned char macroPlayerGetTotal(void)
{
    return macroActive.header.action_count;
}

unsigned int macroPlayerGetDelayMs(void)
{
    return macroDelayMs;
}

unsigned char macroPlayerGetMode(void)
{
    return macroActive.header.mode;
}

static void macroFileName(unsigned int id, char *out, size_t outLen)
{
    snprintf(out, outLen, MACRO_FILE_PREFIX "%u", id);
}

static void macroClearOutputs(void)
{
    memset(macroKeyboardBits, 0, sizeof(macroKeyboardBits));
    macroMediaBits = 0;
    macroMouseBits = 0;
    macroTapActive = 0;
    macroTapType = 0;
    macroTapValue = 0;
}

static int macroLoadOne(unsigned int id, macro_data_t *macro)
{
    char name[24] = {0};
    lfs_file_t file;
    lfs_t *fs = fsInit();
    macroFileName(id, name, sizeof(name));
    debugStage(1, 22);
    int err = lfs_file_open(fs, &file, name, LFS_O_RDONLY);
    if(err < 0)
    {
        debugEvent("macro_load_open_fail", err);
        return -1;
    }
    lfs_ssize_t len = lfs_file_read(fs, &file, macro, sizeof(*macro));
    lfs_file_close(fs, &file);
    if(len != sizeof(*macro)) debugEvent("macro_load_read_fail", (int)len);
    if(len != sizeof(*macro)) return -1;
    if(macro->header.magic != MACRO_MAGIC ||
       macro->header.version != MACRO_VERSION ||
       macro->header.id != id ||
       macro->header.action_count > MACRO_MAX_ACTIONS)
    {
        return -1;
    }
    return 0;
}

static int macroKeyboardBitIndex(unsigned char usage)
{
    if(usage >= 0xe0 && usage <= 0xe7) return usage - 0xe0;
    if(usage <= 0x77) return usage + 8;
    return -1;
}

static void macroSetKeyboardUsage(unsigned char usage, unsigned char down)
{
    int bit = macroKeyboardBitIndex(usage);
    if(bit < 0 || bit >= 128) return;
    if(down) macroKeyboardBits[bit / 8] |= (1u << (bit % 8));
    else macroKeyboardBits[bit / 8] &= ~(1u << (bit % 8));
}

static void macroSetMouseValue(unsigned char value, unsigned char down)
{
    unsigned int mask = 0;
    if(value < 3) mask = (1u << value);
    else mask = (value == 3) ? (1u << 24) : (0xffu << 24);
    if(down) macroMouseBits |= mask;
    else macroMouseBits &= ~mask;
}

static unsigned char macroUsageFromKeyValue(unsigned short keyValue)
{
    unsigned char value = keyValue & 0xff;
    if((keyValue >> 8) != 0) return 0xff;
    if(value >= 8) return value - 8;
    return 0xe0 + value;
}

static unsigned char macroLedByUsage(unsigned char usage)
{
    for(unsigned char x = 0; x < 5; x++)
    {
        for(unsigned char y = 0; y < 16; y++)
        {
            unsigned short keyValue = getKeymapByXY(x, y);
            if(macroUsageFromKeyValue(keyValue) == usage)
            {
                return macroLedByXY(x, y);
            }
        }
    }
    return 64;
}

static void macroShowKeyboardAction(unsigned char usage, unsigned char down)
{
    ws2812MacroPlayKey(macroLedByUsage(usage), down);
}

static void macroStart(unsigned char id)
{
    if(id == 0 || id > MACRO_MAX_ID) return;
    if(macroLoadOne(id, &macroActive) < 0) return;
    if(macroActive.header.action_count == 0) return;
    macroClearOutputs();
    macroRunning = 1;
    macroSourceId = id;
    macroSourceDown = 1;
    macroIndex = 0;
    macroDelayMs = 0;
    ws2812MacroPlayStart();
}

static void macroStop(void)
{
    ws2812MacroPlayStop();
    macroRunning = 0;
    macroSourceId = 0;
    macroSourceDown = 0;
    macroIndex = 0;
    macroDelayMs = 0;
    macroClearOutputs();
}

static void macroClearTap(void)
{
    if(macroTapType == 3)
    {
        macroSetKeyboardUsage(macroTapValue, 0);
        macroShowKeyboardAction(macroTapValue, 0);
    }
    else if(macroTapType == 5 && macroTapValue < 32) macroMediaBits &= ~(1u << macroTapValue);
    else if(macroTapType == 6) macroSetMouseValue(macroTapValue, 0);
    macroTapActive = 0;
    macroTapType = 0;
    macroTapValue = 0;
}

static void macroProcessStep(void)
{
    if(!macroRunning) return;

    if(macroTapActive)
    {
        macroClearTap();
        return;
    }

    if(macroDelayMs > 0)
    {
        if(macroDelayMs > MACRO_TICK_MS) macroDelayMs -= MACRO_TICK_MS;
        else macroDelayMs = 0;
        return;
    }

    for(unsigned int guard = 0; guard < MACRO_MAX_ACTIONS; guard++)
    {
        if(macroIndex >= macroActive.header.action_count)
        {
            if(macroActive.header.mode == 1 && macroSourceDown)
            {
                macroClearOutputs();
                macroIndex = 0;
            }
            else
            {
                macroStop();
                return;
            }
        }

        macro_action_t *action = &macroActive.actions[macroIndex++];
        if(action->type == 1)
        {
            macroSetKeyboardUsage(action->value, 1);
            macroShowKeyboardAction(action->value, 1);
        }
        else if(action->type == 2)
        {
            macroSetKeyboardUsage(action->value, 0);
            macroShowKeyboardAction(action->value, 0);
        }
        else if(action->type == 3)
        {
            macroSetKeyboardUsage(action->value, 1);
            macroShowKeyboardAction(action->value, 1);
            macroTapActive = 1;
            macroTapType = action->type;
            macroTapValue = action->value;
            return;
        }
        else if(action->type == 4)
        {
            macroDelayMs = action->param;
            return;
        }
        else if(action->type == 5)
        {
            ws2812MacroPlayKey(64, 0);
            if(action->value < 32)
            {
                macroMediaBits |= (1u << action->value);
                macroTapActive = 1;
                macroTapType = action->type;
                macroTapValue = action->value;
                return;
            }
        }
        else if(action->type == 6)
        {
            ws2812MacroPlayKey(64, 0);
            macroSetMouseValue(action->value, 1);
            macroTapActive = 1;
            macroTapType = action->type;
            macroTapValue = action->value;
            return;
        }
    }
}

static void macroApplyReports(unsigned char *keyboardReport, unsigned int *mediaReport, unsigned int *mouseReport)
{
    macroProcessStep();
    for(int i = 0; i < 16; i++) keyboardReport[i] |= macroKeyboardBits[i];
    *mediaReport |= macroMediaBits;
    *mouseReport |= macroMouseBits;
}

static unsigned char macroKeyboardOutputActive(void)
{
    for(int i = 0; i < 16; i++)
    {
        if(macroKeyboardBits[i] != 0) return 1;
    }
    return 0;
}

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
static volatile unsigned char keyboardReportPaused = 0;
static unsigned char keyboardReportPauseReleaseSent = 0;

void keyboardReportSetPaused(unsigned char paused)
{
    unsigned char next = paused ? 1 : 0;
    if(keyboardReportPaused == next) return;
    keyboardReportPaused = next;
    debugEvent("hid_pause", next);
    if(paused) keyboardReportPauseReleaseSent = 0;
}

static const unsigned char macroLedNumberList[80] = {64,
    49,50,51,52,53,54,55,56,57,58,59,60,61,62,
    35,36,37,38,39,40,41,42,43,44,45,46,47,48,
    22,23,24,25,26,27,28,29,30,31,32,33,34,64,
    9,10,11,12,13,14,15,16,17,18,19,20,21,8,
    0,1,2,64,64,3,64,64,4,5,6,7,3,3,
    64,64,64,64,64,64,64,64,64
};

static unsigned char macroLedByXY(unsigned char x, unsigned char y)
{
    unsigned int index = x * 14u + y + 1u - 2u;
    if(index >= 80) return 64;
    return macroLedNumberList[index];
}

bool keyMatrix2ReportData(repeating_timer_t *rt)
{
    if ( !tud_hid_ready() ) return true;
    if(keyboardReportPaused)
    {
        if(!keyboardReportPauseReleaseSent)
        {
            unsigned char keyboardEmpty[16] = {0};
            unsigned int empty = 0;
            tud_hid_n_report(0, 0, keyboardEmpty, sizeof(keyboardEmpty));
            tud_hid_n_report(1, 0, (unsigned char *)&empty, sizeof(empty));
            tud_hid_n_report(2, 0, (unsigned char *)&empty, sizeof(empty));
            keyboardReportPauseReleaseSent = 1;
            debugEventText("hid", "pause_release", 0);
        }
        return true;
    }
    static unsigned char init = 1;
    static unsigned char macroKeyboardReportActive = 0;
    unsigned char temp[] = {0x00,0x00,0x00,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned short data[5] =        {0,0,0,0,0};
    static unsigned short keyDataBack[5] = {0,0,0,0,0};
    static unsigned short macroKeyDataBack[5] = {0,0,0,0,0};
    static unsigned short recordKeyDataBack[5] = {0,0,0,0,0};
    unsigned short *keyData = getDataBuff();
    unsigned char jmp = 0;
    sendBuffMediaNew = 0;
    sendBuffMouseNew = 0;
    memcpy(data,keyData,10);

    data[4] &= (~0x8160);
    data[4] = data[4]|(encoderAData);
    encoderAData = 0;
    macroSourceDown = 0;
    
    for(int i = 0;i< 5;i++)
    {
        for(int j = 0; j<16;j++)
        {
            unsigned short t = getKeymapByXY(i,j);
            unsigned char down = ((data[i]) & (0x0001 << j)) != 0;
            unsigned char oldRecordDown = ((recordKeyDataBack[i]) & (0x0001 << j)) != 0;
            unsigned char recordBlocksOutput = macroRecorderIsRecording() && !macroRecorderShouldPassThrough();

            if(down != oldRecordDown)
            {
                macroRecorderHandleKeyEvent(t, down, macroLedByXY(i, j));
            }
            
            if((t >> 8) == 0x04) //层切换                                   //是不是层切换按键
            {   
                unsigned char tempLayer = layerChangeKeyHandle(down,(t&0xff));
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
                if(recordBlocksOutput) continue;
                if(down)
                {
                    t = t & 0xff;
                    if(t > 32) continue;
                    sendBuffMediaNew = (1 << t);
                }
            }
            else if((t >> 8) == 0x01)   //鼠标
            {
                if(recordBlocksOutput) continue;
                if(down)
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
            else if((t >> 8) == 0x03)   //宏
            {
                if(recordBlocksOutput) continue;
                unsigned char macroId = t & 0xff;
                unsigned char oldDown = ((macroKeyDataBack[i]) & (0x0001 << j)) != 0;
                if(macroRunning && macroId == macroSourceId && down) macroSourceDown = 1;
                if(down && oldDown == 0 && macroRunning == 0) macroStart(macroId);
            }
            else
            {   
                if(recordBlocksOutput) continue;
                
                if(((keyDataBack[i]) & (0x0001 << j)) == 0)
                {
                    if(down)
                    {
                        if(t>>8) continue;
                        temp[t/8] |= (1 << (t%8));
                    }
                }
            }
        }
    }
    macroApplyReports(temp, &sendBuffMediaNew, &sendBuffMouseNew);
    unsigned char macroKeyboardActiveNow = macroRunning || macroKeyboardOutputActive();
    memcpy(macroKeyDataBack, data, 10);
    memcpy(recordKeyDataBack, data, 10);
    if(((getlayerTempNumber() !=0xff )&& (jmp == 0)) || macroKeyboardActiveNow || macroKeyboardReportActive)   tud_hid_n_report(0,0,temp,16);
    macroKeyboardReportActive = macroKeyboardActiveNow;
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
    if(keyboardReportPaused) return true;
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
    if(Interval < 1 ) Interval = 1;
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
