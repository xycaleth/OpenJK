/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec4 attr_TexCoord0;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;
uniform vec3 u_ViewOrigin;

out vec2 var_Tex1;
out vec3 normal;
out vec3 position;
out vec3 viewDir;

void main()
{
	gl_Position 	= u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_Tex1 		= attr_TexCoord0.st;
	position  		= (u_ModelMatrix * vec4(attr_Position, 1.0)).xyz;
	normal    		= (u_ModelMatrix * vec4(attr_Normal,   0.0)).xyz;
	viewDir 		= u_ViewOrigin - position;
	
}

/*[Fragment]*/

const float etaR = 1.14;
const float etaG = 1.12;
const float etaB = 1.10;
const float fresnelPower = 2.0;
const float F = ((1.0 - etaG) * (1.0 - etaG)) / ((1.0 + etaG) * (1.0 + etaG));

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_ColorMap;
uniform vec4 u_Color;

in vec2 var_Tex1;
in vec3 normal;
in vec3 position;
in vec3 viewDir;

out vec4 out_Color;


void main()
{	
	vec3 i = normalize(viewDir);
	vec3 n = normalize(normal);
	vec4 color = vec4(1.0);
	float ratio = F + (1.0 - F) * pow(1.0 - dot(-i, n), fresnelPower);
	vec2 refractR = vec2(var_Tex1 + (refract(i, n, etaR)).xy*0.01);
	vec2 refractG = vec2(var_Tex1 + (refract(i, n, etaG)).xy*0.01);
	vec2 refractB = vec2(var_Tex1 + (refract(i, n, etaB)).xy*0.01);
	vec3 refractColor;
	refractColor.r = texture(u_DiffuseMap, refractR).r;
	refractColor.g  = texture(u_DiffuseMap, refractG).g;
	refractColor.b  = texture(u_DiffuseMap, refractB).b;
	vec3 combinedColor = mix(refractColor, texture(u_ColorMap,var_Tex1).rgb, ratio);
	out_Color = vec4(combinedColor, 1.0);

}
