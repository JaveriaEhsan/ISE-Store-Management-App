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

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

# Every .cpp in the project, except build output and tests
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS "*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "^(build[^/]*|out|tests|\\.vs|cmake-build[^/]*)/")

add_executable(store_app ${SOURCES})

target_link_libraries(store_app ${OpenCV_LIBS} Qt6::Widgets)