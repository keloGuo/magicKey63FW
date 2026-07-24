#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "macroRecorder.h"
#include "pageChangeLogic.h"

static lv_obj_t *macroRecHomeId = NULL;
static lv_obj_t *macroRecTitle = NULL;
static lv_obj_t *macroRecId = NULL;
static lv_obj_t *macroRecInfo = NULL;
static macro_rec_state_t macroRecUiState = 0xff;
static unsigned int macroRecUiId = 0xffffffff;
static unsigned int macroRecUiCount = 0xffffffff;
static unsigned char macroRecUiPass = 0xff;

static void macroRecLabelStyle(lv_obj_t *obj, lv_color_t color, const lv_font_t *font)
{
    lv_obj_set_style_text_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(font != NULL) lv_obj_set_style_text_font(obj, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_obj_t *macroRecCreatePanel(lv_obj_t *parent, lv_coord_t width)
{
    lv_obj_t *panel = lv_label_create(parent);
    lv_label_set_text(panel, "");
    lv_obj_set_size(panel, width, 60);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(panel, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    return panel;
}

static const char *macroRecStateTitle(macro_rec_state_t state)
{
    if(state == MACRO_REC_PASS_CONFIRM) return "PASS";
    if(state == MACRO_REC_RECORDING) return "REC";
    if(state == MACRO_REC_SAVING) return "SAVE";
    if(state == MACRO_REC_SAVED) return "SAVED";
    if(state == MACRO_REC_ERROR) return "ERROR";
    return "READY";
}

static const char *macroRecHintText(macro_rec_state_t state)
{
    if(state == MACRO_REC_PASS_CONFIRM) return "PUSH";
    if(state == MACRO_REC_RECORDING) return "STOP";
    if(state == MACRO_REC_SAVED) return "BACK";
    if(state == MACRO_REC_ERROR) return "BACK";
    return "REC";
}

static void macroRecPageUpdate(void)
{
    macro_rec_state_t state = macroRecorderGetState();
    unsigned int id = macroRecorderGetMacroId();
    unsigned int count = macroRecorderGetActionCount();
    unsigned char pass = macroRecorderGetPassThrough();

    if(macroRecTitle == NULL || macroRecId == NULL || macroRecInfo == NULL) return;
    if(macroRecUiState == state && macroRecUiId == id && macroRecUiCount == count && macroRecUiPass == pass) return;

    macroRecUiState = state;
    macroRecUiId = id;
    macroRecUiCount = count;
    macroRecUiPass = pass;

    char buf[24];
    lv_label_set_text(macroRecTitle, macroRecStateTitle(state));

    if(id == 0) snprintf(buf, sizeof(buf), "M--");
    else snprintf(buf, sizeof(buf), "M%u", id);
    lv_label_set_text(macroRecId, buf);
    if(macroRecHomeId != NULL) lv_label_set_text(macroRecHomeId, buf);

    if(state == MACRO_REC_PASS_CONFIRM)
    {
        snprintf(buf, sizeof(buf), "%s %s", pass ? "ON" : "OFF", macroRecHintText(state));
    }
    else if(state == MACRO_REC_ERROR)
    {
        snprintf(buf, sizeof(buf), "%s", macroRecorderGetErrorText());
    }
    else
    {
        snprintf(buf, sizeof(buf), "%u ACT %s", count, macroRecHintText(state));
    }
    lv_label_set_text(macroRecInfo, buf);
}

static void macroRecPageTimer(lv_timer_t *timer)
{
    (void)timer;
    macroRecPageUpdate();
}

static int MacroRecPageCallback(int t, int inOrOut, int k)
{
    if(inOrOut == 1)
    {
        macroRecorderPageEnter();
        macroRecUiState = 0xff;
        macroRecPageUpdate();
        return 0;
    }
    if(inOrOut == -1)
    {
        macroRecorderWheelLongPress();
        macroRecUiState = 0xff;
        macroRecPageUpdate();
        return 0;
    }

    if(k == 1)
    {
        macroRecorderWheelClick();
        macroRecPageUpdate();
        return 1;
    }
    if(k == 2)
    {
        macroRecorderWheelLongPress();
        macroRecPageUpdate();
        return 1;
    }
    if(k == 0 && (t == 1 || t == -1))
    {
        macroRecorderWheelRotate(t);
        macroRecPageUpdate();
        return 1;
    }

    return 0;
}

static pageInfo *magic63MacroRecordPageShow(lv_obj_t *temp)
{
    lv_obj_t *T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageInfo *tempPage = pageRegister(NULL, temp, T, NULL);

    macroRecCreatePanel(T, 55);

    lv_obj_t *t = lv_label_create(T);
    lv_label_set_text(t, "macro");
    lv_obj_set_size(t, 55, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    macroRecLabelStyle(t, lv_color_make(0x00, 0x00, 0x00), NULL);

    macroRecHomeId = lv_label_create(T);
    lv_label_set_text(macroRecHomeId, "M--");
    lv_obj_set_size(macroRecHomeId, 55, 30);
    lv_obj_align(macroRecHomeId, LV_ALIGN_CENTER, 0, 10);
    macroRecLabelStyle(macroRecHomeId, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_28);

    return tempPage;
}

static void magic63MacroRecordPageSet(lv_obj_t *temp, pageInfo *homePage)
{
    lv_obj_t *T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    macroRecCreatePanel(T, 112);

    macroRecTitle = lv_label_create(T);
    lv_label_set_text(macroRecTitle, "READY");
    lv_obj_set_size(macroRecTitle, 104, 16);
    lv_obj_align(macroRecTitle, LV_ALIGN_CENTER, 0, -20);
    macroRecLabelStyle(macroRecTitle, lv_color_make(0, 0, 0), &lv_font_montserrat_14);

    macroRecId = lv_label_create(T);
    lv_label_set_text(macroRecId, "M--");
    lv_obj_set_size(macroRecId, 104, 24);
    lv_obj_align(macroRecId, LV_ALIGN_CENTER, 0, 0);
    macroRecLabelStyle(macroRecId, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_28);

    macroRecInfo = lv_label_create(T);
    lv_label_set_text(macroRecInfo, "0 ACT REC");
    lv_obj_set_size(macroRecInfo, 104, 15);
    lv_obj_align(macroRecInfo, LV_ALIGN_CENTER, 0, 23);
    macroRecLabelStyle(macroRecInfo, lv_color_make(0, 0, 0), &lv_font_montserrat_14);

    pageRegister(homePage, temp, T, MacroRecPageCallback);
}

lv_obj_t* magic63UIMacroRecordPageinit(lv_obj_t* temp)
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

    pageInfo *tempPage = magic63MacroRecordPageShow(tabview);
    magic63MacroRecordPageSet(tabview, tempPage);

    lv_timer_create(macroRecPageTimer, 150, NULL);
    macroRecorderPageEnter();
    macroRecPageUpdate();
    return T;
}
