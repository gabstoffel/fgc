#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include <glm/vec4.hpp>

class Player;

class Enemy
{
public:
    Enemy(float x, float z, int vida = 100);
    ~Enemy();

    void update(float deltaTime, const Player& player);

    float lookAt(const glm::vec4& targetPosition) const;

    void takeDamage(int damage);
    bool isDead() const { return m_vida <= 0; }

    float getX() const { return m_x; }
    float getZ() const { return m_z; }
    int getVida() const { return m_vida; }
    glm::vec4 getPosition() const { return glm::vec4(m_x, 0.101f, m_z, 1.0f); }
    void setPosition(float x, float z) { m_x = x; m_z = z; }
    void applyKnockback(float dirX, float dirZ, float force);

private:
    float m_x;
    float m_z;
    int m_vida;
    float m_enemySpeed;
    float m_knockbackVelX;
    float m_knockbackVelZ;

    glm::vec4 m_bezierP0;       
    glm::vec4 m_bezierP1;      
    glm::vec4 m_bezierP2;      
    glm::vec4 m_bezierP3;       
    float m_bezierT;             
    float m_curveRecalcTimer;    
    bool m_curveInitialized;    
    void recalculateCurve(const glm::vec4& playerPos);
    glm::vec4 evaluateBezier(float t) const;
    glm::vec4 evaluateBezierDerivative(float t) const;
};

class EnemyManager
{
public:
    EnemyManager();
    ~EnemyManager();

    void update(float deltaTime, const Player& player);

    void spawnEnemy(const glm::vec4& playerPosition);
    void trySpawnEnemy(int currentSecond, const glm::vec4& playerPosition);

    void removeDeadEnemies();

    const std::vector<Enemy>& getEnemies() const { return m_enemies; }
    size_t getEnemyCount() const { return m_enemies.size(); }

    void setEnemySpeed(float speed) { m_enemySpeed = speed; }
    void setMaxEnemies(int maxEnemies) { m_maxEnemies = maxEnemies; }
    void clearEnemies() { m_enemies.clear(); m_previousSecond = -1; }
    void setDifficulty(int difficulty) { m_difficulty = difficulty; }
    int getRandomEnemyHP();

private:
    std::vector<Enemy> m_enemies;
    int m_previousSecond;
    int m_maxEnemies;
    int m_spawnInterval;
    float m_enemySpeed;
    int m_difficulty;
};

#endif 
