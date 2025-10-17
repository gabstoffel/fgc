#ifndef _MATRICES_H
#define _MATRICES_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Matrix creation helper function
glm::mat4 Matrix(
    float m00, float m01, float m02, float m03, // LINHA 1
    float m10, float m11, float m12, float m13, // LINHA 2
    float m20, float m21, float m22, float m23, // LINHA 3
    float m30, float m31, float m32, float m33  // LINHA 4
)
{
    return glm::mat4(
        m00, m10, m20, m30, // COLUNA 1
        m01, m11, m21, m31, // COLUNA 2
        m02, m12, m22, m32, // COLUNA 3
        m03, m13, m23, m33  // COLUNA 4
    );
}

// Identity matrix
glm::mat4 Matrix_Identity()
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f, // LINHA 1
        0.0f, 1.0f, 0.0f, 0.0f, // LINHA 2
        0.0f, 0.0f, 1.0f, 0.0f, // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Translation matrix
glm::mat4 Matrix_Translate(float tx, float ty, float tz)
{
    return Matrix(
        1.0f, 0.0f, 0.0f, tx,   // LINHA 1
        0.0f, 1.0f, 0.0f, ty,   // LINHA 2
        0.0f, 0.0f, 1.0f, tz,   // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Scale matrix
glm::mat4 Matrix_Scale(float sx, float sy, float sz)
{
    return Matrix(
        sx,   0.0f, 0.0f, 0.0f, // LINHA 1
        0.0f, sy,   0.0f, 0.0f, // LINHA 2
        0.0f, 0.0f, sz,   0.0f, // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Rotation around X axis
glm::mat4 Matrix_Rotate_X(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f, // LINHA 1
        0.0f, c,   -s,   0.0f, // LINHA 2
        0.0f, s,    c,   0.0f, // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Rotation around Y axis
glm::mat4 Matrix_Rotate_Y(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return Matrix(
        c,    0.0f, s,   0.0f, // LINHA 1
        0.0f, 1.0f, 0.0f, 0.0f, // LINHA 2
       -s,    0.0f, c,   0.0f, // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Rotation around Z axis
glm::mat4 Matrix_Rotate_Z(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return Matrix(
        c,   -s,   0.0f, 0.0f, // LINHA 1
        s,    c,   0.0f, 0.0f, // LINHA 2
        0.0f, 0.0f, 1.0f, 0.0f, // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f  // LINHA 4
    );
}

// Vector norm
float norm(glm::vec4 v)
{
    float vx = v.x;
    float vy = v.y;
    float vz = v.z;
    return sqrt(vx*vx + vy*vy + vz*vz);
}

// Roatate geral
glm::mat4 Matrix_Rotate(float angle, glm::vec4 axis)
{
    float c = cos(angle);
    float s = sin(angle);

    glm::vec4 v = axis / norm(axis);

    float vx = v.x;
    float vy = v.y;
    float vz = v.z;

    return Matrix(
        vx*vx*(1-c)+c , vx*vy*(1-c)-vz*s , vx*vz*(1-c)+vy*s , 0.0f ,
        vx*vy*(1-c)+vz*s , vy*vy*(1-c)+c , vy*vz*(1-c)-vx*s , 0.0f ,
        vx*vz*(1-c)-vy*s , vy*vz*(1-c)+vx*s , vz*vz*(1-c)+c , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );
}

// Cross product
glm::vec4 crossproduct(glm::vec4 u, glm::vec4 v)
{
    float u1 = u.x;
    float u2 = u.y;
    float u3 = u.z;
    float v1 = v.x;
    float v2 = v.y;
    float v3 = v.z;

    return glm::vec4(
        u2*v3 - u3*v2, // Primeiro coeficiente
        u3*v1 - u1*v3, // Segundo coeficiente
        u1*v2 - u2*v1, // Terceiro coeficiente
        0.0f // w = 0 para vetores
    );
}

// Dot product
float dotproduct(glm::vec4 u, glm::vec4 v)
{
    float u1 = u.x;
    float u2 = u.y;
    float u3 = u.z;
    float v1 = v.x;
    float v2 = v.y;
    float v3 = v.z;

    if (u.w != 0.0f || v.w != 0.0f)
    {
        fprintf(stderr, "ERROR: Dot product not defined for points.\n");
        std::exit(EXIT_FAILURE);
    }

    return u1*v1 + u2*v2 + u3*v3;
}

// Camera view matrix
glm::mat4 Matrix_Camera_View(glm::vec4 position_c, glm::vec4 view_vector, glm::vec4 up_vector)
{
    glm::vec4 w = -view_vector;
    glm::vec4 u = crossproduct(up_vector, w);

    // Normalize vectors
    w = w / norm(w);
    u = u / norm(u);

    glm::vec4 v = crossproduct(w, u);

    glm::vec4 origin_o = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 vec_o_c = position_c - origin_o;

    float ux = u.x;
    float uy = u.y;
    float uz = u.z;
    float vx = v.x;
    float vy = v.y;
    float vz = v.z;
    float wx = w.x;
    float wy = w.y;
    float wz = w.z;

    return Matrix(
        ux, uy, uz, -dotproduct(u, vec_o_c), // LINHA 1
        vx, vy, vz, -dotproduct(v, vec_o_c), // LINHA 2
        wx, wy, wz, -dotproduct(w, vec_o_c), // LINHA 3
        0.0f, 0.0f, 0.0f, 1.0f              // LINHA 4
    );
}

// Orthographic projection matrix
glm::mat4 Matrix_Orthographic(float l, float r, float b, float t, float n, float f)
{
    return Matrix(
        2/(r-l), 0.0f,    0.0f,    -(r+l)/(r-l), // LINHA 1
        0.0f,    2/(t-b), 0.0f,    -(t+b)/(t-b), // LINHA 2
        0.0f,    0.0f,    2/(f-n), -(f+n)/(f-n), // LINHA 3
        0.0f,    0.0f,    0.0f,    1.0f          // LINHA 4
    );
}

// Perspective projection matrix
glm::mat4 Matrix_Perspective(float field_of_view, float aspect, float n, float f)
{
    float t = fabs(n) * tanf(field_of_view / 2.0f);
    float b = -t;
    float r = t * aspect;
    float l = -r;

    glm::mat4 P = Matrix(
        n,    0.0f, 0.0f,   0.0f,  // LINHA 1
        0.0f, n,    0.0f,   0.0f,  // LINHA 2
        0.0f, 0.0f, n+f,    -f*n,  // LINHA 3
        0.0f, 0.0f, 1.0f,   0.0f   // LINHA 4
    );

    glm::mat4 M = Matrix_Orthographic(l, r, b, t, n, f);
    return -M*P;
}

#endif // _MATRICES_H
