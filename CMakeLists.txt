cmake_minimum_required(VERSION 3.0)
project(3D-GPolylla)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_subdirectory(src)
add_executable(gpol main.cpp)
target_link_libraries(gpol GPolylla)
target_include_directories(gpol PUBLIC "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}")
