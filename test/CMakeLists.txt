message("Including all tests")
file(GLOB GPOLYLLA_TEST_SOURCES *.cpp)
add_executable(${PROJECT_NAME}Test ${GPOLYLLA_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest_main)
target_compile_definitions(${PROJECT_NAME}Test PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}Test)



