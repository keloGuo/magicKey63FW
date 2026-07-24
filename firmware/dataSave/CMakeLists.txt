add_library(dataSave dataSaveModel.c)

target_include_directories(dataSave PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${MAGIC63_FIRMWARE_DIR}/lib/littlefs
)
target_link_libraries(dataSave PUBLIC magic63_sdk_headers littleFs)
