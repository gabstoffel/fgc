#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <glm/vec3.hpp>

struct Projectile
{
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    bool active;

    static const int TRAIL_LENGTH = 6;
    glm::vec3 trailPositions[TRAIL_LENGTH];
    int trailIndex;
    float trailTimer;

    Projectile()
        : position(0.0f)
        , velocity(0.0f)
        , lifetime(0.0f)
        , active(false)
        , trailIndex(0)
        , trailTimer(0.0f)
    {
        for (int i = 0; i < TRAIL_LENGTH; i++)
            trailPositions[i] = glm::vec3(0.0f);
    }
};

class ProjectileManager
{
public:
    ProjectileManager();
    ~ProjectileManager();

    void spawnProjectile(const glm::vec3& origin, const glm::vec3& direction);
    void update(float deltaTime);
    void removeInactive();
    void clear();

    const std::vector<Projectile>& getProjectiles() const { return m_projectiles; }
    std::vector<Projectile>& getProjectiles() { return m_projectiles; }

private:
    std::vector<Projectile> m_projectiles;

    float m_projectileSpeed;
    float m_maxLifetime;
    float m_trailUpdateInterval;
    size_t m_maxProjectiles;
};

#endif
