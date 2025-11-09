#include "Game.h"
#include "Input.h"
#include "matrices.h"
#include "collisions.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <glm/gtc/type_ptr.hpp>

Game::Game()
    : m_window(nullptr)
    , m_lastFrameTime(0.0)
    , m_segundoAnterior(0)
{
}

Game::~Game()
{
}

bool Game::init()
{
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        return false;
    }

    glfwSetErrorCallback(Input::errorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(800, 600, "Implementação Inicial", NULL, NULL);
    if (!m_window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        return false;
    }

    glfwSetKeyCallback(m_window, Input::keyCallback);
    glfwSetMouseButtonCallback(m_window, Input::mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, Input::cursorPosCallback);
    glfwSetScrollCallback(m_window, Input::scrollCallback);
    glfwSetFramebufferSizeCallback(m_window, Input::framebufferSizeCallback);

    glfwSetWindowSize(m_window, 800, 600);
    glfwMakeContextCurrent(m_window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Input::init(&m_player, this);

    if (!m_renderer.init(m_window))
    {
        fprintf(stderr, "ERROR: Renderer initialization failed.\n");
        return false;
    }

    m_lastFrameTime = glfwGetTime();

    return true;
}

void Game::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
        m_lastFrameTime = currentTime;

        update(deltaTime);
        render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Game::update(float deltaTime)
{
    int segundos = (int)glfwGetTime();

    m_enemyManager.trySpawnEnemy(segundos, m_player.getPosition());
    m_player.update(m_window, deltaTime);
    m_enemyManager.update(deltaTime, m_player);
    handleCollisions();
    handleShooting();
    handleDebugKillKey();
    m_enemyManager.removeDeadEnemies();
}

void Game::render()
{
    glm::mat4 view = m_player.getCameraView();

    float nearplane = -0.1f;
    float farplane  = -5000.0f;
    float field_of_view = 3.141592f / 3.0f;
    glm::mat4 projection = Matrix_Perspective(field_of_view, Input::getScreenRatio(), nearplane, farplane);

    m_renderer.setView(view);
    m_renderer.setProjection(projection);
    m_renderer.renderScene(m_player, m_enemyManager);
}

void Game::handleCollisions()
{
    glm::vec3 player_extents = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec4 player_pos_4d = m_player.getPosition();
    glm::vec3 player_pos_3d = glm::vec3(player_pos_4d.x, player_pos_4d.y, player_pos_4d.z);

    glm::vec4 floor_plane = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    resolveAABBPlane(player_pos_3d, player_extents, floor_plane);

    glm::vec3 arena_min = glm::vec3(-0.9f, -10.0f, -0.9f);
    glm::vec3 arena_max = glm::vec3(0.9f, 10.0f, 0.9f);
    clampPositionToBox(player_pos_3d, arena_min, arena_max);

    m_player.updatePositionAfterCollision(player_pos_3d);
}

void Game::handleShooting()
{
    if (!Input::isShootingRequested())
        return;

    printf("\n=== SHOOTING ===\n");
    printf("Camera mode: %s\n", m_player.isFirstPerson() ? "First Person" : "Third Person");

    glm::vec4 ray_origin = m_player.getCameraPosition();
    glm::vec4 ray_direction = m_player.getCameraDirection();

    glm::vec3 ray_origin_3d = glm::vec3(ray_origin.x, ray_origin.y, ray_origin.z);
    glm::vec3 ray_dir_3d = glm::vec3(ray_direction.x, ray_direction.y, ray_direction.z);

    float dir_length = sqrt(ray_dir_3d.x * ray_dir_3d.x +
                           ray_dir_3d.y * ray_dir_3d.y +
                           ray_dir_3d.z * ray_dir_3d.z);
    if (dir_length > 0.0001f)
    {
        ray_dir_3d.x /= dir_length;
        ray_dir_3d.y /= dir_length;
        ray_dir_3d.z /= dir_length;
    }

    printf("Ray origin: (%.2f, %.2f, %.2f)\n", ray_origin_3d.x, ray_origin_3d.y, ray_origin_3d.z);
    printf("Ray direction: (%.2f, %.2f, %.2f)\n", ray_dir_3d.x, ray_dir_3d.y, ray_dir_3d.z);

    std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());
    printf("Checking %zu enemies...\n", enemies.size());

    bool hit_any = false;
    for (size_t i = 0; i < enemies.size(); i++)
    {
        glm::vec4 enemy_pos = enemies[i].getPosition();
        glm::vec3 enemy_center = glm::vec3(enemy_pos.x, enemy_pos.y, enemy_pos.z);
        float enemy_radius = 0.2f;
        float t;

        float dx = enemy_center.x - ray_origin_3d.x;
        float dy = enemy_center.y - ray_origin_3d.y;
        float dz = enemy_center.z - ray_origin_3d.z;
        float distance_to_enemy = sqrt(dx*dx + dy*dy + dz*dz);

        printf("  Enemy %zu at (%.2f, %.2f, %.2f), distance: %.2f",
               i, enemy_pos.x, enemy_pos.y, enemy_pos.z, distance_to_enemy);

        if (testRaySphere(ray_origin_3d, ray_dir_3d, enemy_center, enemy_radius, t))
        {
            enemies[i].takeDamage(100);
            printf(" -> HIT! (t=%.2f)\n", t);
            hit_any = true;
            break;
        }
        else
        {
            printf(" -> miss\n");
        }
    }

    if (!hit_any && enemies.size() > 0)
    {
        printf("MISS! No enemies hit.\n");
    }
    else if (enemies.size() == 0)
    {
        printf("No enemies to shoot!\n");
    }
    printf("================\n\n");
}

void Game::handleDebugKillKey()
{
    static bool k_was_pressed = false;
    bool k_is_pressed = (glfwGetKey(m_window, GLFW_KEY_K) == GLFW_PRESS);

    if (k_is_pressed && !k_was_pressed)
    {
        std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());

        if (enemies.empty())
        {
            printf("DEBUG: No enemies to kill!\n");
            k_was_pressed = k_is_pressed;
            return;
        }

        glm::vec4 player_pos = m_player.getPosition();
        float min_distance = std::numeric_limits<float>::max();
        size_t nearest_index = 0;

        for (size_t i = 0; i < enemies.size(); i++)
        {
            glm::vec4 enemy_pos = enemies[i].getPosition();
            float dx = enemy_pos.x - player_pos.x;
            float dz = enemy_pos.z - player_pos.z;
            float distance = sqrt(dx * dx + dz * dz);

            if (distance < min_distance)
            {
                min_distance = distance;
                nearest_index = i;
            }
        }

        enemies[nearest_index].takeDamage(100);
        printf("DEBUG: Killed nearest enemy at distance %.2f\n", min_distance);
    }

    k_was_pressed = k_is_pressed;
}

void Game::cleanup()
{
    glfwTerminate();
}

void Game::setShouldClose(bool shouldClose)
{
    if (m_window)
    {
        glfwSetWindowShouldClose(m_window, shouldClose ? GL_TRUE : GL_FALSE);
    }
}
