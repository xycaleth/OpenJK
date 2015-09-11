uniform vec2 u_InvTexRes;
uniform vec3 u_ViewOrigin;
uniform vec4 u_ViewInfo; // zfar / znear, zfar
uniform sampler2D u_ScreenDepthMap;
uniform sampler2D u_ScreenNormalMap;
uniform sampler2D u_ScreenSpecularAndGlossMap;
uniform sampler2D u_ScreenDiffuseMap;

#define u_ZFarDivZNear u_ViewInfo.x
#define u_ZFar u_ViewInfo.y

in vec3 var_ViewDir;
in flat vec3 var_LightPosition;
in flat vec3 var_LightColor;

out vec4 out_Color;

float LinearDepth(float zBufferDepth, float zFarDivZNear)
{
	return 1.0 / mix(zFarDivZNear, 1.0, zBufferDepth);
}

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

void main()
{
	ivec2 windowCoord = ivec2(gl_FragCoord.xy);
	vec4 specularAndGloss = texelFetch(u_ScreenSpecularAndGlossMap, windowCoord, 0);
	vec3 albedo = texelFetch(u_ScreenDiffuseMap, windowCoord, 0).rgb;
	vec2 normal = texelFetch(u_ScreenNormalMap, windowCoord, 0).rg;
	float depth = texelFetch(u_ScreenDepthMap, windowCoord, 0).r;

	float linearDepth = LinearDepth(depth, u_ZFarDivZNear);
	vec3 position = u_ViewOrigin + normalize(var_ViewDir) * u_ZFar * linearDepth;

	vec3 N = normalize(DecodeNormal(normal));
	vec3 vecToLight = var_LightPosition - position;
	float distanceToLight = length(vecToLight);
	vec3 L = normalize(vecToLight);

	float NdotL = clamp(dot(N, L), 0.0, 1.0);
	float attenuation = 1.0 / (distanceToLight * distanceToLight);

	vec3 diffuse = albedo * var_Light;
	
	out_Color = vec4(result, 1.0);
}