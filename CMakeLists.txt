cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Simple-Scales)

add_executable(Scales main.cpp applicationmanager.hpp applicationmanager.cpp constants.hpp scalemanager.hpp scalemanager.cpp musiclibrary.hpp musiclibrary.cpp)