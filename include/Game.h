#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include "Enemy.h"
#include "Renderer.h"
#include "Projectile.h"

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

    void render();

    void handleCollisions();

    void handleShooting();

    void handleDebugKillKey();

    void handleProjectileCollisions();

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
};

#endif 
