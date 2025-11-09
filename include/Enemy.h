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

private:
    float m_x;    
    float m_z;    
    int m_vida;   
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

private:
    std::vector<Enemy> m_enemies;
    int m_previousSecond; 
    int m_maxEnemies;     
    int m_spawnInterval;  
};

#endif 
