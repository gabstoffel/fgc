#ifndef PLAYER_H
#define PLAYER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

class Player
{
public:
    Player();
    ~Player();

    void update(GLFWwindow* window, float deltaTime);

    glm::mat4 getCameraView() const;
    void toggleCamera();
    void handleMouseMove(float dx, float dy);
    void handleScroll(float offset);

    glm::vec4 getPosition() const { return m_position; }
    bool isFirstPerson() const { return m_firstPerson; }
    glm::vec4 getCameraDirection() const;
    glm::vec4 getCameraPosition() const;

    void setPosition(const glm::vec4& position) { m_position = position; }

    void updatePositionAfterCollision(const glm::vec3& newPosition);

private:
    glm::vec4 m_position;

    bool m_firstPerson;

    float m_cameraTheta;    
    float m_cameraPhi;      
    float m_cameraDistance; 

    float m_cameraYaw;      
    float m_cameraPitch;    

    float m_movementSpeed;

    double m_lastCursorPosX;
    double m_lastCursorPosY;
};

#endif 
