/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec2 attr_TexCoord;
#if defined(USE_SKELETAL_ANIMATION)
in uvec4 attr_BoneIndexes;
in vec4 attr_BoneWeights;
#endif

out vec3 var_PositionWS;
out vec3 var_Normal;
out vec2 var_TexCoord;

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
	var_TexCoord = attr_TexCoord;
}

/*[Fragment]*/
in vec3 var_Normal;
in vec2 var_TexCoord;

out vec4 out_Color;

uniform sampler2D u_DiffuseMap;

void main()
{
    vec3 N = normalize(var_Normal);
    vec4 diffuse = texture(u_DiffuseMap, var_TexCoord);

    out_Color.rgb = diffuse.rgb * (N * 0.5 + vec3(0.5));
	out_Color.a = diffuse.a;
}
