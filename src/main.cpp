//Headers C
#include <cmath>
#include <cstdio>
#include <cstdlib>
//Chamadas tipos de dados
#include <set>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
//Headers para OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>
//Funções auxilaires
#include "utils.h"
#include "matrices.h"
// Estrutura que representa um modelo geométrico carregado a partir de um .obj
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Achar caminho correto caso basepath=NULL
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }
        //Carrega objeto
        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            //Objeto sem nome
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }
        printf("OK.\n");
    }
};
// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);
//Funções lógicas
float DiferencaAngulo(glm::vec4 v, glm::vec4 u);
//Declaração de funções iniciais
void BuildTriangles(ObjModel*);
void ComputeNormals(ObjModel* model);
void LoadShadersFromFiles();
void DrawVirtualObject(const char* object_name);
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


// Definimos uma estrutura que armazenará dados necessários para renderizar cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t        first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTriangles()
    size_t          num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTriangles()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
};

// Estrutura referente a um inimigo no jogo
struct Inimigo
{
    float x; //Posição x do inimgo
    float z; //Posição z do inimgo
    int vida; //Vida atual do inimigo
};
 //Váriávies globais
std::map<std::string, SceneObject> g_VirtualScene;
float g_ScreenRatio = 1.0f;
bool g_LeftMouseButtonPressed = false;
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.5f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.0f; // Distância da câmera para a origem
GLuint g_GpuProgramID = 0;
//Varíavel de flag do modo de camera (true = primeira pessoa, false = look-at)
bool first_person = false;
//Váriavel do ponto da posição do jogador no plano (look-at sempre aponta para o jogador)
glm::vec4 pos_player = glm::vec4 (0.0f,0.101f,0.0f,1.0f);
// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;
// Variáveis que definem um programa de GPU (shaders)
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;
int main(int argc, char* argv[])
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
    //Carrega modelos .obj e sontrói em malhas de triângulos
    ObjModel monstermodel("../../modelos/monstro.obj");
    ComputeNormals(&monstermodel);
    BuildTriangles(&monstermodel);
    ObjModel cubemodel("../../modelos/cube.obj");
    ComputeNormals(&cubemodel);
    BuildTriangles(&cubemodel);
    ObjModel planemodel("../../modelos/plane.obj");
    ComputeNormals(&planemodel);
    BuildTriangles(&planemodel);
    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTriangles(&model);
    }

    // Habilitamos o Z-buffer.
    glEnable(GL_DEPTH_TEST);
    //Backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;
    //Váriavel para impedir ações controladas por tempo ocorrerem várias vezes no mesmo segundo
    int segundo_anterior=(int)glfwGetTime();
    //Loop de renderização e lógica do jogo
    while (!glfwWindowShouldClose(window))
    {
        //Captura tempo para controlar spawn de inimigos
        int segundos = (int)glfwGetTime();
        //Inimigo spawna a cada 5 segundos, máximo de 5 inimigos
        if(segundos%5==0 && inimigos.size()<5 && segundo_anterior!=segundos){
            Inimigo novo_inimigo;
            //Gera um numero aleatório de -0.9 a 0.9
            float x_aletorio =  1.8f * rand() / (static_cast <float> (RAND_MAX))-0.9; //Rand/RAND_MAX retorna valor entre 0.0 e 1.0, multiplicando por 1.8 vai de 0.0 a 1.8, subtraindo 0.9 vai de -0.9 a 0.9
            float z_aletorio =  1.8f * rand() / (static_cast <float> (RAND_MAX))-0.9;
            //Gera até se afastar suficientemente do player, considera que player e inimigos sempre estão no mesmo ponto y
            while((x_aletorio-pos_player.x)*(x_aletorio-pos_player.x)+(z_aletorio-pos_player.z)*(z_aletorio-pos_player.z)<0.1){
                x_aletorio = 1.8f * rand() / (static_cast <float> (RAND_MAX))-0.9;
                z_aletorio = 1.8f * rand() / (static_cast <float> (RAND_MAX))-0.9;
            }
            //Salva informações do novo inimigo e guarda no vetor
            novo_inimigo.x = x_aletorio;
            novo_inimigo.z = z_aletorio;
            novo_inimigo.vida = 100;
            inimigos.push_back(novo_inimigo);
        }
        //Garantia de realizar ações temporizadas apenas uma vez quando chegar seu tempo
        segundo_anterior=segundos;
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(g_GpuProgramID);
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
        #define MONSTRO 0
        #define CUBE 1
        #define PLANO 2
        // Desenho de coubo para teste de câmera
        glm::mat4 model;
        // Matriz do modelo
        model = Matrix_Identity();
        //Matriz vai para GPU
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(object_id_uniform, 2);
        //Desenho do piso
        DrawVirtualObject("the_plane");
        //Cubo translado para posição do player
        PushMatrix(model);
            //Posição do player
            model = model*Matrix_Translate(pos_player.x,pos_player.y,pos_player.z)*Matrix_Scale(0.1f,0.1f,0.1f);
            // Matriz vai para GPU
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(object_id_uniform, 1);
            // Desenho do cubo
            DrawVirtualObject("the_cube");
        PopMatrix(model);
        //Desenho de inimigos se existirem
        for(int inimigos_presentes =0; inimigos_presentes<inimigos.size(); inimigos_presentes++){
            PushMatrix(model);
            //Posição inimigo
            glm::vec4 vetor_front = glm::vec4(0.0f,0.0f,1.0f,0.0f);
            model = model*Matrix_Translate(inimigos[inimigos_presentes].x,0.101f,inimigos[inimigos_presentes].z)*Matrix_Scale(0.15f,0.15f,0.15f);
            //Rotação inimigo
            glm::vec4 olha_player = pos_player - glm::vec4(inimigos[inimigos_presentes].x, 0.101f, inimigos[inimigos_presentes].z, 1.0f);
            //Verifica diferença de ângulo entre para onde o inimigo está olhando e o vetor do inimigo pro jogador
            float diff_angulo = DiferencaAngulo(vetor_front,olha_player);
            model = model*Matrix_Rotate_Y(diff_angulo);
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(object_id_uniform, 0);
            // Desenho do cubo
            DrawVirtualObject("turle");
            PopMatrix(model);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        //Movimento básico para testar rotação dos inimigos
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            pos_player.z -= 0.0001f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            pos_player.z += 0.0001f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            pos_player.x -= 0.0001f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            pos_player.x += 0.0001f;
        }
        }
    glfwTerminate();
    // Fim do programa
    return 0;
}

//Calcula a diferença de ângulo entre 2 vetores
float DiferencaAngulo(glm::vec4 v, glm::vec4 u){
    //Divisão por 0 dá erro, evitar
    if(norm(v)==0||norm(u)==0){
        return 0.0;
    }
    //cos(angulo) = v.u/||v||*||u|| (função para descobrir diferença de ângulos entre vetores) -> angulo = arccos(v.u/||v||*||u||)
    float angulo = acos(dotproduct(u,v)/(norm(v)*norm(u)));
    //Verifica qual o sinal correto do ângulo para aplicar (se u está a esquerda ou direita de v)
    if(u.x*v.z-u.z*v.x<0){
        angulo=-angulo;
    }
    return angulo;
}

// Função que desenha um objeto armazenado em g_VirtualScene
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ apontados pelo VAO como linhas
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);
}

// Função que computa as normais de um ObjModel
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".
    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        assert(model->shapes[shape].mesh.smoothing_group_ids.size() == num_triangles);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            assert(sgroup >= 0);
            sgroup_ids.insert(sgroup);
        }
    }
    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );
    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));
        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];
                if (sgroup_tri != sgroup)
                    continue;
                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }
                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];
                //Cria vetor u = b-a e v = c-a
                glm::vec4 u = glm::vec4(b - a);
                glm::vec4 v = glm::vec4(c - a);
                //Produto vetorial
                const glm::vec4  n = crossproduct(u,v);
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }
        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);
        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;
            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);
            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }
        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];
                if (sgroup_tri != sgroup)
                    continue;
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
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
void BuildTriangles(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                indices.push_back(first_index + 3*triangle + vertex);
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W
                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.
                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }
                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }
        size_t last_index = indices.size() - 1;
        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;
        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
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
    model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
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

