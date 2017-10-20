cmake_minimum_required(VERSION 3.8)
project(sat)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES sat.c clause.h arraylist.c context.c linkedlist.c arraylist.h literal.h context.h linkedlist.h)
add_executable(sat ${SOURCE_FILES})
