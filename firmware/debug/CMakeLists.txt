add_library(debugModule debug.c)

target_include_directories(debugModule PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(debugModule PUBLIC magic63_tinyusb_headers)
