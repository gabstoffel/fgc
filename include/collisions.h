#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include <glm/glm.hpp>

bool testAABBPlane(glm::vec3 boxMin, glm::vec3 boxMax, glm::vec4 plane);

bool resolveAABBPlane(glm::vec3& position, glm::vec3 extents, glm::vec4 plane);

bool testSphereSphere(glm::vec3 center1, float radius1,
                      glm::vec3 center2, float radius2);

bool testRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDir,
                   glm::vec3 sphereCenter, float radius, float& t);

void clampPositionToBox(glm::vec3& position, glm::vec3 minBounds, glm::vec3 maxBounds);

#endif 
