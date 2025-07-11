#version 330 core
in vec4 FragPos;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(FragPos.xyz - u_LightPosition);

	// map to [0; 1] range by dividing by the far plane
	lightDistance = lightDistance / u_FarPlane;

	// write the computed value as modified depth
	gl_FragDepth = lightDistance;
}