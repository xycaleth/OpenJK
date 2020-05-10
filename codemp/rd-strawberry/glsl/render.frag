#version 450

layout(set = 0, binding = 0) uniform sampler2D u_Texture0;

layout(location = 0) in vec2 in_TexCoord0;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out vec4 out_Color;

#if defined(USE_MULTIEXTURE)
layout(set = 0, binding = 1) uniform sampler2D u_Texture1;
layout(location = 2) in vec2 in_TexCoord1;
#endif

void main()
{
	vec4 color = texture(u_Texture0, in_TexCoord0);
#if defined(USE_MULTIEXTURE)
    vec4 color2 = texture(u_Texture1, in_TexCoord1);
#if USE_MULTIEXTURE == 1
    color = color + color2;
#elif USE_MULTITEXTURE == 2
    color = color * color2;
#endif
#endif
    
	out_Color = in_Color * color;
}
