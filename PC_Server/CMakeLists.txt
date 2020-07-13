cmake_minimum_required(VERSION 3.10)
project(pc_server VERSION 1.0)
configure_file(serverConfig.h.in serverConfig.h)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#include_directories(include build)

add_executable(pc_server serv.cpp)


target_include_directories(pc_server  PUBLIC  "${PROJECT_BINARY_DIR}"  )

