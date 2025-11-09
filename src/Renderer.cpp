#include "Renderer.h"
#include "Player.h"
#include "Enemy.h"
#include "matrices.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <string>
#include <glm/gtc/type_ptr.hpp>

void TextRendering_Init();
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale);

static std::stack<glm::mat4> g_MatrixStack;

static void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

static void PopMatrix(glm::mat4& M)
{
    if (g_MatrixStack.empty())
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

static float DiferencaAngulo(glm::vec4 v, glm::vec4 u)
{
    if (norm(v) == 0 || norm(u) == 0)
    {
        return 0.0;
    }
    float angulo = acos(dotproduct(u, v) / (norm(v) * norm(u)));
    if (u.x * v.z - u.z * v.x < 0)
    {
        angulo = -angulo;
    }
    return angulo;
}

Renderer::Renderer()
    : m_vertexArrayObjectID(0)
    , m_gpuProgramID(0)
    , m_modelUniform(0)
    , m_viewUniform(0)
    , m_projectionUniform(0)
    , m_renderAsBlackUniform(0)
    , m_currentView(Matrix_Identity())
    , m_currentProjection(Matrix_Identity())
    , m_screenRatio(1.0f)
    , m_window(nullptr)
{
}

Renderer::~Renderer()
{
    if (m_gpuProgramID != 0)
    {
        glDeleteProgram(m_gpuProgramID);
    }
}

bool Renderer::init(GLFWwindow* window)
{
    m_window = window;

    loadShadersFromFiles();

    m_vertexArrayObjectID = buildGeometry();

    m_modelUniform = glGetUniformLocation(m_gpuProgramID, "model");
    m_viewUniform = glGetUniformLocation(m_gpuProgramID, "view");
    m_projectionUniform = glGetUniformLocation(m_gpuProgramID, "projection");
    m_renderAsBlackUniform = glGetUniformLocation(m_gpuProgramID, "render_as_black");

    glEnable(GL_DEPTH_TEST);

    TextRendering_Init();

    return true;
}

void Renderer::setProjection(const glm::mat4& projection)
{
    m_currentProjection = projection;
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::setView(const glm::mat4& view)
{
    m_currentView = view;
    glUniformMatrix4fv(m_viewUniform, 1, GL_FALSE, glm::value_ptr(view));
}

void Renderer::renderScene(const Player& player, const EnemyManager& enemyManager)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_gpuProgramID);
    glBindVertexArray(m_vertexArrayObjectID);

    renderArena();

    renderPlayer(player.getPosition());

    renderEnemies(enemyManager, player.getPosition());

    renderCrosshair(player.isFirstPerson());
}

void Renderer::renderArena()
{
    glm::mat4 model = Matrix_Identity();
    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(m_renderAsBlackUniform, false);

    glDrawElements(
        m_virtualScene["piso"].rendering_mode,
        m_virtualScene["piso"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["piso"].first_index
    );

    glDrawElements(
        m_virtualScene["parede_norte"].rendering_mode,
        m_virtualScene["parede_norte"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["parede_norte"].first_index
    );
    glDrawElements(
        m_virtualScene["parede_sul"].rendering_mode,
        m_virtualScene["parede_sul"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["parede_sul"].first_index
    );
    glDrawElements(
        m_virtualScene["parede_leste"].rendering_mode,
        m_virtualScene["parede_leste"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["parede_leste"].first_index
    );
    glDrawElements(
        m_virtualScene["parede_oeste"].rendering_mode,
        m_virtualScene["parede_oeste"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["parede_oeste"].first_index
    );

    glDrawElements(
        m_virtualScene["teto"].rendering_mode,
        m_virtualScene["teto"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["teto"].first_index
    );
}

void Renderer::renderPlayer(const glm::vec4& position)
{
    glm::mat4 model = Matrix_Identity();

    PushMatrix(model);
    model = model * Matrix_Translate(position.x, position.y, position.z);
    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(m_renderAsBlackUniform, false);
    glDrawElements(
        m_virtualScene["cube_faces"].rendering_mode,
        m_virtualScene["cube_faces"].num_indices,
        GL_UNSIGNED_INT,
        (void*)m_virtualScene["cube_faces"].first_index
    );
    PopMatrix(model);
}

void Renderer::renderEnemies(const EnemyManager& enemyManager, const glm::vec4& playerPosition)
{
    const std::vector<Enemy>& enemies = enemyManager.getEnemies();
    glm::mat4 model = Matrix_Identity();

    for (size_t i = 0; i < enemies.size(); i++)
    {
        PushMatrix(model);
        model = model * Matrix_Translate(enemies[i].getX(), 0.101f, enemies[i].getZ());

        float rotation_angle = enemies[i].lookAt(playerPosition);
        model = model * Matrix_Rotate_Y(rotation_angle);

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_renderAsBlackUniform, false);
        glDrawElements(
            m_virtualScene["cube_faces"].rendering_mode,
            m_virtualScene["cube_faces"].num_indices,
            GL_UNSIGNED_INT,
            (void*)m_virtualScene["cube_faces"].first_index
        );
        glLineWidth(2.0f);
        glDrawElements(
            m_virtualScene["eixo_z"].rendering_mode,
            m_virtualScene["eixo_z"].num_indices,
            GL_UNSIGNED_INT,
            (void*)m_virtualScene["eixo_z"].first_index
        );
        PopMatrix(model);
    }
}

void Renderer::renderCrosshair(bool isFirstPerson)
{
    if (!isFirstPerson || m_window == nullptr)
        return; 

    TextRendering_PrintString(m_window, "+", 0.0f, 0.0f, 2.0f);
}

GLuint Renderer::buildGeometry()
{
    GLfloat model_coefficients[] = {
        -0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f, -0.1f,  0.1f, 1.0f,
         0.1f, -0.1f,  0.1f, 1.0f,
         0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f,  0.1f, -0.1f, 1.0f,
        -0.1f, -0.1f, -0.1f, 1.0f,
         0.1f, -0.1f, -0.1f, 1.0f,
         0.1f,  0.1f, -0.1f, 1.0f,
         1000.0f, 0.0f, 1000.0f, 1.0f,
         -1000.0f, 0.0f, 1000.0f, 1.0f,
         1000.0f, 0.0f, -1000.0f, 1.0f,
         -1000.0f, 0.0f, -1000.0f, 1.0f,
         0.0f,  0.0f,  0.0f, 1.0f,
         0.0f,  0.0f,  0.4f, 1.0f,
         2000.0f, 3.0f, 2000.0f, 1.0f,
         -2000.0f, 3.0f, 2000.0f, 1.0f,
         2000.0f, 3.0f, -2000.0f, 1.0f,
         -2000.0f, 3.0f, -2000.0f, 1.0f,
          5.0f, 0.0f, -5.0f, 1.0f,
         -5.0f, 0.0f, -5.0f, 1.0f,
         -5.0f, 3.0f, -5.0f, 1.0f,
          5.0f, 3.0f, -5.0f, 1.0f,
         -5.0f, 0.0f,  5.0f, 1.0f,
          5.0f, 0.0f,  5.0f, 1.0f,
          5.0f, 3.0f,  5.0f, 1.0f,
         -5.0f, 3.0f,  5.0f, 1.0f,
          5.0f, 0.0f, -5.0f, 1.0f,
          5.0f, 0.0f,  5.0f, 1.0f,
          5.0f, 3.0f,  5.0f, 1.0f,
          5.0f, 3.0f, -5.0f, 1.0f,
         -5.0f, 0.0f,  5.0f, 1.0f,
         -5.0f, 0.0f, -5.0f, 1.0f,
         -5.0f, 3.0f, -5.0f, 1.0f,
         -5.0f, 3.0f,  5.0f, 1.0f,
    };

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);
    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat color_coefficients[] = {
        1.0f, 0.5f, 0.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.3f, 0.3f, 0.3f, 1.0f,
        0.3f, 0.3f, 0.3f, 1.0f,
        0.3f, 0.3f, 0.3f, 1.0f,
        0.3f, 0.3f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
        0.5f, 0.4f, 0.3f, 1.0f,
    };

    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1;
    number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint indices[] = {
        0, 1, 2,
        7, 6, 5,
        3, 2, 6,
        4, 0, 3,
        4, 5, 1,
        1, 5, 6,
        0, 2, 3,
        7, 5, 4,
        3, 6, 7,
        4, 3, 7,
        4, 1, 0,
        1, 6, 2,
        8, 9, 10,
        9, 10, 11,
        12, 13,
        14, 15, 16,
        15, 17, 16,
        18, 19, 21,
        19, 20, 21,
        22, 23, 25,
        23, 24, 25,
        26, 27, 29,
        27, 28, 29,
        30, 31, 33,
        31, 32, 33
    };

    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = (void*)0;
    cube_faces.num_indices    = 36;
    cube_faces.rendering_mode = GL_TRIANGLES;
    m_virtualScene["cube_faces"] = cube_faces;

    SceneObject piso;
    piso.name = "Piso";
    piso.first_index = (void*)(36*sizeof(GLuint));
    piso.num_indices = 6;
    piso.rendering_mode = GL_TRIANGLES;
    m_virtualScene["piso"] = piso;

    SceneObject eixo_z;
    eixo_z.name = "Z";
    eixo_z.first_index = (void*)(42*sizeof(GLuint));
    eixo_z.num_indices = 2;
    eixo_z.rendering_mode = GL_LINES;
    m_virtualScene["eixo_z"] = eixo_z;

    SceneObject teto;
    teto.name = "Teto";
    teto.first_index = (void*)(44*sizeof(GLuint));
    teto.num_indices = 6;
    teto.rendering_mode = GL_TRIANGLES;
    m_virtualScene["teto"] = teto;

    SceneObject parede_norte;
    parede_norte.name = "Parede Norte";
    parede_norte.first_index = (void*)(50*sizeof(GLuint));
    parede_norte.num_indices = 6;
    parede_norte.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_norte"] = parede_norte;

    SceneObject parede_sul;
    parede_sul.name = "Parede Sul";
    parede_sul.first_index = (void*)(56*sizeof(GLuint));
    parede_sul.num_indices = 6;
    parede_sul.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_sul"] = parede_sul;

    SceneObject parede_leste;
    parede_leste.name = "Parede Leste";
    parede_leste.first_index = (void*)(62*sizeof(GLuint));
    parede_leste.num_indices = 6;
    parede_leste.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_leste"] = parede_leste;

    SceneObject parede_oeste;
    parede_oeste.name = "Parede Oeste";
    parede_oeste.first_index = (void*)(68*sizeof(GLuint));
    parede_oeste.num_indices = 6;
    parede_oeste.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_oeste"] = parede_oeste;

    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    glBindVertexArray(0);
    return vertex_array_object_id;
}

void Renderer::loadShadersFromFiles()
{
    GLuint vertex_shader_id = loadShader_Vertex("src/shaders/shader_vertex.glsl");
    GLuint fragment_shader_id = loadShader_Fragment("src/shaders/shader_fragment.glsl");
    if (m_gpuProgramID != 0)
        glDeleteProgram(m_gpuProgramID);
    m_gpuProgramID = createGpuProgram(vertex_shader_id, fragment_shader_id);
}

GLuint Renderer::loadShader_Vertex(const char* filename)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    loadShader(filename, vertex_shader_id);
    return vertex_shader_id;
}

GLuint Renderer::loadShader_Fragment(const char* filename)
{
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    loadShader(filename, fragment_shader_id);
    return fragment_shader_id;
}

void Renderer::loadShader(const char* filename, GLuint shader_id)
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
    const GLint   shader_string_length = static_cast<GLint>(str.length());
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
    glCompileShader(shader_id);
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);
    if (log_length != 0)
    {
        std::string output;

        if (!compiled_ok)
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }
    delete [] log;
}

GLuint Renderer::createGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
    if (linked_ok == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        GLchar* log = new GLchar[log_length];
        glGetProgramInfoLog(program_id, log_length, &log_length, log);
        std::string output;
        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";
        delete [] log;
        fprintf(stderr, "%s", output.c_str());
    }
    return program_id;
}
