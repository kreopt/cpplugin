project(plugload)
cmake_minimum_required(VERSION 2.8)
add_definitions(-std=c++1y)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} main.cpp common.hpp)
add_library("dummy" SHARED plugin.cpp common.hpp)

