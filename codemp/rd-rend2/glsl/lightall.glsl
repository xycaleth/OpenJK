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

void SkinVertex(
    in vec3 basePosition,
    in vec3 baseNormal,
    in uvec4 boneIndexes,
    in vec4 boneWeights,
    in mat4x3 boneMatrices[20],
    out vec3 skinnedPosition,
    out vec3 skinnedNormal)
{
    mat4x3 influence = mat4x3(0.0);

	for (int i = 0; i < 4; i++)
	{
		uint boneIndex = boneIndexes[i];
		influence += boneMatrices[boneIndex] * boneWeights[i];
	}

	skinnedPosition = influence * vec4(basePosition, 1.0);
    skinnedNormal = normalize(influence * vec4(baseNormal, 0.0));
}

void main()
{
    vec3 position = attr_Position;
    vec3 normal = attr_Normal;

#if defined(USE_SKELETAL_ANIMATION)
	SkinVertex(
	    position,
	    normal - vec3(0.5),
	    attr_BoneIndexes,
	    attr_BoneWeights,
	    u_BoneMatrices,
	    position,
	    normal);
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
