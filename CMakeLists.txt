
cmake_minimum_required(VERSION 3.9)

set(CMAKE_TOOLCHAIN_FILE E:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
    JustThreadPool
    VERSION 0.1
    LANGUAGES C CXX
)

set(SRC
    JustThreadPool.h
    JustThreadPool.cpp

    test.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::concurrentqueue::concurrentqueue)
