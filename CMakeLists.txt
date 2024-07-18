cmake_minimum_required(VERSION 3.28)
project(Playground)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(Playground main.cpp
)

target_link_libraries(Playground sfml-graphics)
