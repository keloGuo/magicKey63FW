#include <stdio.h>
#include <string.h>

#include "lvgl.h"
#include "keyboardScan.h"
#include "pageChangeLogic.h"

static pageInfo bounceDiagPage;
static unsigned char bounceDiagPageReady = 0;
static unsigned char bounceDiagPageActive = 0;
static lv_obj_t *bounceDiagTitle = NULL;
static lv_obj_t *bounceDiagState = NULL;
static lv_obj_t *bounceDiagHint = NULL;
static key_bounce_diag_state_t bounceDiagUiState = 0xff;

void keyBounceDiagCancel(void);
key_bounce_diag_result_t keyBounceDiagGetResult(void);
void bounceDiagUiExitRequest(void);

static const char *bounceDiagTitleText(key_bounce_diag_state_t state)
{
    if(state == KEY_BOUNCE_DIAG_SELECT) return "PRESS KEY";
    if(state == KEY_BOUNCE_DIAG_WAIT_RELEASE) return "RELEASE";
    if(state == KEY_BOUNCE_DIAG_WAIT_PRESS) return "PRESS";
    if(state == KEY_BOUNCE_DIAG_CAPTURE) return "HOLD";
    if(state == KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE) return "RELEASE";
    if(state == KEY_BOUNCE_DIAG_RELEASE_CAPTURE) return "REL CAP";
    if(state == KEY_BOUNCE_DIAG_DONE) return "DONE";
    if(state == KEY_BOUNCE_DIAG_ERROR) return "ERROR";
    return "BOUNCE";
}

static const char *bounceDiagHintText(key_bounce_diag_state_t state)
{
    if(state == KEY_BOUNCE_DIAG_SELECT) return "ANY KEY";
    if(state == KEY_BOUNCE_DIAG_WAIT_RELEASE) return "WAIT STABLE";
    if(state == KEY_BOUNCE_DIAG_WAIT_PRESS) return "TARGET KEY";
    if(state == KEY_BOUNCE_DIAG_CAPTURE) return "KEEP DOWN";
    if(state == KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE) return "KEY UP";
    if(state == KEY_BOUNCE_DIAG_RELEASE_CAPTURE) return "KEEP UP";
    if(state == KEY_BOUNCE_DIAG_DONE) return "NEXT KEY";
    if(state == KEY_BOUNCE_DIAG_ERROR) return "LONG EXIT";
    return "LONG EXIT";
}

static void bounceDiagLabelStyle(lv_obj_t *obj, lv_color_t color, const lv_font_t *font)
{
    lv_obj_set_style_text_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(font != NULL) lv_obj_set_style_text_font(obj, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void bounceDiagPageUpdate(void)
{
    if(!bounceDiagPageReady) return;

    key_bounce_diag_result_t result = keyBounceDiagGetResult();
    if(bounceDiagUiState == result.state) return;
    bounceDiagUiState = result.state;

    lv_label_set_text(bounceDiagTitle, bounceDiagTitleText(result.state));

    char buf[24];
    if(result.row < 0xff && result.col < 0xff) snprintf(buf, sizeof(buf), "R%u C%u", result.row + 1, result.col + 1);
    else snprintf(buf, sizeof(buf), "--");
    lv_label_set_text(bounceDiagState, buf);
    lv_label_set_text(bounceDiagHint, bounceDiagHintText(result.state));

    if(bounceDiagPageActive && result.state == KEY_BOUNCE_DIAG_IDLE)
    {
        bounceDiagPageActive = 0;
        pageTemporaryLeave(&bounceDiagPage);
    }
}

static void bounceDiagPageTimer(lv_timer_t *timer)
{
    (void)timer;
    bounceDiagPageUpdate();
}

static int BounceDiagPageCallback(int t, int inOrOut, int k)
{
    (void)t;
    if(inOrOut == 1)
    {
        bounceDiagUiState = 0xff;
        bounceDiagPageUpdate();
        return 1;
    }
    if(k == 2)
    {
        bounceDiagUiExitRequest();
        bounceDiagPageActive = 0;
        pageTemporaryLeave(&bounceDiagPage);
        return 1;
    }
    return 1;
}

void magic63UIBounceDiagPageEnter(void)
{
    if(!bounceDiagPageReady) return;
    bounceDiagPageActive = 1;
    bounceDiagUiState = 0xff;
    pageTemporaryEnter(&bounceDiagPage);
    bounceDiagPageUpdate();
}

void magic63UIBounceDiagPageLeave(void)
{
    if(!bounceDiagPageReady) return;
    bounceDiagPageActive = 0;
    pageTemporaryLeave(&bounceDiagPage);
}

lv_obj_t* magic63UIBounceDiagPageinit(lv_obj_t* temp)
{
    (void)temp;
    lv_obj_t *T = lv_obj_create(lv_scr_act());
    lv_obj_set_size(T, 160, 80);
    lv_obj_set_pos(T, 0, 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);
    lv_obj_set_style_bg_color(T, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(T, 255, 0);
    lv_obj_clear_flag(T, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(T, LV_OBJ_FLAG_HIDDEN);

    bounceDiagPage.pageHandleX = T;
    bounceDiagPage.pageHandleY = T;
    bounceDiagPage.keyHandleCallback = BounceDiagPageCallback;
    bounceDiagPage.previousPage = &bounceDiagPage;
    bounceDiagPage.nextPage = &bounceDiagPage;
    bounceDiagPage.subpage = NULL;
    bounceDiagPage.homepage = NULL;

    lv_obj_t *panel = lv_obj_create(T);
    lv_obj_set_size(panel, 138, 62);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(panel, lv_color_make(0x12, 0x1a, 0x24), 0);
    lv_obj_set_style_bg_opa(panel, 255, 0);
    lv_obj_set_style_border_color(panel, lv_color_make(0x4d, 0x93, 0xdb), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);

    bounceDiagTitle = lv_label_create(panel);
    lv_label_set_text(bounceDiagTitle, "BOUNCE");
    lv_obj_set_size(bounceDiagTitle, 130, 18);
    lv_obj_align(bounceDiagTitle, LV_ALIGN_TOP_MID, 0, 5);
    bounceDiagLabelStyle(bounceDiagTitle, lv_color_make(0xff, 0xff, 0xff), &lv_font_montserrat_14);

    bounceDiagState = lv_label_create(panel);
    lv_label_set_text(bounceDiagState, "--");
    lv_obj_set_size(bounceDiagState, 130, 26);
    lv_obj_align(bounceDiagState, LV_ALIGN_CENTER, 0, 3);
    bounceDiagLabelStyle(bounceDiagState, lv_color_make(0x4d, 0x93, 0xdb), &lv_font_montserrat_22);

    bounceDiagHint = lv_label_create(panel);
    lv_label_set_text(bounceDiagHint, "LONG EXIT");
    lv_obj_set_size(bounceDiagHint, 130, 16);
    lv_obj_align(bounceDiagHint, LV_ALIGN_BOTTOM_MID, 0, -4);
    bounceDiagLabelStyle(bounceDiagHint, lv_color_make(0xa8, 0xb3, 0xc2), &lv_font_montserrat_14);

    bounceDiagPageReady = 1;
    lv_timer_create(bounceDiagPageTimer, 150, NULL);
    return T;
}
