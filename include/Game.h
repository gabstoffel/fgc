#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Renderer.h"
#include "Projectile.h"
#include "sfx.h"
struct HealthPickup
{
    glm::vec3 position;
    bool active;
    int healAmount;
};

struct Pillar
{
    glm::vec3 position;
    float sizeXZ;
    float height;
};

struct Torch
{
    glm::vec3 position;
    bool active;
};

enum class GameState {
    MENU,
    COUNTDOWN,
    PLAYING,
    GAME_OVER,
    WIN
};

class Game
{
public:
    Game();
    ~Game();

    bool init();

    void run();

    void cleanup();

    GLFWwindow* getWindow() { return m_window; }
    Player& getPlayer() { return m_player; }
    Renderer& getRenderer() { return m_renderer; }
    const Enemy& getDragonBoss() const { return m_dragonBoss; }
    bool isDragonBossAlive() const { return m_dragonBossAlive; }
    EnemyManager& getEnemyManager() { return m_enemyManager; }

    void setShouldClose(bool shouldClose);

    GameState getGameState() const { return m_gameState; }
    void setDifficulty(int difficulty);
    void startGame();
    void resetGame();
    void returnToMenu();

private:
    void update(float deltaTime);

    void render(float deltaTime);

    void handleCollisions();
    void handleEnemyEnvironmentCollisions();

    void handleShooting();

    void handleDebugKillKey();

    void handleProjectileCollisions();
    void handleDragonAttack(float deltaTime);
    void handleHealthPickups(float deltaTime);
    void spawnHealthPickup();
    void updateEnemySpeed(float deltaTime);

    Player m_player;
    EnemyManager m_enemyManager;
    Renderer m_renderer;
    Enemy m_dragonBoss;
    bool m_dragonBossAlive;

    GLFWwindow* m_window;

    double m_lastFrameTime;
    int m_segundoAnterior;

    GameState m_gameState;
    int m_difficulty;
    int m_enemyDamage;
    float m_countdownTimer;

    ProjectileManager m_projectileManager;

    float m_hitMarkerTimer;
    float m_muzzleFlashTimer;
    static constexpr float HIT_MARKER_DURATION = 0.15f;
    static constexpr float MUZZLE_FLASH_DURATION = 0.08f;

    float m_dragonAttackTimer;
    float m_dragonAttackInterval;

    std::vector<HealthPickup> m_healthPickups;
    float m_healthSpawnTimer;
    static constexpr float HEALTH_SPAWN_INTERVAL = 10.0f;
    static constexpr int MAX_HEALTH_PICKUPS = 3;
    bool result_sfx;
    bool menu_music;
    std::vector<Pillar> m_pillars;

    std::vector<Torch> m_torches;
    float m_gameTime;
    float m_baseEnemySpeed;
};

#endif
