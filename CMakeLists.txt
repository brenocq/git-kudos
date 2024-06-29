cmake_minimum_required(VERSION 3.10)
project(git-kudos VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp)

add_executable(git-kudos src/main.cpp)
