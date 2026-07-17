cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH})
  set(PICO_SDK_PATH "/mnt/d/kelo/magicKey63/pico-sdk" CACHE PATH "Path to the Raspberry Pi Pico SDK")
endif()

set(PICO_BOARD pico CACHE STRING "Target board")
include(zCmakeFile/pico_sdk_import.cmake)

project(firmware C CXX ASM )
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)  

add_compile_options(-Wall 
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized -O3
         
        )
 
#add_link_options( -Wl,--section-start=.config_addr=0x10020000)

pico_sdk_init()

add_subdirectory(./keyboardScan)
add_subdirectory(./rndis) 
add_subdirectory(./usb) 
add_subdirectory(./st7735) 
add_subdirectory(./lib/lvgl) 
add_subdirectory(./lib/littlefs) 
add_subdirectory(./UI) 
add_subdirectory(./lib/cJson) 
add_subdirectory(./ws2812) 


include_directories(./rndis)
include_directories(./keyboardScan)
include_directories(./usb) 
include_directories(./st7735)
include_directories(./lib/lvgl) 
include_directories(./lib/littlefs) 
include_directories(./UI) 
include_directories(./lib/cJson) 
include_directories(./lvglPort) 
include_directories(./ws2812) 

add_definitions(-DDUAL_CONFIG=1) 
add_definitions(-DPICO_FLASH_SPI_CLKDIV=2) 
add_executable(firmware 
  main.c
  keymap.c 
  dataSaveModel.c
  apmCount.c
  boot.c
  debug.c
  ./lvglPort/lv_port_disp.c
  gif.c
  HCR.c
)

target_include_directories(firmware PUBLIC . ${PICO_SDK_PATH}/lib/tinyusb/lib/networking) 
target_link_libraries(firmware pico_stdlib pico_multicore rndis userUsb keyboardScan st7735 lvgl UI tinyusb_device hardware_flash littleFs cjson ws2812)
pico_enable_stdio_usb(firmware 0)    # Route stdio
pico_enable_stdio_uart(firmware 1)   # to the UART
pico_add_extra_outputs(firmware)     # create map/bin/hex file etc.  
