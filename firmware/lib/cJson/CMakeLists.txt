
aux_source_directory(. DIR_SRCS)

add_library(cjson ${DIR_SRCS})
target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(cjson PUBLIC magic63_sdk_headers)    
