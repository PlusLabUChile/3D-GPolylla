add_library(${PROJECT_NAME} "mesh.cc")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# add_executable(${PROJECT_NAME}Run main.cpp)
# target_link_libraries(${PROJECT_NAME}Run ${PROJECT_NAME} polyscope tetgen)
