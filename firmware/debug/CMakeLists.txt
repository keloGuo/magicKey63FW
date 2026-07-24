add_library(debugModule debug.c)

target_include_directories(debugModule PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${MAGIC63_FIRMWARE_DIR}/usb
)
target_link_libraries(debugModule PUBLIC magic63_sdk_headers)
