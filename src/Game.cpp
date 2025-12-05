// ============================================================================
// GAME.CPP - Loop Principal e Lógica do Jogo
// ============================================================================
//
// Este arquivo implementa o loop principal do jogo e gerencia:
// - Estados do jogo (menu, jogando, pausado, game over, vitória)
// - Atualização de entidades baseada no tempo (deltaTime)
// - Detecção e resolução de colisões
// - Spawn de inimigos e power-ups
//
// REQUISITOS IMPLEMENTADOS:
// - REQUISITO 5: Testes de colisão (AABB-AABB, AABB-Plano, Ponto-Esfera)
// - REQUISITO 10: Animações baseadas no tempo (deltaTime)
//
// ============================================================================

#include "Game.h"
#include "Input.h"
#include "matrices.h"
#include "collisions.h"
#include "sfx.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <glm/gtc/type_ptr.hpp>

Game::Game()
    : m_dragonBoss(-3.5f, 0.0f, 5000)
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
    , m_dragonAttackTimer(0.0f)
    , m_dragonAttackInterval(2.5f)
    , m_healthSpawnTimer(0.0f)
    , m_gameTime(0.0f)
    , m_baseEnemySpeed(0.4f)
    , m_pauseCameraTheta(0.0f)
    , m_pauseCameraPhi(0.5f)
    , m_pauseCameraDistance(2.0f)
    , m_pauseCameraTarget(0.0f, 0.5f, 0.0f, 1.0f)
    , m_pauseFocusTarget(PauseFocusTarget::PLAYER)
    , m_pauseFocusEnemyIndex(0)
{
    m_pillars.push_back({glm::vec3(-3.0f, 0.0f, 1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(-1.5f, 0.0f, 1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(0.0f, 0.0f, 1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(1.5f, 0.0f, 1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(3.0f, 0.0f, 1.2f), 0.5f, 3.0f});

    m_pillars.push_back({glm::vec3(-3.0f, 0.0f, -1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(-1.5f, 0.0f, -1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(0.0f, 0.0f, -1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(1.5f, 0.0f, -1.2f), 0.5f, 3.0f});
    m_pillars.push_back({glm::vec3(3.0f, 0.0f, -1.2f), 0.5f, 3.0f});

    m_torches.push_back({glm::vec3(-2.25f, 1.5f, 1.3f), true});
    m_torches.push_back({glm::vec3(-0.75f, 1.5f, 1.3f), true});
    m_torches.push_back({glm::vec3(0.75f, 1.5f, 1.3f), true});
    m_torches.push_back({glm::vec3(2.25f, 1.5f, 1.3f), true});

    m_torches.push_back({glm::vec3(-2.25f, 1.5f, -1.3f), true});
    m_torches.push_back({glm::vec3(-0.75f, 1.5f, -1.3f), true});
    m_torches.push_back({glm::vec3(0.75f, 1.5f, -1.3f), true});
    m_torches.push_back({glm::vec3(2.25f, 1.5f, -1.3f), true});
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
    sfx.start();
    menu_music=true;
    return true;
}

// ============================================================================
// LOOP PRINCIPAL DO JOGO
// ============================================================================
// REQUISITO 10: Animações baseadas no tempo
//
// O loop principal usa deltaTime para garantir que o jogo rode na mesma
// velocidade independente da taxa de frames. Isso é calculado como:
//
//     deltaTime = tempoAtual - tempoUltimoFrame
//
// Todas as animações e movimentos usam essa variável para interpolar
// posições de forma suave e consistente.
// ============================================================================
void Game::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        // Calcula o tempo decorrido desde o último frame
        // Isso garante movimento suave independente do FPS
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
        m_lastFrameTime = currentTime;

        // Atualiza a lógica do jogo (física, colisões, IA)
        update(deltaTime);
        // Renderiza a cena atual
        render(deltaTime);

        // Troca os buffers (double buffering) e processa eventos
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

    m_gameTime += deltaTime;
    updateEnemySpeed(deltaTime);

    m_enemyManager.trySpawnEnemy(segundos, m_player.getPosition());
    m_player.update(m_window, deltaTime);
    m_enemyManager.update(deltaTime, m_player);
    handleEnemyEnvironmentCollisions();
    handleCollisions();
    handleShooting();
    handleDebugKillKey();
    m_enemyManager.removeDeadEnemies();

    handleDragonAttack(deltaTime);

    m_projectileManager.update(deltaTime);
    handleProjectileCollisions();
    m_projectileManager.removeInactive();

    handleHealthPickups(deltaTime);

    if (m_hitMarkerTimer > 0.0f)
        m_hitMarkerTimer -= deltaTime;
    if (m_muzzleFlashTimer > 0.0f)
        m_muzzleFlashTimer -= deltaTime;

    if (m_player.isDead())
    {
        m_gameState = GameState::GAME_OVER;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (result_sfx){
            sfx.game_over();
            result_sfx=false;
        }
    }

    if (!m_dragonBossAlive)
    {
        m_gameState = GameState::WIN;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (result_sfx){
            sfx.vitoria();
            result_sfx=false;
        }
    }
}

void Game::render(float deltaTime)
{
    switch (m_gameState)
    {
    case GameState::MENU:
        if(menu_music){
            sfx.musicaPrincipalStart("sfx/menu.mp3", true);
            menu_music=false;
        }
        m_renderer.renderMenu(m_difficulty);
        break;

    case GameState::COUNTDOWN:
        {
            float progress = (4.0f - m_countdownTimer) / 4.0f;
            float camHeight = 2.5f;
            float camX = -3.5f + progress * 7.0f;
            glm::vec4 camera_position = glm::vec4(
                camX,
                camHeight,
                0.0f,
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
            m_renderer.renderScene(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive, deltaTime);
            m_renderer.renderPillars(m_pillars);
            m_renderer.renderTorches(m_torches, deltaTime);

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
            m_renderer.renderScene(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive, deltaTime, &m_projectileManager);
            m_renderer.renderPillars(m_pillars);
            m_renderer.renderTorches(m_torches, deltaTime);
            m_renderer.renderHealthPickups(m_healthPickups, deltaTime);
            m_renderer.renderHUD(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive);
            m_renderer.renderCrosshair(m_player.isFirstPerson());

            if (m_hitMarkerTimer > 0.0f)
                m_renderer.renderHitMarker();
        }
        break;

    case GameState::PAUSED:
        {
            float camX = m_pauseCameraTarget.x + m_pauseCameraDistance * sin(m_pauseCameraPhi) * cos(m_pauseCameraTheta);
            float camY = m_pauseCameraTarget.y + m_pauseCameraDistance * cos(m_pauseCameraPhi);
            float camZ = m_pauseCameraTarget.z + m_pauseCameraDistance * sin(m_pauseCameraPhi) * sin(m_pauseCameraTheta);

            glm::vec4 camera_position = glm::vec4(camX, camY, camZ, 1.0f);

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
            glm::vec4 camera_lookat = m_pauseCameraTarget;
            glm::vec4 camera_view = camera_lookat - camera_position;
            glm::vec4 camera_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            glm::mat4 view = Matrix_Camera_View(camera_position, camera_view, camera_up);

            float nearplane = -0.1f;
            float farplane = -5000.0f;
            float field_of_view = 3.141592f / 3.0f;
            glm::mat4 projection = Matrix_Perspective(field_of_view, Input::getScreenRatio(), nearplane, farplane);

            m_renderer.setView(view);
            m_renderer.setProjection(projection);
            m_renderer.renderScenePaused(m_player, m_enemyManager, m_dragonBoss, m_dragonBossAlive, camera_position, deltaTime, &m_projectileManager);
            m_renderer.renderPillars(m_pillars);
            m_renderer.renderTorches(m_torches, deltaTime);
            m_renderer.renderHealthPickups(m_healthPickups, deltaTime);
        }
        break;

    case GameState::GAME_OVER:
        m_renderer.renderGameOver();
        sfx.musicaPrincipalStop();
        break;

    case GameState::WIN:
        m_renderer.renderWin();
        sfx.musicaPrincipalStop();
        break;
    }
}

// ============================================================================
// SISTEMA DE COLISÕES
// ============================================================================
// REQUISITO 5: Três tipos de testes de intersecção
//
// Este sistema implementa detecção e resolução de colisões usando:
//
// 1. AABB x Plano (testAABBPlane, resolveAABBPlane):
//    - Jogador/inimigos vs chão, paredes e teto
//    - Mantém entidades dentro da arena
//
// 2. AABB x AABB (testAABBAABB):
//    - Jogador vs pilares (obstáculos)
//    - Jogador vs inimigos (causa dano)
//    - Inimigos vs pilares (recalcula caminho Bézier)
//
// 3. Ponto x Esfera (testPointSphere):
//    - Projéteis vs inimigos/boss/jogador
//    - Jogador vs boss dragão
//    - Jogador vs power-ups de vida
//
// As funções de colisão estão implementadas em collisions.cpp
// ============================================================================
void Game::handleCollisions()
{
    // Define a "caixa" de colisão do jogador (half-extents da AABB)
    glm::vec3 player_extents = glm::vec3(0.108f, 0.108f, 0.108f);
    glm::vec4 player_pos_4d = m_player.getPosition();
    glm::vec3 player_pos_3d = glm::vec3(player_pos_4d.x, player_pos_4d.y, player_pos_4d.z);

    // ─────────────────────────────────────────────────────────────────────────
    // COLISÃO AABB x PLANO: Jogador vs Chão
    // ─────────────────────────────────────────────────────────────────────────
    // O plano do chão é definido pela equação: y = 0 (normal apontando para cima)
    // Se o jogador penetrar o chão, ele é empurrado de volta para cima
    glm::vec4 floor_plane = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    resolveAABBPlane(player_pos_3d, player_extents, floor_plane);

    // Limita a posição do jogador aos limites da arena
    glm::vec3 arena_min = glm::vec3(-4.3f, -10.0f, -1.3f);
    glm::vec3 arena_max = glm::vec3(4.3f, 10.0f, 1.3f);
    clampPositionToBox(player_pos_3d, arena_min, arena_max);

    // ─────────────────────────────────────────────────────────────────────────
    // COLISÃO AABB x AABB: Jogador vs Pilares
    // ─────────────────────────────────────────────────────────────────────────
    // Testa colisão entre a AABB do jogador e cada pilar da arena
    // Se houver colisão, empurra o jogador para fora no eixo de menor penetração
    for (const Pillar& pillar : m_pillars)
    {
        glm::vec3 pillarExtents(pillar.sizeXZ * 0.5f, pillar.height * 0.5f, pillar.sizeXZ * 0.5f);
        glm::vec3 pillarCenter = glm::vec3(pillar.position.x,pillar.position.y + pillarExtents.y,pillar.position.z);
        glm::vec3 pillarMin = pillarCenter - pillarExtents;
        glm::vec3 pillarMax = pillarCenter + pillarExtents;
        glm::vec3 playerMin = player_pos_3d - player_extents;
        glm::vec3 playerMax = player_pos_3d + player_extents;
        if (testAABBAABB(playerMin, playerMax, pillarMin, pillarMax))
        {
            glm::vec3 overlapMin = pillarMax - playerMin;
            glm::vec3 overlapMax = playerMax - pillarMin;

            float resolveX = std::min(overlapMin.x, overlapMax.x);
            float resolveY = std::min(overlapMin.y, overlapMax.y);
            float resolveZ = std::min(overlapMin.z, overlapMax.z);

            // Empurrar no eixo de menor penetração
            if (resolveX < resolveY && resolveX < resolveZ)
                player_pos_3d.x += (player_pos_3d.x < pillarCenter.x ? -resolveX : resolveX);
            else if (resolveY < resolveZ)
                player_pos_3d.y += (player_pos_3d.y < pillarCenter.y ? -resolveY : resolveY);
            else
                player_pos_3d.z += (player_pos_3d.z < pillarCenter.z ? -resolveZ : resolveZ);
        }
    }

    m_player.updatePositionAfterCollision(player_pos_3d);

    // ─────────────────────────────────────────────────────────────────────────
    // COLISÃO AABB x AABB: Jogador vs Inimigos
    // ─────────────────────────────────────────────────────────────────────────
    // Testa colisão entre jogador e cada inimigo
    // Se houver colisão: jogador toma dano, é empurrado para fora,
    // e o inimigo recebe knockback na direção oposta
    std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());
    float enemyRadius = 0.10f;
    glm::vec3 enemyExtents(enemyRadius, enemyRadius, enemyRadius);

    glm::vec3 playerMin = player_pos_3d - player_extents;
    glm::vec3 playerMax = player_pos_3d + player_extents;

    for (Enemy& enemy : enemies)
    {
        glm::vec4 enemyPos4 = enemy.getPosition();
        glm::vec3 enemyPos(enemyPos4.x, enemyPos4.y, enemyPos4.z);

        glm::vec3 enemyMin = enemyPos - enemyExtents;
        glm::vec3 enemyMax = enemyPos + enemyExtents;

        if (testAABBAABB(playerMin, playerMax, enemyMin, enemyMax))
        {
            // Registrar dano
            m_player.takeDamage(m_enemyDamage);

            glm::vec3 overlapMin = enemyMax - playerMin;
            glm::vec3 overlapMax = playerMax - enemyMin;

            float resolveX = std::min(overlapMin.x, overlapMax.x);
            float resolveY = std::min(overlapMin.y, overlapMax.y);
            float resolveZ = std::min(overlapMin.z, overlapMax.z);

            // Empurrar Player para fora
            if (resolveX < resolveY && resolveX < resolveZ)
                player_pos_3d.x += (player_pos_3d.x < enemyPos.x ? -resolveX : resolveX);
            else if (resolveY < resolveZ)
                player_pos_3d.y += (player_pos_3d.y < enemyPos.y ? -resolveY : resolveY);
            else
                player_pos_3d.z += (player_pos_3d.z < enemyPos.z ? -resolveZ : resolveZ);

            // Atualiza AABB do player após correção
            playerMin = player_pos_3d - player_extents;
            playerMax = player_pos_3d + player_extents;

            // Knockback no enemy
            glm::vec3 pushDir = (enemyPos - player_pos_3d)/norm(enemyPos - player_pos_3d);
            enemy.applyKnockback(pushDir.x, pushDir.z, 6.0f);

            m_player.updatePositionAfterCollision(player_pos_3d);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // COLISÃO PONTO x ESFERA: Jogador vs Boss Dragão
    // ─────────────────────────────────────────────────────────────────────────
    // O boss usa uma esfera de colisão (bounding sphere) em vez de AABB
    // Isso funciona bem para o modelo do dragão que é mais arredondado
    // testPointSphere verifica se ||jogador - dragão|| <= raio
    if (m_dragonBossAlive)
    {
        glm::vec4 dragonPos4 = m_dragonBoss.getPosition();
        glm::vec3 dragonPos(dragonPos4.x, dragonPos4.y, dragonPos4.z);

        float bossCollisionRadius = 0.55f;

        // Teste Ponto x Esfera: verifica se o jogador está dentro da esfera do boss
        if (testPointSphere(player_pos_3d, dragonPos, bossCollisionRadius))
        {
            m_player.takeDamage(m_enemyDamage * 2);

            // Direção do empurrão
            glm::vec3 pushDir = (player_pos_3d - dragonPos)/norm((player_pos_3d - dragonPos));

            // Calcula o empurrão para fora da esfera
            float dist = norm(player_pos_3d - dragonPos);
            float overlap = bossCollisionRadius - dist;
            float pushDistance = overlap + 0.01f;

            player_pos_3d += pushDir * pushDistance;

            // Atualiza posição do player
            m_player.updatePositionAfterCollision(player_pos_3d);
        }
    }
}

void Game::handleEnemyEnvironmentCollisions()
{
    std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());
    float enemyRadius = 0.15f;

    for (Enemy& enemy : enemies)
    {
        glm::vec4 enemyPos4 = enemy.getPosition();
        glm::vec3 enemyPos(enemyPos4.x, enemyPos4.y, enemyPos4.z);

        glm::vec3 enemyExtents(enemyRadius, enemyRadius, enemyRadius);

        glm::vec3 enemyMin = enemyPos - enemyExtents;
        glm::vec3 enemyMax = enemyPos + enemyExtents;

        bool positionChanged = false;

        for (const Pillar& pillar : m_pillars)
        {
            glm::vec3 pillarExtents(pillar.sizeXZ * 0.5f, pillar.height * 0.5f, pillar.sizeXZ * 0.5f);
            glm::vec3 pillarCenter(pillar.position.x,
                                   pillar.position.y + pillarExtents.y,
                                   pillar.position.z);

            glm::vec3 pillarMin = pillarCenter - pillarExtents;
            glm::vec3 pillarMax = pillarCenter + pillarExtents;

            if (testAABBAABB(enemyMin, enemyMax, pillarMin, pillarMax))
            {
                glm::vec3 overlapMin = pillarMax - enemyMin;
                glm::vec3 overlapMax = enemyMax - pillarMin;

                float resolveX = std::min(overlapMin.x, overlapMax.x);
                float resolveY = std::min(overlapMin.y, overlapMax.y);
                float resolveZ = std::min(overlapMin.z, overlapMax.z);

                // Empurrar para fora no eixo de menor penetração
                if (resolveX < resolveY && resolveX < resolveZ)
                    enemyPos.x += (enemyPos.x < pillarCenter.x ? -resolveX : resolveX);
                else if (resolveY < resolveZ)
                    enemyPos.y += (enemyPos.y < pillarCenter.y ? -resolveY : resolveY);
                else
                    enemyPos.z += (enemyPos.z < pillarCenter.z ? -resolveZ : resolveZ);

                enemyMin = enemyPos - enemyExtents;
                enemyMax = enemyPos + enemyExtents;

                positionChanged = true;
            }
        }

    if (positionChanged)
    {
        enemy.setPosition(enemyPos.x, enemyPos.z);
        enemy.onObstacleCollision();  // recalcula o caminho, usando curva de bezier
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

// ============================================================================
// COLISÃO DE PROJÉTEIS
// ============================================================================
// REQUISITO 5: Teste Ponto x Esfera
//
// Projéteis usam teste Ponto x Esfera para colisão porque:
// 1. São objetos pequenos e rápidos
// 2. A esfera aproxima bem o formato de uma bola de fogo
// 3. É mais eficiente que AABB para objetos esféricos
//
// testPointSphere(ponto, centro, raio) retorna true se:
//     ||ponto - centro||² <= raio²
// ============================================================================
void Game::handleProjectileCollisions()
{
    std::vector<Projectile>& projectiles = m_projectileManager.getProjectiles();
    std::vector<Enemy>& enemies = const_cast<std::vector<Enemy>&>(m_enemyManager.getEnemies());

    // Raios das esferas de colisão para cada tipo de entidade
    float enemyRadius = 0.10f;
    float bossRadius = 0.45f;
    float projectileRadius = 0.05f;
    float playerRadius = 0.10f;

    glm::vec4 playerPos4 = m_player.getPosition();
    glm::vec3 playerPos(playerPos4.x, playerPos4.y + 0.1f, playerPos4.z);

    for (Projectile& proj : projectiles)
    {
        if (!proj.active)
            continue;

        glm::vec3 projPos = proj.position;

        // ─────────────────────────────────────────────
        // Projetil inimigo → Colisão com Player
        // ─────────────────────────────────────────────
        if (proj.isEnemyProjectile)
        {
            if (testPointSphere(projPos, playerPos, playerRadius + projectileRadius))
            {
                m_player.takeDamage(15);
                proj.active = false;

                printf("Player hit by fireball! HP: %d/%d\n",
                    m_player.getVida(), m_player.getMaxVida());
            }
            continue; // Enemy projectiles don't hit enemies
        }

        // ─────────────────────────────────────────────
        // Projetil do Jogador → Colisão com Inimigos
        // ─────────────────────────────────────────────
        for (Enemy& enemy : enemies)
        {
            glm::vec4 epos4 = enemy.getPosition();
            glm::vec3 enemyPos(epos4.x, epos4.y, epos4.z);

            if (testPointSphere(projPos, enemyPos, enemyRadius + projectileRadius))
            {
                enemy.takeDamage(100);
                proj.active = false;
                m_hitMarkerTimer = HIT_MARKER_DURATION;

                printf("Projectile hit enemy! Enemy HP: %d\n", enemy.getVida());
                break;
            }
        }

        if (!proj.active)
            continue;

        // ─────────────────────────────────────────────
        // Projetil → Colisão com Boss
        // ─────────────────────────────────────────────
        if (m_dragonBossAlive)
        {
            glm::vec4 dpos4 = m_dragonBoss.getPosition();
            glm::vec3 dragonPos(dpos4.x, dpos4.y + 0.15f, dpos4.z);

            if (testPointSphere(projPos, dragonPos, bossRadius + projectileRadius))
            {
                m_dragonBoss.takeDamage(100);
                proj.active = false;
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

void Game::handleDragonAttack(float deltaTime)
{
    if (!m_dragonBossAlive)
        return;

    m_dragonAttackTimer += deltaTime;
    if (m_dragonAttackTimer >= m_dragonAttackInterval)
    {
        m_dragonAttackTimer = 0.0f;

        glm::vec4 dragonPos4 = m_dragonBoss.getPosition();
        glm::vec3 dragonPos = glm::vec3(dragonPos4.x, 0.25f, dragonPos4.z);

        glm::vec4 playerPos4 = m_player.getPosition();
        glm::vec3 playerPos = glm::vec3(playerPos4.x, 0.15f, playerPos4.z);

        glm::vec3 dir = playerPos - dragonPos;
        float len = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.001f)
        {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        m_projectileManager.spawnProjectile(dragonPos, dir, true);
        printf("Dragon fires at player!\n");
    }
}

void Game::handleHealthPickups(float deltaTime)
{
    m_healthSpawnTimer += deltaTime;

    int currentHealth = m_player.getVida();
    int maxHealth = m_player.getMaxVida();

    bool lowHealth = currentHealth < (maxHealth / 2);

    if (m_healthSpawnTimer >= HEALTH_SPAWN_INTERVAL &&
        m_healthPickups.size() < MAX_HEALTH_PICKUPS &&
        lowHealth)
    {
        m_healthSpawnTimer = 0.0f;
        spawnHealthPickup();
    }

    glm::vec4 playerPos4 = m_player.getPosition();
    glm::vec3 playerPos = glm::vec3(playerPos4.x, playerPos4.y, playerPos4.z);
    float pickupRadius = 0.15f;

    for (size_t i = 0; i < m_healthPickups.size(); i++)
    {
        if (!m_healthPickups[i].active)
            continue;

        glm::vec3 pickupPos = m_healthPickups[i].position;

        if (testPointSphere(playerPos, pickupPos, pickupRadius))
        {
            m_player.heal(m_healthPickups[i].healAmount);
            m_healthPickups[i].active = false;
            printf("Player picked up health! +%d HP\n", m_healthPickups[i].healAmount);
        }
    }

    for (size_t i = 0; i < m_healthPickups.size(); )
    {
        if (!m_healthPickups[i].active)
            m_healthPickups.erase(m_healthPickups.begin() + i);
        else
            i++;
    }
}

void Game::spawnHealthPickup()
{
    float x = ((float)rand() / RAND_MAX) * 6.0f - 3.0f;
    float z = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

    HealthPickup pickup;
    pickup.position = glm::vec3(x, 0.05f, z);
    pickup.active = true;
    pickup.healAmount = 25;

    m_healthPickups.push_back(pickup);
    printf("Health pickup spawned at (%.2f, %.2f)\n", x, z);
}

void Game::updateEnemySpeed(float deltaTime)
{
    float speedIncreaseInterval = 30.0f;
    float speedMultiplier = 1.0f + (m_gameTime / speedIncreaseInterval) * 0.15f;

    if (speedMultiplier > 2.0f)
        speedMultiplier = 2.0f;

    float newSpeed = m_baseEnemySpeed * speedMultiplier;
    m_enemyManager.setEnemySpeed(newSpeed);
}

void Game::cleanup()
{
    sfx.stop();
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
    m_enemyManager.setDifficulty(difficulty);

    switch (difficulty)
    {
    case 0: // Fácil
        m_player.setVida(150, 150);
        m_enemyManager.setEnemySpeed(0.15f);
        m_enemyManager.setMaxEnemies(1);
        m_enemyDamage = 5;
        m_dragonAttackInterval = 4.0f;  // Fireball a cada 4 segundos
        break;
    case 1: // Normal
        m_player.setVida(100, 100);
        m_enemyManager.setEnemySpeed(0.25f);
        m_enemyManager.setMaxEnemies(2);
        m_enemyDamage = 10;
        m_dragonAttackInterval = 2.5f;  // Fireball a cada 2.5 segundos
        break;
    case 2: // Difícil
        m_player.setVida(75, 75);
        m_enemyManager.setEnemySpeed(0.35f);
        m_enemyManager.setMaxEnemies(3);
        m_enemyDamage = 20;
        m_dragonAttackInterval = 1.0f;  // Fireball a cada 1.0 segundos
        break;
    }
}

void Game::startGame()
{
    result_sfx=true;
    menu_music=true;
    sfx.musicaPrincipalStop();
    sfx.musicaPrincipalStart("sfx/main.mp3", true);
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
    m_healthPickups.clear();
    m_dragonBoss = Enemy(-3.5f, 0.0f, 5000);
    m_dragonBossAlive = true;
    m_hitMarkerTimer = 0.0f;
    m_muzzleFlashTimer = 0.0f;
    m_dragonAttackTimer = 0.0f;
    m_healthSpawnTimer = 0.0f;
    m_gameTime = 0.0f;
    setDifficulty(m_difficulty);
    startGame();
}

void Game::returnToMenu()
{
    m_gameState = GameState::MENU;
    m_player.reset();
    m_enemyManager.clearEnemies();
    m_projectileManager.clear();
    m_healthPickups.clear();
    m_dragonBoss = Enemy(-3.5f, 0.0f, 5000);
    m_dragonBossAlive = true;
    m_hitMarkerTimer = 0.0f;
    m_muzzleFlashTimer = 0.0f;
    m_dragonAttackTimer = 0.0f;
    m_healthSpawnTimer = 0.0f;
    m_gameTime = 0.0f;
}

void Game::togglePause()
{
    if (m_gameState == GameState::PLAYING)
    {
        m_gameState = GameState::PAUSED;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        m_pauseFocusTarget = PauseFocusTarget::PLAYER;
        m_pauseFocusEnemyIndex = 0;
        glm::vec4 playerPos = m_player.getPosition();
        m_pauseCameraTarget = glm::vec4(playerPos.x, 0.3f, playerPos.z, 1.0f);
        m_pauseCameraTheta = 0.0f;
        m_pauseCameraPhi = 1.2f;
        m_pauseCameraDistance = 1.5f;
    }
    else if (m_gameState == GameState::PAUSED)
    {
        m_gameState = GameState::PLAYING;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Game::handlePauseCameraMove(float dx, float dy)
{
    if (m_gameState != GameState::PAUSED)
        return;

    float sensitivity = 0.005f;
    m_pauseCameraTheta -= dx * sensitivity;
    m_pauseCameraPhi += dy * sensitivity;

    float phiMin = 0.1f;
    float phiMax = 3.14159f - 0.1f;
    if (m_pauseCameraPhi < phiMin) m_pauseCameraPhi = phiMin;
    if (m_pauseCameraPhi > phiMax) m_pauseCameraPhi = phiMax;
}

void Game::handlePauseCameraZoom(float offset)
{
    if (m_gameState != GameState::PAUSED)
        return;

    m_pauseCameraDistance -= offset * 0.3f;
    if (m_pauseCameraDistance < 0.3f) m_pauseCameraDistance = 0.3f;
    if (m_pauseCameraDistance > 8.0f) m_pauseCameraDistance = 8.0f;
}

void Game::cyclePauseFocusTarget(bool forward)
{
    if (m_gameState != GameState::PAUSED)
        return;

    const std::vector<Enemy>& enemies = m_enemyManager.getEnemies();

    if (forward)
    {
        switch (m_pauseFocusTarget)
        {
        case PauseFocusTarget::PLAYER:
            if (!enemies.empty())
            {
                m_pauseFocusTarget = PauseFocusTarget::ENEMY;
                m_pauseFocusEnemyIndex = 0;
            }
            else if (m_dragonBossAlive)
            {
                m_pauseFocusTarget = PauseFocusTarget::DRAGON;
            }
            break;
        case PauseFocusTarget::ENEMY:
            m_pauseFocusEnemyIndex++;
            if (m_pauseFocusEnemyIndex >= (int)enemies.size())
            {
                if (m_dragonBossAlive)
                    m_pauseFocusTarget = PauseFocusTarget::DRAGON;
                else
                    m_pauseFocusTarget = PauseFocusTarget::PLAYER;
                m_pauseFocusEnemyIndex = 0;
            }
            break;
        case PauseFocusTarget::DRAGON:
            m_pauseFocusTarget = PauseFocusTarget::PLAYER;
            m_pauseFocusEnemyIndex = 0;
            break;
        }
    }
    else
    {
        switch (m_pauseFocusTarget)
        {
        case PauseFocusTarget::PLAYER:
            if (m_dragonBossAlive)
                m_pauseFocusTarget = PauseFocusTarget::DRAGON;
            else if (!enemies.empty())
            {
                m_pauseFocusTarget = PauseFocusTarget::ENEMY;
                m_pauseFocusEnemyIndex = enemies.size() - 1;
            }
            break;
        case PauseFocusTarget::ENEMY:
            m_pauseFocusEnemyIndex--;
            if (m_pauseFocusEnemyIndex < 0)
            {
                m_pauseFocusTarget = PauseFocusTarget::PLAYER;
                m_pauseFocusEnemyIndex = 0;
            }
            break;
        case PauseFocusTarget::DRAGON:
            if (!enemies.empty())
            {
                m_pauseFocusTarget = PauseFocusTarget::ENEMY;
                m_pauseFocusEnemyIndex = enemies.size() - 1;
            }
            else
                m_pauseFocusTarget = PauseFocusTarget::PLAYER;
            break;
        }
    }

    setPauseFocusTarget(m_pauseFocusTarget);
}

void Game::setPauseFocusTarget(PauseFocusTarget target)
{
    m_pauseFocusTarget = target;
    const std::vector<Enemy>& enemies = m_enemyManager.getEnemies();

    switch (target)
    {
    case PauseFocusTarget::PLAYER:
        {
            glm::vec4 pos = m_player.getPosition();
            m_pauseCameraTarget = glm::vec4(pos.x, 0.3f, pos.z, 1.0f);
        }
        break;
    case PauseFocusTarget::ENEMY:
        if (!enemies.empty())
        {
            if (m_pauseFocusEnemyIndex >= (int)enemies.size())
                m_pauseFocusEnemyIndex = 0;
            glm::vec4 pos = enemies[m_pauseFocusEnemyIndex].getPosition();
            m_pauseCameraTarget = glm::vec4(pos.x, 0.15f, pos.z, 1.0f);
        }
        break;
    case PauseFocusTarget::DRAGON:
        if (m_dragonBossAlive)
        {
            glm::vec4 pos = m_dragonBoss.getPosition();
            m_pauseCameraTarget = glm::vec4(pos.x, 0.3f, pos.z, 1.0f);
        }
        break;
    }

    m_pauseCameraTheta = 0.0f;
    m_pauseCameraPhi = 1.2f;
    m_pauseCameraDistance = 1.5f;
}

const char* Game::getFocusTargetName() const
{
    switch (m_pauseFocusTarget)
    {
    case PauseFocusTarget::PLAYER:
        return "HEROI";
    case PauseFocusTarget::ENEMY:
        return "INIMIGO";
    case PauseFocusTarget::DRAGON:
        return "DRAGAO";
    }
    return "???";
}
