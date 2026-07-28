set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)  
set(MAGIC63_FIRMWARE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(MAGIC63_FIRMWARE_VERSION "0.1.0-dev" CACHE STRING "MagicKey63 firmware version")

add_compile_options(-Wall 
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized -O3
         
        )
add_compile_definitions(PICO_MALLOC_PANIC=0)
option(MAGIC63_ENABLE_USB_CDC_DEBUG "Enable USB CDC debug serial interface" OFF)
option(MAGIC63_ENABLE_UART_STDIO "Enable UART stdio printf output" OFF)
option(MAGIC63_ENABLE_DEBUG_OUTPUT "Enable formatted debug output" OFF)
add_compile_definitions(
  MAGIC63_ENABLE_USB_CDC_DEBUG=$<BOOL:${MAGIC63_ENABLE_USB_CDC_DEBUG}>
  MAGIC63_ENABLE_UART_STDIO=$<BOOL:${MAGIC63_ENABLE_UART_STDIO}>
  MAGIC63_ENABLE_DEBUG_OUTPUT=$<BOOL:${MAGIC63_ENABLE_DEBUG_OUTPUT}>
  MAGIC63_FIRMWARE_VERSION="${MAGIC63_FIRMWARE_VERSION}"
)

add_library(magic63_sdk_headers INTERFACE)
target_link_libraries(magic63_sdk_headers INTERFACE
  pico_stdlib_headers
  pico_multicore_headers
  pico_bootrom_headers
  hardware_clocks_headers
  hardware_dma_headers
  hardware_flash_headers
  hardware_pio_headers
  hardware_spi_headers
)

add_library(magic63_tinyusb_headers INTERFACE)
target_link_libraries(magic63_tinyusb_headers INTERFACE
  magic63_sdk_headers
)
target_include_directories(magic63_tinyusb_headers INTERFACE
  ${MAGIC63_FIRMWARE_DIR}/usb
  ${PICO_SDK_PATH}/lib/tinyusb/src
  ${PICO_SDK_PATH}/lib/tinyusb/lib/networking
)
target_compile_definitions(magic63_tinyusb_headers INTERFACE
  CFG_TUSB_MCU=OPT_MCU_RP2040
  CFG_TUSB_OS=OPT_OS_PICO
)

#add_link_options( -Wl,--section-start=.config_addr=0x10020000)

add_subdirectory(./scratch)
add_subdirectory(./keyboardScan)
add_subdirectory(./rndis) 
add_subdirectory(./usb) 
add_subdirectory(./st7735) 
add_subdirectory(./lib/lvgl) 
add_subdirectory(./lib/littlefs) 
add_subdirectory(./keymap)
add_subdirectory(./dataSave)
add_subdirectory(./apm)
add_subdirectory(./boot)
add_subdirectory(./debug)
add_subdirectory(./macroRecorder)
add_subdirectory(./UI) 
add_subdirectory(./lib/cJson) 
add_subdirectory(./ws2812) 

add_executable(firmware 
  main.c
  ./lvglPort/lv_port_disp.c
)

target_include_directories(firmware PRIVATE
  .
  ./lvglPort
)
target_compile_definitions(firmware PRIVATE
  DUAL_CONFIG=1
  PICO_FLASH_SPI_CLKDIV=2
)
target_link_libraries(firmware
  rndis
  userUsb
  keyboardScan
  st7735
  lvgl
  UI
  keymap
  dataSave
  apm
  boot
  debugModule
  littleFs
  cjson
  macroRecorder
  ws2812
  scratch
  magic63_tinyusb_headers
  pico_unique_id
  pico_stdlib
  pico_multicore
  pico_bootrom
  tinyusb_device
  hardware_clocks
  hardware_dma
  hardware_flash
  hardware_pio
  hardware_spi
)
set_target_properties(firmware PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_options(firmware PRIVATE
  -Wl,--wrap=_malloc_r
  -Wl,--wrap=_calloc_r
  -Wl,--wrap=_realloc_r
  -Wl,--wrap=_free_r
)
pico_enable_stdio_usb(firmware 0)    # Route stdio
pico_enable_stdio_uart(firmware ${MAGIC63_ENABLE_UART_STDIO})   # to the UART
pico_add_extra_outputs(firmware)     # create map/bin/hex file etc.  
add_custom_target(firmware_root_uf2 ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/firmware.uf2
    ${CMAKE_BINARY_DIR}/firmware.uf2
  DEPENDS firmware
)
