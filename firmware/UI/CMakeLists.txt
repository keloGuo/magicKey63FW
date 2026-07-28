
aux_source_directory(. DIR_UI_SRCS)
list(APPEND DIR_UI_SRCS magic63UIBounceDiagPage.c)

add_library(UI ${DIR_UI_SRCS})
target_include_directories(UI PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(UI PUBLIC lvgl magic63_sdk_headers scratch keyboardScan macroRecorder)
