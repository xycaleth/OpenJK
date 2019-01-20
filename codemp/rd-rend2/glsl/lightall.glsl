/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec2 attr_TexCoord0;
#if defined(USE_LIGHTMAP)
in vec2 attr_TexCoord1;
#endif
#if defined(USE_SKELETAL_ANIMATION)
in uvec4 attr_BoneIndexes;
in vec4 attr_BoneWeights;
#endif

out vec3 var_PositionWS;
out vec3 var_Normal;
out vec2 var_TexCoord0;
#if defined(USE_LIGHTMAP)
out vec2 var_TexCoord1;
#endif

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;
#if defined(USE_SKELETAL_ANIMATION)
uniform mat4x3 u_BoneMatrices[20];
#endif

void main()
{
    vec3 position = attr_Position;
    vec3 normal = attr_Normal;

#if defined(USE_SKELETAL_ANIMATION)
    mat4x3 influence =
        u_BoneMatrices[attr_BoneIndexes[0]] * attr_BoneWeights[0] +
        u_BoneMatrices[attr_BoneIndexes[1]] * attr_BoneWeights[1] +
        u_BoneMatrices[attr_BoneIndexes[2]] * attr_BoneWeights[2] +
        u_BoneMatrices[attr_BoneIndexes[3]] * attr_BoneWeights[3];

    position = influence * vec4(position, 1.0);
    normal = normalize(influence * vec4(normal - vec3(0.5), 0.0));
#endif

    var_PositionWS = (u_ModelMatrix * vec4(position, 1.0)).xyz;

	gl_Position = u_ModelViewProjectionMatrix * vec4(var_PositionWS, 1.0);
	var_Normal = (u_ModelMatrix * vec4(normal, 0.0)).xyz;
	var_TexCoord0 = attr_TexCoord0;
#if defined(USE_LIGHTMAP)
    var_TexCoord1 = attr_TexCoord1;
#endif
}

/*[Fragment]*/
in vec3 var_Normal;
in vec2 var_TexCoord0;
#if defined(USE_LIGHTMAP)
in vec2 var_TexCoord1;
#endif

out vec4 out_Color;

uniform vec4 u_PrimaryLightOrigin;
uniform vec3 u_PrimaryLightColor;

uniform sampler2D u_DiffuseMap;
#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;
#endif

vec3 sRGBDecode(in vec3 srgb)
{
    vec3 lo = srgb / 12.92;
    vec3 hi = pow(((srgb + vec3(0.055)) / 1.055), vec3(2.4));
    return mix(lo, hi, greaterThan(srgb, vec3(0.04045)));
}

void main()
{
    vec3 N = normalize(var_Normal);
    vec4 diffuse = texture(u_DiffuseMap, var_TexCoord0);
    diffuse.rgb = sRGBDecode(diffuse.rgb);

    vec4 radiance = diffuse;

    vec3 incomingLight = vec3(0.0);
#if defined(USE_LIGHTMAP)
    {
        vec3 lightmap = sRGBDecode(texture(u_LightMap, var_TexCoord1).rgb);
        incomingLight += lightmap;
    }
#endif

    float NL = dot(N, u_PrimaryLightOrigin.xyz);
    if (NL > 0.0)
    {
        incomingLight += NL * u_PrimaryLightColor;
    }

    radiance.rgb *= incomingLight;

    out_Color.rgb = radiance.rgb;
	out_Color.a = diffuse.a;
}
