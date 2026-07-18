add_library(debugModule debug.c)

target_include_directories(debugModule PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_SOURCE_DIR}/usb
  ${PICO_SDK_PATH}/lib/tinyusb/src
)
target_compile_definitions(debugModule PUBLIC
  CFG_TUSB_MCU=OPT_MCU_RP2040
  CFG_TUSB_OS=OPT_OS_PICO
)
target_link_libraries(debugModule PUBLIC pico_stdlib pico_multicore pico_bootrom hardware_clocks)
