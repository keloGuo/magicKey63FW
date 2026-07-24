
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "magic63UI.h"
#include "pageChangeLogic.h"

#include "pico/stdlib.h"

/*
    3个虚拟的LED，单向数据
    层，双向数据，还用临时项，多处使用
    回报率。单向数据
*/

//管理LED状态，

static unsigned char ledstate = 0; 
unsigned char getStateLedState(void)
{
    return ledstate;
}
unsigned char __not_in_flash_func(updateVirtuallyLed)(unsigned char data)
{
    ledstate = data;
    return 0;
}
unsigned char virtualyLedUpdateProcess(void)
{
    static unsigned char ledStateOld = 0;
    if(ledStateOld != ledstate) updateLedState(ledstate);
    ledStateOld = ledstate;
    return 0;
}

//通过键盘长按临时切换层
static unsigned char layerNumberTemp = 0xff;
unsigned char getlayerTempNumber(void)
{
    return layerNumberTemp;
}

unsigned char updateLayerTempNumber(unsigned char data)
{
    layerNumberTemp = data;
    return 0;
}

unsigned char layerNumberShowUpdateStatePage(void);
//键盘的按键检测在另外一个核心上完成的，需要UI核心一直检测是否改变，然后刷新页面
unsigned char layerNumberTempUpdateProcess(void)
{
    static unsigned char layerNumberOld = 0XFF;
    if(layerNumberOld != layerNumberTemp) 
    {
        layerNumberOld = layerNumberTemp;
        layerNumberShowUpdateStatePage();
    }
    
    return 0;
}

unsigned char APMShowUpdateStatePage(unsigned int apm);
unsigned char updataApmData(unsigned int a);

static unsigned int apmCount = 0;
unsigned char updataApm(unsigned int v)
{
    apmCount = v;
    return 0;
}

unsigned char getGifPlaying(void);
unsigned char getGamePlaying(void);
unsigned char apmCountUpdataProcess(void)
{
    static unsigned int apm = 0;
    if(apm != apmCount  && (getGifPlaying() == 0) && (getGamePlaying() == 0))//gif 播放的啥时候不要更新页面，即使没有在前台，)
    {
        updataApmData(apmCount);
        APMShowUpdateStatePage(apmCount);
        apm = apmCount;
    }
    return 0;
}

//扫描LED状态，更新UI

bool UIdataUpdate(repeating_timer_t *rt)
{
    virtualyLedUpdateProcess();
    layerNumberTempUpdateProcess();
    apmCountUpdataProcess();
    return 1;
}

struct repeating_timer uiUpdataTimer;
unsigned char uiDataUpdateInit(void)
{
    add_repeating_timer_ms(30,UIdataUpdate,0,&uiUpdataTimer);
    return 0;
}

//通过UI设置的固定的当前层
unsigned char getFlashLayerInfo(void);
unsigned char layerShowUpdte(unsigned char layer);

static unsigned char layerNumber= 1;
unsigned char layerNumberInit(void)
{
    //从flash里面获取当前设置测层
    layerNumber = getFlashLayerInfo();
    layerShowUpdte(layerNumber);
    layerNumberShowUpdateStatePage();
    
    return 0;
}

unsigned char GetlayerNumber(void)
{
    return layerNumber;
}

//层显示有多处，状态页得显示，状态页得子页显示，层页的显示，层页的子页显示
unsigned char layerNumberChange(signed char layer)
{
    layerNumber += layer;
    if (layerNumber > 4) layerNumber = 1;
    if (layerNumber < 1) layerNumber = 4;

    layerShowUpdte(layerNumber);
    layerNumberShowUpdateStatePage();

    unsigned char writeFlashLayerInfo(unsigned char layer);
    writeFlashLayerInfo(layerNumber);

    return 0;
}

unsigned char layerNumberSet(signed char layer)
{
    layerNumber = layer;
    if (layerNumber > 4) layerNumber = 1;
    if (layerNumber < 1) layerNumber = 4;

    layerShowUpdte(layerNumber);
    layerNumberShowUpdateStatePage();

    unsigned char writeFlashLayerInfo(unsigned char layer);
    writeFlashLayerInfo(layerNumber);

    return 0;
}


//通过UI设置当前回报率
unsigned char getFlashRateInfo(void);
unsigned char rateNumberShowUpdateStatePage(unsigned char rate);
unsigned char rateShowUpdte(unsigned char rate);

static unsigned char rateNumber= 1;
unsigned char rateNumberInit(void)
{
    //从flash里面获取当前回报率
    rateNumber = getFlashRateInfo();
    rateShowUpdte(rateNumber);
    rateNumberShowUpdateStatePage(rateNumber);
    return 0;
}

unsigned char GetRateNumber(void)
{
    return rateNumber;
}

//层显示有多出，状态页得显示，状态页得子页显示，层页的显示，层页的子页显示
unsigned char rateNumberChange(signed char rate)
{
    rateNumber += rate;
    if (rateNumber > 20) rateNumber = 1;
    if (rateNumber < 1) rateNumber = 20;

    rateShowUpdte(rateNumber);
    rateNumberShowUpdateStatePage(rateNumber);

    unsigned char writeFlashRateInfo(unsigned char layer);
    writeFlashRateInfo(rateNumber);

    return 0;
}


//通过UI设置当前背光效果
unsigned char getFlashBackLightInfo(void);
unsigned char BackLightShowUpdateStatePage(unsigned char rate);
unsigned char BackLightShowUpdte(unsigned char rate);
unsigned char writeFlashBackLightInfo(unsigned char layer);

static unsigned char BackLightNumber= 1;
unsigned char BackLightInit(void)
{
    //从flash里面获取当前回报率
    BackLightNumber = getFlashBackLightInfo();
    BackLightShowUpdte(BackLightNumber);
    BackLightShowUpdateStatePage(BackLightNumber);
    return 0;
}

unsigned char GetBackLightNumber(void)
{
    return BackLightNumber;
}

//层显示有多出，状态页得显示，状态页得子页显示，层页的显示，层页的子页显示
unsigned char BackLightChange(signed char v)
{
    BackLightNumber += v;
    if (BackLightNumber > 6) BackLightNumber = 1;
    if (BackLightNumber < 1) BackLightNumber = 6;

    BackLightShowUpdte(BackLightNumber);
    BackLightShowUpdateStatePage(BackLightNumber);
    writeFlashBackLightInfo(BackLightNumber);

    return 0;
}
