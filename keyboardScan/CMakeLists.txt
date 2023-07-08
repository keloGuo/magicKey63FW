
aux_source_directory(. DIR_KeyScan_SRCS)

add_library(keyboardScan ${DIR_KeyScan_SRCS})
target_link_libraries(keyboardScan PUBLIC  hardware_pio hardware_dma pico_stdlib)  