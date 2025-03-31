add_subdirectory(eigen)


# polyhedron workarounds
set(cinolib_DIR ${PROJECT_SOURCE_DIR}/extern/polyhedron_kernel/cinolib)
set(CINOLIB_USES_SHEWCHUK_PREDICATES OFF)
find_package(cinolib REQUIRED)
add_library(polyhedron_kernel "polyhedron_kernel/polyhedron_kernel.cpp" "polyhedron_kernel/extendedplane.cpp") 
target_link_libraries(polyhedron_kernel cinolib)
target_include_directories(polyhedron_kernel PUBLIC polyhedron_kernel)
