#version 450

layout(set = 0, binding = 0) uniform sampler2D u_Diffuse;

layout(location = 0) in vec2 in_TexCoord0;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec4 color = texture(u_Diffuse, in_TexCoord0);
	out_Color = in_Color * color;
}
