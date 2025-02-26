#Set CMake version
cmake_minimum_required(VERSION 3.12)

#Include the Pico-SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

#Set languages and standards for the project
project(amtr2_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Initialize the Pico-SDK
pico_sdk_init()

#Link to the Micro-ROS directory
link_directories($ENV{MICRO_ROS_PICO_SDK_PATH}/libmicroros)

#Compiled executable
add_executable(amtr2_pico
    src/main_server.c
    src/amtr2_communicator.c
    src/pico_w_interface.c
    $ENV{MICRO_ROS_PICO_SDK_PATH}/pico_uart_transport.c
)

#Link Micro-ROS and Pico-SDK libraries
target_link_libraries(amtr2_pico
    pico_stdlib
    $ENV{MICRO_ROS_PICO_SDK_PATH}/libmicroros/microros.a
)

#Link to Micro-ROS include files
target_include_directories(amtr2_pico PUBLIC
    $ENV{MICRO_ROS_PICO_SDK_PATH}/libmicroros/include
    ./include
)

#Set C and C++ compile flags
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

#Configure Pico Peripherals - Turn on USB, Turn off UART
pico_enable_stdio_usb(amtr2_pico 1)
pico_enable_stdio_uart(amtr2_pico 0)

#Turn off carriage-return and line-feed characters (Needed for Micro-ROS)
add_compile_definitions(PICO_UART_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_DEFAULT_CRLF=0)

#Generate UF2 and other output files for debugging
pico_add_extra_outputs(amtr2_pico)
