cmake_minimum_required(VERSION 3.0)
project(3D-GPolylla)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_subdirectory(extern)
add_subdirectory(src)
# target_include_directories(gpol PUBLIC "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}")
