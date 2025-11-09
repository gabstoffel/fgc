#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

struct SceneObject
{
    const char*  name;        
    void*        first_index; 
    int          num_indices; 
    GLenum       rendering_mode; 
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

    std::map<const char*, SceneObject> m_virtualScene;

    glm::mat4 m_currentView;
    glm::mat4 m_currentProjection;

    float m_screenRatio;

    GLFWwindow* m_window;
};

#endif 
