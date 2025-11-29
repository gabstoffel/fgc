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
    float getMovementAngle() const { return m_movementAngle; }

    void setPosition(const glm::vec4& position) { m_position = position; }

    void updatePositionAfterCollision(const glm::vec3& newPosition);

    int getVida() const { return m_vida; }
    int getMaxVida() const { return m_maxVida; }
    void takeDamage(int damage);
    bool isDead() const { return m_vida <= 0; }
    void setVida(int vida, int maxVida);
    void reset();

private:
    glm::vec4 m_position;

    bool m_firstPerson;

    float m_cameraTheta;    
    float m_cameraPhi;      
    float m_cameraDistance; 

    float m_cameraYaw;
    float m_cameraPitch;

    float m_movementSpeed;
    float m_movementAngle;

    double m_lastCursorPosX;
    double m_lastCursorPosY;

    int m_vida;
    int m_maxVida;
    float m_damageCooldown;
    float m_damageCooldownTimer;
};

#endif 
