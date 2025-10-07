// Dear ImGui: Application with full layout - navbar, viewport, sidebar, and docking
// Features:
// - Dockable windows with ImGui docking branch
// - Main viewport for texture rendering
// - Navigation bar with menu items
// - Sidebar with tools and properties
// - Demo window for ImGui examples
#include "imgui_internal.h"
#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <stdio.h>
#include <unordered_map>
#include <unordered_set>

#include <gpolylla/polylla.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EPSILON 0.00001f

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Forward declarations for mouse callbacks - will be defined after app::State
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

glm::vec3 toVec3(const Polylla::Vertex &vertex)
{
    return glm::vec3(vertex.x(), vertex.y(), vertex.z());
}

namespace render
{

struct Camera
{
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Orbit camera parameters
    float distance = 5.0f;
    float azimuth = 0.0f;   // horizontal rotation (around Y axis)
    float elevation = 0.3f; // vertical rotation (up/down)

    // Camera constraints
    float minDistance = 1.0f;
    float maxDistance = 50.0f;
    float minElevation = -1.5f; // about -85 degrees
    float maxElevation = 1.5f;  // about +85 degrees

    glm::mat4 view()
    {
        return glm::lookAt(position, target, up);
    }

    void update()
    {
        // Calculate camera position using spherical coordinates
        float x = target.x + distance * cos(elevation) * cos(azimuth);
        float y = target.y + distance * sin(elevation);
        float z = target.z + distance * cos(elevation) * sin(azimuth);

        position = glm::vec3(x, y, z);
    }
};

struct Shader
{
    GLuint program = 0;
    std::unordered_map<std::string, GLint> locations;

    void init(const char *vertexShaderSource, const char *fragmentShaderSource)
    {
        GLuint vertexShader = compileSource(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileSource(fragmentShaderSource, GL_FRAGMENT_SHADER);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return;
        }

        GLuint aProgram = glCreateProgram();
        glAttachShader(aProgram, vertexShader);
        glAttachShader(aProgram, fragmentShader);
        glLinkProgram(aProgram);

        GLint success;
        glGetProgramiv(aProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(aProgram, 512, nullptr, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
            return;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        program = aProgram;
        return;
    }

    void set(const char *name, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set(const char *name, const glm::mat3 &matrix)
    {
        glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set(const char *name, const glm::vec3 &vector)
    {
        glUniform3fv(getLocation(name), 1, glm::value_ptr(vector));
    }

    void set(const char *name, int value)
    {
        glUniform1i(getLocation(name), value);
    }

    void set(const char *name, float value)
    {
        glUniform1f(getLocation(name), value);
    }

    void use()
    {
        glUseProgram(program);
    }

    void end()
    {
        if (program != 0)
        {
            glDeleteProgram(program);
        }
    }

  private:
    GLint getLocation(const char *name)
    {
        if (locations.find(name) == locations.end())
        {
            locations[name] = glGetUniformLocation(program, name);
        }
        return locations[name];
    }

    GLuint compileSource(const char *source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
            return 0;
        }

        return shader;
    }
};

struct Frame
{
    int width = 800;
    int height = 600;
    float backgroundColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};

    GLuint buffer = 0;
    GLuint colorTexture = 0;
    GLuint renderBuffer = 0;

    void init()
    {
        // Clean up existing framebuffer
        end();

        // Create framebuffer
        glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);

        // Create color texture
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

        // Create depth renderbuffer
        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Framebuffer not complete!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, width, height);
        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glm::mat4 projection()
    {
        float aspect = (float)width / (float)height;
        return glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100000.0f);
    }

    void end()
    {
        if (buffer != 0)
        {
            glDeleteFramebuffers(1, &buffer);
            glDeleteTextures(1, &colorTexture);
            glDeleteRenderbuffers(1, &renderBuffer);
        }
    }
};

struct Layout
{
    struct Attribute
    {
        int type;
        int size;
        int offset;
    };

    GLuint vao = 0;
    int stride = 0;

    std::vector<Attribute> attributes;

    void init()
    {
        glGenVertexArrays(1, &vao);
    }

    void bind()
    {
        glBindVertexArray(vao);
    }

    void unbind()
    {
        glBindVertexArray(0);
    }

    template <typename T> void push(int size);

    template <> void push<float>(int size)
    {
        attributes.push_back({GL_FLOAT, size, stride});
        stride += size * sizeof(float);
    }

    void build()
    {
        bind();

        int location = 0;
        for (auto [type, size, offset] : attributes)
        {
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, size, type, GL_FALSE, stride, reinterpret_cast<void *>(offset));
            location++;
        }

        unbind();
    }

    void end()
    {
        glDeleteVertexArrays(1, &vao);
    }
};

struct Buffer
{
    Layout layout;
    GLuint vbo = 0;
    GLuint ebo = 0;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void init(std::vector<float> vertices, std::vector<unsigned int> indices)
    {
        this->vertices = vertices;
        this->indices = indices;

        layout.init();
        buildLayout(); // Define layout before building

        layout.bind();

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        layout.build();
        layout.unbind();
    }

    void draw()
    {
        layout.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        layout.unbind();
    }

    void end()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        layout.end();
    }

  protected:
    virtual void buildLayout()
    {
        layout.push<float>(3);
    }
};

struct CellBuffer : public Buffer
{
    void init(const Polylla::Tetrahedron &cell, const Polylla::Mesh &mesh)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int i = 0;
        for (auto fi : cell.faces)
        {
            float color[3] = {rand() % 255 / 255.0f, rand() % 255 / 255.0f, rand() % 255 / 255.0f};
            for (auto vi : mesh.faces[fi].vertices)
            {
                const auto &v = mesh.vertices[vi];
                vertices.push_back(v.x());
                vertices.push_back(v.y());
                vertices.push_back(v.z());
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                indices.push_back(i++);
            }
        }

        Buffer::init(vertices, indices);
    }

    void buildLayout() override
    {
        layout.push<float>(3); // Position
        layout.push<float>(3); // Color
    }
};

struct PhongCellBuffer : public Buffer
{
    void init(const Polylla::Tetrahedron &cell, const Polylla::Mesh &mesh)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int index = 0;
        for (int fi = 0; fi < cell.faces.size(); fi++)
        {
            // Calculate face normal
            const auto &face = mesh.faces.at(cell.faces.at(fi));

            auto faceVertices = face.vertices;

            const auto &other = mesh.vertices.at(cell.vertices.at(fi));

            const auto &v0 = mesh.vertices.at(faceVertices.at(0));
            const auto &v1 = mesh.vertices.at(faceVertices.at(1));
            const auto &v2 = mesh.vertices.at(faceVertices.at(2));

            glm::vec3 p0(v0.x(), v0.y(), v0.z());
            glm::vec3 p1(v1.x(), v1.y(), v1.z());
            glm::vec3 p2(v2.x(), v2.y(), v2.z());
            glm::vec3 pOther(other.x(), other.y(), other.z());

            glm::vec3 toOther = pOther - p0;

            glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            if (glm::dot(normal, toOther) > 0)
            {
                normal = -normal;
                std::ranges::reverse(faceVertices);
            }

            for (auto vi : faceVertices)
            {
                const auto &v = mesh.vertices.at(vi);
                // Position
                vertices.push_back(v.x());
                vertices.push_back(v.y());
                vertices.push_back(v.z());
                // Normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                indices.push_back(index++);
            }
        }

        Buffer::init(vertices, indices);
    }

    void buildLayout() override
    {
        layout.push<float>(3); // Position
        layout.push<float>(3); // Normal
    }
};

struct SphereBuffer : public Buffer
{
    void init(float radius = 0.2f, int slices = 16, int stacks = 16)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generate sphere vertices
        for (int i = 0; i <= stacks; ++i)
        {
            float phi = M_PI * i / stacks;
            for (int j = 0; j <= slices; ++j)
            {
                float theta = 2.0f * M_PI * j / slices;

                float x = radius * sin(phi) * cos(theta);
                float y = radius * cos(phi);
                float z = radius * sin(phi) * sin(theta);

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normal (same as position for unit sphere)
                vertices.push_back(x / radius);
                vertices.push_back(y / radius);
                vertices.push_back(z / radius);
            }
        }

        // Generate indices
        for (int i = 0; i < stacks; ++i)
        {
            for (int j = 0; j < slices; ++j)
            {
                int first = i * (slices + 1) + j;
                int second = first + slices + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        Buffer::init(vertices, indices);
    }

    void buildLayout() override
    {
        layout.push<float>(3); // Position
        layout.push<float>(3); // Normal
    }
};

struct AABB
{
    glm::vec3 min = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 max = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Model
{
    enum Visibility
    {
        VISIBLE,
        HIDDEN,
        SELECTED,
        TRANSPARENT
    };

    Buffer *buffer = nullptr;
    Shader *shader = nullptr;
    Shader *outlinedShader = nullptr;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    Visibility visibility = VISIBLE;
    AABB collider;
};

struct VolumeMesh
{
    std::vector<Model> cells;
    std::vector<std::unique_ptr<Buffer>> buffers;

    void init()
    {
    }

    void draw()
    {
    }

    void end()
    {
        for (auto &buffer : buffers)
        {
            buffer->end();
        }

        buffers.clear();
        cells.clear();
    }
};

struct Light
{
    glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
};

struct RenderSettings
{
    bool faceCulling = true;
    bool wireframe = false;
    bool showGrid = true;
    bool showLightSphere = true;
};

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    static Ray fromMouse(float screenX, float screenY, int viewportWidth, int viewportHeight,
                         const glm::mat4 &projection, const glm::mat4 &view)
    {
        // Convert screen coordinates to normalized device coordinates
        float x = (2.0f * screenX) / viewportWidth - 1.0f;
        float y = 1.0f - (2.0f * screenY) / viewportHeight;

        // Create projection and view matrices
        glm::mat4 viewProjection = projection * view;
        glm::mat4 invViewProjection = glm::inverse(viewProjection);

        // Transform to world coordinates
        glm::vec4 nearPoint = invViewProjection * glm::vec4(x, y, -1.0f, 1.0f);
        glm::vec4 farPoint = invViewProjection * glm::vec4(x, y, 1.0f, 1.0f);

        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        Ray ray;
        ray.origin = glm::vec3(nearPoint);
        ray.direction = glm::normalize(glm::vec3(farPoint) - glm::vec3(nearPoint));

        return ray;
    }

    std::optional<glm::vec3> intersect(const AABB &box) const
    {
        // Slab method for Ray-AABB intersection
        // box must have .min and .max as glm::vec3
        float tmin = (box.min.x - origin.x) / direction.x;
        float tmax = (box.max.x - origin.x) / direction.x;
        if (tmin > tmax)
            std::swap(tmin, tmax);

        float tymin = (box.min.y - origin.y) / direction.y;
        float tymax = (box.max.y - origin.y) / direction.y;
        if (tymin > tymax)
            std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return {};

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        float tzmin = (box.min.z - origin.z) / direction.z;
        float tzmax = (box.max.z - origin.z) / direction.z;
        if (tzmin > tzmax)
            std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return {};

        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        // Only consider intersection in the positive ray direction
        if (tmax < EPSILON)
            return {};

        float t = tmin;
        if (t < EPSILON)
            t = tmax; // If tmin is behind, use tmax (exit point)

        if (t < EPSILON)
            return {};

        return origin + direction * t;
    }

    std::optional<glm::vec3> intersect(const Polylla::Face &face, const Polylla::Mesh &mesh) const
    {
        using namespace glm;

        vec3 a = toVec3(mesh.vertices[face.vertices[0]]);
        vec3 b = toVec3(mesh.vertices[face.vertices[1]]);
        vec3 c = toVec3(mesh.vertices[face.vertices[2]]);

        // Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        vec3 edge1 = b - a;
        vec3 edge2 = c - a;
        vec3 ray_cross_e2 = cross(direction, edge2);
        float det = dot(edge1, ray_cross_e2);

        if (det > -EPSILON && det < EPSILON)
            return {}; // This ray is parallel to this triangle.

        float inv_det = 1.0 / det;
        vec3 s = origin - a;
        float u = inv_det * dot(s, ray_cross_e2);

        if ((u < 0 && abs(u) > EPSILON) || (u > 1 && abs(u - 1) > EPSILON))
            return {};

        vec3 s_cross_e1 = cross(s, edge1);
        float v = inv_det * dot(direction, s_cross_e1);

        if ((v < 0 && abs(v) > EPSILON) || (u + v > 1 && abs(u + v - 1) > EPSILON))
            return {};

        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = inv_det * dot(edge2, s_cross_e1);

        if (t > EPSILON) // ray intersection
        {
            return vec3(origin + direction * t);
        }
        else // This means that there is a line intersection but not a ray intersection.
            return {};
    }

    std::optional<glm::vec3> intersect(const Polylla::Tetrahedron &tetrahedron, const Polylla::Mesh &mesh) const
    {
        for (const auto &fi : tetrahedron.faces)
        {
            const auto &face = mesh.faces[fi];
            std::optional<glm::vec3> intersection = intersect(face, mesh);
            if (intersection)
                return intersection;
        }
        return {};
    }
};

// struct Selection
// {
//     int selectedCellIndex = -1; // -1 means no selection
//     glm::vec3 selectedCellCenter = glm::vec3(0.0f);
//     bool isHighlighted = false;
// };

static struct State
{
    // Viewport texture
    float backgroundColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};

    // OpenGL rendering state
    Frame frame;
    Camera camera;
    Light light;
    RenderSettings renderSettings;
    int selectedCellIndex = -1;
    glm::vec3 selectedCellCenter = glm::vec3(0.0f);
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, Model> models;
    VolumeMesh mesh;

} state;

// Initialize grid geometry
void initializeGrid(Shader &shader)
{
    // Create a large quad for the infinite plane
    std::vector<float> gridVertices = {
        // Positions          // Colors (not used, but needed for shader)
        -100.0f, 0.0f, -100.0f, 100.0f, 0.0f, -100.0f, 100.0f, 0.0f, 100.0f, -100.0f, 0.0f, 100.0f,
    };

    std::vector<unsigned int> gridIndices = {0, 1, 2, 2, 3, 0};

    state.models["grid"] = Model();
    state.models["grid"].buffer = new Buffer();
    state.models["grid"].buffer->init(gridVertices, gridIndices);
    state.models["grid"].shader = &shader;
    state.models["grid"].color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Generate and bind VAO
    // glGenVertexArrays(1, &state.grid.vao);
    // glBindVertexArray(state.grid.vao);

    // // Generate and bind VBO
    // glGenBuffers(1, &state.grid.vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, state.grid.vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

    // // Generate and bind EBO
    // glGenBuffers(1, &state.grid.ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.grid.ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridIndices), gridIndices, GL_STATIC_DRAW);

    // // Position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    // glEnableVertexAttribArray(0);

    // glBindVertexArray(0);
}

void init()
{
    state.frame.init();
    // Load grid shader from files
    state.shaders["grid"] = Shader();
    std::ifstream gridVertexShaderFile("shaders/grid.vs");
    std::ifstream gridFragmentShaderFile("shaders/grid.fs");
    std::string gridVertexShaderSource(std::istreambuf_iterator<char>(gridVertexShaderFile), {});
    std::string gridFragmentShaderSource(std::istreambuf_iterator<char>(gridFragmentShaderFile), {});
    state.shaders["grid"].init(gridVertexShaderSource.c_str(), gridFragmentShaderSource.c_str());

    // Load cell shader from files
    state.shaders["basic"] = Shader();
    std::ifstream cellVertexShaderFile("shaders/cell.vs");
    std::ifstream cellFragmentShaderFile("shaders/cell.fs");
    std::string cellVertexShaderSource(std::istreambuf_iterator<char>(cellVertexShaderFile), {});
    std::string cellFragmentShaderSource(std::istreambuf_iterator<char>(cellFragmentShaderFile), {});
    state.shaders["basic"].init(cellVertexShaderSource.c_str(), cellFragmentShaderSource.c_str());

    // Load phong shader from files
    state.shaders["phong"] = Shader();
    std::ifstream phongVertexShaderFile("shaders/phong.vs");
    std::ifstream phongFragmentShaderFile("shaders/phong.fs");
    std::string phongVertexShaderSource(std::istreambuf_iterator<char>(phongVertexShaderFile), {});
    std::string phongFragmentShaderSource(std::istreambuf_iterator<char>(phongFragmentShaderFile), {});
    state.shaders["phong"].init(phongVertexShaderSource.c_str(), phongFragmentShaderSource.c_str());

    initializeGrid(state.shaders["grid"]);

    // Initialize light sphere
    state.models["lightSphere"] = Model();
    auto sphereBuffer = new SphereBuffer();
    sphereBuffer->init(0.1f, 16, 16);
    state.models["lightSphere"].buffer = sphereBuffer;
    state.models["lightSphere"].shader = &state.shaders["phong"];

    // state.models["cell"].buffer->init(cellVertices, cellIndices);
    // state.models["cell"].shader = &state.shaders["cell"];
    // state.models["cell"].color = glm::vec3(1.0f, 0.0f, 0.0f);

    // Camera will be initialized by controller
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable stencil test for outlining
    glEnable(GL_STENCIL_TEST);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // Cull back faces
    glFrontFace(GL_CCW); // Counter-clockwise vertices define front faces
}

void render()
{
    state.frame.bind();

    // Clear color, depth, and stencil buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00); // Enable writing to stencil buffer

    // Camera will be updated by external controller (handled in app layer)

    for (auto &shader : state.shaders)
    {
        shader.second.use();
        shader.second.set("view", state.camera.view());
        shader.second.set("projection", state.frame.projection());
    }

    // Render grid first with depth offset to ensure it's always behind the mesh
    // glEnable(GL_POLYGON_OFFSET_FILL);
    // glPolygonOffset(1.0f, 1.0f); // Push grid further back

    // // Set grid-specific uniforms
    // state.shaders["grid"].use();
    // state.shaders["grid"].set("showGrid", state.renderSettings.showGrid);
    // state.shaders["grid"].set("lineWidth", 0.01f);
    // state.shaders["grid"].set("gridSize", 1.0f);

    // state.models["grid"].model = glm::mat4(1.0f);
    // state.models["grid"].draw();

    // state.models["grid"].model =
    //     glm::rotate(state.models["grid"].model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // state.models["grid"].draw();

    // glDisable(GL_POLYGON_OFFSET_FILL);

    // Render the mesh with Phong lighting (will appear in front of grid)
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    state.shaders["phong"].use();

    // Set up global Phong lighting uniforms
    // Material properties
    state.shaders["phong"].set("ambientStrength", 0.3f);
    state.shaders["phong"].set("diffuseStrength", 1.0f);
    state.shaders["phong"].set("specularStrength", 0.5f);
    state.shaders["phong"].set("shininess", 32.0f);

    // Light properties from state
    state.shaders["phong"].set("lightPos", state.light.position);
    state.shaders["phong"].set("lightColor", state.light.color * state.light.intensity);
    state.shaders["phong"].set("viewPos", state.camera.position);

    // First render the selected cell to stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glDisable(GL_DEPTH_TEST);
    for (auto &cell : state.mesh.cells)
    {
        if (cell.visibility == Model::SELECTED)
        {
            state.shaders["phong"].use();
            state.shaders["phong"].set("model", cell.model);
            state.shaders["phong"].set("materialColor", cell.color);
            cell.buffer->draw();
        }
    }

    glEnable(GL_DEPTH_TEST);
    glStencilMask(0x00);
    for (auto &cell : state.mesh.cells)
    {
        if (cell.visibility == Model::HIDDEN)
            continue;

        cell.color.a = 1.0f;
        if (cell.visibility == Model::TRANSPARENT)
            cell.color.a = 0.5f;

        state.shaders["phong"].use();
        state.shaders["phong"].set("model", cell.model);
        state.shaders["phong"].set("materialColor", cell.color);
        cell.buffer->draw();
    }

    if (state.renderSettings.wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        for (auto &cell : state.mesh.cells)
        {
            if (cell.visibility == Model::HIDDEN || cell.visibility == Model::TRANSPARENT)
                continue;

            state.shaders["basic"].use();
            state.shaders["basic"].set("model", cell.model);
            state.shaders["basic"].set("materialColor", glm::vec3(0.0f, 0.0f, 0.0f));
            cell.buffer->draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Then render the outline using the basic shader
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glDisable(GL_DEPTH_TEST);
    for (auto &cell : state.mesh.cells)
    {
        if (cell.visibility == Model::SELECTED)
        {
            state.shaders["basic"].use();

            // Create scaling matrix from tetrahedron center
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -state.selectedCellCenter);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f)); // 5% larger
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), state.selectedCellCenter);
            glm::mat4 scaledModel = translateBack * scale * translateToOrigin * cell.model;

            state.shaders["basic"].set("model", scaledModel);
            state.shaders["basic"].set("materialColor", glm::vec3(1.0f, 1.0f, 1.0f));
            cell.buffer->draw();
        }
    }

    // Restore stencil and depth state
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
    // state.mesh.draw();
    // }

    // Render light sphere at light position (if enabled)
    if (state.renderSettings.showLightSphere)
    {
        state.models["lightSphere"].model = glm::mat4(1.0f);
        state.models["lightSphere"].model = glm::translate(state.models["lightSphere"].model, state.light.position);

        // Set light sphere material to emit light color
        state.shaders["phong"].set("materialColor", state.light.color);
        state.shaders["phong"].set("ambientStrength", 1.0f);  // Make it fully bright
        state.shaders["phong"].set("diffuseStrength", 0.0f);  // No diffuse
        state.shaders["phong"].set("specularStrength", 0.0f); // No specular
    }

    state.frame.unbind();
}

// Helper function to convert HSV to RGB color space
glm::vec4 hsvToRgb(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r, g, b;
    if (h >= 0 && h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h >= 60 && h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h >= 120 && h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h >= 180 && h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h >= 240 && h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    return glm::vec4(r + m, g + m, b + m, 1.0f);
}

// Test if point is inside tetrahedron using barycentric coordinates
bool pointInTetrahedron(const glm::vec3 &point, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
                        const glm::vec3 &v3)
{
    glm::mat4 M(glm::vec4(v0, 1.0f), glm::vec4(v1, 1.0f), glm::vec4(v2, 1.0f), glm::vec4(v3, 1.0f));

    float det = glm::determinant(M);
    if (abs(det) < 1e-6f)
        return false; // Degenerate tetrahedron

    // Calculate barycentric coordinates
    glm::mat4 M0 = M;
    M0[0] = glm::vec4(point, 1.0f);
    float lambda0 = glm::determinant(M0) / det;

    glm::mat4 M1 = M;
    M1[1] = glm::vec4(point, 1.0f);
    float lambda1 = glm::determinant(M1) / det;

    glm::mat4 M2 = M;
    M2[2] = glm::vec4(point, 1.0f);
    float lambda2 = glm::determinant(M2) / det;

    glm::mat4 M3 = M;
    M3[3] = glm::vec4(point, 1.0f);
    float lambda3 = glm::determinant(M3) / det;

    const float tolerance = -1e-6f; // Small negative tolerance for numerical stability
    return (lambda0 >= tolerance && lambda1 >= tolerance && lambda2 >= tolerance && lambda3 >= tolerance);
}

// Test ray-tetrahedron intersection and return distance to closest intersection
bool rayIntersectsTetrahedron(const Ray &ray, const Polylla::Tetrahedron &cell, const Polylla::Mesh &mesh,
                              float &closestDistance)
{
    // Safety checks
    if (cell.vertices.size() != 4)
        return false;

    // Get tetrahedron vertices
    std::vector<glm::vec3> vertices;
    for (int vi : cell.vertices)
    {
        if (vi < 0 || vi >= mesh.vertices.size())
            return false; // Invalid vertex index

        const auto &v = mesh.vertices[vi];
        vertices.push_back(glm::vec3(v.x(), v.y(), v.z()));
    }

    if (vertices.size() != 4)
        return false;

    glm::vec3 v0 = vertices[0];
    glm::vec3 v1 = vertices[1];
    glm::vec3 v2 = vertices[2];
    glm::vec3 v3 = vertices[3];

    // Test multiple points along the ray to find the closest intersection
    const float maxDistance = 100.0f;
    const int samples = 200;

    closestDistance = maxDistance + 1.0f; // Initialize to beyond max range
    bool found = false;

    for (int i = 0; i < samples; i++)
    {
        float t = (float)i / (samples - 1) * maxDistance;
        glm::vec3 point = ray.origin + t * ray.direction;

        if (pointInTetrahedron(point, v0, v1, v2, v3))
        {
            closestDistance = t;
            found = true;
            break; // Found first intersection, which is closest due to sequential sampling
        }
    }

    return found;
}

// Find the closest cell that intersects with the ray
int findIntersectedCell(const Ray &ray, const Polylla::Mesh &mesh)
{
    if (mesh.cells.empty())
        return -1;

    int closestCellIndex = -1;
    float closestDistance = FLT_MAX;

    for (size_t i = 0; i < mesh.cells.size(); i++)
    {
        const auto intersection = ray.intersect(mesh.cells[i], mesh);
        if (intersection)
        {
            if (glm::length(intersection.value() - ray.origin) < closestDistance)
            {
                closestDistance = glm::length(intersection.value() - ray.origin);
                closestCellIndex = (int)i;
            }
        }
    }
    return closestCellIndex;
}

} // namespace render

namespace app
{

struct Controller
{
    // Mouse state for input tracking
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

// Application state
static struct State
{
    bool showDemoWindow = false;
    int selectedTab = 0; // 0=Tools, 1=Properties, 2=Info
    float minWidth = 300.0f;
    GLFWwindow *window = nullptr;
    Polylla::Mesh mesh;
    Polylla::PolyMesh polyMesh;
    Polylla::TetgenReader reader;
    Polylla::VisFWriter writer;
    Controller controller;
    std::vector<std::string> meshes = {"mage", "3D_100", "1000points", "socket", };
    int currentMesh = 0;
    std::mt19937 gen;
    bool transformed = false;
    std::vector<int> owners;
    bool showTetras = false;
    std::vector<float> hullVolumes;
    std::vector<float> hullAreas;
    std::vector<float> polyVolumes;
    std::vector<float> polyAreas;
    int loners;
    int maxSize;
    int minSize;
    float avgSize;
    float maxConvexity;
    float minConvexity;
    float avgConvexity;
    int polyVertices;
} state;

// Define mouse callbacks now that State is declared
static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    // Only handle camera movement if left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!state.controller.isDragging)
        {
            state.controller.isDragging = true;
            state.controller.lastMouseX = xpos;
            state.controller.lastMouseY = ypos;
        }
        else
        {
            double deltaX = xpos - state.controller.lastMouseX;
            double deltaY = ypos - state.controller.lastMouseY;

            // Sensitivity
            float sensitivity = 0.005f;

            // Apply rotation to camera
            render::state.camera.azimuth += -deltaX * sensitivity;
            render::state.camera.elevation += -deltaY * sensitivity;
            render::state.camera.elevation = glm::clamp(
                render::state.camera.elevation, render::state.camera.minElevation, render::state.camera.maxElevation);

            state.controller.lastMouseX = xpos;
            state.controller.lastMouseY = ypos;
        }
    }
    else
    {
        state.controller.isDragging = false;
    }
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    float zoomSensitivity = 0.1f;
    float deltaDistance = -yoffset * zoomSensitivity * render::state.camera.distance;
    render::state.camera.distance += deltaDistance;
    render::state.camera.distance =
        glm::clamp(render::state.camera.distance, render::state.camera.minDistance, render::state.camera.maxDistance);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Get window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Simple coordinate mapping - we'll do a basic check if it's in the main area
        // Account for approximate ImGui layout: navbar (25px) and sidebar (300px)
        float viewportX = xpos - 0;     // No horizontal offset for now
        float viewportY = ypos - 25.0f; // Account for navbar height

        // Basic bounds check - if click is roughly in the main viewport area
        if (viewportX >= 0 && viewportX < (windowWidth - 300) && viewportY >= 0 && viewportY < (windowHeight - 25))
        {
            if (render::state.selectedCellIndex >= 0)
            {
                if (state.transformed)
                {
                    int pi = app::state.polyMesh.tetras[render::state.selectedCellIndex].polyhedron;
                    for (int ti : app::state.polyMesh.cells[pi].cells)
                    {
                        render::state.mesh.cells[ti].visibility = render::Model::VISIBLE;
                    }
                }
                else
                {
                    render::state.mesh.cells[render::state.selectedCellIndex].visibility = render::Model::VISIBLE;
                }
            }

            // Map to framebuffer coordinates
            float fbX = (viewportX / (windowWidth - 300)) * render::state.frame.width;
            float fbY = (viewportY / (windowHeight - 25)) * render::state.frame.height;

            // Create ray from screen coordinates
            render::Ray ray = render::Ray::fromMouse(fbX, fbY, render::state.frame.width, render::state.frame.height,
                                                     render::state.frame.projection(), render::state.camera.view());

            // Check if mesh is loaded
            if (!app::state.mesh.cells.empty() && !app::state.mesh.vertices.empty())
            {

                // Find intersected cell
                int cellIndex = render::findIntersectedCell(ray, app::state.mesh);

                if (cellIndex >= 0)
                {
                    if (state.transformed)
                    {
                        // Highlight the polyhedron
                        int pi = app::state.polyMesh.tetras[cellIndex].polyhedron;
                        glm::vec3 center(0.0f);
                        for (int ti : app::state.polyMesh.cells[pi].cells)
                        {
                            render::state.mesh.cells[ti].visibility = render::Model::SELECTED;
                            center += render::state.mesh.cells[ti].center;
                        }
                        center /= app::state.polyMesh.cells[pi].cells.size();
                        render::state.selectedCellCenter = center;
                    }
                    else
                    {
                        render::state.mesh.cells[cellIndex].visibility = render::Model::SELECTED;
                        render::state.selectedCellCenter = render::state.mesh.cells[cellIndex].center;
                    }
                    render::state.selectedCellIndex = cellIndex;
                }
            }
        }
    }
}

void calculateStat()
{
    state.loners = 0;
    state.minSize = std::numeric_limits<int>::max();
    state.maxSize = std::numeric_limits<int>::min();
    state.avgSize = 0;
    state.minConvexity = std::numeric_limits<float>::max();
    state.maxConvexity = std::numeric_limits<float>::min();
    state.avgConvexity = 0;
    std::unordered_set<int> used;
    for (int i = 0; i < state.polyMesh.cells.size(); i++)
    {
        const auto &poly = state.polyMesh.cells[i];
        state.loners += poly.cells.size() == 1;
        state.minSize = std::min(state.minSize, static_cast<int>(poly.cells.size()));
        state.maxSize = std::max(state.maxSize, static_cast<int>(poly.cells.size()));
        state.avgSize += static_cast<float>(poly.cells.size());
        float correctness = state.polyVolumes[i] / state.hullVolumes[i];
        if (correctness == 0.0f)
        {
            std::cout << "Error: " << correctness << std::endl;
            std::cout << "Volume: " << state.polyVolumes[i] << " Hull: " << state.hullVolumes[i] << std::endl;
        }
        state.minConvexity = std::min(state.minConvexity, correctness);
        state.maxConvexity = std::max(state.maxConvexity, correctness);
        state.avgConvexity += correctness;
        for (int fi: poly.faces)
        {
            const auto& face = state.polyMesh.faces.at(fi);
            for (int vi: face.vertices)
                used.insert(vi);
        }
    }
    state.avgSize /= state.polyMesh.cells.size();
    state.avgConvexity /= state.polyMesh.cells.size();
    state.polyVertices = used.size();
}

void paintMesh()
{
    std::uniform_real_distribution<float> hueDistribution(0.0f, 360.0f);
    for (size_t i = 0; i < state.mesh.cells.size(); i++)
    {
        float hue = hueDistribution(state.gen);
        float saturation = 0.8f; // High saturation for vibrant colors
        float value = 0.8f;      // High value for bright colors
        glm::vec4 color = render::hsvToRgb(hue, saturation, value);
        if (state.transformed)
        {
            color = render::state.mesh.cells[state.owners[i]].color;
        }
        render::state.mesh.cells[i].color = color;
    }
}

void highlightCell(int cellIndex, glm::vec3 color)
{
}

void loadMesh(const std::string &meshName)
{
    state.transformed = false;
    std::string meshPath = DATA_DIR + meshName;
    state.reader.nodeFile = meshPath + ".node";
    state.reader.eleFile = meshPath + ".ele";
    Polylla::Mesh mesh = state.reader.readMesh();

    app::state.mesh = mesh;

    // Clear existing render mesh data
    render::state.mesh.end();

    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    for (size_t cellIndex = 0; cellIndex < mesh.cells.size(); cellIndex++)
    {
        const auto &cell = mesh.cells.at(cellIndex);

        auto buffer = std::make_unique<render::PhongCellBuffer>();
        buffer->init(cell, mesh);

        // Generate a unique color for each cell using HSV color space

        auto model = render::Model();
        model.buffer = buffer.get(); // Get pointer before moving
        model.shader = &render::state.shaders["phong"];
        model.outlinedShader = &render::state.shaders["basic"];

        // calculates center and bounds
        glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        for (const auto &vi : cell.vertices)
        {
            const auto &vertex = mesh.vertices.at(vi);
            auto v = toVec3(vertex);
            center += v;
            min = glm::min(min, v);
            max = glm::max(max, v);
        }
        center /= cell.vertices.size();
        model.center = center;
        model.collider.min = min;
        model.collider.max = max;

        render::state.mesh.buffers.push_back(std::move(buffer)); // Move after getting pointer
        render::state.mesh.cells.push_back(model);
    }
    for (const auto &vertex : mesh.vertices)
    {
        glm::vec3 pos(vertex.x(), vertex.y(), vertex.z());
        minBounds = glm::min(minBounds, pos);
        maxBounds = glm::max(maxBounds, pos);
    }

    render::state.camera.target = (minBounds + maxBounds) * 0.5f;

    // Set initial distance based on mesh size
    glm::vec3 size = maxBounds - minBounds;
    float meshSize = glm::length(size);
    render::state.camera.distance = meshSize * 1.5f; // Start at 1.5x the mesh size

    // Update distance constraints based on mesh size
    render::state.camera.minDistance = meshSize * 0.1f;
    render::state.camera.maxDistance = meshSize * 5.0f;

    paintMesh();
}

void drawMenuBar()
{
    if (ImGui::BeginMenuBar())

    {
        if (ImGui::BeginMenu("Meshes"))
        {
            for (int i = 0; i < app::state.meshes.size(); i++)
            {
                if (ImGui::MenuItem(app::state.meshes[i].c_str()))
                {
                    app::state.currentMesh = i;
                    loadMesh(app::state.meshes[i]);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Demo Window", nullptr, &state.showDemoWindow);
            ImGui::EndMenu();
        }

        // if (ImGui::BeginMenu("Tools"))
        // {
        //     if (ImGui::MenuItem("Reset Layout"))
        //     {
        //         // Handle reset layout
        //     }
        //     ImGui::Separator();
        //     if (ImGui::MenuItem("Preferences"))
        //     {
        //         // Handle preferences
        //     }
        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("Help"))
        // {
        //     if (ImGui::MenuItem("About"))
        //     {
        //         // Handle about
        //     }
        //     ImGui::EndMenu();
        // }

        ImGui::EndMenuBar();
    }
}

void drawViewport()
{
    // Calculate viewport width: total width minus fixed sidebar width
    float viewportWidth = ImGui::GetContentRegionAvail().x - state.minWidth;

    ImGui::BeginChild("Viewport", ImVec2(viewportWidth, 0));

    ImVec2 availableSize = ImGui::GetContentRegionAvail();

    // Resize framebuffer if needed
    if ((int)availableSize.x != render::state.frame.width || (int)availableSize.y != render::state.frame.height)
    {
        render::state.frame.width = (int)availableSize.x;
        render::state.frame.height = (int)availableSize.y;
        if (render::state.frame.width > 0 && render::state.frame.height > 0)
        {
            render::state.frame.init();
        }
    }

    // Render to texture
    if (render::state.frame.width > 0 && render::state.frame.height > 0)
    {
        // Render the scene to the framebuffer
        render::render();

        // Display the texture
        ImGui::Image((void *)(intptr_t)render::state.frame.colorTexture, availableSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::EndChild();
}

void drawSidebar()
{
    ImGui::BeginGroup();

    // Fixed width sidebar with padding
    ImGui::BeginChild("Sidebar", ImVec2(0, 0));

    // Tab bar
    if (ImGui::BeginTabBar("SidebarTabs", ImGuiTabBarFlags_None))
    {
        // Info Tab
        if (ImGui::BeginTabItem("Information"))
        {
            ImGui::Spacing();
            ImGui::Text("Mesh");
            ImGui::Separator();

            ImGui::TextWrapped("Node File: %s", state.reader.nodeFile.c_str());
            ImGui::TextWrapped("Ele File: %s", state.reader.eleFile.c_str());

            ImGui::Text("Cells: %zu", state.mesh.cells.size());
            ImGui::Text("Vertices: %zu", state.mesh.vertices.size());
            ImGui::Text("Faces: %zu", state.mesh.faces.size());

            ImGui::Spacing();
            ImGui::Text("Performance");
            ImGui::Separator();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

            ImGui::Spacing();
            ImGui::Text("Memory Usage");
            ImGui::Separator();
            ImGui::Text("GPU Memory: N/A");
            ImGui::Text("System Memory: N/A");

            ImGui::Spacing();
            ImGui::Text("Viewport");
            ImGui::Separator();
            ImGui::Text("Size: %dx%d", render::state.frame.width, render::state.frame.height);
            ImGui::Text("Aspect: %.2f", (float)render::state.frame.width / render::state.frame.height);

            ImGui::EndTabItem();
        }

        // Tools Tab
        if (ImGui::BeginTabItem("Tools"))
        {
            ImGui::Spacing();
            ImGui::Text("Selection");
            ImGui::Separator();
            if (render::state.selectedCellIndex >= 0)
            {
                ImGui::Text("Selected Cell: %d", render::state.selectedCellIndex);
                ImGui::Text("Right-click to deselect");
                if (ImGui::Button("Center View"))
                {
                    // Handle delete
                }

                if (ImGui::Button("Show Neighbors"))
                {
                    // Handle show neighbors
                }

                if (ImGui::Button("Show Circumsphere"))
                {
                    // Handle show circumsphere
                }

                if (ImGui::Button("Isolate"))
                {
                    // Handle show faces
                }
            }
            else
            {
                ImGui::Text("No cell selected");
                ImGui::Text("Right-click on a cell to select");
            }

            ImGui::Spacing();
            ImGui::Text("Planes");
            ImGui::Separator();

            if (ImGui::Button("Add a plane"))
            {
                // Handle select tool
            }

            ImGui::Spacing();
            ImGui::Text("Algorithm");
            ImGui::Separator();

            static int algorithm = 0;
            ImGui::RadioButton("Cavity", &algorithm, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Face", &algorithm, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Edge", &algorithm, 2);

            if (algorithm == 0)
            {
                if (ImGui::Button("Start"))
                {
                    Polylla::CavityAlgorithm worker;
                    state.polyMesh = worker(state.mesh, true);
                    state.transformed = true;
                    state.owners = worker.info.poly.seeds;
                    state.hullAreas = worker.info.poly.hullAreas;
                    state.hullVolumes = worker.info.poly.hullVolumes;
                    state.polyAreas = worker.info.poly.areas;
                    state.polyVolumes = worker.info.poly.volumes;
                    paintMesh();
                    calculateStat();
                }
                if (ImGui::Button(state.showTetras ? "Hide Tetras" : "Show Tetras"))
                {
                    state.showTetras = !state.showTetras;
                }
                if (ImGui::Button("Reset"))
                {
                    state.transformed = false;
                    paintMesh();
                }
            }
            else
            {
                ImGui::Text("Not implemented");
            }
            if (state.transformed)
            {
                ImGui::Spacing();
                ImGui::Text("Statistics");
                ImGui::Separator();
                ImGui::Text("%d loners of %d cells (%.2f %%)", state.loners, state.polyMesh.cells.size(), 100.0f * state.loners / state.polyMesh.cells.size());
                ImGui::Text("Min Size: %d", state.minSize);
                ImGui::Text("Max Size: %d", state.maxSize);
                ImGui::Text("Avg Size: %.2f", state.avgSize);
                ImGui::Text("Min Correctness: %.2f", state.minConvexity);
                ImGui::Text("Max Correctness: %.2f", state.maxConvexity);
                ImGui::Text("Avg Correctness: %.2f", state.avgConvexity);
                ImGui::Text("Poly Vertices: %d", state.polyVertices);
            }

            ImGui::EndTabItem();
        }

        // Scene Tab
        if (ImGui::BeginTabItem("Scene"))
        {
            ImGui::Spacing();
            ImGui::Text("Lighting");
            ImGui::Separator();

            // Light position controls
            ImGui::Text("Light Position");
            ImGui::DragFloat3("Position", glm::value_ptr(render::state.light.position), 0.1f);

            // Light color controls
            ImGui::Text("Light Color");
            ImGui::ColorEdit3("Color", glm::value_ptr(render::state.light.color));

            // Light intensity
            ImGui::Text("Light Intensity");
            ImGui::SliderFloat("Intensity", &render::state.light.intensity, 0.0f, 3.0f);

            // Show light sphere
            ImGui::Text("Light Visualization");
            ImGui::Checkbox("Show Light Sphere", &render::state.renderSettings.showLightSphere);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Show/hide the light position indicator sphere");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::Spacing();
            ImGui::Text("Rendering");
            ImGui::Separator();

            // Wireframe control
            ImGui::Checkbox("Wireframe", &render::state.renderSettings.wireframe);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Show mesh structure as wireframe");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            // Show grid
            ImGui::Checkbox("Show Grid", &render::state.renderSettings.showGrid);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Show grid lines");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Details"))
        {
            for (int i = 0; i < state.polyMesh.cells.size(); i++)
            {
                ImGui::Text("Cell %d", i);
                ImGui::Text("  Volume: %f", state.polyVolumes[i]);
                ImGui::Text("  Area: %f", state.polyAreas[i]);
                ImGui::Text("  Hull Volume: %f", state.hullVolumes[i]);
                ImGui::Text("  Hull Area: %f", state.hullAreas[i]);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

// Draw simple application layout based on ImGui ShowExampleAppLayout
void drawLayout()
{
    // Create main application window
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin("3D-GPolylla", nullptr, windowFlags))
    {
        drawMenuBar();
        drawViewport();
        ImGui::SameLine();
        drawSidebar();
    }

    ImGui::End();
}

void init()
{
    std::random_device rd;
    state.gen = std::mt19937(rd());
    // Load mesh

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    app::state.window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                                         "3D-GPolylla - ImGui Application", nullptr, nullptr);
    if (app::state.window == nullptr)
        return;
    glfwMakeContextCurrent(app::state.window);
    glfwSwapInterval(0); // Disable vsync to unlock FPS

    // Set up mouse callbacks
    glfwSetCursorPosCallback(app::state.window, mouse_callback);
    glfwSetScrollCallback(app::state.window, scroll_callback);
    glfwSetMouseButtonCallback(app::state.window, mouse_button_callback);

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // Note: Docking and Viewports require the docking branch of ImGui
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
                                     // changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
                                     // unnecessary. We leave both here for documentation purpose)

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular
    // ones. Note: This requires the docking branch of ImGui if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     style.WindowRounding = 0.0f;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    // }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(app::state.window, true); // Let ImGui install its own callbacks
    ImGui_ImplOpenGL3_Init(glsl_version);

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

    // Initialize application state

    render::init();
    loadMesh(state.meshes[state.currentMesh]);
}

void draw()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw main application layout
    drawLayout();

    // Show the demo window
    if (state.showDemoWindow)
        ImGui::ShowDemoWindow(&state.showDemoWindow);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(state.window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(state.window);
}

void end()
{

    // Cleanup models
    for (auto &model : render::state.models)
    {
        model.second.buffer->end();
    }

    // Cleanup shaders
    for (auto &shader : render::state.shaders)
    {
        shader.second.end();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(app::state.window);
    glfwTerminate();
}

void update(float deltaTime)
{
    // Poll and handle events
    glfwPollEvents();
    if (glfwGetWindowAttrib(app::state.window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }
    render::state.camera.update();
    // render::state.selection.selectedCellCenter = glm::vec3(0.0f);
    // if (render::state.selection.selectedCellIndex < render::state.mesh.cells.size())
    // {
    //     const auto &selectedCell = state.mesh.cells[render::state.selection.selectedCellIndex];
    //     for (int vi : selectedCell.vertices)
    //     {
    //         if (vi >= 0 && vi < app::state.mesh.vertices.size())
    //         {
    //             const auto &v = app::state.mesh.vertices[vi];
    //             render::state.selection.selectedCellCenter += glm::vec3(v.x(), v.y(), v.z());
    //         }
    //     }
    //     render::state.selection.selectedCellCenter /= 4.0f; // Average of 4 vertices
    // }
}

bool isRunning()
{
    return !glfwWindowShouldClose(app::state.window);
}

} // namespace app

// Main code
int main(int, char **)
{
    app::init();

    // Main loop
    float t0, t1, deltaTime;
    t0 = glfwGetTime();
    while (app::isRunning())
    {
        // Update and draw
        t1 = glfwGetTime();
        deltaTime = t1 - t0;
        app::update(deltaTime);
        app::draw();
        t0 = t1;
    }

    app::end();
    return 0;
}
