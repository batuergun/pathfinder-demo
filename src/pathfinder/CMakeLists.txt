cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(pathfinder C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(pathfinder
    pathfinder.c
)
pico_enable_stdio_usb(pathfinder 1)
pico_enable_stdio_uart(pathfinder 1)

target_link_libraries(pathfinder pico_stdlib hardware_pwm hardware_i2c)

pico_add_extra_outputs(pathfinder) 