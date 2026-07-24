
aux_source_directory(. DIR_user_usb_SRCS)

add_library(userUsb ${DIR_user_usb_SRCS})

include_directories(./ ../lib/littlefs)
target_include_directories(userUsb PUBLIC ${MAGIC63_FIRMWARE_DIR})
target_link_libraries(userUsb PUBLIC magic63_sdk_headers littleFs pico_unique_id) 
