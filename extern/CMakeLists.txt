add_subdirectory(eigen)
# add_subdirectory(polyhedron_kernel)


# polyhedron workarounds
add_library(polyhedron_kernel INTERFACE "polyhedron_kernel/polyhedron_kernel.h") 
set(cinolib_DIR ${PROJECT_SOURCE_DIR}/extern/polyhedron_kernel/cinolib)
set(CINOLIB_USES_SHEWCHUK_PREDICATES OFF)
find_package(cinolib REQUIRED)
target_link_libraries(polyhedron_kernel INTERFACE cinolib)
# target_link_directories()
# target_include_directories(polyhedron_kernel INTERFACE polyhedron_kernel)
# target_compile_definitions(polyhedron_kernel PRIVATE CINO_STATIC_LIB)
