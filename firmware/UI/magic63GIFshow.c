#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"

extern const lv_img_dsc_t HCR;

void userPrintf(const char* format, ...);
void debugEvent(const char *tag, int value);
void debugEventText(const char *tag, const char *text, int value);
void lv_gif_pause(lv_obj_t * obj);
void lv_gif_resume(lv_obj_t * obj);

static lv_obj_t* tab = NULL;
static lv_obj_t* img = NULL;            //gif 对象
static unsigned char gifPlaying = 0;    //gif是否播放中

unsigned char getGifPlaying(void)
{
    return gifPlaying;
}

static unsigned char PlayState = 1;
void GifPlayDoneOver_cb(lv_event_t * event);

static unsigned char gifCreate(void)
{
    if(img != NULL) return 0;
    if(tab == NULL) return 1;

    img = lv_gif_create(tab);
    if(img == NULL) return 1;
    lv_gif_set_src(img, &HCR);
    lv_gif_pause(img);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(img, GifPlayDoneOver_cb, LV_EVENT_READY, NULL);
    PlayState = 1;
    return 0;
}

static void gifDestroy(void)
{
    if(img == NULL) return;
    lv_obj_del(img);
    img = NULL;
}

int PageGifChangeCallback(int t,int inOrOut,int k) //0,没有按键，1是短按，2是长按
{
    if(inOrOut == 1)
    {
        debugEventText("ui", "gif_in", 0);
        if(gifCreate() != 0) return 0;
        gifPlaying = 1;
        lv_gif_resume(img);
        return 0;
    }
    else if(inOrOut == -1)
    {
         debugEventText("ui", "gif_out", 0);
         if(img != NULL) lv_gif_pause(img);
         gifDestroy();
         gifPlaying = 0;
 
        return 0;       
    }
    
    if(k == 1) //暂停和继续
    {
        if(img == NULL && gifCreate() != 0) return 0;
        if(PlayState % 2) lv_gif_pause(img);
        else lv_gif_resume(img);
        PlayState ++;
    }

    return 0;
}
 
pageInfo* magic63GifPageShow(lv_obj_t* temp)
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
    lv_label_set_text(t, "GIF");
    lv_obj_set_size(t, 50, 20);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, -17);
    lv_obj_set_style_text_color(t, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    t = lv_label_create(T);
    lv_label_set_text(t, "PLAY");
    lv_obj_set_size(t, 50, 30);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 13);
    lv_obj_set_style_text_color(t, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(t, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    return tempPage;
}

pageInfo* magic63GifPlayPageSet(lv_obj_t* temp,pageInfo* homePage)
{
    // 创建一个 table
    lv_obj_t* T = lv_tabview_add_tab(temp, " ");
    lv_obj_set_style_border_color(T, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T, 0, 0);
    lv_obj_set_style_pad_all(T, 0, 0);
    lv_obj_remove_style(T, NULL, LV_PART_SCROLLBAR);


    lv_obj_t* tabview = lv_tabview_create(T, LV_DIR_BOTTOM, 2);        //上下两级菜单
    lv_obj_set_size(tabview, 160, 78);
    lv_obj_set_pos(tabview, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);
    
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, 0, LV_PART_ITEMS | LV_STATE_CHECKED);    

    lv_obj_t* T1 = lv_tabview_add_tab(tabview, " ");
    lv_obj_set_style_border_color(T1, lv_color_make(0xff, 0, 0), 0);
    lv_obj_set_style_border_width(T1, 0, 0);
    lv_obj_set_style_pad_all(T1, 0, 0);
    lv_obj_remove_style(T1, NULL, LV_PART_SCROLLBAR);
    tab = T1;

    pageRegister(homePage, temp, T, PageGifChangeCallback);
 
    return NULL;

}

void GifPlayDoneOver_cb(lv_event_t * event)						//事件回调函数
{
	debugEvent("gif_ready", (int)event->code);
    PlayState = 0;
}

unsigned char gifReload(void)
{
    gifDestroy();
    return gifCreate();
}

lv_obj_t* magic63UIGifPageinit(lv_obj_t* temp)
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

    // lv_obj_set_style_bg_opa(tabview, 0, LV_PART_ITEMS | LV_STATE_CHECKED);

    pageInfo* tempPage = magic63GifPageShow(tabview);
    magic63GifPlayPageSet(tabview,tempPage);
    return T;
}
