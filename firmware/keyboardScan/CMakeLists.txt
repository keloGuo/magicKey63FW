
aux_source_directory(. DIR_KeyScan_SRCS)

add_library(keyboardScan ${DIR_KeyScan_SRCS})
pico_generate_pio_header(keyboardScan ${CMAKE_CURRENT_LIST_DIR}/keyboardScan.pio)
pico_generate_pio_header(keyboardScan ${MAGIC63_FIRMWARE_DIR}/PIO/keyBounceDiag.pio)
target_include_directories(keyboardScan PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(keyboardScan PUBLIC magic63_sdk_headers)  
