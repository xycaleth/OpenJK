#version 450

layout(set = 0, binding = 0) uniform sampler2D u_Texture0;
#if defined(USE_MULTITEXTURE)
layout(set = 0, binding = 1) uniform sampler2D u_Texture1;
#endif

layout(location = 0) in vec2 in_TexCoord0;
layout(location = 1) in vec4 in_Color;
#if defined(USE_MULTITEXTURE)
layout(location = 2) in vec2 in_TexCoord1;
#endif

layout(location = 0) out vec4 out_Color;

#if defined(USE_MULTITEXTURE)
vec4 BlendTexture(in vec4 c0, in vec4 c1)
{
#if USE_MULTITEXTURE == 1
    return c0 + c1;
#elif USE_MULTITEXTURE == 2
    return c0 * c1;
#endif
}
#endif

#if defined(USE_ALPHA_TESTING)
bool AlphaTestFailed(in float alpha)
{
#if USE_ALPHA_TESTING == 0
    return alpha == 0.0;
#elif USE_ALPHA_TESTING == 1
    return alpha >= 0.5;
#elif USE_ALPHA_TESTING == 2
    return alpha < 0.5;
#elif USE_ALPHA_TESTING == 3
    return alpha < 0.75;
#else
#error Unhandled USE_ALPHA_TESTING value
#endif
}
#endif

void main()
{
	vec4 color = texture(u_Texture0, in_TexCoord0);
#if defined(USE_MULTITEXTURE)
    vec4 color2 = texture(u_Texture1, in_TexCoord1);
    color = BlendTexture(color, color2);
#else  // defined(USE_MULTITEXTURE)
#if defined(USE_ALPHA_TESTING)
    if (AlphaTestFailed(color.a))
    {
        discard;
    }
#endif  // defined(USE_ALPHA_TESTING)
#endif  // defined(USE_MULTITEXTURE)
    
	out_Color = in_Color * color;
}
