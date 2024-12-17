message("Including executable demo")

add_subdirectory(deps)

set(DEMOS
        polyscope.cpp
        normal.cpp
        #        quality.cpp
)

foreach (DEMO ${DEMOS})
    get_filename_component(DEMO_NAME ${DEMO} NAME_WE)
    add_executable(${PROJECT_NAME}_${DEMO_NAME} ${DEMO})
    target_link_libraries(${PROJECT_NAME}_${DEMO_NAME} GPolylla::GPolylla polyscope #[[qhullcpp polyhedron_kernel_lib]])
    #    target_include_directories(${PROJECT_NAME}_${DEMO_NAME} PRIVATE
    #            ${PROJECT_SOURCE_DIR}/demo/deps/polyhedron_kernel
    #            ${PROJECT_SOURCE_DIR}/demo/deps/qhull/src/libqhullcpp)
    target_compile_definitions(${PROJECT_NAME}_${DEMO_NAME} PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
endforeach ()

