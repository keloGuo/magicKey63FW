#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"
#include "pico/bootrom.h"

int goTOBootMode(int t)
{
    //(void) t;
    // void (*bootMode)() = ((void (*)())0x000000004);
    // bootMode();
    if(t!=2) return 1;
    reset_usb_boot(0, 0);
    printf("goTOBootMode \r\n");
    return 0;
}

//创建一个view，显示长按进入BOOT模式
lv_obj_t* FK64UIBootPageinit(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageRegister(NULL, temp, T, goTOBootMode);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "Long press to \renter boot mode");
    lv_obj_set_size(t, 120, 36);
    lv_obj_align(t, LV_ALIGN_CENTER, 0,0);     //居中对象

    lv_obj_set_style_text_line_space(t, 2, 0);
    lv_obj_set_style_text_color(t, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_color(t, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(t, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return T;
}

