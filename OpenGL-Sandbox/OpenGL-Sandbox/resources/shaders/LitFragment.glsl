#version 330 core
out vec4 FragColor;

in vec4 LightingColor; // This will be set by the vertex shader or application code

uniform vec4 u_TintColor;

void main()
{
	FragColor = LightingColor * u_TintColor;
}