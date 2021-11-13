
cmake_minimum_required(VERSION 3.9)

set(CMAKE_TOOLCHAIN_FILE /opt/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

project(
    test
    VERSION 0.1
    LANGUAGES C CXX
)

set(SRC
    test.cpp
)

add_subdirectory(Just)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} JustThreadPool)
