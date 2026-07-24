#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"

#define NET_IP_MODE_LEGACY 0
#define NET_IP_MODE_PRESET_1 1
#define NET_IP_MODE_PRESET_2 2
#define NET_IP_MODE_PRESET_3 3
#define NET_IP_MODE_CUSTOM 4

static lv_obj_t *netHomeValue = NULL;
static lv_obj_t *netSetModeLabel = NULL;
static lv_obj_t *netSetIpLabel = NULL;
static lv_obj_t *netSetFieldLabel = NULL;
static lv_obj_t *netSetTipLabel = NULL;

static unsigned char netEditField = 0;
static unsigned char netMode = NET_IP_MODE_PRESET_1;
static unsigned char netCustomIp[4] = {10, 63, 63, 1};
static const unsigned char netPresetIp[][4] = {
    {192, 168, 3, 1},
    {10, 63, 27, 1},
    {172, 23, 63, 1},
    {192, 168, 231, 1},
};

unsigned char dataSaveGetNetIpMode(void);
unsigned char dataSaveSetNetIpMode(unsigned char mode);
unsigned char dataSaveGetNetIp(unsigned char *ip);
unsigned char dataSaveGetNetCustomIp(unsigned char *ip);
unsigned char dataSaveSetNetCustomIp(const unsigned char *ip);

static const char *netModeName(unsigned char mode)
{
    switch(mode)
    {
        case NET_IP_MODE_PRESET_1: return "P1";
        case NET_IP_MODE_PRESET_2: return "P2";
        case NET_IP_MODE_PRESET_3: return "P3";
        case NET_IP_MODE_CUSTOM: return "CUS";
        default: return "P1";
    }
}

static void netGetEditIp(unsigned char *ip)
{
    if(ip == NULL) return;
    if(netMode == NET_IP_MODE_CUSTOM)
    {
        ip[0] = netCustomIp[0];
        ip[1] = netCustomIp[1];
        ip[2] = netCustomIp[2];
        ip[3] = netCustomIp[3];
    }
    else
    {
        unsigned char mode = netMode;
        if(mode < NET_IP_MODE_PRESET_1 || mode > NET_IP_MODE_PRESET_3) mode = NET_IP_MODE_PRESET_1;
        ip[0] = netPresetIp[mode][0];
        ip[1] = netPresetIp[mode][1];
        ip[2] = netPresetIp[mode][2];
        ip[3] = netPresetIp[mode][3];
    }
}

static void netRefresh(void)
{
    unsigned char ip[4] = {10, 63, 27, 1};
    char text[32] = {'\0'};

    netGetEditIp(ip);

    if(netHomeValue != NULL)
    {
        snprintf(text, sizeof(text), "%s", netModeName(netMode));
        lv_label_set_text(netHomeValue, text);
    }

    if(netSetModeLabel != NULL)
    {
        snprintf(text, sizeof(text), "%s", netModeName(netMode));
        lv_label_set_text(netSetModeLabel, text);
    }

    if(netSetIpLabel != NULL)
    {
        snprintf(text, sizeof(text), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
        lv_label_set_text(netSetIpLabel, text);
    }

    if(netSetFieldLabel != NULL)
    {
        if(netEditField == 0) snprintf(text, sizeof(text), "< mode >");
        else snprintf(text, sizeof(text), "< ip%u >", netEditField);
        lv_label_set_text(netSetFieldLabel, text);
    }

    if(netSetTipLabel != NULL)
    {
        lv_label_set_text(netSetTipLabel, "save reboot");
    }
}

static void netLoad(void)
{
    netMode = dataSaveGetNetIpMode();
    dataSaveGetNetCustomIp(netCustomIp);
    if(netMode > NET_IP_MODE_CUSTOM) netMode = NET_IP_MODE_PRESET_1;
    netEditField = 0;
}

static void netSave(void)
{
    if(netMode == NET_IP_MODE_CUSTOM)
    {
        dataSaveSetNetCustomIp(netCustomIp);
    }
    else if(netMode >= NET_IP_MODE_PRESET_1 && netMode <= NET_IP_MODE_PRESET_3)
    {
        dataSaveSetNetIpMode(netMode);
    }
}

static unsigned char netLimitOctet(unsigned char field, int value)
{
    if(field == 1)
    {
        if(value < 1) value = 223;
        if(value > 223) value = 1;
        if(value == 127) value = 128;
    }
    else if(field == 4)
    {
        if(value < 1) value = 254;
        if(value > 254) value = 1;
    }
    else
    {
        if(value < 0) value = 255;
        if(value > 255) value = 0;
    }
    return (unsigned char)value;
}

static int PageNetSetCallback(int t, int inOrOut, int k)
{
    if(inOrOut == 1)
    {
        netLoad();
        netRefresh();
        return 1;
    }
    if(inOrOut == -1)
    {
        netSave();
        netRefresh();
        return 1;
    }

    if(k == 1)
    {
        if(netMode == NET_IP_MODE_CUSTOM)
        {
            netEditField++;
            if(netEditField > 4) netEditField = 0;
        }
        else
        {
            netEditField = 0;
        }
        netRefresh();
        return 1;
    }

    if(t == 1 || t == -1)
    {
        if(netEditField == 0)
        {
            signed char mode = (signed char)netMode;
            if(mode < NET_IP_MODE_PRESET_1 || mode > NET_IP_MODE_CUSTOM) mode = NET_IP_MODE_PRESET_1;
            else mode += (signed char)t;
            if(mode > NET_IP_MODE_CUSTOM) mode = NET_IP_MODE_PRESET_1;
            if(mode < NET_IP_MODE_PRESET_1) mode = NET_IP_MODE_CUSTOM;
            netMode = (unsigned char)mode;
        }
        else
        {
            netMode = NET_IP_MODE_CUSTOM;
            netCustomIp[netEditField - 1] = netLimitOctet(netEditField, (int)netCustomIp[netEditField - 1] + t);
        }
        netSave();
        netRefresh();
        return 1;
    }
    return 1;
}

static lv_obj_t *netCreateCard(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *card = lv_label_create(parent);
    lv_label_set_text(card, "");
    lv_obj_set_size(card, w, h);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(card, lv_color_make(0x00, 0xff, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    return card;
}

static pageInfo* magic63NetPageShow(lv_obj_t* temp)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    pageInfo* tempPage = pageRegister(NULL, temp, T, NULL);
    netCreateCard(T, 58, 60);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "net");
    lv_obj_set_size(t, 58, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    netHomeValue = lv_label_create(T);
    lv_label_set_text(netHomeValue, "P1");
    lv_obj_set_size(netHomeValue, 58, 30);
    lv_obj_align(netHomeValue, LV_ALIGN_CENTER, 0, 12);
    lv_obj_set_style_text_color(netHomeValue, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(netHomeValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(netHomeValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(netHomeValue, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(netHomeValue, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    netLoad();
    netRefresh();
    return tempPage;
}

static void magic63NetPageSet(lv_obj_t* temp, pageInfo* homePage)
{
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);

    netCreateCard(T, 150, 66);

    lv_obj_t* t = lv_label_create(T);
    lv_label_set_text(t, "ip group");
    lv_obj_set_size(t, 70, 18);
    lv_obj_set_pos(t, 8, 8);
    lv_obj_set_style_text_color(t, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    netSetModeLabel = lv_label_create(T);
    lv_label_set_text(netSetModeLabel, "P1");
    lv_obj_set_size(netSetModeLabel, 60, 26);
    lv_obj_set_pos(netSetModeLabel, 88, 6);
    lv_obj_set_style_text_font(netSetModeLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(netSetModeLabel, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(netSetModeLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(netSetModeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    netSetIpLabel = lv_label_create(T);
    lv_label_set_text(netSetIpLabel, "10.63.27.1");
    lv_obj_set_size(netSetIpLabel, 140, 18);
    lv_obj_set_pos(netSetIpLabel, 10, 34);
    lv_obj_set_style_text_color(netSetIpLabel, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(netSetIpLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(netSetIpLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    netSetFieldLabel = lv_label_create(T);
    lv_label_set_text(netSetFieldLabel, "< mode >");
    lv_obj_set_size(netSetFieldLabel, 70, 14);
    lv_obj_set_pos(netSetFieldLabel, 6, 58);
    lv_obj_set_style_text_color(netSetFieldLabel, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(netSetFieldLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(netSetFieldLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    netSetTipLabel = lv_label_create(T);
    lv_label_set_text(netSetTipLabel, "save reboot");
    lv_obj_set_size(netSetTipLabel, 78, 14);
    lv_obj_set_pos(netSetTipLabel, 78, 58);
    lv_obj_set_style_text_color(netSetTipLabel, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(netSetTipLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(netSetTipLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    pageRegister(homePage, temp, T, PageNetSetCallback);
}

lv_obj_t* magic63UINetPageinit(lv_obj_t* temp)
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

    pageInfo* tempPage = magic63NetPageShow(tabview);
    magic63NetPageSet(tabview, tempPage);
    return T;
}
