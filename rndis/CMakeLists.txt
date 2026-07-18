
aux_source_directory(. DIR_RNDIS_SRCS)

add_library(rndis ${DIR_RNDIS_SRCS})

# Mongoose build flags
add_definitions(-DMG_ENABLE_TCPIP=1)
add_definitions(-DMG_ENABLE_PACKED_FS=1)
add_definitions(-DMG_ENABLE_FILE=0)
add_definitions(-DDISABLE_ROUTING=1)
  
# Example build options 
add_definitions(-DHTTP_URL="http://0.0.0.0/") 

include_directories(../  ${PICO_SDK_PATH}/lib/tinyusb/lib/networking ${CMAKE_CURRENT_LIST_DIR}/ ../usb ../lib/cJson ../lib/littlefs)
target_link_libraries(rndis PUBLIC   pico_stdlib tinyusb_device userUsb cjson hardware_flash littleFs)
