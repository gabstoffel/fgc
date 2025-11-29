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
    : m_dragonBoss(0.0f, 0.7f, 500)
    , m_dragonBossAlive(true)
    , m_window(nullptr)
    , m_lastFrameTime(0.0)
    , m_segundoAnterior(0)
    , m_gameState(GameState::MENU)
    , m_difficulty(1)
    , m_enemyDamage(10)
    , m_countdownTimer(0.0f)
    , m_hitMarkerTimer(0.0f)
    , m_muzzleFlashTimer(0.0f)
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
    if (m_gameState == GameState::COUNTDOWN)
    {
        m_countdownTimer -= deltaTime;
        if (m_countdownTimer <= 0.0f)
        {
            m_gameState = GameState::PLAYING;
            if (!m_player.isFirstPerson())
                m_player.toggleCamera();
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        return;
    }

    if (m_gameState != GameState::PLAYING)
        return;

    int segundos = (int)glfwGetTime();

    m_enemyManager.trySpawnEnemy(segundos, m_player.getPosition());
    m_player.update(m_window, deltaTime);
    m_enemyManager.update(deltaTime, m_player);
    handleCollisions();
    handleShooting();
    handleDebugKillKey();
    m_enemyManager.removeDeadEnemies();

    m_projectileManager.update(deltaTime);
    handleProjectileCollisions();
    m_projectileManager.removeInactive();

    if (m_hitMarkerTimer > 0.0f)
        m_hitMarkerTimer -= deltaTime;
    if (m_muzzleFlashTimer > 0.0f)
        m_muzzleFlashTimer -= deltaTime;

    if (m_player.isDead())
    {
        m_gameState = GameState::GAME_OVER;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (!m_dragonBossAlive)
    {
        m_gameState = GameState::WIN;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Game::render()
{
    switch (m_gameState)
    {
    case GameState::MENU:
        m_renderer.renderMenu(m_difficulty);
        break;

    case GameState::COUNTDOWN:
        {
            float angle = (4.0f - m_countdownTimer) * 0.5f; 
            float camHeight = 2.5f;
            float camDist = 2.0f;
            glm::vec4 camera_position = glm::vec4(
                sin(angle) * camDist,
                camHeight,
                cos(angle) * camDist,
                1.0f
            );
            glm::vec4 camera_lookat = glm::vec4(0.0f, 0.3f, 0.0f, 1.0f);
            glm::vec4 camera_view = camera_lookat - camera_position;
            glm::vec4 camera_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            glm::mat4 view = Matrix_Camera_View(camera_position, camera_view, camera_up);

            float nearplane = -0.1f;
            float farplane = -5000.0f;
            float field_of_view = 3.141592f / 3.0f;
            glm::mat4 projection = Matrix_Perspective(field_of_view, Input::getScreenRatio(), nearplane, farplane);

            m_renderer.setView(view);
            m_renderer.setProjection(projection);
            m_renderer.renderScene(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive);

            int countdownNum = (int)ceilf(m_countdownTimer);
            m_renderer.renderCountdown(countdownNum);
        }
        break;

    case GameState::PLAYING:
        {
            glm::mat4 view = m_player.getCameraView();

            float nearplane = -0.1f;
            float farplane  = -5000.0f;
            float field_of_view = 3.141592f / 3.0f;
            glm::mat4 projection = Matrix_Perspective(field_of_view, Input::getScreenRatio(), nearplane, farplane);

            m_renderer.setView(view);
            m_renderer.setProjection(projection);
            m_renderer.renderScene(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive, &m_projectileManager);
            m_renderer.renderHUD(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive);
            m_renderer.renderCrosshair(m_player.isFirstPerson());

            if (m_hitMarkerTimer > 0.0f)
                m_renderer.renderHitMarker();
            if (m_muzzleFlashTimer > 0.0f)
                m_renderer.renderMuzzleFlash();
        }
        break;

    case GameState::GAME_OVER:
        m_renderer.renderGameOver();
        break;

    case GameState::WIN:
        m_renderer.renderWin();
        break;
    }
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

    const std::vector<Enemy>& enemies = m_enemyManager.getEnemies();
    float collisionRadius = 0.25f;
    float collisionRadiusSq = collisionRadius * collisionRadius;

    for (const Enemy& enemy : enemies)
    {
        glm::vec4 enemyPos = enemy.getPosition();
        float dx = player_pos_3d.x - enemyPos.x;
        float dz = player_pos_3d.z - enemyPos.z;
        float distSq = dx * dx + dz * dz;

        if (distSq < collisionRadiusSq)
        {
            m_player.takeDamage(m_enemyDamage);
            break;
        }
    }

    if (m_dragonBossAlive)
    {
        glm::vec4 dragonPos = m_dragonBoss.getPosition();
        float dx = player_pos_3d.x - dragonPos.x;
        float dz = player_pos_3d.z - dragonPos.z;
        float distSq = dx * dx + dz * dz;
        float bossCollisionRadius = 0.35f;

        if (distSq < bossCollisionRadius * bossCollisionRadius)
        {
            m_player.takeDamage(m_enemyDamage * 2);
        }
    }
}

void Game::handleShooting()
{
    if (!Input::isShootingRequested())
        return;

    if (!m_player.isFirstPerson())
        return; 

    glm::vec4 origin = m_player.getCameraPosition();
    glm::vec4 dir = m_player.getCameraDirection();

    glm::vec3 dir3 = glm::vec3(dir.x, dir.y, dir.z);
    float len = sqrt(dir3.x * dir3.x + dir3.y * dir3.y + dir3.z * dir3.z);
    if (len > 0.0001f)
    {
        dir3.x /= len;
        dir3.y /= len;
        dir3.z /= len;
    }

    glm::vec3 spawnPos = glm::vec3(origin.x, origin.y, origin.z) + dir3 * 0.3f;
    m_projectileManager.spawnProjectile(spawnPos, dir3);

    m_muzzleFlashTimer = MUZZLE_FLASH_DURATION;

    printf("Projectile spawned at (%.2f, %.2f, %.2f)\n", spawnPos.x, spawnPos.y, spawnPos.z);
}

void Game::handleProjectileCollisions()
{
    std::vector<Projectile>& projectiles = m_projectileManager.getProjectiles();
    std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());

    float enemyRadius = 0.2f;
    float bossRadius = 0.3f;
    float projectileRadius = 0.05f;

    for (size_t p = 0; p < projectiles.size(); p++)
    {
        if (!projectiles[p].active)
            continue;

        glm::vec3 projPos = projectiles[p].position;

        for (size_t e = 0; e < enemies.size(); e++)
        {
            glm::vec4 enemyPos4 = enemies[e].getPosition();
            glm::vec3 enemyPos = glm::vec3(enemyPos4.x, enemyPos4.y, enemyPos4.z);

            float dx = projPos.x - enemyPos.x;
            float dy = projPos.y - enemyPos.y;
            float dz = projPos.z - enemyPos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float combinedRadius = enemyRadius + projectileRadius;

            if (distSq < combinedRadius * combinedRadius)
            {
                enemies[e].takeDamage(100);
                projectiles[p].active = false;
                m_hitMarkerTimer = HIT_MARKER_DURATION;
                printf("Projectile hit enemy! Enemy HP: %d\n", enemies[e].getVida());
                break;
            }
        }

        if (!projectiles[p].active)
            continue;

        if (m_dragonBossAlive)
        {
            glm::vec4 dragonPos4 = m_dragonBoss.getPosition();
            glm::vec3 dragonPos = glm::vec3(dragonPos4.x, dragonPos4.y + 0.15f, dragonPos4.z);

            float dx = projPos.x - dragonPos.x;
            float dy = projPos.y - dragonPos.y;
            float dz = projPos.z - dragonPos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float combinedRadius = bossRadius + projectileRadius;

            if (distSq < combinedRadius * combinedRadius)
            {
                m_dragonBoss.takeDamage(100);
                projectiles[p].active = false;
                m_hitMarkerTimer = HIT_MARKER_DURATION;
                printf("Projectile hit Dragon Boss! HP: %d\n", m_dragonBoss.getVida());

                if (m_dragonBoss.isDead())
                {
                    m_dragonBossAlive = false;
                    printf("*** DRAGON BOSS DEFEATED! ***\n");
                }
            }
        }
    }
}

void Game::handleDebugKillKey()
{
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

void Game::setDifficulty(int difficulty)
{
    m_difficulty = difficulty;

    switch (difficulty)
    {
    case 0: 
        m_player.setVida(150, 150);
        m_enemyManager.setEnemySpeed(0.15f);
        m_enemyManager.setMaxEnemies(1);
        m_enemyDamage = 5;
        break;
    case 1: 
        m_player.setVida(100, 100);
        m_enemyManager.setEnemySpeed(0.25f);
        m_enemyManager.setMaxEnemies(2);
        m_enemyDamage = 10;
        break;
    case 2: 
        m_player.setVida(75, 75);
        m_enemyManager.setEnemySpeed(0.35f);
        m_enemyManager.setMaxEnemies(3);
        m_enemyDamage = 20;
        break;
    }
}

void Game::startGame()
{
    m_gameState = GameState::COUNTDOWN;
    m_countdownTimer = 4.0f;
    m_lastFrameTime = glfwGetTime();

    if (m_player.isFirstPerson())
        m_player.toggleCamera();
}

void Game::resetGame()
{
    m_player.reset();
    m_enemyManager.clearEnemies();
    m_projectileManager.clear();
    m_dragonBoss = Enemy(0.0f, 0.7f, 500);
    m_dragonBossAlive = true;
    m_hitMarkerTimer = 0.0f;
    m_muzzleFlashTimer = 0.0f;
    setDifficulty(m_difficulty);
    startGame();
}

void Game::returnToMenu()
{
    m_gameState = GameState::MENU;
    m_player.reset();
    m_enemyManager.clearEnemies();
    m_projectileManager.clear();
    m_dragonBoss = Enemy(0.0f, 0.7f, 500);
    m_dragonBossAlive = true;
    m_hitMarkerTimer = 0.0f;
    m_muzzleFlashTimer = 0.0f;
}
