add_library(keymap keymap.c)

target_include_directories(keymap PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(keymap PUBLIC magic63_sdk_headers)
