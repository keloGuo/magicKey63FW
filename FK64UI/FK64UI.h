#include "lvgl.h"

#define MAIN_MENU_NUMBER 5

lv_obj_t* FK64UIStatePageinit(lv_obj_t* temp);
lv_obj_t* FK64UILayerPageinit(lv_obj_t* temp);
lv_obj_t* FK64UILinghtPageinit(lv_obj_t* temp);
lv_obj_t* FK64UIRatePageinit(lv_obj_t* temp);
unsigned char getStateLedState(void);
unsigned char updateLedState(unsigned char state);

