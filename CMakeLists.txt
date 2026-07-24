cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
  set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}" CACHE PATH "Path to the Raspberry Pi Pico SDK")
endif()

set(PICO_BOARD pico CACHE STRING "Target board")
include(firmware/zCmakeFile/pico_sdk_import.cmake)

project(firmware C CXX ASM)

pico_sdk_init()

add_subdirectory(firmware)
