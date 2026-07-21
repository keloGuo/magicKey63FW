#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"
#include "magic63UI.h"

static lv_obj_t *codexNameObj = NULL;
static unsigned char codexUiStatus = 0xff;

unsigned char ws2812GetCodexStatus(void);

static const char *codexStatusName(unsigned char status)
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

static void codexPageShowStatus(unsigned char status)
{
    if (codexNameObj == NULL) return;
    if (codexUiStatus == status) return;

    codexUiStatus = status;
    lv_label_set_text(codexNameObj, codexStatusName(status));
}

static void codexPageTimer(lv_timer_t *timer)
{
    (void)timer;
    codexPageShowStatus(ws2812GetCodexStatus());
}

pageInfo* magic63CodexPageShow(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageInfo* tempPage = pageRegister(NULL, temp, T, NULL);

    lv_obj_t* layerCont = lv_label_create(T);
    lv_label_set_text(layerCont, "");
    lv_obj_set_size(layerCont, 55, 60);
    lv_obj_align(layerCont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_bg_color(layerCont, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(layerCont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(layerCont, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "codex");
    lv_obj_set_size(t, 55, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    codexNameObj = lv_label_create(T);
    lv_label_set_text(codexNameObj, "");
    lv_obj_set_size(codexNameObj, 55, 30);
    lv_obj_align(codexNameObj, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_text_color(codexNameObj, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(codexNameObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(codexNameObj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(codexNameObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(codexNameObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(codexNameObj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    codexPageShowStatus(ws2812GetCodexStatus());
    lv_timer_create(codexPageTimer, 250, NULL);

    return tempPage;
}

lv_obj_t* magic63UICodexPageinit(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    lv_obj_t* tabview = lv_tabview_create(T, LV_DIR_RIGHT, 2);
    lv_obj_set_size(tabview, 160, 78);
    lv_obj_set_pos(tabview, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);

    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, 0, LV_PART_ITEMS | LV_STATE_CHECKED);

    magic63CodexPageShow(tabview);
    return T;
}
