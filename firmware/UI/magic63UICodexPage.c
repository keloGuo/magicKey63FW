#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"
#include "magic63UI.h"

static lv_obj_t *codexNameObj = NULL;
static lv_obj_t *codexDetailStatusObj = NULL;
static lv_obj_t *codexDetailArcObj = NULL;
static lv_obj_t *codexDetailUsageObj = NULL;
static lv_obj_t *codexDetailDateObj = NULL;
static unsigned char codexUiStatus = 0xff;
static unsigned char codexUsageValid = 0;
static unsigned char codexUsagePercent = 0;
static unsigned int codexTokenTotal = 0;
static char codexUsageDate[16] = "--";
static unsigned char codexUiUsageValid = 0xff;
static unsigned char codexUiUsagePercent = 0xff;
static unsigned int codexUiTokenTotal = 0xffffffffu;
static char codexUiUsageDate[16] = "";

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

static lv_color_t codexStatusColor(unsigned char status)
{
    switch (status)
    {
        case 1: return lv_color_make(0x00, 0x50, 0xff);
        case 2: return lv_color_make(0x00, 0x90, 0x20);
        case 3: return lv_color_make(0xd0, 0x00, 0x00);
        case 4: return lv_color_make(0xff, 0xc0, 0x00);
        default: return lv_color_make(0x55, 0x55, 0x55);
    }
}

static lv_color_t codexStatusTextColor(unsigned char status)
{
    return status == 4 ? lv_color_make(0x00, 0x00, 0x00) : lv_color_make(0xff, 0xff, 0xff);
}

static void codexPageShowStatus(unsigned char status)
{
    if (codexUiStatus == status) return;

    codexUiStatus = status;
    if(codexNameObj != NULL) lv_label_set_text(codexNameObj, codexStatusName(status));
    if(codexDetailStatusObj != NULL)
    {
        lv_label_set_text(codexDetailStatusObj, codexStatusName(status));
        lv_obj_set_style_bg_color(codexDetailStatusObj, codexStatusColor(status), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(codexDetailStatusObj, codexStatusTextColor(status), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void magic63CodexUsageUpdate(unsigned char hasUsage, unsigned char usagePercent, unsigned int totalTokens, const char *usageDate)
{
    if(hasUsage)
    {
        if(usagePercent > 100) usagePercent = 100;
        codexUsageValid = 1;
        codexUsagePercent = usagePercent;
    }
    codexTokenTotal = totalTokens;
    if(usageDate != NULL && usageDate[0] != '\0')
    {
        snprintf(codexUsageDate, sizeof(codexUsageDate), "%s", usageDate);
    }
}

unsigned char magic63CodexUsageValid(void)
{
    return codexUsageValid;
}

unsigned char magic63CodexUsagePercent(void)
{
    return codexUsagePercent;
}

unsigned int magic63CodexTokenTotal(void)
{
    return codexTokenTotal;
}

static void codexPageShowUsage(void)
{
    if(codexDetailUsageObj == NULL || codexDetailDateObj == NULL) return;
    if(codexUiUsageValid == codexUsageValid &&
       codexUiUsagePercent == codexUsagePercent &&
       codexUiTokenTotal == codexTokenTotal &&
       strcmp(codexUiUsageDate, codexUsageDate) == 0) return;

    codexUiUsageValid = codexUsageValid;
    codexUiUsagePercent = codexUsagePercent;
    codexUiTokenTotal = codexTokenTotal;
    snprintf(codexUiUsageDate, sizeof(codexUiUsageDate), "%s", codexUsageDate);

    if(codexUsageValid)
    {
        char text[24];
        if(codexDetailArcObj != NULL) lv_arc_set_value(codexDetailArcObj, codexUsagePercent);
        snprintf(text, sizeof(text), "%u%%", (unsigned int)codexUsagePercent);
        lv_label_set_text(codexDetailUsageObj, text);
    }
    else
    {
        if(codexDetailArcObj != NULL) lv_arc_set_value(codexDetailArcObj, 0);
        lv_label_set_text(codexDetailUsageObj, "--%");
    }
    char dateText[24];
    snprintf(dateText, sizeof(dateText), "%s", codexUsageDate);
    lv_label_set_text(codexDetailDateObj, dateText);
}

static void codexLabelBase(lv_obj_t *obj, lv_color_t color, const lv_font_t *font)
{
    lv_obj_set_style_text_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void codexPageTimer(lv_timer_t *timer)
{
    (void)timer;
    codexPageShowStatus(ws2812GetCodexStatus());
    codexPageShowUsage();
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
    codexPageShowUsage();
    lv_timer_create(codexPageTimer, 250, NULL);

    return tempPage;
}

pageInfo* magic63CodexPageDetail(lv_obj_t* temp,pageInfo* homePage)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    lv_obj_set_style_bg_color(T, lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(T, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(T, NULL, LV_PART_SCROLLBAR);

    codexDetailStatusObj = lv_label_create(T);
    lv_label_set_text(codexDetailStatusObj, "idle");
    lv_obj_set_size(codexDetailStatusObj, 54, 30);
    lv_obj_align(codexDetailStatusObj, LV_ALIGN_CENTER, -42, 0);
    lv_obj_set_style_bg_color(codexDetailStatusObj, codexStatusColor(0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(codexDetailStatusObj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(codexDetailStatusObj, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_text_align(codexDetailStatusObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(codexDetailStatusObj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(codexDetailStatusObj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);

    codexDetailArcObj = lv_arc_create(T);
    lv_obj_set_size(codexDetailArcObj, 62, 62);
    lv_obj_align(codexDetailArcObj, LV_ALIGN_CENTER, 38, 0);
    lv_arc_set_range(codexDetailArcObj, 0, 100);
    lv_arc_set_value(codexDetailArcObj, 0);
    lv_arc_set_bg_angles(codexDetailArcObj, 135, 45);
    lv_obj_clear_flag(codexDetailArcObj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(codexDetailArcObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(codexDetailArcObj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(codexDetailArcObj, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(codexDetailArcObj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(codexDetailArcObj, lv_color_make(0x18, 0x18, 0x18), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(codexDetailArcObj, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(codexDetailArcObj, lv_color_make(0x00, 0xff, 0x00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(codexDetailArcObj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(codexDetailArcObj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    codexDetailUsageObj = lv_label_create(T);
    lv_label_set_text(codexDetailUsageObj, "--%");
    lv_obj_set_size(codexDetailUsageObj, 50, 18);
    lv_obj_align(codexDetailUsageObj, LV_ALIGN_CENTER, 38, 2);
    codexLabelBase(codexDetailUsageObj, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_14);

    codexDetailDateObj = lv_label_create(T);
    lv_label_set_text(codexDetailDateObj, "--");
    lv_obj_set_size(codexDetailDateObj, 76, 14);
    lv_obj_align(codexDetailDateObj, LV_ALIGN_BOTTOM_RIGHT, -2, -1);
    codexLabelBase(codexDetailDateObj, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_14);

    pageRegister(homePage, temp, T, NULL);
    codexUiStatus = 0xff;
    codexUiUsageValid = 0xff;
    codexPageShowStatus(ws2812GetCodexStatus());
    codexPageShowUsage();
    return NULL;
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

    pageInfo* tempPage = magic63CodexPageShow(tabview);
    magic63CodexPageDetail(tabview,tempPage);
    return T;
}
