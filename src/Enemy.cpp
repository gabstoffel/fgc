#include "Enemy.h"
#include "Player.h"
#include "matrices.h"
#include "sfx.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float z, int vida)
    : m_x(x)
    , m_z(z)
    , m_vida(vida)
    , m_enemySpeed(0.4f)
    , m_knockbackVelX(0.0f)
    , m_knockbackVelZ(0.0f)
    , m_bezierP0(x, 0.0f, z, 1.0f)
    , m_bezierP1(x, 0.0f, z, 1.0f)
    , m_bezierP2(x, 0.0f, z, 1.0f)
    , m_bezierP3(x, 0.0f, z, 1.0f)
    , m_bezierT(0.0f)
    , m_curveRecalcTimer(0.0f)
    , m_curveInitialized(false)
    , m_dying(false)
    , m_deathTimer(0.0f)
{
}

Enemy::~Enemy()
{
}

void Enemy::update(float deltaTime, const Player& player)
{
    if (m_dying)
    {
        m_deathTimer += deltaTime;
        return;
    }

    glm::vec4 playerPos = player.getPosition();

    bool inKnockback = (m_knockbackVelX != 0.0f || m_knockbackVelZ != 0.0f);

    if (inKnockback) {
        m_x += m_knockbackVelX * deltaTime;
        m_z += m_knockbackVelZ * deltaTime;

        float decay = 8.0f * deltaTime;
        m_knockbackVelX *= std::max(0.0f, 1.0f - decay);
        m_knockbackVelZ *= std::max(0.0f, 1.0f - decay);

        if (std::abs(m_knockbackVelX) < 0.05f && std::abs(m_knockbackVelZ) < 0.05f) {
            m_knockbackVelX = 0.0f;
            m_knockbackVelZ = 0.0f;
            m_bezierP0 = glm::vec4(m_x, 0.0f, m_z, 1.0f);
            m_bezierT = 0.0f;
            m_curveRecalcTimer = 0.0f;
            m_curveInitialized = false;
        }
    } else {
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

    const float arenaMinX = -4.2f;
    const float arenaMaxX = 4.2f;
    const float arenaMinZ = -1.2f;
    const float arenaMaxZ = 1.2f;

    bool hitWall = false;
    if (m_x < arenaMinX) { m_x = arenaMinX; hitWall = true; }
    if (m_x > arenaMaxX) { m_x = arenaMaxX; hitWall = true; }
    if (m_z < arenaMinZ) { m_z = arenaMinZ; hitWall = true; }
    if (m_z > arenaMaxZ) { m_z = arenaMaxZ; hitWall = true; }

    // recalcula a curva se bate numa parede
    if (hitWall && !inKnockback) {
        onObstacleCollision();
    }
}

// ============================================================================
// RECÁLCULO DA CURVA DE BÉZIER - Conversão Hermite para Bézier
// ============================================================================
// CUBIC HERMITE SPLINES:
//    Definidas por pontos P0, P3 e vetores tangentes v0, v1
///
// CONTINUIDADE C1:
//    Para transição suave entre curvas consecutivas, a derivada
//    no fim da curva anterior deve ser igual à derivada no início
//    da nova curva. Usamos evaluateBezierDerivative(t) para obter
//    a velocidade atual e usá-la como v0 da nova curva.
//
// CATMULL-ROM (para curva inicial):
//    Quando não há curva anterior, usamos a fórmula de Catmull-Rom:
// ============================================================================
void Enemy::recalculateCurve(const glm::vec4& playerPos)
{
    // P0: posição atual do inimigo (início da curva)
    m_bezierP0 = glm::vec4(m_x, 0.0f, m_z, 1.0f);

    // P3: posição do jogador (fim da curva)
    m_bezierP3 = glm::vec4(playerPos.x, 0.0f, playerPos.z, 1.0f);

    // Vetor direção e distância até o alvo
    glm::vec4 toTarget = m_bezierP3 - m_bezierP0;
    float distance = norm(toTarget);

    if (distance < 0.01f) {
        // Muito perto do alvo: curva degenerada (linha reta)
        m_bezierP1 = m_bezierP0;
        m_bezierP2 = m_bezierP3;
    } else {
        // Direção normalizada e vetor perpendicular (para variação)
        glm::vec4 dir = toTarget / distance;
        glm::vec4 perp = glm::vec4(-dir.z, 0.0f, dir.x, 0.0f);

        // ================================================================
        // v0: Vetor tangente inicial (para continuidade C1)
        // ================================================================
        // Se já existe uma curva, usa a derivada atual para manter
        // a velocidade contínua (sem "saltos" na direção do movimento)
        glm::vec4 v0;
        if (m_curveInitialized && m_bezierT > 0.001f) {
            // Continuidade C1: v0 = derivada da curva anterior no ponto atual
            v0 = evaluateBezierDerivative(m_bezierT);
        } else {
            // Sem curva anterior: usa fórmula de Catmull-Rom 
            v0 = toTarget * 0.5f;
        }

        // ================================================================
        // v1: Vetor tangente final (com variação aleatória)
        // ================================================================
        // Direção base para o alvo com offset perpendicular aleatório
        // Isso cria trajetórias mais interessantes (curvas S ou C)
        float side = (rand() % 2 == 0) ? 1.0f : -1.0f;
        float perpOffset = ((float)rand() / RAND_MAX) * 0.3f;
        glm::vec4 approachDir = dir + perp * (side * perpOffset);
        float approachMag = norm(approachDir);
        if (approachMag > 0.001f) {
            approachDir = approachDir / approachMag;
        }
        // Escala v1 proporcionalmente à distância (estilo Catmull-Rom)
        glm::vec4 v1 = approachDir * distance * 0.5f;

        // ================================================================
        // CONVERSÃO HERMITE → BÉZIER
        // ================================================================
        // Das notas de aula:
        //   P1 = P0 + v0/3  (ponto de controle inicial)
        //   P2 = P3 - v1/3  (ponto de controle final)
        m_bezierP1 = m_bezierP0 + v0 * (1.0f / 3.0f);
        m_bezierP2 = m_bezierP3 - v1 * (1.0f / 3.0f);

        // Restringe pontos de controle aos limites da arena
        // (evita trajetórias que saem muito da área de jogo)
        const float arenaMinX = -4.1f;
        const float arenaMaxX = 4.1f;
        const float arenaMinZ = -1.1f;
        const float arenaMaxZ = 1.1f;

        m_bezierP1.x = std::max(arenaMinX, std::min(m_bezierP1.x, arenaMaxX));
        m_bezierP1.z = std::max(arenaMinZ, std::min(m_bezierP1.z, arenaMaxZ));
        m_bezierP2.x = std::max(arenaMinX, std::min(m_bezierP2.x, arenaMaxX));
        m_bezierP2.z = std::max(arenaMinZ, std::min(m_bezierP2.z, arenaMaxZ));
    }

    // Reinicia o parâmetro t para o início da nova curva
    m_bezierT = 0.0f;
    // Timer para recalcular a curva periodicamente (2-3 segundos)
    m_curveRecalcTimer = 2.0f + ((float)rand() / RAND_MAX) * 1.0f;
}

// ============================================================================
// AVALIAÇÃO DA CURVA DE BÉZIER CÚBICA
// ============================================================================
// ============================================================================
glm::vec4 Enemy::evaluateBezier(float t) const
{
    // Calcula (1-t) e suas potências
    float u = 1.0f - t;      // u = (1-t)
    float u2 = u * u;        // u² = (1-t)²
    float u3 = u2 * u;       // u³ = (1-t)³

    // Calcula t e suas potências
    float t2 = t * t;        // t²
    float t3 = t2 * t;       // t³

    // Avalia a curva usando polinômios de Bernstein:
    return u3 * m_bezierP0              // b₀,₃(t)·P0
         + 3.0f * u2 * t * m_bezierP1   // b₁,₃(t)·P1
         + 3.0f * u * t2 * m_bezierP2   // b₂,₃(t)·P2
         + t3 * m_bezierP3;             // b₃,₃(t)·P3
}

// ============================================================================
// DERIVADA DA CURVA DE BÉZIER CÚBICA
// ============================================================================
// CASOS ESPECIAIS (usados para continuidade C1):
//    c'(0) = 3(P1 - P0) = v0  (tangente no início)
//    c'(1) = 3(P3 - P2) = v1  (tangente no fim)
//
// Esta função é usada para obter a velocidade atual do inimigo
// e garantir transições suaves entre curvas consecutivas.
// ============================================================================
glm::vec4 Enemy::evaluateBezierDerivative(float t) const
{
    float u = 1.0f - t;  // u = (1-t)

    // Derivada: c'(t) = 3(1-t)²(P1-P0) + 6(1-t)t(P2-P1) + 3t²(P3-P2)
    return 3.0f * u * u * (m_bezierP1 - m_bezierP0)      // 3(1-t)²(P1-P0)
         + 6.0f * u * t * (m_bezierP2 - m_bezierP1)      // 6(1-t)t(P2-P1)
         + 3.0f * t * t * (m_bezierP3 - m_bezierP2);     // 3t²(P3-P2)
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
    else{
        sfx.hit_monstro();
    }
}

void Enemy::applyKnockback(float dirX, float dirZ, float force)
{
    m_knockbackVelX = dirX * force;
    m_knockbackVelZ = dirZ * force;
}

void Enemy::onObstacleCollision()
{
    m_bezierP0 = glm::vec4(m_x, 0.0f, m_z, 1.0f);
    m_bezierT = 0.0f;
    m_curveRecalcTimer = 0.0f; 
    m_curveInitialized = false;
}

void Enemy::startDying()
{
    if (!m_dying)
    {
        m_dying = true;
        m_deathTimer = 0.0f;
        sfx.morte_monstro();
    }
}

float Enemy::getDeathProgress() const
{
    if (!m_dying) return 0.0f;
    return std::min(m_deathTimer / DEATH_ANIM_DURATION, 1.0f);
}

float Enemy::getDeathScale() const
{
    if (!m_dying) return 1.0f;
    float t = getDeathProgress();
    return 1.0f + t * (2.0f - t);
}

bool Enemy::isReadyForRemoval() const
{
    return m_dying && m_deathTimer >= DEATH_ANIM_DURATION;
}

EnemyManager::EnemyManager()
    : m_previousSecond(-1)
    , m_maxEnemies(2)
    , m_spawnInterval(5)
    , m_enemySpeed(0.4f)
    , m_difficulty(1)
{
}

int EnemyManager::getRandomEnemyHP()
{
    int roll = rand() % 100;
    int easyChance, mediumChance;

    switch (m_difficulty) {
        case 0: easyChance = 70; mediumChance = 95; break;
        case 1: easyChance = 33; mediumChance = 67; break;
        case 2: easyChance = 5;  mediumChance = 30; break;
        default: easyChance = 33; mediumChance = 67; break;
    }

    if (roll < easyChance) {
        return 200 + rand() % 101;  // 200-300 HP
    } else if (roll < mediumChance) {
        return 400 + rand() % 101;  // 400-500 HP
    } else {
        return 600 + rand() % 101;  // 600-700 HP
    }
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
    float x_aleatorio = 5.5f * rand() / (static_cast<float>(RAND_MAX)) - 3.5f;
    float z_aleatorio = 2.0f * rand() / (static_cast<float>(RAND_MAX)) - 1.0f;

    while ((x_aleatorio - playerPosition.x) * (x_aleatorio - playerPosition.x) +
           (z_aleatorio - playerPosition.z) * (z_aleatorio - playerPosition.z) < 1.0f)
    {
        x_aleatorio = 5.5f * rand() / (static_cast<float>(RAND_MAX)) - 3.5f;
        z_aleatorio = 2.0f * rand() / (static_cast<float>(RAND_MAX)) - 1.0f;
    }

    int enemyHP = getRandomEnemyHP();
    Enemy novo_inimigo(x_aleatorio, z_aleatorio, enemyHP);
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
        {
            if (!m_enemies[i].isDying())
            {
                m_enemies[i].startDying();
                i++;
            }
            else if (m_enemies[i].isReadyForRemoval())
            {
                m_enemies.erase(m_enemies.begin() + i);
            }
            else
            {
                i++;
            }
        }
        else
        {
            i++;
        }
    }
}


