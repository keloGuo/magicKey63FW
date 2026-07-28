
aux_source_directory(. DIR_RNDIS_SRCS)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(MAGIC63_UPLOAD_PAGE_HTML ${CMAKE_SOURCE_DIR}/web/webUpdte/index.html)
set(MAGIC63_PACKED_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(MAGIC63_PACKED_UPDATE_PAGE_C ${MAGIC63_PACKED_GENERATED_DIR}/packed_update_page.c)
set(MAGIC63_PACKED_UPDATE_PAGE_H ${MAGIC63_PACKED_GENERATED_DIR}/packed_update_page.h)

add_custom_command(
  OUTPUT
    ${MAGIC63_PACKED_UPDATE_PAGE_C}
    ${MAGIC63_PACKED_UPDATE_PAGE_H}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAGIC63_PACKED_GENERATED_DIR}
  COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/tool/pack_upload_page.py
    --input ${MAGIC63_UPLOAD_PAGE_HTML}
    --output-c ${MAGIC63_PACKED_UPDATE_PAGE_C}
    --output-h ${MAGIC63_PACKED_UPDATE_PAGE_H}
    --symbol magic63_update_page_html
  DEPENDS
    ${MAGIC63_UPLOAD_PAGE_HTML}
    ${CMAKE_SOURCE_DIR}/tool/pack_upload_page.py
  VERBATIM
)

list(APPEND DIR_RNDIS_SRCS ${MAGIC63_PACKED_UPDATE_PAGE_C})
set_source_files_properties(${MAGIC63_PACKED_UPDATE_PAGE_C} PROPERTIES GENERATED TRUE)

add_library(rndis ${DIR_RNDIS_SRCS})

target_include_directories(rndis PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${MAGIC63_PACKED_GENERATED_DIR}
  ${MAGIC63_FIRMWARE_DIR}
)
target_compile_definitions(rndis PRIVATE
  MG_ENABLE_TCPIP=1
  MG_ENABLE_PACKED_FS=1
  MG_ENABLE_FILE=0
  DISABLE_ROUTING=1
  HTTP_URL="http://0.0.0.0/"
)
target_link_libraries(rndis PUBLIC
  magic63_tinyusb_headers
  cjson
  littleFs
  keyboardScan
  pico_unique_id_headers
)
