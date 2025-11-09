#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include "Enemy.h"
#include "Renderer.h"

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

    void setShouldClose(bool shouldClose);

private:
    void update(float deltaTime);

    void render();

    void handleCollisions();

    void handleShooting();

    void handleDebugKillKey();

    Player m_player;
    EnemyManager m_enemyManager;
    Renderer m_renderer;

    GLFWwindow* m_window;

    double m_lastFrameTime;
    int m_segundoAnterior; 
};

#endif 
