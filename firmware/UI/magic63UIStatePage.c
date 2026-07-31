#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"

#include "pageChangeLogic.h"
#include "magic63UI.h"

#define STATE_PAGE_FUNC_COUNT 4

unsigned char showFuncNumber = 2; //灯光效果，APM, 回包率, Codex状态
unsigned int showFuncNumberValuelist[STATE_PAGE_FUNC_COUNT] = {0,0,0,0};
const char* showFuncNumberList[STATE_PAGE_FUNC_COUNT] = { "light" ,"APM","rate","codex" };

//第一页的功能设置页。当前生效层，APM、灯效、回报率、Codex状态选择一项显示
lv_obj_t* FuncSetActiveTabList[STATE_PAGE_FUNC_COUNT] = { NULL,NULL,NULL,NULL };
lv_obj_t* FuncSetActiveTabDataList[STATE_PAGE_FUNC_COUNT] = { NULL,NULL,NULL,NULL };
static int FuncSetActiveCount = 0;

unsigned char ws2812GetCodexStatus(void);

static const char *statePageCodexStatusName(unsigned char status)
{
    switch (status)
    {
        case 1: return "active";
        case 2: return "success";
        case 3: return "error";
        case 4: return "waiting";
        default: return "idle";
    }
}

static const lv_font_t *statePageFuncValueFont(unsigned char v)
{
    return (v == 3) ? &lv_font_montserrat_14 : &lv_font_montserrat_28;
}

static void statePageFuncValueText(unsigned char v, char *out, size_t outLen)
{
    if(v == 3)
    {
        snprintf(out, outLen, "%s", statePageCodexStatusName(ws2812GetCodexStatus()));
    }
    else
    {
        snprintf(out, outLen, "%d", showFuncNumberValuelist[v]);
    }
}

static void statePageSetValueLabel(lv_obj_t *label, unsigned char v)
{
    if(label == NULL) return;
    char tempValue[12] = {'\0'};
    statePageFuncValueText(v, tempValue, sizeof(tempValue));
    lv_label_set_text(label, tempValue);
    lv_obj_set_style_text_font(label, statePageFuncValueFont(v), LV_PART_MAIN | LV_STATE_DEFAULT);
}

//更新3个虚拟的led灯
lv_obj_t* VirtuallyLed[3] = {NULL,NULL,NULL};
unsigned char updateLedState(unsigned char state)
{
    lv_obj_set_style_bg_color(VirtuallyLed[0], (state &0x01)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(VirtuallyLed[1], (state &0x02)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(VirtuallyLed[2], (state &0x04)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    return 0;
}

unsigned char VirtuallyLedInit(lv_obj_t* T)
{
    unsigned char ledState = getStateLedState();
    VirtuallyLed[0] = lv_label_create(T);

    lv_label_set_text(VirtuallyLed[0], "num");
    lv_obj_set_size(VirtuallyLed[0], 40, 20);
    lv_obj_set_pos(VirtuallyLed[0], 5, 53);

    lv_obj_set_style_text_color(VirtuallyLed[0], lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_color(VirtuallyLed[0], (ledState &0x01)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(VirtuallyLed[0], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(VirtuallyLed[0], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(VirtuallyLed[0], 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(VirtuallyLed[0], 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    VirtuallyLed[1] = lv_label_create(T);
    lv_label_set_text(VirtuallyLed[1], "caps");
    lv_obj_set_size(VirtuallyLed[1], 40, 20);
    lv_obj_set_pos(VirtuallyLed[1], 5, 3);

    lv_obj_set_style_text_color(VirtuallyLed[1], lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_color(VirtuallyLed[1], (ledState &0x02)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(VirtuallyLed[1], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(VirtuallyLed[1], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(VirtuallyLed[1], 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(VirtuallyLed[1], 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    VirtuallyLed[2] = lv_label_create(T);
    lv_label_set_text(VirtuallyLed[2], "scr");
    lv_obj_set_size(VirtuallyLed[2], 40, 20);
    lv_obj_set_pos(VirtuallyLed[2], 5, 28);

    lv_obj_set_style_text_color(VirtuallyLed[2], lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_color(VirtuallyLed[2], (ledState &0x04)?lv_color_make(0x00, 0xff, 0x00):lv_color_make(0x73, 0x73, 0x73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(VirtuallyLed[2], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(VirtuallyLed[2], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(VirtuallyLed[2], 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(VirtuallyLed[2], 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

unsigned char GetlayerNumber(void);
unsigned char getlayerTempNumber(void);
unsigned char keymapChange(unsigned char layer,unsigned char layerTemp);

//层变化的时候更新显示
lv_obj_t* layerNumberObj = NULL;
unsigned char layerNumberShowUpdateStatePage(void)
{
    char temp[10] = {'\0'};
    if(getlayerTempNumber() == 0xff)  
    {
        sprintf(temp,"%d",GetlayerNumber());
    }
    else 
    {
        sprintf(temp,"%d",getlayerTempNumber());
    }
    keymapChange(GetlayerNumber(),getlayerTempNumber());
    if(layerNumberObj != NULL)
        lv_label_set_text(layerNumberObj, temp);
    return 0;
}
//回报率改变的时候更新显示
unsigned char upadtaStatePageShow(lv_obj_t* T, unsigned char v);
unsigned char rateNumberShowUpdateStatePage(unsigned char rate)
{
    showFuncNumberValuelist[2] = rate;
    upadtaStatePageShow(NULL, showFuncNumber);
    return 0;
}

unsigned char APMShowUpdateStatePage(unsigned int apm)
{
    showFuncNumberValuelist[1] = apm;
    upadtaStatePageShow(NULL, showFuncNumber);
    return 0;
}

unsigned char BackLightShowUpdateStatePage(unsigned char v)
{
    showFuncNumberValuelist[0] = v;
    upadtaStatePageShow(NULL, showFuncNumber);
    return 0;
}


//第二个功能改变的时候更新显示，
unsigned char upadtaStatePageShow(lv_obj_t* T, unsigned char v)
{
    static lv_obj_t* layerCont = NULL;
    static lv_obj_t* tName = NULL;
    static lv_obj_t* tValue = NULL;
    if(v >= STATE_PAGE_FUNC_COUNT) v = 2;

    if ((T != NULL) && (tName == NULL)) //初始化
    {
        layerCont = lv_label_create(T);
        lv_label_set_text(layerCont, "");
        lv_obj_set_size(layerCont, 50, 60);
        lv_obj_set_pos(layerCont, 105, 10);

        lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(layerCont, true, LV_PART_MAIN | LV_STATE_DEFAULT);

        tName = lv_label_create(T);
        lv_label_set_text(tName, showFuncNumberList[showFuncNumber]);
        lv_obj_set_size(tName, 50, 20);
        lv_obj_set_pos(tName, 105, 12);

        lv_obj_set_style_text_color(tName, lv_color_make(0x00, 0x00, 0x00), 0);
        lv_obj_set_style_bg_opa(tName, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(tName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(tName, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(tName, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

        tValue = lv_label_create(T);
        statePageSetValueLabel(tValue, showFuncNumber);
        lv_obj_set_size(tValue, 50, 30);
        lv_obj_set_pos(tValue, 105, 37);

        lv_obj_set_style_text_color(tValue, lv_color_make(0xff, 0xff, 0xff), 0);
        lv_obj_set_style_bg_opa(tValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(tValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(tValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(tValue, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        lv_label_set_text(tName, showFuncNumberList[v]);
        if(showFuncNumber != v)
        {
            unsigned char dataSaveStatePageFunction(unsigned char v);
            dataSaveStatePageFunction(v);
        }
        showFuncNumber = v;
        statePageSetValueLabel(tValue, v);
        statePageSetValueLabel(FuncSetActiveTabDataList[v], v);

    }
    if(layerCont != NULL)
    {
        lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    return 0;
}

static void statePageCodexTimer(lv_timer_t *timer)
{
    (void)timer;
    unsigned char status = ws2812GetCodexStatus();
    if(showFuncNumberValuelist[3] == status) return;

    showFuncNumberValuelist[3] = status;
    statePageSetValueLabel(FuncSetActiveTabDataList[3], 3);
    if(showFuncNumber == 3)
    {
        upadtaStatePageShow(NULL, 3);
    }
}

//第一个主页，显示一些状态，固定显示3个灯，当前生效层，APM和灯效，回报率3选一
pageInfo* magic63UIStatePageShow(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageInfo* tempPage = pageRegister(NULL, temp, T, NULL);

    VirtuallyLedInit(T);
    
    lv_obj_t* layerCont = lv_label_create(T);
    lv_label_set_text(layerCont, "");
    lv_obj_set_size(layerCont, 50, 60);
    lv_obj_set_pos(layerCont, 50, 10);

    lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

     lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "layer");
    lv_obj_set_size(t, 50, 20);
    lv_obj_set_pos(t, 50, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0x00, 0x00, 0x00), 0); 
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    layerNumberObj = lv_label_create(T);
    lv_label_set_text(layerNumberObj, "1");
    lv_obj_set_size(layerNumberObj, 50, 30);
    lv_obj_set_pos(layerNumberObj, 50, 37);

    lv_obj_set_style_text_color(layerNumberObj, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(layerNumberObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(layerNumberObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(layerNumberObj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(layerNumberObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerNumberObj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    upadtaStatePageShow(T,0);
    return tempPage;
}


int PageFuncSetActiveChange(int t,int x,int k)
{
    if ((t == 1) || (t == -1))
    {
        FuncSetActiveCount += t;
        if (FuncSetActiveCount >= STATE_PAGE_FUNC_COUNT) FuncSetActiveCount = 0;
        if (FuncSetActiveCount < 0) FuncSetActiveCount = STATE_PAGE_FUNC_COUNT - 1;
        lv_obj_scroll_to_view_recursive(FuncSetActiveTabList[FuncSetActiveCount], LV_ANIM_ON);
        upadtaStatePageShow(NULL, FuncSetActiveCount);

        lv_obj_t* t = FuncSetActiveTabDataList[FuncSetActiveCount];
        statePageSetValueLabel(t, FuncSetActiveCount);
        lv_obj_set_size(t, 50, 30);
        lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);
    }
    else if(t>1)
    {
        FuncSetActiveCount = t - 2;
        FuncSetActiveCount %= STATE_PAGE_FUNC_COUNT;
        lv_obj_scroll_to_view_recursive(FuncSetActiveTabList[FuncSetActiveCount], LV_ANIM_ON);
        upadtaStatePageShow(NULL, FuncSetActiveCount);
    }
    return 0;
}
unsigned char getFlashBackLightInfo(void);
unsigned char getFlashRateInfo(void);

unsigned char PageFuncSetActiveInit(lv_obj_t* temp)
{
    showFuncNumberValuelist[0] =  getFlashBackLightInfo();
    showFuncNumberValuelist[1] =  0;
    showFuncNumberValuelist[2] =  getFlashRateInfo();
    showFuncNumberValuelist[3] =  ws2812GetCodexStatus();
    if(showFuncNumber >= STATE_PAGE_FUNC_COUNT) showFuncNumber = 2;
    FuncSetActiveCount = showFuncNumber;

    lv_obj_t* tabview;
    tabview = lv_tabview_create(temp, LV_DIR_BOTTOM, 1);
    lv_obj_set_size(tabview, 50, 60);
    lv_obj_set_style_radius(tabview, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_align(tabview, LV_ALIGN_CENTER, 0, -3);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0xff, 0), 0);

    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0, 0, 0), LV_PART_ITEMS | LV_STATE_CHECKED);

    FuncSetActiveTabList[0] = lv_tabview_add_tab(tabview, " ");
    lv_obj_set_style_border_color(FuncSetActiveTabList[0], lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(FuncSetActiveTabList[0], 0, 0);
    lv_obj_set_style_pad_all(FuncSetActiveTabList[0], 0, 0);

    lv_obj_t* t = lv_label_create(FuncSetActiveTabList[0]);
    lv_label_set_text(t, "light");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -18);

    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(FuncSetActiveTabList[0]);
    FuncSetActiveTabDataList[0] = t;
    statePageSetValueLabel(t, 0);
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    FuncSetActiveTabList[1] = lv_tabview_add_tab(tabview, " ");
    lv_obj_set_style_border_color(FuncSetActiveTabList[1], lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(FuncSetActiveTabList[1], 0, 0);
    lv_obj_set_style_pad_all(FuncSetActiveTabList[1], 0, 0);

    t = lv_label_create(FuncSetActiveTabList[1]);
    lv_label_set_text(t, "APM");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -18);

    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(FuncSetActiveTabList[1]);
    FuncSetActiveTabDataList[1] = t;
    statePageSetValueLabel(t, 1);
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    FuncSetActiveTabList[2] = lv_tabview_add_tab(tabview, " ");
    lv_obj_set_style_border_color(FuncSetActiveTabList[2], lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(FuncSetActiveTabList[2], 0, 0);
    lv_obj_set_style_pad_all(FuncSetActiveTabList[2], 0, 0);

    t = lv_label_create(FuncSetActiveTabList[2]);
    lv_label_set_text(t, "rate");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -18);

    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(FuncSetActiveTabList[2]);
    FuncSetActiveTabDataList[2] = t;
    statePageSetValueLabel(t, 2);
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    FuncSetActiveTabList[3] = lv_tabview_add_tab(tabview, " ");
    lv_obj_set_style_border_color(FuncSetActiveTabList[3], lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(FuncSetActiveTabList[3], 0, 0);
    lv_obj_set_style_pad_all(FuncSetActiveTabList[3], 0, 0);

    t = lv_label_create(FuncSetActiveTabList[3]);
    lv_label_set_text(t, "codex");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -18);

    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(FuncSetActiveTabList[3]);
    FuncSetActiveTabDataList[3] = t;
    statePageSetValueLabel(t, 3);
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_scroll_to_view_recursive(FuncSetActiveTabList[showFuncNumber], LV_ANIM_OFF);
    return 0;
}

lv_obj_t* magic63UIStatePageFuncSet(lv_obj_t* temp, pageInfo* homePage)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");                        
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, LV_SYMBOL_LEFT);
    lv_obj_set_size(t, 20,20);
    lv_obj_align(t, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(T);
    lv_label_set_text(t, LV_SYMBOL_RIGHT);
    lv_obj_set_size(t, 20, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 35, 0);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    PageFuncSetActiveInit(T);

    pageRegister(homePage, temp, T, PageFuncSetActiveChange);
    return NULL;
}

lv_obj_t* magic63UIStatePageinit(lv_obj_t* temp)
{



    lv_obj_t* T = lv_tabview_add_tab(temp, " ");                        //创建一个 tab
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);
    unsigned char dataSaveGetStatePageFunction(void);
    showFuncNumber = dataSaveGetStatePageFunction();

    lv_obj_t* tabview = lv_tabview_create(T, LV_DIR_RIGHT, 2);          //上下两级菜单
    lv_obj_set_size(tabview, 160, 78);
    lv_obj_set_pos(tabview, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);
    
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, 0, LV_PART_ITEMS | LV_STATE_CHECKED);

    pageInfo* tempPage =  magic63UIStatePageShow(tabview);
    magic63UIStatePageFuncSet(tabview, tempPage);
    lv_timer_create(statePageCodexTimer, 250, NULL);
   
    return T;
}


//编码器，不能直接切换页面，需要做一个，页面切换逻辑

