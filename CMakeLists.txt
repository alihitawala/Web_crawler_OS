cmake_minimum_required(VERSION 3.3)
project(p4a)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES
    web_tester.c)

add_executable(p4a ${SOURCE_FILES})