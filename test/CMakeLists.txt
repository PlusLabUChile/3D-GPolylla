message("-- [GPolylla] Adding tests")

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL " " FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

set(GPOL_TEST_SRCS
        mesh_test.cpp
        tetgen_test.cpp
)

add_executable(GPolyllaTests ${GPOL_TEST_SRCS})
target_link_libraries(GPolyllaTests GPolylla::gpolylla GTest::gtest_main)
target_compile_definitions(GPolyllaTests PRIVATE DATA_DIR=" ${PROJECT_SOURCE_DIR}/data/" TEMP_DIR="${PROJECT_SOURCE_DIR}/temp/")

include(GoogleTest)
gtest_discover_tests(GPolyllaTests)
