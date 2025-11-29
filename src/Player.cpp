#include "Player.h"
#include "matrices.h"
#include "Logger.h"
#include <cmath>

Player::Player()
    : m_position(3.5f, 0.101f, 0.0f, 1.0f)
    , m_firstPerson(true)  
    , m_cameraTheta(0.0f)
    , m_cameraPhi(0.5f)
    , m_cameraDistance(2.0f)
    , m_cameraYaw(-1.57079632f)
    , m_cameraPitch(0.0f)
    , m_movementSpeed(0.4f)
    , m_movementAngle(0.0f)
    , m_lastCursorPosX(0.0)
    , m_lastCursorPosY(0.0)
    , m_vida(100)
    , m_maxVida(100)
    , m_damageCooldown(1.0f)
    , m_damageCooldownTimer(0.0f)
{
}

Player::~Player()
{
}

void Player::update(GLFWwindow* window, float deltaTime)
{
    if (m_damageCooldownTimer > 0.0f)
        m_damageCooldownTimer -= deltaTime;

    static int frame_count = 0;
    bool should_log = (frame_count++ % 60 == 0);

    glm::vec2 movement_input(0.0f, 0.0f);

    if (m_firstPerson)
    {
        glm::vec4 camera_front_xz = glm::vec4(sin(m_cameraYaw), 0.0f, cos(m_cameraYaw), 0.0f);
        camera_front_xz = camera_front_xz / norm(camera_front_xz);

        glm::vec4 camera_right_xz = glm::vec4(sin(m_cameraYaw - 3.141592f/2.0f), 0.0f, cos(m_cameraYaw - 3.141592f/2.0f), 0.0f);
        camera_right_xz = camera_right_xz / norm(camera_right_xz);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            m_position += camera_front_xz * m_movementSpeed*deltaTime;
            movement_input.y += 1.0f;
            if (should_log) printf("[FP Movement] W pressed, pos: (%.2f, %.2f, %.2f)\n", m_position.x, m_position.y, m_position.z);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            m_position -= camera_front_xz * m_movementSpeed*deltaTime;
            movement_input.y -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            m_position -= camera_right_xz * m_movementSpeed*deltaTime;
            movement_input.x -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            m_position += camera_right_xz * m_movementSpeed*deltaTime;
            movement_input.x += 1.0f;
        }

        if (movement_input.x != 0.0f || movement_input.y != 0.0f)
        {
            m_movementAngle = atan2(movement_input.x, movement_input.y) + m_cameraYaw;
        }
    }
    else
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            m_position.z -= m_movementSpeed*deltaTime;
            movement_input.y += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            m_position.z += m_movementSpeed*deltaTime;
            movement_input.y -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            m_position.x -= m_movementSpeed*deltaTime;
            movement_input.x -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            m_position.x += m_movementSpeed*deltaTime;
            movement_input.x += 1.0f;
        }

        if (movement_input.x != 0.0f || movement_input.y != 0.0f)
        {
            m_movementAngle = atan2(movement_input.x, movement_input.y);
        }
    }

    if (should_log)
        printf("[Player] Pos: (%.2f,%.2f,%.2f) Yaw:%.2f Pitch:%.2f FP:%d Angle:%.2f\n",
               m_position.x, m_position.y, m_position.z, m_cameraYaw, m_cameraPitch, m_firstPerson, m_movementAngle);
}

glm::mat4 Player::getCameraView() const
{
    static int view_log_count = 0;
    bool should_log = (view_log_count++ % 60 == 0);

    if (!m_firstPerson)
    {
        float r = m_cameraDistance;
        float y = r * sin(m_cameraPhi);
        float z = r * cos(m_cameraPhi) * cos(m_cameraTheta);
        float x = r * cos(m_cameraPhi) * sin(m_cameraTheta);

        glm::vec4 camera_position_c  = glm::vec4(x, y, z, 1.0f);
        glm::vec4 camera_lookat_l    = m_position;
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
        glm::vec4 camera_up_vector   = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        return Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
    }
    else
    {
        float eye_height = 0.15f;
        glm::vec4 camera_position_c = m_position + glm::vec4(0.0f, eye_height, 0.0f, 0.0f);

        float camera_view_x = cos(m_cameraPitch) * sin(m_cameraYaw);
        float camera_view_y = sin(m_cameraPitch);
        float camera_view_z = cos(m_cameraPitch) * cos(m_cameraYaw);
        glm::vec4 camera_view_vector = glm::vec4(camera_view_x, camera_view_y, camera_view_z, 0.0f);

        glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        if (should_log)
            Logger::logEvent("Player.getCameraView.firstPerson",
                "{\"yaw\":%.2f,\"pitch\":%.2f,\"viewDir\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},\"camPos\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}}",
                m_cameraYaw, m_cameraPitch,
                camera_view_x, camera_view_y, camera_view_z,
                camera_position_c.x, camera_position_c.y, camera_position_c.z);

        return Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
    }
}

void Player::toggleCamera()
{
    bool was_fp = m_firstPerson;
    m_firstPerson = !m_firstPerson;
    Logger::logEvent("Player.toggleCamera",
        "{\"wasFirstPerson\":%s,\"nowFirstPerson\":%s}",
        was_fp ? "true" : "false",
        m_firstPerson ? "true" : "false");
}

void Player::handleMouseMove(float dx, float dy)
{
    static int log_counter = 0;
    bool should_log = (log_counter++ % 30 == 0 && (dx != 0 || dy != 0));

    if (!m_firstPerson)
    {
        m_cameraTheta -= 0.01f * dx;
        m_cameraPhi   += 0.01f * dy;

        float phimax = 3.141592f / 2;
        float phimin = 0.05f;
        if (m_cameraPhi > phimax)
            m_cameraPhi = phimax;
        if (m_cameraPhi < phimin)
            m_cameraPhi = phimin;

        if (should_log)
            Logger::logEvent("Player.handleMouseMove.thirdPerson",
                "{\"dx\":%.1f,\"dy\":%.1f,\"theta\":%.2f,\"phi\":%.2f}",
                dx, dy, m_cameraTheta, m_cameraPhi);
    }
    else
    {
        float sensitivity = 0.001f;
        float old_yaw = m_cameraYaw;
        float old_pitch = m_cameraPitch;

        m_cameraYaw   -= sensitivity * dx;
        m_cameraPitch += sensitivity * dy;

        float pitchmax = 3.141592f / 2.0f - 0.01f;
        float pitchmin = -pitchmax;
        if (m_cameraPitch > pitchmax)
            m_cameraPitch = pitchmax;
        if (m_cameraPitch < pitchmin)
            m_cameraPitch = pitchmin;

        if (should_log)
            Logger::logEvent("Player.handleMouseMove.firstPerson",
                "{\"dx\":%.1f,\"dy\":%.1f,\"oldYaw\":%.2f,\"oldPitch\":%.2f,\"newYaw\":%.2f,\"newPitch\":%.2f}",
                dx, dy, old_yaw, old_pitch, m_cameraYaw, m_cameraPitch);
    }
}

void Player::handleScroll(float offset)
{
    m_cameraDistance -= 0.1f * offset;

    float zoom_min = 0.4f;
    float zoom_max = 2.0f;
    if (m_cameraDistance < zoom_min)
        m_cameraDistance = zoom_min;
    if (m_cameraDistance > zoom_max)
        m_cameraDistance = zoom_max;
}

void Player::updatePositionAfterCollision(const glm::vec3& newPosition)
{
    m_position.x = newPosition.x;
    m_position.y = newPosition.y;
    m_position.z = newPosition.z;
}

glm::vec4 Player::getCameraDirection() const
{
    if (m_firstPerson)
    {
        float camera_view_x = cos(m_cameraPitch) * sin(m_cameraYaw);
        float camera_view_y = sin(m_cameraPitch);
        float camera_view_z = cos(m_cameraPitch) * cos(m_cameraYaw);
        return glm::vec4(camera_view_x, camera_view_y, camera_view_z, 0.0f);
    }
    else
    {
        float r = m_cameraDistance;
        float y = r * sin(m_cameraPhi);
        float z = r * cos(m_cameraPhi) * cos(m_cameraTheta);
        float x = r * cos(m_cameraPhi) * sin(m_cameraTheta);

        glm::vec4 camera_position = glm::vec4(x, y, z, 1.0f);
        glm::vec4 direction = m_position - camera_position;
        return direction / norm(direction);
    }
}

glm::vec4 Player::getCameraPosition() const
{
    if (m_firstPerson)
    {
        float eye_height = 0.15f;
        return m_position + glm::vec4(0.0f, eye_height, 0.0f, 0.0f);
    }
    else
    {
        float r = m_cameraDistance;
        float y = r * sin(m_cameraPhi);
        float z = r * cos(m_cameraPhi) * cos(m_cameraTheta);
        float x = r * cos(m_cameraPhi) * sin(m_cameraTheta);
        return m_position + glm::vec4(x, y, z, 0.0f);
    }
}

void Player::takeDamage(int damage)
{
    if (m_damageCooldownTimer <= 0.0f)
    {
        m_vida -= damage;
        if (m_vida < 0)
            m_vida = 0;
        m_damageCooldownTimer = m_damageCooldown;
        printf("[Player] Took %d damage! HP: %d/%d\n", damage, m_vida, m_maxVida);
    }
}

void Player::heal(int amount)
{
    m_vida += amount;
    if (m_vida > m_maxVida)
        m_vida = m_maxVida;
    printf("[Player] Healed %d HP! HP: %d/%d\n", amount, m_vida, m_maxVida);
}

void Player::setVida(int vida, int maxVida)
{
    m_maxVida = maxVida;
    m_vida = vida;
}

void Player::reset()
{
    m_position = glm::vec4(3.5f, 0.101f, 0.0f, 1.0f);
    m_vida = m_maxVida;
    m_damageCooldownTimer = 0.0f;
    m_firstPerson = true;  
    m_cameraTheta = 0.0f;
    m_cameraPhi = 0.5f;
    m_cameraDistance = 2.0f;
    m_cameraYaw = -1.57079632f;  
    m_cameraPitch = 0.0f;
}
