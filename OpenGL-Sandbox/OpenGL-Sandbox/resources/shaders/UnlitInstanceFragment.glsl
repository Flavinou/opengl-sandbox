#version 330 core
out vec4 FragColor;

in vec3 vs_Color;

void main()
{
	FragColor = vec4(vs_Color, 1.0);
}