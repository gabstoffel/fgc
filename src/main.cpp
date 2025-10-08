
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>

// OpenGL headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM headers
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Local headers
#include "utils.h"
#include "matrices.h"

// Function declarations
GLuint BuildTriangle();
GLuint LoadShader_Vertex(const char* filename);
GLuint LoadShader_Fragment(const char* filename);
void LoadShader(const char* filename, GLuint shader_id);
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);

// Callback functions
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
    // Initialize GLFW
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Set error callback
    glfwSetErrorCallback(ErrorCallback);

    // Request OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Use core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "OpenGL Template", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Set callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Make context current
    glfwMakeContextCurrent(window);

    // Load OpenGL functions
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Print GPU information
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Load shaders
    GLuint vertex_shader_id = LoadShader_Vertex("shaders/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("shaders/shader_fragment.glsl");

    // Create GPU program
    GLuint program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Build triangle
    GLuint vertex_array_object_id = BuildTriangle();

    // Get uniform locations
    GLint model_uniform = glGetUniformLocation(program_id, "model");
    GLint view_uniform = glGetUniformLocation(program_id, "view");
    GLint projection_uniform = glGetUniformLocation(program_id, "projection");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(program_id);

        // Set up matrices
        glm::mat4 model = Matrix_Identity();
        glm::mat4 view = Matrix_Identity();
        glm::mat4 projection = Matrix_Perspective(3.141592f / 3.0f, 800.0f / 600.0f, 0.1f, 10.0f);

        // Send matrices to GPU
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind VAO and draw
        glBindVertexArray(vertex_array_object_id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll events
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}

// Build a simple triangle for rendering
GLuint BuildTriangle()
{
    // Define triangle vertices in NDC coordinates
    GLfloat vertices[] = {
        // Position (X, Y, Z, W)    // Color (R, G, B, A)
        -0.5f, -0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f,  // Bottom left
         0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // Bottom right
         0.0f,  0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f   // Top center
    };

    // Define indices for two triangles forming a quad
    GLubyte indices[] = {
        0, 1, 2  // First triangle
    };

    // Create and bind VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and bind VBO for vertices
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location = 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create and bind EBO for indices
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);

    return VAO;
}

// Load vertex shader from file
GLuint LoadShader_Vertex(const char* filename)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    LoadShader(filename, vertex_shader_id);
    return vertex_shader_id;
}

// Load fragment shader from file
GLuint LoadShader_Fragment(const char* filename)
{
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    LoadShader(filename, fragment_shader_id);
    return fragment_shader_id;
}

// Load and compile shader from file
void LoadShader(const char* filename, GLuint shader_id)
{
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch (std::exception& e) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint shader_string_length = static_cast<GLint>(str.length());

    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
    glCompileShader(shader_id);

    // Check for compilation errors
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0)
    {
        GLchar* log = new GLchar[log_length];
        glGetShaderInfoLog(shader_id, log_length, &log_length, log);

        if (!compiled_ok)
        {
            fprintf(stderr, "ERROR: OpenGL compilation of \"%s\" failed.\n", filename);
            fprintf(stderr, "== Start of compilation log\n");
            fprintf(stderr, "%s", log);
            fprintf(stderr, "== End of compilation log\n");
            std::exit(EXIT_FAILURE);
        }
        else
        {
            fprintf(stderr, "WARNING: OpenGL compilation of \"%s\".\n", filename);
            fprintf(stderr, "== Start of compilation log\n");
            fprintf(stderr, "%s", log);
            fprintf(stderr, "== End of compilation log\n");
        }

        delete[] log;
    }
}

// Create GPU program from vertex and fragment shaders
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    GLuint program_id = glCreateProgram();

    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    // Check for linking errors
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    if (linked_ok == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        GLchar* log = new GLchar[log_length];
        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        fprintf(stderr, "ERROR: OpenGL linking of program failed.\n");
        fprintf(stderr, "== Start of link log\n");
        fprintf(stderr, "%s", log);
        fprintf(stderr, "== End of link log\n");

        delete[] log;
        std::exit(EXIT_FAILURE);
    }

    // Clean up shaders
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}

// Framebuffer size callback
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Key callback
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// Error callback
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}
