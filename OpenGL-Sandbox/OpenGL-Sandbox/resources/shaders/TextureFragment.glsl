#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Texture1;

void main()
{             
    vec4 texColor = texture(u_Texture1, TexCoords);
    FragColor = texColor;
}