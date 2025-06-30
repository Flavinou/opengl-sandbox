// ExplodeGeometry.glsl - Geometry shader for exploding geometry (i.e. translating vertices outward along their normals)
#version 330 core
layout (triangles) in; // Input primitive type is triangles
layout (triangle_strip, max_vertices = 3) out; // Output primitive type is triangle strip with a maximum of 3 vertices

in VS_OUT
{
	vec3 FragPos; // Texture coordinates passed from the vertex shader
	vec3 Normal; // Normal vector in world space
	vec2 TexCoords; // Texture coordinates passed from the vertex shader
} gs_in[]; // Input from the vertex shader

out GS_OUT
{
	vec3 FragPos; // Texture coordinates passed from the vertex shader
	vec3 Normal; // Normal vector in world space
	vec2 TexCoords; // Texture coordinates passed from the vertex shader
} gs_out; // Output to the fragment shader

uniform float u_Time;

vec3 get_normal();
vec4 explode(vec4 position, vec3 normal);

void main()
{
	vec3 normal = get_normal(); // Calculate the normal vector for the triangle

	gl_Position = explode(gl_in[0].gl_Position, normal); // Explode the first vertex
	gs_out.TexCoords = gs_in[0].TexCoords; // Pass texture coordinates to the fragment shader
	gs_out.FragPos = vec3(gl_in[0].gl_Position); // Pass the position to the fragment shader
	gs_out.Normal = normal; // Pass the normal to the fragment shader
	EmitVertex(); // Emit the vertex

	gl_Position = explode(gl_in[1].gl_Position, normal); // Explode the second vertex
	gs_out.TexCoords = gs_in[1].TexCoords; // Pass texture coordinates to the fragment shader
	gs_out.FragPos = vec3(gl_in[1].gl_Position); // Pass the position to the fragment shader
	gs_out.Normal = normal; // Pass the normal to the fragment shader
	EmitVertex(); // Emit the vertex

	gl_Position = explode(gl_in[2].gl_Position, normal); // Explode the third vertex
	gs_out.TexCoords = gs_in[2].TexCoords; // Pass texture coordinates to the fragment shader
	gs_out.FragPos = vec3(gl_in[2].gl_Position); // Pass the position to the fragment shader
	gs_out.Normal = normal; // Pass the normal to the fragment shader
	EmitVertex(); // Emit the vertex
	EndPrimitive(); // End the primitive
}

vec3 get_normal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b)); // Calculate the normal vector
}

vec4 explode(vec4 position, vec3 normal)
{
	float magnitude = 2.0; // Adjust this value to control the explosion magnitude
	vec3 direction = normal * ((sin(u_Time) + 1.0) / 2.0) * magnitude;
	return position + vec4(direction, 1.0); // Translate the vertex outward along the normal
}