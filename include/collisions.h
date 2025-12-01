#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include <glm/glm.hpp>

bool testAABBPlane(glm::vec3 boxMin, glm::vec3 boxMax, glm::vec4 plane);

bool resolveAABBPlane(glm::vec3& position, glm::vec3 extents, glm::vec4 plane);

void clampPositionToBox(glm::vec3& position, glm::vec3 minBounds, glm::vec3 maxBounds);

bool testAABBAABB(const glm::vec3& minA, const glm::vec3& maxA,const glm::vec3& minB, const glm::vec3& maxB);

bool testPointSphere(const glm::vec3& point, const glm::vec3& sphereCenter, float sphereRadius);
#endif
