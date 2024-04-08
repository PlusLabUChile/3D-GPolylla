set(TEST_SOURCES
    mesh_test.cc
)
add_executable(${PROJECT_NAME}Test ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest_main)
target_compile_definitions(${PROJECT_NAME}Test PRIVATE _DATA_DIR="${PROJECT_SOURCE_DIR}/data")
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}Test)



