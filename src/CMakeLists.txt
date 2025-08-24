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

add_library(GPolyllaLib ${GPOL_SRCS})
target_link_libraries(GPolyllaLib PUBLIC Eigen3::Eigen)
target_include_directories(GPolyllaLib PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_library(GPolylla::gpolylla ALIAS GPolyllaLib)

#target_compile_definitions(${PROJECT_NAME} PRIVATE GPOLYLLA_LIB)

#add_executable(GPolyllaTest main.cpp)
#target_link_libraries(GPolyllaTest PUBLIC Eigen3::Eigen)
#target_link_libraries(GPolyllaTest PRIVATE GPolyllaLib)