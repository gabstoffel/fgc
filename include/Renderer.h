#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <tiny_obj_loader.h>

struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true);
};

struct SceneObject
{
    std::string  name;
    size_t       first_index;
    size_t       num_indices;
    GLenum       rendering_mode;
    GLuint       vertex_array_object_id;
};

class Player;
class EnemyManager;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool init(GLFWwindow* window);

    void renderScene(const Player& player, const EnemyManager& enemyManager);

    void renderArena();
    void renderPlayer(const glm::vec4& position);
    void renderEnemies(const EnemyManager& enemyManager, const glm::vec4& playerPosition);
    void renderCrosshair(bool isFirstPerson);

    void setProjection(const glm::mat4& projection);
    void setView(const glm::mat4& view);

    GLuint getGpuProgramID() const { return m_gpuProgramID; }
    float getScreenRatio() const { return m_screenRatio; }
    void setScreenRatio(float ratio) { m_screenRatio = ratio; }

private:
    GLuint buildGeometry();

    void computeNormals(ObjModel* model);
    void buildTrianglesFromObj(ObjModel* model);
    void drawVirtualObject(const std::string& object_name);

    void loadShadersFromFiles();
    GLuint loadShader_Vertex(const char* filename);
    GLuint loadShader_Fragment(const char* filename);
    void loadShader(const char* filename, GLuint shader_id);
    GLuint createGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);

    GLuint m_vertexArrayObjectID;
    GLuint m_gpuProgramID;

    GLint m_modelUniform;
    GLint m_viewUniform;
    GLint m_projectionUniform;
    GLint m_renderAsBlackUniform;
    GLint m_objectIdUniform;

    std::map<std::string, SceneObject> m_virtualScene;

    glm::mat4 m_currentView;
    glm::mat4 m_currentProjection;

    float m_screenRatio;

    GLFWwindow* m_window;
};

#endif
