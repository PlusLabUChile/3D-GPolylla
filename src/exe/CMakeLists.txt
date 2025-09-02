message("-- [GPolylla] Adding GPolylla executable")


add_executable(GPolyllaExe main.cpp)
target_link_libraries(GPolyllaExe PRIVATE GPolylla::gpolylla)

