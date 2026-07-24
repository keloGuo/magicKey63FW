add_library(scratch scratch.c)

target_include_directories(scratch PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(scratch PUBLIC magic63_sdk_headers)
