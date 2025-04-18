message("Including all tests")
add_subdirectory(extern)
# find_package(GTest REQUIRED)
# file(GLOB GPOLYLLA_TEST_SOURCES *.cpp)

set(TEST_SRCS
        cavity_algorithm_test.cpp
        face_mesh_test.cpp
        mesh_test.cpp
        tetgen_test.cpp
        visf_test.cpp
        utils_test.cpp
)

add_executable(${PROJECT_NAME}_tests ${TEST_SRCS})
target_link_libraries(${PROJECT_NAME}_tests GPolylla::GPolylla GTest::gtest GTest::gtest_main)
target_compile_definitions(${PROJECT_NAME}_tests PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)



