add_library(macroRecorder macroRecorder.c)
target_include_directories(macroRecorder PUBLIC ${CMAKE_CURRENT_LIST_DIR} ../lib/littlefs)
target_link_libraries(macroRecorder PUBLIC magic63_sdk_headers littleFs)
