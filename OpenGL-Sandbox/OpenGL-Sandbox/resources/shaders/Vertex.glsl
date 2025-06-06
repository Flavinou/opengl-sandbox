#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute location 0
layout (location = 1) in vec3 aColor;

uniform float u_HorizontalOffset;

out vec3 vertexColor; // output a color to the fragment shader
out vec3 vertexPosition;

void main()
{
	gl_Position = vec4(aPos.x, aPos.yz, 1.0);
	vertexColor = aColor;
	vertexPosition = aPos;
}