#version 330 core
out vec4 FragColor;

in vec3 gs_color; // Color from the geometry shader

void main()
{
	FragColor = vec4(gs_color, 1.0); // Set the fragment color to the color passed from the geometry shader
}