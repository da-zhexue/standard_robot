cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set_source_files_properties(
        Core/Src/stm32f4xx_it.c
        bsp/dwt/bsp_dwt.c
        PROPERTIES LANGUAGE CXX
)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME standard_robot)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}

)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        bsp/uart/bsp_uart.cpp
        bsp/can/bsp_can.cpp
        bsp/algorithm/crc.c
        bsp/algorithm/user_lib.c
        bsp/bsp_init.cpp
        module/motor/dji/M3508.cpp
        module/upc/upc.cpp
        app/comm.cpp
        app/test.cpp
        bsp/online_detect/onl_det.cpp
        bsp/dtm/dtm.cpp
        bsp/ulog/ulog.c
        app/task_init.c
        bsp/dwt/bsp_dwt.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        bsp
        module
        app
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
        _GLIBCXX_USE_CXX11_ABI=0
        USART1_DOUBLE_BUFFER_ENABLE
        USART3_DOUBLE_BUFFER_ENABLE
        USART6_DOUBLE_BUFFER_ENABLE
        LOG_ENABLE
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
list(REMOVE_ITEM CMAKE_CXX_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
        stdc++
    # Add user defined libraries
)
