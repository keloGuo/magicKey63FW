
aux_source_directory(. DIR_KeyScan_SRCS)

add_library(littleFs ${DIR_KeyScan_SRCS})
target_include_directories(littleFs PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_definitions(littleFs PRIVATE
  LFS_NO_DEBUG
  LFS_NO_WARN
  LFS_NO_ERROR
)
target_link_libraries(littleFs PUBLIC magic63_sdk_headers)    
