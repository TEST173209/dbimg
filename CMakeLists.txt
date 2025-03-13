cmake_minimum_required(VERSION 3.20)
project(dbimg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找OpenCV4依find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc imgcodecs highgui)

add_executable(dbimg
    argparse/argparse.hpp
    main.cpp
    img.cpp
    img.hpp
)

target_include_directories(dbimg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(dbimg PRIVATE
    ${OpenCV_LIBS}
)

install(TARGETS dbimg DESTINATION bin)
