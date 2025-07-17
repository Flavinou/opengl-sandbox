#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_HDRTexture;

uniform bool u_UseHDR;
uniform float u_Exposure;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(u_HDRTexture, TexCoords).rgb;

	if (u_UseHDR)
	{
		// Reinhard tone mapping
		// vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

		// Exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * u_Exposure);

		// Gamma correction
		mapped = pow(mapped, vec3(1.0 / gamma));

		FragColor = vec4(mapped, 1.0);
	}
	else
	{
		// Only gamma correction is applied
		vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
	}
}