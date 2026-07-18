add_library(dataSave dataSaveModel.c)

target_include_directories(dataSave PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_SOURCE_DIR}/lib/littlefs
)
target_link_libraries(dataSave PUBLIC pico_stdlib littleFs)
