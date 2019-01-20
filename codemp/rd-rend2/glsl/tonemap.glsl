/*[Vertex]*/
out vec2 var_TexCoord;

void main()
{
    vec2 position = vec2(
        2.0 * float(gl_VertexID & 2) - 1.0,
        4.0 * float(gl_VertexID & 1) - 1.0);
	gl_Position = vec4(position, 0.0, 1.0);
	var_TexCoord = position * 0.5 + vec2(0.5);
}

/*[Fragment]*/
#if 1
uniform sampler2D u_TextureMap;

in vec2 var_TexCoord;

out vec4 out_Color;

vec3 sRGBEncode(in vec3 linear)
{
    vec3 lo = linear * 12.92;
    vec3 hi = 1.055 * pow(linear, vec3(1.0 / 2.4)) - vec3(0.055);
    return mix(lo, hi, greaterThan(linear, vec3(0.0031308)));
}

void main()
{
    // Stupid simple tonemapping operator for now
    vec3 color = texture(u_TextureMap, var_TexCoord).rgb;
    out_Color.rgb = sRGBEncode(color);
    out_Color.a = 1.0;
}
#else
uniform sampler2D u_TextureMap;
uniform sampler2D u_LevelsMap;
uniform vec4 u_Color;
uniform vec2  u_AutoExposureMinMax;
uniform vec3   u_ToneMinAvgMaxLinear;

in vec2 var_TexCoord;

out vec4 out_Color;

const vec3  LUMINANCE_VECTOR =   vec3(0.2125, 0.7154, 0.0721); //vec3(0.299, 0.587, 0.114);

vec3 LinearTosRGB( in vec3 color )
{
	vec3 clampedColor = clamp(color, 0.0, 1.0);

	vec3 lo = 12.92 * clampedColor;
	vec3 hi = 1.055 * pow(clampedColor, vec3(0.41666)) - 0.055;
	return mix(lo, hi, greaterThanEqual(color, vec3(0.0031308)));
}

vec3 FilmicTonemap(vec3 x)
{
	const float SS  = 0.22; // Shoulder Strength
	const float LS  = 0.30; // Linear Strength
	const float LA  = 0.10; // Linear Angle
	const float TS  = 0.20; // Toe Strength
	const float TAN = 0.01; // Toe Angle Numerator
	const float TAD = 0.30; // Toe Angle Denominator
	
	vec3 SSxx = SS * x * x;
	vec3 LSx = LS * x;
	vec3 LALSx = LSx * LA;
	
	return ((SSxx + LALSx + TS * TAN) / (SSxx + LSx + TS * TAD)) - TAN / TAD;

	//return ((x*(SS*x+LA*LS)+TS*TAN)/(x*(SS*x+LS)+TS*TAD)) - TAN/TAD;

}

void main()
{
	vec4 color = texture(u_TextureMap, var_TexCoord) * u_Color;
	vec3 minAvgMax = texture(u_LevelsMap, var_TexCoord).rgb;
	vec3 logMinAvgMaxLum = clamp(minAvgMax * 20.0 - 10.0, -u_AutoExposureMinMax.y, -u_AutoExposureMinMax.x);
		
	float avgLum = exp2(logMinAvgMaxLum.y);
	//float maxLum = exp2(logMinAvgMaxLum.z);

	color.rgb *= u_ToneMinAvgMaxLinear.y / avgLum;
	color.rgb = max(vec3(0.0), color.rgb - vec3(u_ToneMinAvgMaxLinear.x));

	vec3 fWhite = 1.0 / FilmicTonemap(vec3(u_ToneMinAvgMaxLinear.z - u_ToneMinAvgMaxLinear.x));
	color.rgb = FilmicTonemap(color.rgb) * fWhite;
	//color.rgb = LinearTosRGB(color.rgb);
	
	out_Color = clamp(color, 0.0, 1.0);
}
#endif