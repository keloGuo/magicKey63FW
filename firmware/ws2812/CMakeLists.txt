
aux_source_directory(. DIR_WS2812_SRCS)

add_library(ws2812 ${DIR_WS2812_SRCS})
pico_generate_pio_header(ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
target_include_directories(ws2812 PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ws2812 PUBLIC magic63_sdk_headers)  
