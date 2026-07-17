#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "magic63UI.h"
#include "pageChangeLogic.h" 
lv_obj_t* magic63UIBootPageinit(lv_obj_t* temp);
lv_obj_t* magic63UILinghtPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIGifPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIGongDePageinit(lv_obj_t* temp);
void magic63UI(void)
{
    lv_obj_t* tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 2);
    lv_obj_set_size(tabview, 160, 80);                              
    lv_obj_set_pos(tabview, 0, 0);                                  
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);

    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);

    // lv_tabview_set_anim_time(tabview,1000);

    magic63UIStatePageinit(tabview);
    magic63UILayerPageinit(tabview);
    magic63UILinghtPageinit(tabview);
    magic63UIRatePageinit(tabview);
    magic63UIAPMPageinit(tabview);
    magic63UIGifPageinit(tabview);
    magic63UIGongDePageinit(tabview);
    magic63UIBootPageinit(tabview);
}


