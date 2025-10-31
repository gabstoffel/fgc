//Headers
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <stack>
#include <random>
#include <limits>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils.h"
#include "matrices.h"
// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);
//Declaração de funções iniciais
GLuint BuildTriangles();
void LoadShadersFromFiles();
GLuint LoadShader_Vertex(const char* filename);
GLuint LoadShader_Fragment(const char* filename);
void LoadShader(const char* filename, GLuint shader_id);
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);
// Funções callback
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    const char*  name;        // Nome do objeto
    void*        first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTriangles()
    int          num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTriangles()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
};

struct Inimigo
{
    float x; //Posição x do inimgo
    float z; //Posição z do inimgo
    int vida; //Vida atual do inimigo
};
 //Váriávies globais
std::map<const char*, SceneObject> g_VirtualScene;
float g_ScreenRatio = 1.0f;
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;
bool g_LeftMouseButtonPressed = false;
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.5f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.0f; // Distância da câmera para a origem
GLuint g_GpuProgramID = 0;
//Varíavel de flag do modo de camera (true = primeira pessoas, false = look-at)
bool first_person = false;
//Váriavel do ponto da posição do jogador no plano (look-at sempre aponta para o jogador)
glm::vec4 pos_player = glm::vec4 (0.0f,0.1f,0.0f,1.0f);
// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

int main()
{
    std::vector<Inimigo> inimigos;
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }
    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Implementação Inicial", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 600);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    LoadShadersFromFiles();
    GLuint vertex_array_object_id = BuildTriangles();
    GLint model_uniform           = glGetUniformLocation(g_GpuProgramID, "model");
    GLint view_uniform            = glGetUniformLocation(g_GpuProgramID, "view");
    GLint projection_uniform      = glGetUniformLocation(g_GpuProgramID, "projection");
    GLint render_as_black_uniform = glGetUniformLocation(g_GpuProgramID, "render_as_black");
    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;
    //Loop de renderização
    int segundo_anterior=0;
    while (!glfwWindowShouldClose(window))
    {
        //Captura tempo para controlar spawn de inimigos
        int segundos = (int)glfwGetTime();
        //Inimigo spawna a cada 5 segundos, máximo de 5 inimigos
        if(segundos%5==0 && inimigos.size()<5 && segundo_anterior!=segundos){
            Inimigo novo_inimigo;
            //Gera um numero aleatório de -1.0 a 1.0
            float x_aletorio =  2.0f * rand() / (static_cast <float> (RAND_MAX))-1.0;
            float z_aletorio =  2.0f * rand() / (static_cast <float> (RAND_MAX))-1.0;
            //Gera até se afastar suficientemente do player
            while((x_aletorio-pos_player.x)*(x_aletorio-pos_player.x)+(z_aletorio-pos_player.z)*(z_aletorio-pos_player.z)<0.1){
                x_aletorio = 2.0f * rand() / (static_cast <float> (RAND_MAX))-1.0;
                z_aletorio = 2.0f * rand() / (static_cast <float> (RAND_MAX))-1.0;
            }
            novo_inimigo.x = x_aletorio;
            novo_inimigo.z = z_aletorio;
            novo_inimigo.vida = 100;
            inimigos.push_back(novo_inimigo);
        }
        segundo_anterior=segundos;
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(g_GpuProgramID);
        glBindVertexArray(vertex_array_object_id);
        //Declaração amtriz view
        glm::mat4 view;
        //Posição e distância da câmera
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);
        //Câmera look-at só ativada com flag false
        if(!first_person){
            glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Centro da câmera
            glm::vec4 camera_lookat_l    = pos_player; // Camerâ mirada no ponto que se localiza o jogador
            glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; //Sentido da câmera
            glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); //Vetor Up
            //Definição da matriz de view da câmera
            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        }

        //Senão ativa camêra primeria pessoas
        else{

        }
        //Matriz projeção
        glm::mat4 projection;
        // Planos near e far
        float nearplane = -0.1f;
        float farplane  = -10.0f;
        // Projeção Perspectiva.
        //FOV
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        // Matrizes vão para GPU
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        // Desenho de coubo para teste de câmera
        glm::mat4 model;
        // Matriz do modelo
        model = Matrix_Identity();
        //Matriz vai para GPU
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(render_as_black_uniform, false);
        //Desenho do piso
        glDrawElements(
            g_VirtualScene["piso"].rendering_mode,
            g_VirtualScene["piso"].num_indices,
            GL_UNSIGNED_INT,
            (void*)g_VirtualScene["piso"].first_index
        );
        //Cubo translado para posição do player
        PushMatrix(model);
            //Posição do player
            model = model*Matrix_Translate(pos_player.x,pos_player.y,pos_player.z);
            // Matriz vai para GPU
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(render_as_black_uniform, false);
            // Desenho do cubo
            glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode,
                g_VirtualScene["cube_faces"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
            );
        PopMatrix(model);
        //Desenho de inimigos se existirem
        for(int inimigos_presentes =0; inimigos_presentes<inimigos.size(); inimigos_presentes++){
            PushMatrix(model);
            //Posição inimigo
            model = model*Matrix_Translate(inimigos[inimigos_presentes].x,0.1f,inimigos[inimigos_presentes].z);
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(render_as_black_uniform, false);
            // Desenho do cubo
            glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode,
                g_VirtualScene["cube_faces"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
            );
            PopMatrix(model);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    // Fim do programa
    return 0;
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

//Construção de triângulos
GLuint BuildTriangles()
{
    //Gemoetria
    GLfloat model_coefficients[] = {
        //Cubo
        -0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f, -0.1f,  0.1f, 1.0f,
         0.1f, -0.1f,  0.1f, 1.0f,
         0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f,  0.1f, -0.1f, 1.0f,
        -0.1f, -0.1f, -0.1f, 1.0f,
         0.1f, -0.1f, -0.1f, 1.0f,
         0.1f,  0.1f, -0.1f, 1.0f,
         //Piso
         1.0f, 0.0f, 1.0f, 1.0f,
         -1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, 0.0f, -1.0f, 1.0f,
         -1.0f, 0.0f, -1.0f, 1.0f,
    };

    //VBO
    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    //VAO
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    //VAO ligado
    glBindVertexArray(vertex_array_object_id);
    //VBO ligado
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    //Aloca memória
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);
    //Copia para o VBO
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);
    //Localização
    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    //Ativa atributos
    glEnableVertexAttribArray(location);
    // Desliga VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Cor dos vértices
    GLfloat color_coefficients[] = {
        //Cubo
        1.0f, 0.5f, 0.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        //Piso
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
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

    // Definição polígonos
    GLuint indices[] = {
    //Faces Cubo
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
    //Faces Piso
        8, 9, 10,
        9, 10, 11,
    };
    // Criação de objeto virtual
    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = (void*)0;
    cube_faces.num_indices    = 36;
    cube_faces.rendering_mode = GL_TRIANGLES;
    // Adiciona a cena
    g_VirtualScene["cube_faces"] = cube_faces;
    //Piso
    SceneObject piso;
    piso.name = "Piso";
    piso.first_index = (void*)(36*sizeof(GLuint));
    piso.num_indices = 6;
    piso.rendering_mode = GL_TRIANGLES;
    //Adiciona a cena
    g_VirtualScene["piso"] = piso;
    // Vetor de índices
    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    // Liga o buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    // Aloca memória
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    // Copia para o buffer
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    //Desliga VAO
    glBindVertexArray(0);
    // Retorna ID do VAO
    return vertex_array_object_id;
}

// Carrega Vertex Shader
GLuint LoadShader_Vertex(const char* filename)
{
    // ID do shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    // Carrega shader
    LoadShader(filename, vertex_shader_id);
    // Retorna ID
    return vertex_shader_id;
}
// Carrega  Fragment Shader .
GLuint LoadShader_Fragment(const char* filename)
{
    // ID do shader
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    // Carrega shader
    LoadShader(filename, fragment_shader_id);
    // Retorna ID
    return fragment_shader_id;
}
// Compila Shader
void LoadShader(const char* filename, GLuint shader_id)
{
    // Leitura do arquivo
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );
    //Definição código
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
    //Compila código do shader
    glCompileShader(shader_id);
    // Verificação de sucesso na compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
    // Salva log
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);
    // Imprime no terminal
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
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
    //Libera espaço
    delete [] log;
}

// Função que busca shaders
void LoadShadersFromFiles()
{
    //Definir de acordo com caminho correto para os shaders
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shaders/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shaders/shader_fragment.glsl");
    // Deletamos programa anterior se existir
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);
    // Criamos programa com shaders achados
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
}

// Cria programa com shader
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    //Id do programa
    GLuint program_id = glCreateProgram();
    //Shaders escolhidos
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    // Link dos shaders ao programa
    glLinkProgram(program_id);
    // Verificação de sucesso
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
    // Log
    if ( linked_ok == GL_FALSE )
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
    // Retorna ID
    return program_id;
}

// Redimensão janela do programa
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    // Atualiza ratio da tela
    g_ScreenRatio = (float)width / height;
}

// Pos do mouse
double g_LastCursorPosX, g_LastCursorPosY;

// Clqiue do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //Posição do mouse ao ser clicado
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        //Liga flag
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        //Baixa flag ao "descliclar" do mouse
        g_LeftMouseButtonPressed = false;
    }
}

// movimentação do cursor
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    //Movimenta apenas com cursor pressionado (quando for look-at)
    if (!g_LeftMouseButtonPressed && !first_person)
        return;
    // Deslocamento do mouse
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;
    // Atualiza câmera
    g_CameraTheta -= 0.01f*dx;
    g_CameraPhi   += 0.01f*dy;
    float phimax = 3.141592f/2;
    float phimin = -phimax;
    if (g_CameraPhi > phimax)
        g_CameraPhi = phimax;
    if (g_CameraPhi < phimin)
        g_CameraPhi = phimin;
    // Atualiza posição do cursor
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
}

// Zoom-out e zoom-in
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualzia distância da câmera
    g_CameraDistance -= 0.1f*yoffset;

    // Restrição de zoom máximo e zoom mínimo
    float zoom_min=0.4;
    float zoom_max=2;
    if (g_CameraDistance < zoom_min){
        g_CameraDistance = zoom_min;
    }
    if(g_CameraDistance > zoom_max){
        g_CameraDistance = zoom_max;
    }
}

// Clique de teclas
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    //Se usuário apertar a letra F, troca o modo de câmera
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if(first_person){
            first_person=false;
        }
        else{
            first_person=true;
        }
    }
}

// Impressão de erros
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

