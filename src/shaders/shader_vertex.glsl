#version 330 core

// Vertex attributes received as input
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 color_coefficients;

// Vertex attributes that will be output to the fragment shader
out vec4 color_interpolated;

// Matrices computed in C++ and sent to GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform vertex position through model, view, and projection matrices
    gl_Position = projection * view * model * model_coefficients;
    
    // Pass color to fragment shader
    color_interpolated = color_coefficients;
}
