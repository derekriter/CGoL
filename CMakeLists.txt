cmake_minimum_required(VERSION 3.28)
project(CGoL)

set(CMAKE_CXX_STANDARD 17)

add_executable(CGoL
    src/main.cpp
    src/include/tigr.h
    src/tigr.c
    src/include/button.hpp
    src/button.cpp
)
target_include_directories(CGoL PRIVATE src/include)
target_link_libraries(CGoL GLU GL X11) #required by TIGR
