
cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

#target_link_libraries(${PROJECT_NAME} JustThreadPool)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::concurrentqueue::concurrentqueue JustThreadPool pthread)


