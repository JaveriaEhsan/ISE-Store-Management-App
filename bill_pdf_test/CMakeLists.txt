cmake_minimum_required(VERSION 3.16)
project(bill_pdf_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

add_executable(test_billpdf
    test_billpdf.cpp
    ../services/BillPdfExporter.cpp
)
target_include_directories(test_billpdf PRIVATE ..)
target_link_libraries(test_billpdf PRIVATE Qt6::Core Qt6::Gui)
