cmake_minimum_required(VERSION 3.8)
project(sat)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES sat.c clause.h arraylist.c context.c linkedlist.c arraymap.c)
set(SOURCE_FILES ${SOURCE_FILES} arraylist.h literal.h context.h linkedlist.h arraymap.h function.h variable.h variable.c)
add_executable(sat ${SOURCE_FILES})
