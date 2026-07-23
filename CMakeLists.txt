cmake_minimum_required(VERSION 3.10)

project(PathTracer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(vendor/FSWindow)

add_executable(PathTracer 
    src/main.cpp
)

target_link_libraries(PathTracer PRIVATE 
    FSWindow pthread
)

if(MSVC)
    target_compile_options(PathTracer PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(PathTracer PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
    )
endif()