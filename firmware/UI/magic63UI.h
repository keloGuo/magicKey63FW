#include "lvgl.h"

#define MAIN_MENU_NUMBER 5

lv_obj_t* magic63UIStatePageinit(lv_obj_t* temp);
lv_obj_t* magic63UILayerPageinit(lv_obj_t* temp);
lv_obj_t* magic63UILinghtPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIRatePageinit(lv_obj_t* temp);
lv_obj_t* magic63UINetPageinit(lv_obj_t* temp);
lv_obj_t* magic63UICodexPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIMacroRecordPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIMacroPlayPageinit(lv_obj_t* temp);
lv_obj_t* magic63UIBounceDiagPageinit(lv_obj_t* temp);
void magic63UIBounceDiagPageEnter(void);
void magic63UIBounceDiagPageLeave(void);
unsigned char getStateLedState(void);
unsigned char updateLedState(unsigned char state);
lv_obj_t* magic63UIAPMPageinit(lv_obj_t* temp);

