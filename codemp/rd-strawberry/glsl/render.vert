#version 450

struct FogParameters
{
	vec3 start;  // or density if mode is exp/exp2
	int mode;  // 0 = no fog, 1 = linear, 2 = exp, 3 = exp2
	vec3 end;
	uint color;  // packed color
};

layout(set = 0, binding = 0) uniform Camera
{
	mat4 projectionMatrix;
} u_Camera;

layout(set = 1, binding = 0) uniform Entity
{
	mat4 modelViewMatrix;
} u_Entity;

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord0;
layout(location = 2) in vec4 in_Color;
#if defined(USE_MULTITEXTURE)
layout(location = 3) in vec2 in_TexCoord1;
#endif

layout(location = 0) out vec2 out_TexCoord0;
layout(location = 1) out vec4 out_Color;
#if defined(USE_MULTITEXTURE)
layout(location = 2) out vec2 out_TexCoord1;
#endif

void main()
{
	gl_Position = u_Camera.projectionMatrix * u_Entity.modelviewMatrix * in_Position;

	out_TexCoord0 = in_TexCoord0;
	out_Color = in_Color;
#if defined(USE_MULTITEXTURE)
	out_TexCoord1 = in_TexCoord1;
#endif
}