cmake_minimum_required(VERSION 3.16)
project(ISE_Store_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Lets every file write #include "shared.h" and #include "barcode/encoder.h"
include_directories(${CMAKE_SOURCE_DIR})

if(MSVC)
    add_compile_options(/utf-8)
endif()

# OpenCV: set the OpenCV_DIR environment variable to the folder
# that contains OpenCVConfig.cmake (for example C:/opencv/build)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Picks up every .cpp in these folders, so nobody edits this file when adding files
file(GLOB SOURCES CONFIGURE_DEPENDS
    main.cpp
    barcode/*.cpp
    camera/*.cpp
    models/*.cpp
    data/*.cpp
    services/*.cpp
    gui/*.cpp
)

add_executable(store_app ${SOURCES})

target_link_libraries(store_app ${OpenCV_LIBS} Qt6::Widgets)