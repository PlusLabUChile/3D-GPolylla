add_library(GPolylla "mesh.cpp" "face.cpp")
target_include_directories(GPolylla PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(gpol main.cpp)
target_link_libraries(gpol GPolylla polyscope tetgen)
