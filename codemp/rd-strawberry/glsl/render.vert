#version 450

layout(push_constant) uniform Matrices
{
	mat4 u_ModelviewMatrix;
	mat4 u_ProjectionMatrix;
};

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord0;
layout(location = 2) in vec4 in_Color;

layout(location = 0) out vec2 out_TexCoord0;
layout(location = 1) out vec4 out_Color;

void main()
{
	gl_Position.x = (2.0 / 640.0) * in_Position.x - 1.0;
	gl_Position.y = (2.0 / 480.0) * in_Position.y - 1.0;
	gl_Position.z = in_Position.z;
	gl_Position.w = 1.0;

	out_TexCoord0 = in_TexCoord0;
	out_Color = in_Color;
}
