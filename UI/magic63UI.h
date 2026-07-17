#include "lvgl.h"

#define MAIN_MENU_NUMBER 5

lv_obj_t* magic63UIStatePageinit(lv_obj_t* temp);
lv_obj_t* magic63UILayerPageinit(lv_obj_t* temp);
lv_obj_t* magic63UILinghtPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIRatePageinit(lv_obj_t* temp);
unsigned char getStateLedState(void);
unsigned char updateLedState(unsigned char state);
lv_obj_t* magic63UIAPMPageinit(lv_obj_t* temp);

