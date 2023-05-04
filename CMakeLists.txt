cmake_minimum_required(VERSION 3.12)

# build functions from pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(framework C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(${PROJECT_NAME})

# Disable SDK alarm support for this framework
target_compile_definitions(${PROJECT_NAME} PRIVATE
        PICO_DISABLE_SHARED_IRQ_HANDLERS=1
        PICO_TIME_DEFAULT_ALARM_POOL_DISABLED=1)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/)

target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c       
        ${CMAKE_CURRENT_LIST_DIR}/src/timer_functions.c
        ${CMAKE_CURRENT_LIST_DIR}/src/schedules.c
        ${CMAKE_CURRENT_LIST_DIR}/src/task1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/task2.c
        ${CMAKE_CURRENT_LIST_DIR}/src/task3.c
        ${CMAKE_CURRENT_LIST_DIR}/src/task4.c
        )
# pull in common dependencies = library
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_structs hardware_regs pico_multicore)

# LINK with a custom linker script
set_target_properties(${PROJECT_NAME} PROPERTIES PICO_TARGET_LINKER_SCRIPT 
    ${CMAKE_SOURCE_DIR}/linker_script.ld)

# pico_enable_stdio_usb(${PROJECT_NAME} 0)
# pico_enable_stdio_uart(${PROJECT_NAME} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# pico_set_float_implementation(${PROJECT_NAME} compiler)
# pico_set_double_implementation(${PROJECT_NAME} compiler)

# add url via pico_set_program_url
# pico_set_program_url(${PROJECT_NAME})
