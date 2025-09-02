set(SRC_FILES
    main.cpp
)

find_package(OpenGL REQUIRED)

include(FetchContent)
# GLFW3
FetchContent_Declare(
    GLFW3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(GLFW3)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.2
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
    add_library(imgui_glfw STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    )
    target_link_libraries(imgui_glfw PRIVATE glfw OpenGL::GL)
    target_include_directories(imgui_glfw PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imgui_SOURCE_DIR}/misc/cpp)
endif()

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad
    GIT_TAG        v2.0.6
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_MakeAvailable(glad)
    add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
    glad_add_library(glad REPRODUCIBLE API gl:core=4.6)
endif()

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)


add_executable(GPolyllaRenderer ${SRC_FILES})
target_link_libraries(GPolyllaRenderer PRIVATE GPolylla::gpolylla imgui_glfw glad glfw OpenGL::GL glm)
target_compile_definitions(GPolyllaRenderer PRIVATE DATA_DIR="${PROJECT_SOURCE_DIR}/data/")

# Copy shaders folder to build directory
add_custom_command(TARGET GPolyllaRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:GPolyllaRenderer>/shaders)

add_executable(GPolylla::renderer ALIAS GPolyllaRenderer)