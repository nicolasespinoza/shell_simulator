cmake_minimum_required(VERSION 3.20)
project(cpe357_assignment2 C)

set(CMAKE_C_STANDARD 99)

add_executable(cpe357_assignment2 util/array_list.c util/toolkit.c emulator.c virtual_file_system.c main.c)
