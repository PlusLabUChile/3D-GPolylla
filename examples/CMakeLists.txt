message("-- [GPolylla] Adding examples")


function(add_example CPP_FILE EXEC_NAME LIBRARIES)
    add_executable(${EXEC_NAME} ${CPP_FILE})
    target_link_libraries(${EXEC_NAME} ${LIBRARIES})
    target_compile_definitions(${EXEC_NAME} PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
endfunction()


#add_example(simple.cpp simple "GPolylla::gpolylla")
#add_example(quality.cpp quality "GPolylla::gpolylla;polyhedron_kernel")

