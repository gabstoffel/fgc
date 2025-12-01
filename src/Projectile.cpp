#include "Projectile.h"
#include "sfx.h"
#include <cmath>

ProjectileManager::ProjectileManager()
    : m_projectileSpeed(6.0f)
    , m_enemyProjectileSpeed(3.0f)
    , m_maxLifetime(3.0f)
    , m_trailUpdateInterval(0.015f)
    , m_maxProjectiles(30)
{
}

ProjectileManager::~ProjectileManager()
{
}

void ProjectileManager::spawnProjectile(const glm::vec3& origin, const glm::vec3& direction, bool isEnemy)
{
    float speed = isEnemy ? m_enemyProjectileSpeed : m_projectileSpeed;
    if(!isEnemy){
        sfx.tiro_player();
    }
    else{
        sfx.fireball();
    }
    if (m_projectiles.size() >= m_maxProjectiles)
    {
        for (size_t i = 0; i < m_projectiles.size(); i++)
        {
            if (!m_projectiles[i].active)
            {
                Projectile& proj = m_projectiles[i];
                proj.position = origin;
                proj.velocity = direction * speed;
                proj.lifetime = m_maxLifetime;
                proj.active = true;
                proj.isEnemyProjectile = isEnemy;
                proj.trailIndex = 0;
                proj.trailTimer = 0.0f;
                for (int j = 0; j < Projectile::TRAIL_LENGTH; j++)
                    proj.trailPositions[j] = origin;
                return;
            }
        }
        return;
    }

    Projectile proj;
    proj.position = origin;
    proj.velocity = direction * speed;
    proj.lifetime = m_maxLifetime;
    proj.active = true;
    proj.isEnemyProjectile = isEnemy;
    proj.trailIndex = 0;
    proj.trailTimer = 0.0f;
    for (int i = 0; i < Projectile::TRAIL_LENGTH; i++)
        proj.trailPositions[i] = origin;

    m_projectiles.push_back(proj);
}

void ProjectileManager::update(float deltaTime)
{
    for (size_t i = 0; i < m_projectiles.size(); i++)
    {
        if (!m_projectiles[i].active)
            continue;

        Projectile& proj = m_projectiles[i];

        proj.position += proj.velocity * deltaTime;

        proj.lifetime -= deltaTime;
        if (proj.lifetime <= 0.0f)
        {
            proj.active = false;
            continue;
        }

        proj.trailTimer += deltaTime;
        if (proj.trailTimer >= m_trailUpdateInterval)
        {
            proj.trailTimer = 0.0f;
            proj.trailPositions[proj.trailIndex] = proj.position;
            proj.trailIndex = (proj.trailIndex + 1) % Projectile::TRAIL_LENGTH;
        }

        float maxDist = 10.0f;
        if (fabs(proj.position.x) > maxDist ||
            fabs(proj.position.y) > maxDist ||
            fabs(proj.position.z) > maxDist)
        {
            proj.active = false;
        }
    }
}

void ProjectileManager::removeInactive()
{
    for (size_t i = 0; i < m_projectiles.size(); )
    {
        if (!m_projectiles[i].active)
            m_projectiles.erase(m_projectiles.begin() + i);
        else
            i++;
    }
}

void ProjectileManager::clear()
{
    m_projectiles.clear();
}
