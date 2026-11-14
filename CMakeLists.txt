cmake_minimum_required(VERSION 3.15)
project(ISE_Store_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lets every file write #include "shared.h" and #include "barcode/encoder.h"
include_directories(${CMAKE_SOURCE_DIR})

# Visual Studio: read source files as UTF-8
if(MSVC)
    add_compile_options(/utf-8)
endif()

# OpenCV: set the OpenCV_DIR environment variable to the folder
# that contains OpenCVConfig.cmake (for example C:/opencv/build)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(store_app
    main.cpp
    barcode/encoder.cpp
    barcode/drawer.cpp
    barcode/reader.cpp
    barcode/decoder.cpp
    barcode/camera.cpp
)

target_link_libraries(store_app ${OpenCV_LIBS})