#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"

lv_obj_t* apmShow = NULL;

lv_obj_t* FK64UIAPMPageinit(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageRegister(NULL, temp, T, NULL);
    
    lv_obj_t* layerCont = lv_label_create(T);
    lv_label_set_text(layerCont, "");
    lv_obj_set_size(layerCont, 60, 60);
    lv_obj_align(layerCont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "APM");
    lv_obj_set_size(t, 60, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_opa(t, 0x0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);


    apmShow = lv_label_create(T);
    lv_label_set_text(apmShow, "0");
    lv_obj_set_size(apmShow, 60, 36);
    lv_obj_align(apmShow, LV_ALIGN_CENTER, 0,10);     //居中对象

    lv_obj_set_style_text_font(apmShow, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_line_space(apmShow, 2, 0);
    lv_obj_set_style_text_color(apmShow, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_color(apmShow, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(apmShow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(apmShow, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(apmShow, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(apmShow, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return T;
}

unsigned char updataApmData(unsigned int a)
{
    char temp[10] = {'\0'};
    sprintf(temp,"%d",a);
    lv_label_set_text(apmShow, temp);
    return 0;
}