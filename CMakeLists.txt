cmake_minimum_required(VERSION 3.5)
project(simple_shell C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(simple_shell
        src/main.c
        src/simpleShell.c
        src/simpleShell.h)
