// PointsGeometry.glsl - "pass-through" geometry shader for points
#version 330 core
layout (points) in; // Input primitive type is points
layout (triangle_strip, max_vertices = 5) out; // Output primitive type is triangle strip with a maximum of 5 vertices

in VS_OUT
{
	vec3 color; // Color passed from the vertex shader
} gs_in[]; // Input from the vertex shader

out vec3 gs_color; // Output color to the fragment shader

vec3 get_normal();
void build_house(vec4 position);

void main()
{
	build_house(gl_in[0].gl_Position); // Call the function to build a house at the position of the point
}

vec3 get_normal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b)); // Calculate the normal vector
}

void build_house(vec4 position)
{
	gs_color = gs_in[0].color; // Pass the color from the vertex shader to the geometry shader output
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // Bottom left
	EmitVertex(); // Emit the vertex
	gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0); // Bottom right
	EmitVertex(); // Emit the vertex
	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0); // Top left
	EmitVertex(); // Emit the vertex
	gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0); // Top right
	EmitVertex(); // Emit the vertex
	gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0); // Top of the roof
	gs_color = vec3(1.0, 1.0, 1.0); // Change color for the roof to a snowy white
	EmitVertex(); // Emit the vertex
	EndPrimitive(); // End the primitive
}