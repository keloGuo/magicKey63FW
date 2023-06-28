#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"

#include "pageChangeLogic.h"
#include "FK64UI.h"

unsigned char showFuncNumber = 2; //灯光效果，APM, 回包率
unsigned char showFuncNumberValuelist[] = {0,0,0};
const char* showFuncNumberList[3] = { "light" ,"APM","rate" };

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

//第二个功能改变的时候更新显示，
unsigned char upadtaStatePageShow(lv_obj_t* T, unsigned char v)
{
    static lv_obj_t* tName = NULL;
    static lv_obj_t* tValue = NULL;
    char tempValue[] = "     ";
    if ((T != NULL) && (tName == NULL)) //初始化
    {
        sprintf(tempValue, "%d", showFuncNumberValuelist[showFuncNumber]);

        lv_obj_t* layerCont = lv_label_create(T);
        lv_label_set_text(layerCont, "");
        lv_obj_set_size(layerCont, 50, 60);
        lv_obj_set_pos(layerCont, 105, 10);

        lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

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
        lv_label_set_text(tValue, tempValue);
        lv_obj_set_size(tValue, 50, 30);
        lv_obj_set_pos(tValue, 105, 37);

        lv_obj_set_style_text_color(tValue, lv_color_make(0xff, 0xff, 0xff), 0);
        lv_obj_set_style_bg_opa(tValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(tValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(tValue, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(tValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(tValue, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        sprintf(tempValue, "%d", showFuncNumberValuelist[v]);
        lv_label_set_text(tName, showFuncNumberList[v]);
        showFuncNumber = v;
        lv_label_set_text(tValue, tempValue);
    }
    return 0;
}

//第一个主页，显示一些状态，固定显示3个灯，当前生效层，APM和灯效，回报率3选一
pageInfo* FK64UIStatePageShow(lv_obj_t* temp)
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

//第一页的红能设置页。当前生效层，APM和灯效，回报率3选一
lv_obj_t* FuncSetActiveTabList[3] = { NULL,NULL,NULL };

int PageFuncSetActiveChange(int t)
{
    static int count = 0;
    if ((t == 1) || (t == -1))
    {
        count += t;
        if (count > 2) count = 0;
        if (count < 0) count = 2;
        lv_obj_scroll_to_view_recursive(FuncSetActiveTabList[count], LV_ANIM_ON);
        upadtaStatePageShow(NULL, count);
    }
    else if(t>1)
    {
        count = t - 2;
        count %= 3;
        lv_obj_scroll_to_view_recursive(FuncSetActiveTabList[count], LV_ANIM_ON);
        upadtaStatePageShow(NULL, count);
    }
    return 0;
}

unsigned char PageFuncSetActiveInit(lv_obj_t* temp)
{
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
    lv_label_set_text(t, "B");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text(t, "90");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text(t, "125");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 12);

    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    //PageFuncSetActiveChange(2);
    return 0;
}

lv_obj_t* FK64UIStatePageFuncSet(lv_obj_t* temp, pageInfo* homePage)
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

lv_obj_t* FK64UIStatePageinit(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");                        //创建一个 tab
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    lv_obj_t* tabview = lv_tabview_create(T, LV_DIR_RIGHT, 2);          //上下两级菜单
    lv_obj_set_size(tabview, 160, 78);
    lv_obj_set_pos(tabview, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);
    
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, 0, LV_PART_ITEMS | LV_STATE_CHECKED);

    pageInfo* tempPage =  FK64UIStatePageShow(tabview);
    FK64UIStatePageFuncSet(tabview, tempPage);
    return T;
}


//编码器，不能直接切换页面，需要做一个，页面切换逻辑

