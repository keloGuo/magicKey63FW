
aux_source_directory(. DIR_UI_SRCS)

add_library(UI ${DIR_UI_SRCS})
target_link_libraries(UI PUBLIC lvgl  pico_stdlib pico_multicore)      