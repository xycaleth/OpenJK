uniform sampler2D u_DiffuseMap;

in vec2 var_DiffuseTex;

in vec4 var_Color;
in vec3 var_Normal;

out vec4 out_Color;
out vec4 out_SpecularAndGloss;
out vec4 out_Normal;
out vec4 out_Light;

vec2 EncodeNormal( in vec3 N )
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void main()
{
	vec4 color  = texture(u_DiffuseMap, var_DiffuseTex);
	out_Color = color * var_Color;
	out_SpecularAndGloss = vec4(0.04, 0.04, 0.04, 0.01);
	out_Normal = vec4(EncodeNormal(normalize(var_Normal)), 0.0, 0.0);
	out_Light = vec4(0.0);
#if 0
#if defined(USE_GLOW_BUFFER)
	out_Glow = out_Color;
#else
	out_Glow = vec4(0.0);
#endif
#endif
}
