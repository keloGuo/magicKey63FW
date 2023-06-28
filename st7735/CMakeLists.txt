
aux_source_directory(. DIR_st7735_SRCS)

add_library(st7735 ${DIR_st7735_SRCS})
target_link_libraries(st7735 PUBLIC  hardware_spi hardware_dma pico_stdlib) 