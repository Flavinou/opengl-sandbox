#version 330 core
out vec4 FragColor;

uniform vec4 u_LightColor;
uniform vec4 u_TintColor;

void main()
{
	FragColor = vec4(u_LightColor * u_TintColor);
}