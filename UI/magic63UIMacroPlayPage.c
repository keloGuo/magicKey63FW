#include <stdio.h>

#include "lvgl.h"

unsigned char macroPlayerIsRunning(void);
unsigned char macroPlayerGetId(void);
unsigned char macroPlayerGetIndex(void);
unsigned char macroPlayerGetTotal(void);
unsigned int macroPlayerGetDelayMs(void);
unsigned char macroPlayerGetMode(void);

static lv_obj_t *macroPlayTitle = NULL;
static lv_obj_t *macroPlayId = NULL;
static lv_obj_t *macroPlayInfo = NULL;
static lv_obj_t *macroPlayDelay = NULL;
static lv_obj_t *macroPlayBar = NULL;
static lv_obj_t *macroPlayRoot = NULL;
static unsigned char macroPlayUiRunning = 0xff;
static unsigned char macroPlayUiId = 0xff;
static unsigned char macroPlayUiIndex = 0xff;
static unsigned char macroPlayUiTotal = 0xff;
static unsigned int macroPlayUiDelay = 0xffffffffu;
static unsigned char macroPlayUiMode = 0xff;

static void macroPlayLabelStyle(lv_obj_t *obj, lv_color_t color, const lv_font_t *font)
{
    lv_obj_set_style_text_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(font != NULL) lv_obj_set_style_text_font(obj, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void macroPlayPanelStyle(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void macroPlayPageUpdate(void)
{
    unsigned char running = macroPlayerIsRunning();
    unsigned char id = macroPlayerGetId();
    unsigned char index = macroPlayerGetIndex();
    unsigned char total = macroPlayerGetTotal();
    unsigned int delay = macroPlayerGetDelayMs();
    unsigned char mode = macroPlayerGetMode();

    if(macroPlayRoot == NULL || macroPlayTitle == NULL || macroPlayId == NULL || macroPlayInfo == NULL || macroPlayDelay == NULL || macroPlayBar == NULL) return;

    if(running)
    {
        lv_obj_clear_flag(macroPlayRoot, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(macroPlayRoot);
    }
    else
    {
        lv_obj_add_flag(macroPlayRoot, LV_OBJ_FLAG_HIDDEN);
    }

    if(macroPlayUiRunning == running &&
       macroPlayUiId == id &&
       macroPlayUiIndex == index &&
       macroPlayUiTotal == total &&
       macroPlayUiDelay == delay &&
       macroPlayUiMode == mode)
    {
        return;
    }

    macroPlayUiRunning = running;
    macroPlayUiId = id;
    macroPlayUiIndex = index;
    macroPlayUiTotal = total;
    macroPlayUiDelay = delay;
    macroPlayUiMode = mode;

    char buf[32];
    if(running)
    {
        lv_label_set_text(macroPlayTitle, mode == 1 ? "MACRO LOOP" : "MACRO RUN");
        snprintf(buf, sizeof(buf), "M%u", id);
        lv_label_set_text(macroPlayId, buf);
        snprintf(buf, sizeof(buf), "%u/%u ACT", index, total);
        lv_label_set_text(macroPlayInfo, buf);
        if(delay > 0) snprintf(buf, sizeof(buf), "%ums", delay);
        else snprintf(buf, sizeof(buf), "RUN");
        lv_label_set_text(macroPlayDelay, buf);
        lv_bar_set_range(macroPlayBar, 0, total == 0 ? 1 : total);
        lv_bar_set_value(macroPlayBar, index, LV_ANIM_OFF);
    }
    else
    {
        lv_label_set_text(macroPlayTitle, "MACRO");
        lv_label_set_text(macroPlayId, "IDLE");
        lv_label_set_text(macroPlayInfo, "0/0 ACT");
        lv_label_set_text(macroPlayDelay, "--");
        lv_bar_set_range(macroPlayBar, 0, 1);
        lv_bar_set_value(macroPlayBar, 0, LV_ANIM_OFF);
    }
}

static void macroPlayPageTimer(lv_timer_t *timer)
{
    (void)timer;
    macroPlayPageUpdate();
}

lv_obj_t* magic63UIMacroPlayPageinit(lv_obj_t* temp)
{
    (void)temp;
    lv_obj_t *T = lv_obj_create(lv_scr_act());
    lv_obj_set_size(T, 160, 80);
    lv_obj_set_pos(T, 0, 0);
    lv_obj_set_style_bg_color(T, lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(T, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(T, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(T, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(T, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(T, LV_OBJ_FLAG_HIDDEN);
    macroPlayRoot = T;

    lv_obj_t *panel = lv_obj_create(T);
    lv_obj_set_size(panel, 128, 64);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    macroPlayPanelStyle(panel);

    macroPlayTitle = lv_label_create(panel);
    lv_label_set_text(macroPlayTitle, "MACRO");
    lv_obj_set_size(macroPlayTitle, 120, 14);
    lv_obj_align(macroPlayTitle, LV_ALIGN_TOP_MID, 0, 4);
    macroPlayLabelStyle(macroPlayTitle, lv_color_make(0, 0, 0), &lv_font_montserrat_14);

    macroPlayId = lv_label_create(panel);
    lv_label_set_text(macroPlayId, "IDLE");
    lv_obj_set_size(macroPlayId, 70, 28);
    lv_obj_align(macroPlayId, LV_ALIGN_LEFT_MID, 8, -3);
    macroPlayLabelStyle(macroPlayId, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_28);

    macroPlayInfo = lv_label_create(panel);
    lv_label_set_text(macroPlayInfo, "0/0 ACT");
    lv_obj_set_size(macroPlayInfo, 48, 14);
    lv_obj_align(macroPlayInfo, LV_ALIGN_RIGHT_MID, -7, -8);
    macroPlayLabelStyle(macroPlayInfo, lv_color_make(0, 0, 0), &lv_font_montserrat_14);

    macroPlayDelay = lv_label_create(panel);
    lv_label_set_text(macroPlayDelay, "--");
    lv_obj_set_size(macroPlayDelay, 48, 14);
    lv_obj_align(macroPlayDelay, LV_ALIGN_RIGHT_MID, -7, 8);
    macroPlayLabelStyle(macroPlayDelay, lv_color_make(0, 0, 0), &lv_font_montserrat_14);

    macroPlayBar = lv_bar_create(panel);
    lv_obj_set_size(macroPlayBar, 112, 6);
    lv_obj_align(macroPlayBar, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(macroPlayBar, lv_color_make(0x20, 0x20, 0x20), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(macroPlayBar, lv_color_make(0xff, 0xff, 0xff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_bar_set_range(macroPlayBar, 0, 1);
    lv_bar_set_value(macroPlayBar, 0, LV_ANIM_OFF);

    lv_timer_create(macroPlayPageTimer, 100, NULL);
    macroPlayPageUpdate();
    return T;
}
