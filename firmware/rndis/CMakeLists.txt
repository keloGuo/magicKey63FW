
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

# Mongoose build flags
add_definitions(-DMG_ENABLE_TCPIP=1)
add_definitions(-DMG_ENABLE_PACKED_FS=1)
add_definitions(-DMG_ENABLE_FILE=0)
add_definitions(-DDISABLE_ROUTING=1)

# Example build options
add_definitions(-DHTTP_URL="http://0.0.0.0/")

include_directories(../ ${CMAKE_CURRENT_LIST_DIR}/ ${MAGIC63_PACKED_GENERATED_DIR} ../usb ../lib/cJson ../lib/littlefs)
target_link_libraries(rndis PUBLIC magic63_sdk_headers cjson littleFs)
