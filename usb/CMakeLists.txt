
aux_source_directory(. DIR_user_usb_SRCS)

add_library(userUsb ${DIR_user_usb_SRCS})

include_directories(./ ${PICO_SDK_PATH}/lib/tinyusb/lib/networking ../lib/littlefs)
target_link_libraries(userUsb PUBLIC tinyusb_device pico_stdlib littleFs) 
