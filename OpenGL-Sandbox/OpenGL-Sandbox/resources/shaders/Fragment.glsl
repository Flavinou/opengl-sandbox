#version 330 core
out vec4 FragColor;

in vec3 vertexColor;
in vec2 texCoord;

uniform vec4 u_Color; // This uniform is used to change the fragment color over time

uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;

void main()
{
	FragColor = mix(texture(u_Texture1, texCoord), texture(u_Texture2, texCoord), 0.2) * u_Color;
}