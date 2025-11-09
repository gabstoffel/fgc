#include "Player.h"
#include "matrices.h"
#include <cmath>

Player::Player()
    : m_position(0.0f, 0.101f, 0.0f, 1.0f)
    , m_firstPerson(false)
    , m_cameraTheta(0.0f)
    , m_cameraPhi(0.5f)
    , m_cameraDistance(2.0f)
    , m_cameraYaw(-1.57079632f)
    , m_cameraPitch(0.0f)
    , m_movementSpeed(0.01f)
    , m_lastCursorPosX(0.0)
    , m_lastCursorPosY(0.0)
{
}

Player::~Player()
{
}

void Player::update(GLFWwindow* window, float deltaTime)
{
    if (m_firstPerson)
    {
        glm::vec4 camera_front_xz = glm::vec4(sin(m_cameraYaw), 0.0f, cos(m_cameraYaw), 0.0f);
        camera_front_xz = camera_front_xz / norm(camera_front_xz);

        glm::vec4 camera_right_xz = glm::vec4(sin(m_cameraYaw - 3.141592f/2.0f), 0.0f, cos(m_cameraYaw - 3.141592f/2.0f), 0.0f);
        camera_right_xz = camera_right_xz / norm(camera_right_xz);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_position += camera_front_xz * m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            m_position -= camera_front_xz * m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            m_position -= camera_right_xz * m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            m_position += camera_right_xz * m_movementSpeed;
    }
    else
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_position.z -= m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            m_position.z += m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            m_position.x -= m_movementSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            m_position.x += m_movementSpeed;
    }
}

glm::mat4 Player::getCameraView() const
{
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

        return Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
    }
}

void Player::toggleCamera()
{
    m_firstPerson = !m_firstPerson;
}

void Player::handleMouseMove(float dx, float dy)
{
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
    }
    else
    {
        float sensitivity = 0.003f;

        m_cameraYaw   -= sensitivity * dx;
        m_cameraPitch += sensitivity * dy;

        float pitchmax = 3.141592f / 2.0f - 0.01f;
        float pitchmin = -pitchmax;
        if (m_cameraPitch > pitchmax)
            m_cameraPitch = pitchmax;
        if (m_cameraPitch < pitchmin)
            m_cameraPitch = pitchmin;
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
        return glm::vec4(x, y, z, 1.0f);
    }
}
