#include "Enemy.h"
#include "Player.h"
#include "matrices.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float z, int vida)
    : m_x(x)
    , m_z(z)
    , m_vida(vida)
    , m_enemySpeed(0.25f) 
    , m_bezierP0(x, 0.0f, z, 1.0f)
    , m_bezierP1(x, 0.0f, z, 1.0f)
    , m_bezierP2(x, 0.0f, z, 1.0f)
    , m_bezierP3(x, 0.0f, z, 1.0f)
    , m_bezierT(0.0f)
    , m_curveRecalcTimer(0.0f)
    , m_curveInitialized(false)
{
}

Enemy::~Enemy()
{
}

void Enemy::update(float deltaTime, const Player& player)
{
    glm::vec4 playerPos = player.getPosition();

    if (!m_curveInitialized) {
        recalculateCurve(playerPos);
        m_curveInitialized = true;
    }

    m_curveRecalcTimer -= deltaTime;

    float distance = norm(m_bezierP3 - m_bezierP0);
    if (distance > 0.001f) {
        m_bezierT += (m_enemySpeed * deltaTime) / distance;
    }

    if (m_bezierT >= 1.0f || m_curveRecalcTimer <= 0.0f) {
        m_bezierT = std::min(m_bezierT, 1.0f); 
        recalculateCurve(playerPos);
    }

    glm::vec4 newPos = evaluateBezier(m_bezierT);
    m_x = newPos.x;
    m_z = newPos.z;
}

void Enemy::recalculateCurve(const glm::vec4& playerPos)
{
    glm::vec4 currentVelocity = glm::vec4(0.0f);
    if (m_curveInitialized && m_bezierT > 0.001f) {
        currentVelocity = evaluateBezierDerivative(m_bezierT);
    }

    // P0 
    m_bezierP0 = glm::vec4(m_x, 0.0f, m_z, 1.0f);

    // P3 
    m_bezierP3 = glm::vec4(playerPos.x, 0.0f, playerPos.z, 1.0f);

    glm::vec4 toTarget = m_bezierP3 - m_bezierP0;
    float distance = norm(toTarget);

    if (distance < 0.01f) {
        m_bezierP1 = m_bezierP0;
        m_bezierP2 = m_bezierP3;
    } else {
        glm::vec4 dir = toTarget / distance;

        glm::vec4 perp = glm::vec4(-dir.z, 0.0f, dir.x, 0.0f);

        float side1 = (rand() % 2 == 0) ? 1.0f : -1.0f;
        float side2 = -side1; // garante que os pontos vão ficar em lados opostos

        float offset1 = 0.3f + ((float)rand() / RAND_MAX) * 0.2f; 
        m_bezierP1 = m_bezierP0 + dir * (distance * 0.33f)
                   + perp * (side1 * distance * offset1);

        float offset2 = 0.3f + ((float)rand() / RAND_MAX) * 0.2f; 
        m_bezierP2 = m_bezierP0 + dir * (distance * 0.66f)
                   + perp * (side2 * distance * offset2);
    }

    m_bezierT = 0.0f;
    m_curveRecalcTimer = 2.0f + ((float)rand() / RAND_MAX) * 1.0f;
}

glm::vec4 Enemy::evaluateBezier(float t) const
{
    // B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
    float u = 1.0f - t;
    float u2 = u * u;
    float u3 = u2 * u;
    float t2 = t * t;
    float t3 = t2 * t;

    return u3 * m_bezierP0
         + 3.0f * u2 * t * m_bezierP1
         + 3.0f * u * t2 * m_bezierP2
         + t3 * m_bezierP3;
}

glm::vec4 Enemy::evaluateBezierDerivative(float t) const
{
    //  B'(t) = 3(1-t)²(P1-P0) + 6(1-t)t(P2-P1) + 3t²(P3-P2)
    float u = 1.0f - t;

    return 3.0f * u * u * (m_bezierP1 - m_bezierP0)
         + 6.0f * u * t * (m_bezierP2 - m_bezierP1)
         + 3.0f * t * t * (m_bezierP3 - m_bezierP2);
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
    : m_previousSecond(-1)
    , m_maxEnemies(2)
    , m_spawnInterval(5)
    , m_enemySpeed(0.25f)
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
    float z_aleatorio = 0.9f * rand() / (static_cast<float>(RAND_MAX)); 

    while ((x_aleatorio - playerPosition.x) * (x_aleatorio - playerPosition.x) +
           (z_aleatorio - playerPosition.z) * (z_aleatorio - playerPosition.z) < 0.3f)
    {
        x_aleatorio = 1.8f * rand() / (static_cast<float>(RAND_MAX)) - 0.9f;
        z_aleatorio = 0.9f * rand() / (static_cast<float>(RAND_MAX));
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
