
aux_source_directory(. DIR_KeyScan_SRCS)

add_library(littleFs ${DIR_KeyScan_SRCS})
target_link_libraries(littleFs PUBLIC pico_stdlib hardware_flash pico_multicore)    