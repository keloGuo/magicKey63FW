#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "FK64UI.h"
#include "pageChangeLogic.h" 
lv_obj_t* FK64UIBootPageinit(lv_obj_t* temp);

void FK64UI(void)
{
    lv_obj_t* tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 2);
    lv_obj_set_size(tabview, 160, 80);                              
    lv_obj_set_pos(tabview, 0, 0);                                  
    lv_obj_set_style_bg_color(tabview, lv_color_make(0, 0, 0), 0);

    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xff, 0xff, 0xff), LV_PART_ITEMS | LV_STATE_CHECKED);

    printf("ui init start %d \r\n",1);
    FK64UIStatePageinit(tabview);
    printf("ui init start %d \r\n",2);
    FK64UILayerPageinit(tabview);
    printf("ui init start %d \r\n",3);
    //FK64UILinghtPageinit(tabview);
    FK64UIRatePageinit(tabview);
    printf("ui init start %d \r\n",4);
    FK64UIBootPageinit(tabview);
    printf("ui init start %d \r\n",5);
 
}


