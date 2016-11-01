cmake_minimum_required(VERSION 3.6)
project(Simple_IPC_Protocol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    README.md main.c functionalities.c functionalities.h constants.h)

include_directories(.)

add_executable(Simple_IPC_Protocol ${SOURCE_FILES})