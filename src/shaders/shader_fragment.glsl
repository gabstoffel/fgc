#version 330 core

// Fragment attributes received as input from vertex shader
in vec4 color_interpolated;

// Final color output
out vec4 color;

void main()
{
    // Set the final color using the interpolated color from vertices
    color = color_interpolated;
}
