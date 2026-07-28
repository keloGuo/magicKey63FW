
aux_source_directory(. DIR_user_usb_SRCS)

add_library(userUsb ${DIR_user_usb_SRCS})

target_include_directories(userUsb PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(userUsb PUBLIC
  magic63_tinyusb_headers
  littleFs
  macroRecorder
  pico_unique_id_headers
)
