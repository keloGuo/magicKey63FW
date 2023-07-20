
aux_source_directory(. DIR_FK64UI_SRCS)

add_library(FK64UI ${DIR_FK64UI_SRCS})
target_link_libraries(FK64UI PUBLIC lvgl  pico_stdlib) 