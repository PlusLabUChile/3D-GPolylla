# file(GLOB GPOLYLLA_SOURCES *.cpp)
message("Building GPolylla")
set(SRCS
        # Algorithms
        cavity_algorithm.cpp
        face_algorithm.cpp

        # Data structures
        face_mesh.cpp
        cavity_mesh.cpp
        mesh.cpp

        # IO formats
        tetgen.cpp
        visf.cpp

        utils.cpp
)

# find_package(Eigen3 3.3 REQUIRED NO_MODULE)
add_library(${PROJECT_NAME} ${SRCS})
# target_compile_definitions(${PROJECT_NAME} PRIVATE GPOLYLLA_LIB)
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} PRIVATE polyhedron_kernel)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(GPolylla::GPolylla ALIAS ${PROJECT_NAME})

# add_executable(${PROJECT_NAME}Run main.cpp)
# target_link_libraries(${PROJECT_NAME}Run ${PROJECT_NAME} polyscope tetgen)
