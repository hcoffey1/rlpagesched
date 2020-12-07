cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")

#Project Name
project(rlpagesched VERSION 1.0)

#Executable
add_executable(pagesim ./src/pageSim.c ./src/rl.c ./src/CommandLine.c)

target_include_directories(pagesim PRIVATE ./inc)
target_compile_options(pagesim PRIVATE -g)
