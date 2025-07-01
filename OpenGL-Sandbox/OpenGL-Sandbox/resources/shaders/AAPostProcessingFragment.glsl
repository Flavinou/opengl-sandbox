#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;

void main()
{
    vec3 col = texture(u_ScreenTexture, TexCoords).rgb;
    float grayscale = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    FragColor = vec4(vec3(grayscale), 1.0);
} 