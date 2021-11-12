
cmake_minimum_required(VERSION 3.9)

project(
    JustThreadPool
    VERSION 0.1
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC
    JustThreadPool.h
    JustThreadPool.cpp
    JustConcurrentQueue.hpp
)

add_library(${PROJECT_NAME} ${SRC})
