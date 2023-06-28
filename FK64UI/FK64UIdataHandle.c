
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "FK64UI.h"
#include "pageChangeLogic.h"

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
unsigned char updateVirtuallyLed(unsigned char data)
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

//扫描LED状态，更新UI
unsigned char UIdataUpdate(void)
{
    virtualyLedUpdateProcess();
    layerNumberTempUpdateProcess();
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

//层显示有多出，状态页得显示，状态页得子页显示，层页的显示，层页的子页显示
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
    if (rateNumber > 9) rateNumber = 1;
    if (rateNumber < 1) rateNumber = 9;

    rateShowUpdte(rateNumber);
    rateNumberShowUpdateStatePage(rateNumber);

    unsigned char writeFlashRateInfo(unsigned char layer);
    writeFlashRateInfo(rateNumber);

    return 0;
}
