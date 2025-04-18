message("Building GPolylla")
set(GPOL_SRCS
        # Algorithms
        cavity_algorithm.cpp

        # Data structures
        cavity_mesh.cpp

        # IO formats
        tetgen_reader.cpp
        visf_writer.cpp

        # Utils
        math.cpp
        utils.cpp
)

# find_package(Eigen3 3.3 REQUIRED NO_MODULE)
add_library(GPolyllaLib ${GPOL_SRCS})
target_link_libraries(GPolyllaLib PUBLIC Eigen3::Eigen)
target_link_libraries(GPolyllaLib PRIVATE polyhedron_kernel)
target_include_directories(GPolyllaLib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(GPolylla::gpolylla ALIAS GPolyllaLib)
# target_compile_definitions(${PROJECT_NAME} PRIVATE GPOLYLLA_LIB)

