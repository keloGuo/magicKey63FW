
aux_source_directory(. DIR_SRCS)

add_library(cjson ${DIR_SRCS})
target_link_libraries(cjson PUBLIC magic63_sdk_headers)    
