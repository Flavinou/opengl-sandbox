#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute location 0
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord; // texture coordinate

out vec3 vertexColor; // output a color to the fragment shader
out vec2 texCoord; // output texture coordinate to the fragment shader

uniform mat4 u_Transform;

void main()
{
	gl_Position = u_Transform * vec4(aPos, 1.0);
	vertexColor = aColor;
	texCoord = aTexCoord; // pass the associated texture coordinate
}