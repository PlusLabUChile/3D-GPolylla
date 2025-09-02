cmake_minimum_required(VERSION 3.20)
project(gpolylla)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# file(COPY "${PROJECT_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}")

#add_subdirectory(extern)

add_subdirectory(src)

option(GPOLYLLA_RENDERER "Build renderer" OFF)
if (GPOLYLLA_RENDERER)
    add_subdirectory(src/renderer)
endif()

option(GPOLYLLA_EXE "Build executable" OFF)
if (GPOLYLLA_EXE)
    add_subdirectory(src/exe)
endif()

option(GPOLYLLA_TEST "Build tests" OFF)
if (GPOLYLLA_TEST)
    add_subdirectory(test)
endif ()
