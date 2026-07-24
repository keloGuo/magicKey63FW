add_library(apm apmCount.c)

target_include_directories(apm PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(apm PUBLIC magic63_sdk_headers)
