#include "collisions.h"
#include "matrices.h"
#include <cmath>
#include <algorithm>


bool testAABBPlane(glm::vec3 boxMin, glm::vec3 boxMax, glm::vec4 plane) {
    // Extract plane normal and distance
    // Plane equation: normal.x * x + normal.y * y + normal.z * z + d = 0
    glm::vec3 normal = glm::vec3(plane.x, plane.y, plane.z);
    float d = plane.w;

    // Compute AABB center and half-extents
    glm::vec3 center = (boxMax + boxMin) * 0.5f;
    glm::vec3 extents = boxMax - center;

    // Compute the projection interval radius of the AABB onto the plane normal
    // This is the "effective radius" of the box in the direction of the plane normal
    float r = extents.x * std::abs(normal.x) +
              extents.y * std::abs(normal.y) +
              extents.z * std::abs(normal.z);

    // Compute signed distance of box center from plane
    float s = dotproduct(normal, center) + d;

    // Intersection occurs when distance s falls within [-r, +r] interval
    return std::abs(s) <= r;
}

bool resolveAABBPlane(glm::vec3& position, glm::vec3 extents, glm::vec4 plane) {
    // Calculate AABB bounds from center position and extents
    glm::vec3 boxMin = position - extents;
    glm::vec3 boxMax = position + extents;

    // Check if collision exists
    if (!testAABBPlane(boxMin, boxMax, plane)) {
        return false; // No collision, no resolution needed
    }

    // Extract plane components
    glm::vec3 normal = glm::vec3(plane.x, plane.y, plane.z);
    float d = plane.w;

    // Compute projection radius (same as in test function)
    float r = extents.x * std::abs(normal.x) +
              extents.y * std::abs(normal.y) +
              extents.z * std::abs(normal.z);

    // Compute signed distance from center to plane
    float s = dotproduct(normal, position) + d;

    // Calculate penetration depth
    float penetrationDepth = r - std::abs(s);

    // Push the AABB out of the plane along the plane's normal
    if (s < 0) {
        // Box is on the negative side of the plane, push in normal direction
        position += normal * penetrationDepth;
    } else {
        // Box is on the positive side of the plane, push opposite to normal
        position -= normal * penetrationDepth;
    }

    return true; // Collision was resolved
}

bool testAABBAABB(const glm::vec3& minA, const glm::vec3& maxA,const glm::vec3& minB, const glm::vec3& maxB)
{
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&(minA.y <= maxB.y && maxA.y >= minB.y) &&(minA.z <= maxB.z && maxA.z >= minB.z);
}

bool testPointSphere(const glm::vec3& point, const glm::vec3& sphereCenter, float sphereRadius) {
    float distSq = norm(point - sphereCenter)*norm(point - sphereCenter);
    return distSq <= sphereRadius * sphereRadius;
}

void clampPositionToBox(glm::vec3& position, glm::vec3 minBounds, glm::vec3 maxBounds) {
    position.x = std::max(minBounds.x, std::min(position.x, maxBounds.x));
    position.y = std::max(minBounds.y, std::min(position.y, maxBounds.y));
    position.z = std::max(minBounds.z, std::min(position.z, maxBounds.z));
}
