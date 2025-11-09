#include "Enemy.h"
#include "Player.h"
#include "matrices.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float z, int vida)
    : m_x(x)
    , m_z(z)
    , m_vida(vida)
{
}

Enemy::~Enemy()
{
}

void Enemy::update(float deltaTime, const Player& player)
{
}

float Enemy::lookAt(const glm::vec4& targetPosition) const
{
    glm::vec4 vetor_front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    glm::vec4 olha_target = targetPosition - getPosition();

    if (norm(vetor_front) == 0 || norm(olha_target) == 0)
        return 0.0f;

    float angulo = acos(dotproduct(olha_target, vetor_front) / (norm(vetor_front) * norm(olha_target)));

    if (olha_target.x * vetor_front.z - olha_target.z * vetor_front.x < 0)
        angulo = -angulo;

    return angulo;
}

void Enemy::takeDamage(int damage)
{
    m_vida -= damage;
    if (m_vida < 0)
        m_vida = 0;
}

EnemyManager::EnemyManager()
    : m_previousSecond(0)
    , m_maxEnemies(5)
    , m_spawnInterval(5)
{
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::update(float deltaTime, const Player& player)
{
    for (size_t i = 0; i < m_enemies.size(); i++)
    {
        m_enemies[i].update(deltaTime, player);
    }
}

void EnemyManager::spawnEnemy(const glm::vec4& playerPosition)
{
    float x_aleatorio = 1.8f * rand() / (static_cast<float>(RAND_MAX)) - 0.9f;
    float z_aleatorio = 1.8f * rand() / (static_cast<float>(RAND_MAX)) - 0.9f;

    while ((x_aleatorio - playerPosition.x) * (x_aleatorio - playerPosition.x) +
           (z_aleatorio - playerPosition.z) * (z_aleatorio - playerPosition.z) < 0.1f)
    {
        x_aleatorio = 1.8f * rand() / (static_cast<float>(RAND_MAX)) - 0.9f;
        z_aleatorio = 1.8f * rand() / (static_cast<float>(RAND_MAX)) - 0.9f;
    }

    Enemy novo_inimigo(x_aleatorio, z_aleatorio, 100);
    m_enemies.push_back(novo_inimigo);
}

void EnemyManager::trySpawnEnemy(int currentSecond, const glm::vec4& playerPosition)
{
    if (currentSecond % m_spawnInterval == 0 &&
        m_enemies.size() < static_cast<size_t>(m_maxEnemies) &&
        m_previousSecond != currentSecond)
    {
        spawnEnemy(playerPosition);
    }

    m_previousSecond = currentSecond;
}

void EnemyManager::removeDeadEnemies()
{
    for (size_t i = 0; i < m_enemies.size(); )
    {
        if (m_enemies[i].isDead())
            m_enemies.erase(m_enemies.begin() + i);
        else
            i++;
    }
}
