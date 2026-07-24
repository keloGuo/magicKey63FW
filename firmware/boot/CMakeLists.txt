add_library(boot boot.c)

target_include_directories(boot PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(boot PUBLIC magic63_sdk_headers)
