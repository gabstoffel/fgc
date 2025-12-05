// ============================================================================
// PLAYER.CPP - Controle do Jogador e Câmeras
// ============================================================================
//
// Este arquivo implementa o controle do jogador, incluindo:
// - Movimento com WASD (relativo à câmera em primeira pessoa)
// - Dois modos de câmera: primeira pessoa (livre) e terceira pessoa (look-at)
// - Física básica (gravidade, pulo)
//
// REQUISITOS IMPLEMENTADOS:
// - REQUISITO 2: Transformações geométricas controladas pelo usuário
//   (translação com WASD, rotação com mouse)
// - REQUISITO 3: Câmera livre e câmera look-at
// - REQUISITO 10: Animações baseadas no tempo (deltaTime)
//
// ============================================================================

#include "Player.h"
#include "matrices.h"
#include "Logger.h"
#include "sfx.h"
#include <cmath>

Player::Player()
    : m_position(3.5f, 0.101f, 0.0f, 1.0f)
    , m_velocityY(0.0f)
    , m_gravity(9.8f)
    , m_jumpForce(2.5f)
    , m_isGrounded(true)
    , m_firstPerson(true)
    , m_cameraTheta(0.0f)
    , m_cameraPhi(0.5f)
    , m_cameraDistance(1.2f)
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

// ============================================================================
// ATUALIZAÇÃO DO JOGADOR
// ============================================================================
// REQUISITO 2: Transformações geométricas controladas pelo usuário
// REQUISITO 10: Animações baseadas no tempo
//
// O movimento do jogador é calculado usando:
//     novaPos = posicaoAtual + direcao * velocidade * deltaTime
//
// Em PRIMEIRA PESSOA:
//     - W/S: move na direção que a câmera está olhando (vetor front)
//     - A/D: move perpendicular à direção da câmera (vetor right)
//     - O vetor front é calculado a partir do ângulo yaw da câmera
//
// Em TERCEIRA PESSOA (Look-At):
//     - Movimento é relativo aos eixos globais X e Z
//     - Independente da rotação da câmera
// ============================================================================
void Player::update(GLFWwindow* window, float deltaTime)
{
    // Atualiza timer de cooldown de dano (evita dano múltiplo instantâneo)
    if (m_damageCooldownTimer > 0.0f)
        m_damageCooldownTimer -= deltaTime;

    static int frame_count = 0;
    bool should_log = (frame_count++ % 60 == 0);

    glm::vec2 movement_input(0.0f, 0.0f);

    // ─────────────────────────────────────────────────────────────────────────
    // MOVIMENTO EM PRIMEIRA PESSOA (Câmera Livre)
    // ─────────────────────────────────────────────────────────────────────────
    // O movimento é relativo à direção que a câmera está olhando
    // camera_front_xz: vetor unitário na direção do olhar (projetado no plano XZ)
    // camera_right_xz: vetor perpendicular ao front (para strafe A/D)
    if (m_firstPerson)
    {
        // Calcula o vetor "frente" baseado no ângulo yaw da câmera
        // sin(yaw) e cos(yaw) dão as componentes X e Z do vetor direção
        glm::vec4 camera_front_xz = glm::vec4(sin(m_cameraYaw), 0.0f, cos(m_cameraYaw), 0.0f);
        camera_front_xz = camera_front_xz / norm(camera_front_xz);

        // Vetor "direita" é perpendicular ao front (rotacionado 90 graus)
        glm::vec4 camera_right_xz = glm::vec4(sin(m_cameraYaw - 3.141592f/2.0f), 0.0f, cos(m_cameraYaw - 3.141592f/2.0f), 0.0f);
        camera_right_xz = camera_right_xz / norm(camera_right_xz);

        // W: move para frente (na direção do olhar)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            // REQUISITO 10: velocidade * deltaTime garante movimento suave
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
            m_movementAngle = m_cameraYaw;
        }
    }
    // ─────────────────────────────────────────────────────────────────────────
    // MOVIMENTO EM TERCEIRA PESSOA (Câmera Look-At)
    // ─────────────────────────────────────────────────────────────────────────
    // O movimento é relativo aos eixos globais (não depende da câmera)
    // W/S: move no eixo Z global
    // A/D: move no eixo X global
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

        // Calcula o ângulo de rotação do modelo baseado na direção do movimento
        // Isso faz o personagem "olhar" na direção que está andando
        if (movement_input.x != 0.0f || movement_input.y != 0.0f)
        {
            m_movementAngle = atan2(movement_input.x, -movement_input.y);
        }
    }

    m_velocityY -= m_gravity * deltaTime;
    m_position.y += m_velocityY * deltaTime;

    const float groundLevel = 0.101f;
    if (m_position.y <= groundLevel) {
        m_position.y = groundLevel;
        m_velocityY = 0.0f;
        m_isGrounded = true;
    }

    if (should_log)
        printf("[Player] Pos: (%.2f,%.2f,%.2f) Yaw:%.2f Pitch:%.2f FP:%d Angle:%.2f\n",
               m_position.x, m_position.y, m_position.z, m_cameraYaw, m_cameraPitch, m_firstPerson, m_movementAngle);
}

// ============================================================================
// MATRIZ VIEW DA CÂMERA
// ============================================================================
// REQUISITO 3: Câmera livre e câmera look-at
//
// Esta função retorna a matriz View que transforma coordenadas do mundo
// para coordenadas da câmera. Implementa dois modos:
//
// 1. CÂMERA LOOK-AT (Terceira Pessoa):
//    - Câmera orbita ao redor do jogador usando coordenadas esféricas
//    - Posição da câmera: (r*sin(phi)*sin(theta), r*sin(phi), r*cos(phi)*cos(theta))
//    - Sempre olha para o jogador (look-at point = posição do jogador)
//    - Usa Matrix_Camera_View(posicao, vetor_view, vetor_up)
//
// 2. CÂMERA LIVRE (Primeira Pessoa):
//    - Câmera na posição dos "olhos" do jogador
//    - Direção do olhar controlada por yaw (horizontal) e pitch (vertical)
//    - Vetor view calculado usando coordenadas esféricas:
//      view = (cos(pitch)*sin(yaw), sin(pitch), cos(pitch)*cos(yaw))
//
// Ambos os modos usam a função Matrix_Camera_View() de matrices.cpp
// ============================================================================
glm::mat4 Player::getCameraView() const
{
    static int view_log_count = 0;
    bool should_log = (view_log_count++ % 60 == 0);

    // ─────────────────────────────────────────────────────────────────────────
    // CÂMERA LOOK-AT (Terceira Pessoa)
    // ─────────────────────────────────────────────────────────────────────────
    // A câmera orbita ao redor do jogador em uma esfera de raio m_cameraDistance
    // O usuário controla os ângulos theta (horizontal) e phi (vertical)
    if (!m_firstPerson)
    {
        // Converte coordenadas esféricas para cartesianas
        // r = distância da câmera ao jogador
        // phi = ângulo vertical (0 = acima, pi/2 = lado)
        // theta = ângulo horizontal (rotação ao redor do jogador)
        float r = m_cameraDistance;
        float y = r * sin(m_cameraPhi);
        float z = r * cos(m_cameraPhi) * cos(m_cameraTheta);
        float x = r * cos(m_cameraPhi) * sin(m_cameraTheta);

        glm::vec4 camera_offset = glm::vec4(x, y, z, 0.0f);
        glm::vec4 camera_position_c = m_position + camera_offset;

        const float arenaMinX = -4.2f;
        const float arenaMaxX = 4.2f;
        const float arenaMinZ = -1.2f;
        const float arenaMaxZ = 1.2f;
        const float minY = 0.3f;
        const float maxY = 2.8f;

        if (camera_position_c.x < arenaMinX) camera_position_c.x = arenaMinX;
        if (camera_position_c.x > arenaMaxX) camera_position_c.x = arenaMaxX;
        if (camera_position_c.z < arenaMinZ) camera_position_c.z = arenaMinZ;
        if (camera_position_c.z > arenaMaxZ) camera_position_c.z = arenaMaxZ;
        if (camera_position_c.y < minY) camera_position_c.y = minY;
        if (camera_position_c.y > maxY) camera_position_c.y = maxY;

        glm::vec4 camera_lookat_l    = m_position;
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
        glm::vec4 camera_up_vector   = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        // Retorna a matriz view usando a função de look-at
        // camera_view_vector = ponto_alvo - posicao_camera
        return Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
    }
    // ─────────────────────────────────────────────────────────────────────────
    // CÂMERA LIVRE (Primeira Pessoa)
    // ─────────────────────────────────────────────────────────────────────────
    // A câmera está na posição dos "olhos" do jogador
    // O usuário controla para onde olhar usando yaw e pitch
    else
    {
        // Posição da câmera = posição do jogador + altura dos olhos
        float eye_height = 0.15f;
        glm::vec4 camera_position_c = m_position + glm::vec4(0.0f, eye_height, 0.0f, 0.0f);

        // Calcula o vetor de direção do olhar usando coordenadas esféricas
        // yaw: rotação horizontal (olhar esquerda/direita)
        // pitch: rotação vertical (olhar cima/baixo)
        // A fórmula converte (yaw, pitch) para um vetor unitário 3D
        float camera_view_x = cos(m_cameraPitch) * sin(m_cameraYaw);
        float camera_view_y = sin(m_cameraPitch);
        float camera_view_z = cos(m_cameraPitch) * cos(m_cameraYaw);
        glm::vec4 camera_view_vector = glm::vec4(camera_view_x, camera_view_y, camera_view_z, 0.0f);

        // Vetor "up" sempre aponta para cima (Y positivo)
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

    float zoom_min = 0.5f;
    float zoom_max = 1.8f;
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

        glm::vec4 camera_position = m_position + glm::vec4(x, y, z, 0.0f);

        const float arenaMinX = -4.2f;
        const float arenaMaxX = 4.2f;
        const float arenaMinZ = -1.2f;
        const float arenaMaxZ = 1.2f;
        const float minY = 0.3f;
        const float maxY = 2.8f;

        if (camera_position.x < arenaMinX) camera_position.x = arenaMinX;
        if (camera_position.x > arenaMaxX) camera_position.x = arenaMaxX;
        if (camera_position.z < arenaMinZ) camera_position.z = arenaMinZ;
        if (camera_position.z > arenaMaxZ) camera_position.z = arenaMaxZ;
        if (camera_position.y < minY) camera_position.y = minY;
        if (camera_position.y > maxY) camera_position.y = maxY;

        return camera_position;
    }
}

void Player::takeDamage(int damage)
{
    if (m_damageCooldownTimer <= 0.0f)
    {
        m_vida -= damage;
        sfx.hit_player();
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
    sfx.cura();
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
    m_velocityY = 0.0f;
    m_isGrounded = true;
    m_firstPerson = true;
    m_cameraTheta = 0.0f;
    m_cameraPhi = 0.5f;
    m_cameraDistance = 1.2f;  
    m_cameraYaw = -1.57079632f;
    m_cameraPitch = 0.0f;
}

void Player::jump()
{
    if (m_isGrounded) {
        m_velocityY = m_jumpForce;
        m_isGrounded = false;
    }
}
