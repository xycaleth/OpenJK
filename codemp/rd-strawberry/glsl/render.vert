#version 450

layout(push_constant) uniform Matrices
{
	mat4 u_ModelviewMatrix;
	mat4 u_ProjectionMatrix;
};

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
	gl_Position = u_ProjectionMatrix * u_ModelviewMatrix * in_Position;

	out_TexCoord0 = in_TexCoord0;
	out_Color = in_Color;
#if defined(USE_MULTITEXTURE)
	out_TexCoord1 = in_TexCoord1;
#endif
}