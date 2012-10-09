cmake_minimum_required(VERSION 2.8)
project(search)

set(CMAKE_CXX_COMPILER "g++-4.7")
set(CMAKE_CXX_FLAGS "-std=c++0x -pipe -march=native -Wall -Wextra -fno-rtti")
set(CMAKE_C_FLAGS "-std=c99 -pipe -march=native -Wall -Wextra")

add_subdirectory(examples)
