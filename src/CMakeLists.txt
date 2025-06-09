message("-- [GPolylla] Building")
set(GPOL_SRCS
        mesh.cpp
        tetgen.cpp
)

include(FetchContent)
FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
)

FetchContent_MakeAvailable(Eigen3)

add_library(GPolyllaLib ${GPOL_SRCS})
target_link_libraries(GPolyllaLib PUBLIC Eigen3::Eigen)
add_library(GPolylla::gpolylla ALIAS GPolyllaLib)

#target_compile_definitions(${PROJECT_NAME} PRIVATE GPOLYLLA_LIB)

#add_executable(GPolyllaTest main.cpp)
#target_link_libraries(GPolyllaTest PUBLIC Eigen3::Eigen)
#target_link_libraries(GPolyllaTest PRIVATE GPolyllaLib)