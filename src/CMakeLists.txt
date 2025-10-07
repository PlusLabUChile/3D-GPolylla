message("-- [GPolylla] Building")
set(GPOL_SRCS
        mesh.cpp
        reader.cpp
        neighbours.cpp
        writer.cpp

        # Extras
        utils.h
        utils.cpp
        logger.h
        logger.cpp
        cavity.cpp
        stat.cpp

        ../include/gpolylla/polylla.h
        ../include/gpolylla/stat.h
)

include(FetchContent)
FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
)

set(BUILD_TESTING OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)
FetchContent_MakeAvailable(Eigen3)

FetchContent_Declare(
        qhull
        GIT_REPOSITORY https://github.com/akuukka/quickhull.git
        GIT_TAG master
)
FetchContent_GetProperties(qhull)
if (NOT qhull_POPULATED)
    FetchContent_MakeAvailable(qhull)
    add_library(quickhull STATIC ${qhull_SOURCE_DIR}/QuickHull.cpp)
    target_include_directories(quickhull PUBLIC ${qhull_SOURCE_DIR})
endif()

FetchContent_Declare(
        poly_kernel
        GIT_REPOSITORY https://github.com/TommasoSorgente/polyhedron_kernel.git
        GIT_TAG main
)
FetchContent_GetProperties(poly_kernel)
if (NOT poly_kernel_POPULATED)
    FetchContent_MakeAvailable(poly_kernel)

    add_library(polyhedron_kernel_lib INTERFACE)
    target_include_directories(polyhedron_kernel_lib INTERFACE ${poly_kernel_SOURCE_DIR})

#    set(cinolib_DIR ${poly_kernel_SOURCE_DIR}/cinolib)
#    set(CINOLIB_USES_SHEWCHUK_PREDICATES OFF)
#    find_package(cinolib REQUIRED)
    target_link_libraries(polyhedron_kernel_lib INTERFACE cinolib)
endif()

add_library(GPolyllaLib ${GPOL_SRCS})
target_link_libraries(GPolyllaLib PUBLIC Eigen3::Eigen quickhull polyhedron_kernel_lib)
target_include_directories(GPolyllaLib PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_library(GPolylla::gpolylla ALIAS GPolyllaLib)

#target_compile_definitions(${PROJECT_NAME} PRIVATE GPOLYLLA_LIB)

#add_executable(GPolyllaTest main.cpp)
#target_link_libraries(GPolyllaTest PUBLIC Eigen3::Eigen)
#target_link_libraries(GPolyllaTest PRIVATE GPolyllaLib)