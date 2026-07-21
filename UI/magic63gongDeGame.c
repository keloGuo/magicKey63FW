#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "pageChangeLogic.h"
extern const lv_img_dsc_t sbmy ;
void lv_gif_pause(lv_obj_t * obj);


static unsigned char gongDeing = 0;     //当前是否在功德界面
static unsigned char gongDeBusy = 1;    //功德gif是否在播放 
static unsigned int gongDeCount = 0;    //功德计数    
static unsigned int gongDeCountH = 0;   //功德计数器整百

static lv_obj_t* gongDeCountShow = NULL;       //功德整百显示控件对象
static lv_obj_t* gongDeBar = NULL;             //功德进度条对象
static lv_obj_t* gongDeGif = NULL;             //功德gif 对象
static lv_obj_t* gongDeGifParent = NULL;        //功德gif 父容器

void gongDeGifPlayDoneOver_cb(lv_event_t * event);
void lv_gif_restart(lv_obj_t * gif);

static unsigned char gongDeGifCreate(void)
{
    if(gongDeGif != NULL) return 0;
    if(gongDeGifParent == NULL) return 1;

    gongDeGif = lv_gif_create(gongDeGifParent);
    if(gongDeGif == NULL) return 1;
    lv_gif_set_src(gongDeGif, &sbmy);
    lv_obj_align(gongDeGif, LV_ALIGN_CENTER, -40, 0);
    lv_obj_add_event_cb(gongDeGif, gongDeGifPlayDoneOver_cb, LV_EVENT_READY, NULL);
    lv_gif_pause(gongDeGif);
    gongDeBusy = 0;
    return 0;
}

static void gongDeGifDestroy(void)
{
    if(gongDeGif == NULL) return;
    lv_obj_del(gongDeGif);
    gongDeGif = NULL;
    gongDeBusy = 0;
}

unsigned char getGamePlaying(void)
{
    return gongDeing;
}

unsigned char gongDePP(void) //检测到有按键按下，功德加1
{
    gongDeCount ++;          //在功德页面才加
    if(gongDeCount >= 100) 
    {
        gongDeCountH ++ ;
        gongDeCount = 0;
        if(gongDeing)  //当前在功德页面 
        {
            char temp[5] = {'\0','\0','\0','\0','\0'};
            sprintf(temp,"%d",gongDeCountH);
            lv_label_set_text(gongDeCountShow, temp);
        }
    }

    if(gongDeing == 1 && gongDeBusy == 0)       //当前在功德页面，且gif没有在播放
    {
        if(gongDeGif == NULL && gongDeGifCreate() != 0) return 0;
        lv_arc_set_value(gongDeBar, gongDeCount); //更新进度条
        gongDeBusy = 1;                           //设置为播放中
        lv_gif_restart(gongDeGif);                //重新开始播放
    }
    return 0;
}


static int PageGongDeGanmeCallback(int t,int inOrOut,int k)
{
    if(inOrOut == 1)
    {
        printf("in  Page gongde game \n");
        gongDeing = 1;
        lv_arc_set_value(gongDeBar, gongDeCount); //更新进度条
        char temp[5] = {'\0','\0','\0','\0','\0'};
        sprintf(temp,"%d",gongDeCountH);
        lv_label_set_text(gongDeCountShow, temp);
        if(gongDeGifCreate() == 0 && gongDeGif != NULL && gongDeBusy == 0)
        {
            gongDeBusy = 1;
            lv_gif_restart(gongDeGif);
        }
        return 0;
    }
    else if(inOrOut == -1)
    {
        gongDeing = 0;           //退出了功德页
        gongDeGifDestroy();
        return 0;       
    }
    return 0;
}
 
static pageInfo* magic63GongDePageShow(lv_obj_t* temp)
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
    lv_label_set_text(t, "game");
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

static pageInfo* magic63GongDePageSet(lv_obj_t* temp,pageInfo* homePage)
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
    gongDeGifParent = T1;


    gongDeBar = lv_arc_create(T1);
    lv_obj_align(gongDeBar, LV_ALIGN_CENTER, 40, 4);
    lv_obj_set_size(gongDeBar, 60, 60);
    lv_obj_clear_flag(gongDeBar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_style(gongDeBar, NULL, LV_PART_KNOB);

    lv_obj_set_style_arc_width(gongDeBar, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_width(gongDeBar, 5, LV_PART_INDICATOR);

    lv_obj_set_style_arc_color(gongDeBar, lv_color_hex(0x00ff00), LV_PART_INDICATOR);

    gongDeCountShow = lv_label_create(gongDeBar);
    lv_label_set_text(gongDeCountShow, "0");
    lv_obj_set_size(gongDeCountShow, 50, 30);
    lv_obj_align(gongDeCountShow, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_text_color(gongDeCountShow, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_set_style_bg_opa(gongDeCountShow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(gongDeCountShow, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(t, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(gongDeCountShow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gongDeCountShow, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_arc_set_value(gongDeBar, 0);

    pageRegister(homePage, temp, T, PageGongDeGanmeCallback);
 
    return NULL;

}

void gongDeGifPlayDoneOver_cb(lv_event_t * event)						//事件回调函数
{
	printf("img my_event_cb %d\n",event->code);
    if(gongDeGif != NULL) lv_gif_pause(gongDeGif);
    gongDeBusy = 0;
}


lv_obj_t* magic63UIGongDePageinit(lv_obj_t* temp)
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

    pageInfo* tempPage = magic63GongDePageShow(tabview);
    magic63GongDePageSet(tabview,tempPage);

    return T;
}
