#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_HCR
#define LV_ATTRIBUTE_IMG_HCR
#endif

const lv_img_dsc_t HCR = {
  .header.cf = LV_IMG_CF_RAW_CHROMA_KEYED,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 160,
  .header.h = 80,
  .data_size = 187295,
  .data = (const uint8_t *)((1024)*1024 + 0x10000000),//HCR_map
};
