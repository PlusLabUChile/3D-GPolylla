// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context
// creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <cmath>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

#define GL_SILENCE_DEPRECATION
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <glad/gl.h>

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Camera class for handling movement and view
class Camera
{
  public:
    Eigen::Vector3f position;
    Eigen::Vector3f front;
    Eigen::Vector3f up;
    Eigen::Vector3f right;
    Eigen::Vector3f worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    Camera(Eigen::Vector3f pos = Eigen::Vector3f(0.0f, 0.0f, 3.0f),
           Eigen::Vector3f worldUpVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f), float yawAngle = -90.0f,
           float pitchAngle = 0.0f)
        : position(pos), worldUp(worldUpVec), yaw(yawAngle), pitch(pitchAngle), movementSpeed(2.5f),
          mouseSensitivity(0.1f)
    {
        updateCameraVectors();
    }

    Eigen::Matrix4f getViewMatrix()
    {
        Eigen::Vector3f f = front.normalized();
        Eigen::Vector3f s = f.cross(up).normalized();
        Eigen::Vector3f u = s.cross(f);

        Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
        view(0, 0) = s.x();
        view(1, 0) = s.y();
        view(2, 0) = s.z();
        view(0, 1) = u.x();
        view(1, 1) = u.y();
        view(2, 1) = u.z();
        view(0, 2) = -f.x();
        view(1, 2) = -f.y();
        view(2, 2) = -f.z();
        view(0, 3) = -s.dot(position);
        view(1, 3) = -u.dot(position);
        view(2, 3) = f.dot(position);

        return view;
    }

    void processKeyboard(int direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == 0) // Forward (W)
            position += front * velocity;
        if (direction == 1) // Backward (S)
            position -= front * velocity;
        if (direction == 2) // Left (A)
            position -= right * velocity;
        if (direction == 3) // Right (D)
            position += right * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch)
        {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        updateCameraVectors();
    }

  private:
    void updateCameraVectors()
    {
        Eigen::Vector3f frontVec;
        frontVec.x() = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
        frontVec.y() = sin(pitch * M_PI / 180.0f);
        frontVec.z() = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
        front = frontVec.normalized();

        right = front.cross(worldUp).normalized();
        up = right.cross(front).normalized();
    }
};

// Global variables for input handling
Camera camera(Eigen::Vector3f(3.0f, 3.0f, 3.0f));
bool keys[1024] = {false};
bool firstMouse = true;
float lastX = 640.0f;
float lastY = 400.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool mouseCaptured = true;

// Input callbacks
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    // Handle our custom keys first
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return; // Don't process other keys when exiting
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
            // Reset mouse position to center when capturing
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            lastX = width / 2.0f;
            lastY = height / 2.0f;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        return; // Don't process TAB for ImGui
    }

    // Only handle camera movement keys if ImGui doesn't want keyboard input
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard)
    {
        if (key >= 0 && key < 1024)
        {
            if (action == GLFW_PRESS)
                keys[key] = true;
            else if (action == GLFW_RELEASE)
                keys[key] = false;
        }
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();

    // Only handle mouse movement for camera if ImGui doesn't want it and mouse is captured
    if (!io.WantCaptureMouse && mouseCaptured)
    {
        if (firstMouse)
        {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top

        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        camera.processMouseMovement(xoffset, yoffset);
    }
}

void processInput()
{
    ImGuiIO &io = ImGui::GetIO();

    // Only process camera movement if ImGui doesn't want keyboard input
    if (!io.WantCaptureKeyboard)
    {
        if (keys[GLFW_KEY_W])
            camera.processKeyboard(0, deltaTime);
        if (keys[GLFW_KEY_S])
            camera.processKeyboard(1, deltaTime);
        if (keys[GLFW_KEY_A])
            camera.processKeyboard(2, deltaTime);
        if (keys[GLFW_KEY_D])
            camera.processKeyboard(3, deltaTime);
    }
}

// Shader utilities
std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(const std::string &source, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram(const std::string &vertexPath, const std::string &fragmentPath)
{
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty())
    {
        return 0;
    }

    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Matrix utility functions using Eigen
Eigen::Matrix4f createPerspectiveMatrix(float fov, float aspect, float near, float far)
{
    Eigen::Matrix4f perspective = Eigen::Matrix4f::Zero();
    float tanHalfFov = tan(fov / 2.0f);

    perspective(0, 0) = 1.0f / (aspect * tanHalfFov);
    perspective(1, 1) = 1.0f / tanHalfFov;
    perspective(2, 2) = -(far + near) / (far - near);
    perspective(2, 3) = -(2.0f * far * near) / (far - near);
    perspective(3, 2) = -1.0f;

    return perspective;
}

Eigen::Matrix4f createViewMatrix(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up)
{
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view(0, 0) = s.x();
    view(1, 0) = s.y();
    view(2, 0) = s.z();
    view(0, 1) = u.x();
    view(1, 1) = u.y();
    view(2, 1) = u.z();
    view(0, 2) = -f.x();
    view(1, 2) = -f.y();
    view(2, 2) = -f.z();
    view(0, 3) = -s.dot(eye);
    view(1, 3) = -u.dot(eye);
    view(2, 3) = f.dot(eye);

    return view;
}

// Cube geometry data
struct CubeData
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    CubeData()
    {
        // Cube vertices with positions and normals (6 floats per vertex: x,y,z,nx,ny,nz)
        vertices = {
            // Front face
            -0.5f,
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            0.5f,
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            0.5f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            -0.5f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,

            // Back face
            -0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            0.5f,
            0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            -0.5f,
            0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,

            // Left face
            -0.5f,
            -0.5f,
            -0.5f,
            -1.0f,
            0.0f,
            0.0f,
            -0.5f,
            -0.5f,
            0.5f,
            -1.0f,
            0.0f,
            0.0f,
            -0.5f,
            0.5f,
            0.5f,
            -1.0f,
            0.0f,
            0.0f,
            -0.5f,
            0.5f,
            -0.5f,
            -1.0f,
            0.0f,
            0.0f,

            // Right face
            0.5f,
            -0.5f,
            -0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.5f,
            -0.5f,
            0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.5f,
            0.5f,
            0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.5f,
            0.5f,
            -0.5f,
            1.0f,
            0.0f,
            0.0f,

            // Top face
            -0.5f,
            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,
            -0.5f,
            0.5f,
            0.5f,
            0.0f,
            1.0f,
            0.0f,
            0.5f,
            0.5f,
            0.5f,
            0.0f,
            1.0f,
            0.0f,
            0.5f,
            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,

            // Bottom face
            -0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            -1.0f,
            0.0f,
            -0.5f,
            -0.5f,
            0.5f,
            0.0f,
            -1.0f,
            0.0f,
            0.5f,
            -0.5f,
            0.5f,
            0.0f,
            -1.0f,
            0.0f,
            0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            -1.0f,
            0.0f,
        };

        // Indices for the cube (2 triangles per face)
        indices = {// Front face
                   0, 1, 2, 2, 3, 0,
                   // Back face
                   4, 6, 5, 6, 4, 7,
                   // Left face
                   8, 9, 10, 10, 11, 8,
                   // Right face
                   12, 14, 13, 14, 12, 15,
                   // Top face
                   16, 17, 18, 18, 19, 16,
                   // Bottom face
                   20, 22, 21, 22, 20, 23};
    }
};

// Main code
int main(int, char **)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow *window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                                          "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
                                     // changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
                                     // unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Let ImGui install its own callbacks
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set additional callback for our custom key handling
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Capture mouse cursor for camera movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize mouse position variables
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    lastX = windowWidth / 2.0f;
    lastY = windowHeight / 2.0f;

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your
    // application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale
    // better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See
    // Makefile.emscripten for details.
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);

    // Setup cube
    CubeData cube;
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cube.vertices.size() * sizeof(float), cube.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indices.size() * sizeof(unsigned int), cube.indices.data(),
                 GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create shader program using basic shaders
    GLuint shaderProgram = createShaderProgram("src/renderer/shaders/basic.vs", "src/renderer/shaders/basic.fs");
    if (shaderProgram == 0)
    {
        std::cerr << "Failed to create shader program" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up lighting parameters
    Eigen::Vector3f lightPos(2.0f, 2.0f, 2.0f);
    Eigen::Vector3f lightColor(1.0f, 1.0f, 1.0f);
    Eigen::Vector3f objectColor(1.0f, 0.5f, 0.31f);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
        // inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
        // clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
        // clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
        // hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Process input
        processInput();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            ImGui::Separator();
            ImGui::Text("Camera Controls:");
            ImGui::Text("WASD - Move camera");
            ImGui::Text("Mouse - Look around (when captured)");
            ImGui::Text("TAB - Toggle mouse capture");
            ImGui::Text("ESC - Close window");
            ImGui::Text("Mouse captured: %s", mouseCaptured ? "Yes" : "No");
            ImGui::Text("Camera position: (%.1f, %.1f, %.1f)", camera.position.x(), camera.position.y(),
                        camera.position.z());

            if (ImGui::Button(
                    "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window",
                         &show_another_window); // Pass a pointer to our bool variable (the window will have a closing
                                                // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate rotation matrix
        float time = static_cast<float>(glfwGetTime());
        Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
        Eigen::AngleAxisf rotation(time * 0.5f, Eigen::Vector3f(0.5f, 1.0f, 0.0f).normalized());
        model.block<3, 3>(0, 0) = rotation.toRotationMatrix();

        // Create view and projection matrices
        Eigen::Matrix4f view = camera.getViewMatrix();
        float aspect = static_cast<float>(display_w) / static_cast<float>(display_h);
        Eigen::Matrix4f projection = createPerspectiveMatrix(45.0f * M_PI / 180.0f, aspect, 0.1f, 100.0f);

        // Render cube
        glUseProgram(shaderProgram);

        // Set uniforms
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data());
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data());
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data());
        glUniform3fv(lightPosLoc, 1, lightPos.data());
        glUniform3fv(viewPosLoc, 1, camera.position.data());
        glUniform3fv(lightColorLoc, 1, lightColor.data());
        glUniform3fv(objectColorLoc, 1, objectColor.data());

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cube.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
