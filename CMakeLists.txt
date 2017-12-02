cmake_minimum_required(VERSION 3.6)
project(Shell)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES
		shell.c binary_tree.c binary_tree.h)

add_executable(shell ${SOURCE_FILES})
