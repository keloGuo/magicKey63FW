/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/dma.h"

#define IS_RGBW false
#define NUM_PIXELS 64

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 27
#endif

void userPrintf(const char* format, ...);
void debugEventText(const char *tag, const char *text, int value);

unsigned short *getDataBuff(void);

static int Dma2812Chan = -1;
static unsigned char ws2812DmaClaimed = 0;
static unsigned char ws2812PioLoaded = 0;
static uint ws2812Offset = 0;

static unsigned int PixelData[64] __attribute__((aligned(256))) = {0};
unsigned int PixelDataP = 0;

unsigned char ws2812DMAinit(void)
{
    if(!ws2812DmaClaimed)
    {
        Dma2812Chan = dma_claim_unused_channel(true);
        ws2812DmaClaimed = 1;
    }

    dma_channel_abort(Dma2812Chan);
    dma_channel_config   c = dma_channel_get_default_config(Dma2812Chan);                  //
    channel_config_set_read_increment(&c, true);                                           //
    channel_config_set_write_increment(&c, false);                                         //
    channel_config_set_transfer_data_size(&c,DMA_SIZE_32);                                 //
    channel_config_set_dreq(&c, pio_get_dreq(pio1, 2, true));                              //
    channel_config_set_chain_to(&c, Dma2812Chan);    
    channel_config_set_irq_quiet(&c, true);                                                //
    channel_config_set_ring(&c, false,8);   
    dma_channel_configure(Dma2812Chan, &c,&pio1->txf[2],PixelData,64,false);               //

    return 0;
}

static void ws2812PioStart(void)
{
    PIO pio = pio1;
    int sm = 2;
    if(!ws2812PioLoaded)
    {
        ws2812Offset = pio_add_program(pio, &ws2812_program);
        ws2812PioLoaded = 1;
    }
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);
    ws2812_program_init(pio, sm, ws2812Offset, WS2812_PIN, 800000, IS_RGBW);
}

void ws2812RestoreHardware(void)
{
    debugEventText("ws2812", "restore", 0);
    PixelDataP = 0;
    ws2812PioStart();
    ws2812DMAinit();
}

static inline void put_pixel(uint32_t pixel_grb) {
    //pio_sm_put_blocking(pio1, 2, pixel_grb << 8u);
    PixelData[PixelDataP] = pixel_grb;
    if(PixelDataP == 63) dma_start_channel_mask(1 << Dma2812Chan);
    PixelDataP ++ ;
    PixelDataP %= 64;
}

static inline uint32_t urgb_u32( uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 24) |
            ((uint32_t) (b) << 8) ;
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    int max = 100; // let's not draw too much current!
    t %= max;
    for (int i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}
void ws2812Clean(uint len, uint t) {
    for (int i = 0; i < len; ++i) {
        put_pixel(0);
    }
}
static unsigned char KeyCount =0 ;
void ws2812test(uint len, uint t) {
    
    for (int i = 0; i < len; ++i) {
        if(KeyCount == i)
        {
            put_pixel(urgb_u32(rand()&0xff,rand()&0xff,rand()&0xff));
        }
        else
        {
            put_pixel(0);
        }
        
    }
}

// void ws2812Clean(uint len, uint t) {
//     for (int i = 0; i < len; ++i) {
//         put_pixel(0);
//     }
// }

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
        {ws2812test,   "clean"},
        {ws2812Clean,   "clean"},
};

struct repeating_timer ws2812PixeUpade;
struct repeating_timer lightTimer;
struct repeating_timer codexRestoreTimer;

typedef enum {
    CODEX_STATUS_IDLE = 0,
    CODEX_STATUS_WORKING,
    CODEX_STATUS_SUCCESS,
    CODEX_STATUS_ERROR,
    CODEX_STATUS_WAITING,
} codex_status_t;

static volatile unsigned char codexStatusActive = 0;
static volatile codex_status_t codexStatus = CODEX_STATUS_IDLE;
static volatile unsigned char codexRestorePending = 0;
static volatile unsigned char macroRecordActive = 0;
static volatile unsigned char macroPlayActive = 0;
static unsigned char macroRecordLedState[NUM_PIXELS] = {0};
static unsigned char macroPlayLedIndex = NUM_PIXELS;
static unsigned char macroPlayLedState = 0;
static unsigned int codexStatusTick = 0;
static uint32_t codexWorkingColors[4] = {0};
static unsigned int codexWorkingColorStep = 0xffffffff;

static bool ws2812CodexRestoreTimerHandle(repeating_timer_t *rt);

bool ws2812PixeUpadeTimerHandle (repeating_timer_t *rt)
{
    static unsigned int count = 0;
    static int t = 0;
    static int pat = 0;
    static int dir = 0;
    if(count == 0) 
    {
        pat = rand() % count_of(pattern_table);
        dir = (rand() >> 30) & 1 ? 1 : -1;
        // puts(pattern_table[pat].name);
        // puts(dir == 1 ? "(forward)" : "(backward)");
    }
    count ++ ;
    count %= 1000;
    pattern_table[pat].pat(NUM_PIXELS, t);
    t += dir;
    
    return true ;
}

const unsigned char ledNumberList[80] = {64,   //0
    49,50,51,52,53,54,55,56,57,58,59,60,61,62, //1  - 14 
    35,36,37,38,39,40,41,42,43,44,45,46,47,48, //15 - 28
    22,23,24,25,26,27,28,29,30,31,32,33,34,64, //29 - 42
    9,10,11,12,13,14,15,16,17,18,19,20,21,8,   //43 - 56
    0,1,2,64,64,3,64,64,4,5,6,7,3,3,  //57 - 70
    64,64,64,64,64,64,64,64,64
    };

bool lightTimerHandel(repeating_timer_t *rt)
{
    static unsigned short dataOld[5] = {0x00,0x00,0x00,0x00,0x00}; 
    unsigned short temp[5] = {0};
    unsigned int tempSum = 0;
    unsigned short *p = getDataBuff();    
    
    for(int i = 0; i<5;i++)
    {
       temp[i] = p[i]^dataOld[i];
       temp[i] = temp[i] & p[i];
       
       dataOld[i] = p[i];
       if(temp[i])
        for(int j =0;j<16;j++)
        {
            if((temp[i] >> j) &0x0001)
            {
                tempSum = i * 14 + j + 1 - 2;
                break;
            }
        }    
    }
    if(tempSum)
    {
        
        unsigned char ws2812Trigger(unsigned char mode,unsigned int frameNumber,unsigned char keyNumber);
        unsigned char getLight(void);

        //userPrintf("lightTimerHandel %d %d\n",tempSum,ledNumberList[tempSum]);

        ws2812Trigger(getLight() - 1,30,ledNumberList[tempSum]);
        KeyCount = ledNumberList[tempSum];

        unsigned char tftBackLightCtrlTimerRestart(void);
        tftBackLightCtrlTimerRestart();
        unsigned char gongDePP(void);
        gongDePP();
    }
    return 1;
}

static unsigned char pat = 0;
static unsigned int franeCount = 0;
static int dir = 0;
static int t = 0;

bool ws2812PixeUpadeTimerHandle2 (repeating_timer_t *rt)
{
    if(franeCount == 1)
    {
        pattern_table[count_of(pattern_table)-1].pat(NUM_PIXELS, t); //关闭所有的灯
    }
    else
    {
        pattern_table[pat].pat(NUM_PIXELS, t);  //这里只是一帧
        t += dir;
    }
    franeCount --;
    return  franeCount;
}

unsigned char ws2812Trigger(unsigned char mode,unsigned int frameNumber,unsigned char keyNumber)
{
    if (macroRecordActive || macroPlayActive) return 0;
    if (codexStatusActive || codexRestorePending)
    {
        codexStatusActive = 0;
        if (codexStatus != CODEX_STATUS_IDLE)
        {
            codexRestorePending = 1;
            cancel_repeating_timer(&codexRestoreTimer);
            add_repeating_timer_ms(5000,ws2812CodexRestoreTimerHandle,0,&codexRestoreTimer);
        }
    }
    pat = mode%count_of(pattern_table);
    franeCount = frameNumber;
    dir = (rand() >> 30) & 1 ? 1 : -1;
    t = 0 ;
    cancel_repeating_timer(&ws2812PixeUpade);
    add_repeating_timer_ms(10,ws2812PixeUpadeTimerHandle2,0,&ws2812PixeUpade);
    return 0;
}

static void ws2812MacroRecordRender(void)
{
    for (int i = 0; i < NUM_PIXELS; ++i)
    {
        if(macroRecordLedState[i] == 1) put_pixel(urgb_u32(96, 0, 0));
        else if(macroRecordLedState[i] == 2) put_pixel(urgb_u32(0, 64, 0));
        else put_pixel(0);
    }
}

bool ws2812MacroRecordTimerHandle(repeating_timer_t *rt)
{
    if (!macroRecordActive) return false;
    ws2812MacroRecordRender();
    return true;
}

void ws2812MacroRecordStart(void)
{
    macroRecordActive = 1;
    codexStatusActive = 0;
    codexRestorePending = 0;
    memset(macroRecordLedState, 0, sizeof(macroRecordLedState));
    cancel_repeating_timer(&codexRestoreTimer);
    cancel_repeating_timer(&ws2812PixeUpade);
    PixelDataP = 0;
    ws2812MacroRecordRender();
    add_repeating_timer_ms(50,ws2812MacroRecordTimerHandle,0,&ws2812PixeUpade);
}

void ws2812MacroRecordStop(void)
{
    if(!macroRecordActive) return;
    macroRecordActive = 0;
    memset(macroRecordLedState, 0, sizeof(macroRecordLedState));
    cancel_repeating_timer(&ws2812PixeUpade);
    PixelDataP = 0;
    ws2812Clean(NUM_PIXELS, 0);
}

void ws2812MacroRecordKey(unsigned char ledIndex, unsigned char pressed)
{
    if(!macroRecordActive) return;
    if(ledIndex >= NUM_PIXELS) return;
    macroRecordLedState[ledIndex] = pressed ? 1 : 2;
}

static void ws2812MacroPlayRender(void)
{
    for (int i = 0; i < NUM_PIXELS; ++i)
    {
        if(i == macroPlayLedIndex && macroPlayLedState == 1) put_pixel(urgb_u32(96, 0, 0));
        else if(i == macroPlayLedIndex && macroPlayLedState == 2) put_pixel(urgb_u32(0, 64, 0));
        else put_pixel(0);
    }
}

void ws2812MacroPlayStart(void)
{
    macroPlayActive = 1;
    codexStatusActive = 0;
    codexRestorePending = 0;
    macroPlayLedIndex = NUM_PIXELS;
    macroPlayLedState = 0;
    cancel_repeating_timer(&codexRestoreTimer);
    cancel_repeating_timer(&ws2812PixeUpade);
    PixelDataP = 0;
    ws2812MacroPlayRender();
}

void ws2812MacroPlayStop(void)
{
    if(!macroPlayActive) return;
    macroPlayActive = 0;
    macroPlayLedIndex = NUM_PIXELS;
    macroPlayLedState = 0;
    cancel_repeating_timer(&ws2812PixeUpade);
    PixelDataP = 0;
    ws2812Clean(NUM_PIXELS, 0);
}

void ws2812MacroPlayKey(unsigned char ledIndex, unsigned char pressed)
{
    if(!macroPlayActive) return;
    macroPlayLedIndex = ledIndex;
    macroPlayLedState = (ledIndex < NUM_PIXELS) ? (pressed ? 1 : 2) : 0;
    PixelDataP = 0;
    ws2812MacroPlayRender();
}

static void ws2812CodexFill(uint32_t color)
{
    for (int i = 0; i < NUM_PIXELS; ++i) put_pixel(color);
}

static const unsigned char codexEdgeRing[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    21, 34, 48, 62,
    61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
    35, 22, 9
};

static int ws2812CodexEdgePos(unsigned char led)
{
    for (int i = 0; i < count_of(codexEdgeRing); ++i)
    {
        if (codexEdgeRing[i] == led) return i;
    }
    return -1;
}

static void ws2812CodexFillEdge(uint32_t color)
{
    for (int led = 0; led < NUM_PIXELS; ++led)
    {
        put_pixel(ws2812CodexEdgePos((unsigned char)led) >= 0 ? color : 0);
    }
}

static uint8_t ws2812CodexRandomComponent(void)
{
    return 16 + (rand() % 80);
}

static uint32_t ws2812CodexRandomColor(void)
{
    uint8_t r = ws2812CodexRandomComponent();
    uint8_t g = ws2812CodexRandomComponent();
    uint8_t b = ws2812CodexRandomComponent();
    if ((r + g + b) < 96)
    {
        b += 48;
    }
    return urgb_u32(r, g, b);
}

static void ws2812CodexRandomizeWorkingColors(void)
{
    for (int i = 0; i < count_of(codexWorkingColors); ++i)
    {
        codexWorkingColors[i] = ws2812CodexRandomColor();
    }
}

static void ws2812CodexRenderWorking(unsigned int tick)
{
    const unsigned int ringLen = count_of(codexEdgeRing);
    unsigned int head = tick % ringLen;
    if (head != codexWorkingColorStep)
    {
        codexWorkingColorStep = head;
        ws2812CodexRandomizeWorkingColors();
    }

    for (int led = 0; led < NUM_PIXELS; ++led)
    {
        int pos = ws2812CodexEdgePos((unsigned char)led);
        if (pos < 0)
        {
            put_pixel(0);
            continue;
        }

        uint32_t color = 0;
        for (int i = 0; i < count_of(codexWorkingColors); ++i)
        {
            if (((head + i) % ringLen) == (unsigned int)pos)
            {
                color = codexWorkingColors[i];
                break;
            }
        }
        put_pixel(color);
    }
}

static void ws2812CodexRender(void)
{
    unsigned int tick = codexStatusTick++;

    if (codexStatus == CODEX_STATUS_IDLE)
    {
        ws2812CodexFill(0);
    }
    else if (codexStatus == CODEX_STATUS_WORKING)
    {
        ws2812CodexRenderWorking(tick);
    }
    else if (codexStatus == CODEX_STATUS_SUCCESS)
    {
        ws2812CodexFill((tick % 24) < 12 ? urgb_u32(0, 48, 0) : 0);
    }
    else if (codexStatus == CODEX_STATUS_ERROR)
    {
        ws2812CodexFill((tick % 24) < 12 ? urgb_u32(64, 0, 0) : 0);
    }
    else if (codexStatus == CODEX_STATUS_WAITING)
    {
        unsigned int phase = tick % 50;
        unsigned int level = phase < 25 ? phase : 49 - phase;
        uint8_t r = (uint8_t)(level * 4);
        uint8_t g = (uint8_t)(level * 3);
        ws2812CodexFillEdge(urgb_u32(r, g, 0));
    }
}

bool ws2812CodexTimerHandle(repeating_timer_t *rt)
{
    if (!codexStatusActive) return false;
    ws2812CodexRender();
    return true;
}

static void ws2812CodexStart(void)
{
    codexStatusActive = 1;
    codexStatusTick = 0;
    PixelDataP = 0;
    if (codexStatus == CODEX_STATUS_WORKING)
    {
        codexWorkingColorStep = 0xffffffff;
        ws2812CodexRandomizeWorkingColors();
    }
    cancel_repeating_timer(&ws2812PixeUpade);
    add_repeating_timer_ms(80,ws2812CodexTimerHandle,0,&ws2812PixeUpade);
}

static bool ws2812CodexRestoreTimerHandle(repeating_timer_t *rt)
{
    codexRestorePending = 0;
    if (codexStatus != CODEX_STATUS_IDLE)
    {
        ws2812CodexStart();
    }
    return false;
}

unsigned char ws2812CodexStatus(unsigned char status)
{
    if (status > CODEX_STATUS_WAITING) status = CODEX_STATUS_IDLE;
    codexRestorePending = 0;
    cancel_repeating_timer(&codexRestoreTimer);
    codexStatus = (codex_status_t)status;
    ws2812CodexStart();
    return 0;
}

unsigned char ws2812GetCodexStatus(void)
{
    return (unsigned char)codexStatus;
}

unsigned char ws2812Init(void)
{
    ws2812RestoreHardware();
    add_repeating_timer_ms(5,lightTimerHandel,0,&lightTimer);
    //add_repeating_timer_ms(10,ws2812PixeUpadeTimerHandle,0,&ws2812PixeUpade);              
    return 0;
}
