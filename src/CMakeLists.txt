
if(GPOLYLLA_LIB)
  message("Building GPolylla as library")
  add_library(${PROJECT_NAME} "mesh.cpp")
else()
  message("Building GPolylla")
  add_executable(${PROJECT_NAME} "mesh.cpp" "main.cpp")
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# add_executable(${PROJECT_NAME}Run main.cpp)
# target_link_libraries(${PROJECT_NAME}Run ${PROJECT_NAME} polyscope tetgen)
