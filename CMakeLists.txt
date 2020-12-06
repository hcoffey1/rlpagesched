cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Project Name
project(rlpagesched VERSION 1.0)

#Executable
add_executable(pagesim ./src/pageSim.c)

target_include_directories(pagesim PRIVATE ./inc)