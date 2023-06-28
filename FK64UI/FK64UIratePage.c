#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"

lv_obj_t *rateShowObjF = NULL;
lv_obj_t *rateShowObjS = NULL;
unsigned char rateNumberChange(signed char rate);

unsigned char rateShowUpdte(unsigned char rate)
{
    unsigned char temp[2] = {rate + 0x30,'\0'};
    lv_label_set_text(rateShowObjF, (char*)(&temp));
    lv_label_set_text(rateShowObjS, (char*)(&temp));
    return 0;
}

int PageRateSetCallback(int t)
{
    if ((t == 1) || (t == -1))
    {
        rateNumberChange(t);
    } 
    return 0;
}


pageInfo* FK64RatePageShow(lv_obj_t* temp)
{
    // 创建一个 table
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageInfo* tempPage = pageRegister(NULL, temp, T, NULL);

    lv_obj_t* layerCont = lv_label_create(T);
    lv_label_set_text(layerCont, "");
    lv_obj_set_size(layerCont, 50, 60);
    lv_obj_align(layerCont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    //一级界面上显示当前层
    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "rate");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);


    t = lv_label_create(T);
    rateShowObjF = t;
    lv_label_set_text(t, "1");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 13);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return tempPage;

}

pageInfo* FK64RatePageSet(lv_obj_t* temp,pageInfo* homePage)
{
    // 创建一个 table
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    lv_obj_t* layerCont = lv_label_create(T);
    lv_label_set_text(layerCont, "");
    lv_obj_set_size(layerCont, 50, 60);
    lv_obj_align(layerCont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    //一级界面上显示当前层
    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "rate");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);


    t = lv_label_create(T);
    rateShowObjS = t;
    lv_label_set_text(t, "1");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(T);
    lv_label_set_text(t, LV_SYMBOL_LEFT);
    lv_obj_set_size(t, 20,20);
    lv_obj_align(t, LV_ALIGN_CENTER, -15, 10);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(T);
    lv_label_set_text(t, LV_SYMBOL_RIGHT);
    lv_obj_set_size(t, 20, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 15, 10);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0x0, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    pageRegister(homePage, temp, T, PageRateSetCallback);
    return NULL;

}


lv_obj_t* FK64UIRatePageinit(lv_obj_t* temp)
{
    //创建一个table
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);
    //在Table上创建一个tabview
    lv_obj_t* tabview = lv_tabview_create(T, LV_DIR_RIGHT, 2);        //上下两级菜单
    lv_obj_set_size(tabview, 160, 78);
    lv_obj_set_pos(tabview, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);
    
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, 0, LV_PART_ITEMS | LV_STATE_CHECKED);

    pageInfo* tempPage = FK64RatePageShow(tabview);
    FK64RatePageSet(tabview,tempPage);
    
    unsigned char rateNumberInit(void);
    rateNumberInit();

    return T;
}

