
aux_source_directory(. DIR_st7735_SRCS)

add_library(st7735 ${DIR_st7735_SRCS})
target_include_directories(st7735 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(st7735 PUBLIC magic63_sdk_headers)   
