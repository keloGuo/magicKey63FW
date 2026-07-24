
aux_source_directory(. DIR_st7735_SRCS)

add_library(st7735 ${DIR_st7735_SRCS})
target_link_libraries(st7735 PUBLIC magic63_sdk_headers)   
