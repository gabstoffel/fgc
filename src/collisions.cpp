// collisions.cpp
// Collision detection implementations for FCG Final Project
//
// Authors: [Your Names]
// Date: 2025-11-06
//
// FONTE: Collision algorithms based on "Real-Time Collision Detection" by Christer Ericson
// and lecture notes from INF01047 - Fundamentos de Computação Gráfica, UFRGS

#include "collisions.h"
#include "matrices.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// AABB-Plane Collision
// ============================================================================

bool testAABBPlane(glm::vec3 boxMin, glm::vec3 boxMax, glm::vec4 plane) {
    // FONTE: Based on AABB-Plane test from "Real-Time Collision Detection" by Christer Ericson
    // Chapter 5.2.3 - Testing Box Against Plane

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

// ============================================================================
// Sphere-Sphere Collision
// ============================================================================

bool testSphereSphere(glm::vec3 center1, float radius1,
                      glm::vec3 center2, float radius2) {
    // FONTE: Standard sphere-sphere collision test
    // Based on distance comparison (avoiding sqrt for performance)

    // Calculate vector between centers
    glm::vec3 delta = center2 - center1;

    // Calculate squared distance between centers
    float distanceSquared = dotproduct(delta, delta);

    // Calculate sum of radii
    float radiusSum = radius1 + radius2;

    // Spheres intersect if distance <= sum of radii
    // Using squared values to avoid expensive sqrt operation
    return distanceSquared <= (radiusSum * radiusSum);
}

// ============================================================================
// Ray-Sphere Collision
// ============================================================================

bool testRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDir,
                   glm::vec3 sphereCenter, float radius, float& t) {
    // FONTE: Ray-sphere intersection based on "Real-Time Collision Detection" by Christer Ericson
    // Chapter 5.3.2 - Intersecting Ray or Segment Against Sphere

    // Ray equation: P(t) = rayOrigin + t * rayDir
    // Sphere equation: |P - sphereCenter|^2 = radius^2
    //
    // Substituting ray into sphere equation and expanding:
    // |rayOrigin + t*rayDir - sphereCenter|^2 = radius^2
    //
    // Let m = rayOrigin - sphereCenter
    // Then: (m + t*rayDir)·(m + t*rayDir) = radius^2
    //       m·m + 2t(rayDir·m) + t^2(rayDir·rayDir) = radius^2
    //
    // Rearranging into quadratic form: at^2 + bt + c = 0
    // where:
    //   a = rayDir·rayDir
    //   b = 2(rayDir·m)
    //   c = m·m - radius^2

    glm::vec3 m = rayOrigin - sphereCenter;

    float a = dotproduct(rayDir, rayDir);
    float b = 2.0f * dotproduct(rayDir, m);
    float c = dotproduct(m, m) - radius * radius;

    // Calculate discriminant
    float discriminant = b * b - 4.0f * a * c;

    // No intersection if discriminant is negative
    if (discriminant < 0.0f) {
        return false;
    }

    // Calculate the two solutions using quadratic formula
    float sqrtDisc = std::sqrt(discriminant);
    float t0 = (-b - sqrtDisc) / (2.0f * a);
    float t1 = (-b + sqrtDisc) / (2.0f * a);

    // We want the nearest positive intersection
    // t0 is always <= t1
    if (t0 > 0.0f) {
        t = t0; // Nearest intersection (ray enters sphere)
        return true;
    } else if (t1 > 0.0f) {
        t = t1; // Ray origin is inside sphere, return exit point
        return true;
    }

    // Both intersections are behind the ray origin
    return false;
}

// ============================================================================
// Utility Functions
// ============================================================================

void clampPositionToBox(glm::vec3& position, glm::vec3 minBounds, glm::vec3 maxBounds) {
    // Clamp each component independently
    position.x = std::max(minBounds.x, std::min(position.x, maxBounds.x));
    position.y = std::max(minBounds.y, std::min(position.y, maxBounds.y));
    position.z = std::max(minBounds.z, std::min(position.z, maxBounds.z));
}
