cmake_minimum_required(VERSION 3.10)
project(mandelbrot C)

set(CMAKE_C_STANDARD 99)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

find_package (Threads)

add_executable(mandelbrot main.c window.h)
target_link_libraries (mandelbrot ${CMAKE_THREAD_LIBS_INIT})