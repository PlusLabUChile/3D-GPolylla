message("-- [GPolylla] Adding GPolylla executable")


add_executable(GPolyllaExe main.cpp)
target_link_libraries(GPolyllaExe PRIVATE GPolylla::gpolylla)

add_executable(GPolyllaExperiment ../../tools/experiment.cpp)
target_link_libraries(GPolyllaExperiment PRIVATE GPolylla::gpolylla)
add_executable(GPolyllaMemory ../../tools/memory_inventory.cpp)
target_link_libraries(GPolyllaMemory PRIVATE GPolylla::gpolylla)
