message("Including executable demo")

add_subdirectory(deps)

add_executable(${PROJECT_NAME}_demo main.cpp)
target_link_libraries(${PROJECT_NAME}_demo GPolylla::GPolylla polyscope)
target_compile_definitions(${PROJECT_NAME}_demo PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
