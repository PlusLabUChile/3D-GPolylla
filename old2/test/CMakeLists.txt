message("Including GPolylla tests")

#include(CTest)
add_subdirectory(extern)
enable_testing()

set(GPOL_TEST_SRCS
        io_test.cpp

)

add_executable(GPolyllaTests ${GPOL_TEST_SRCS})
target_link_libraries(GPolyllaTests GPolylla::gpolylla GTest::gtest_main)
target_compile_definitions(GPolyllaTests PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/" TEMP_DIR="${PROJECT_SOURCE_DIR}/temp/")

include(GoogleTest)
gtest_discover_tests(GPolyllaTests)