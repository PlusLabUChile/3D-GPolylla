message("-- [GPolylla] Adding examples")
include(FetchContent)

# Fetch raylib
set(BUILD_EXAMPLES ON)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)

# Add the GPolyllaVisualizador example executable
add_executable(GPolyllaVisualizador visualizador.cpp)

# Link raylib to the example
target_link_libraries(GPolyllaVisualizador PRIVATE raylib GPolylla::gpolylla)

target_compile_definitions(GPolyllaVisualizador PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

