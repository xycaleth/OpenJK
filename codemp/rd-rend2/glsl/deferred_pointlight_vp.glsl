#if defined(LIGHT_POINT)

uniform vec4 u_DlightTransforms[MAX_DLIGHTS]; // xyz = position, w = scale
uniform vec3 u_DlightColors[MAX_DLIGHTS];
uniform mat4 u_ModelViewProjectionMatrix;
uniform vec3 u_ViewOrigin;

in vec3 in_Position;

out flat vec3 var_LightPosition;
out flat vec3 var_LightColor;

#elif defined(LIGHT_GRID)

uniform vec3 u_ViewForward;
uniform vec3 u_ViewLeft;
uniform vec3 u_ViewUp;

#endif

out vec3 var_ViewDir;

void main()
{
#if defined(LIGHT_POINT)

	vec4 transform = u_DlightTransforms[gl_InstanceID];
	vec3 worldSpacePosition = in_Position * transform.w * 4.0 + transform.xyz;

	var_ViewDir = normalize(worldSpacePosition - u_ViewOrigin);
	var_LightPosition = transform.xyz;
	var_LightColor = u_DlightColors[gl_InstanceID] * transform.w * 4.0;
	gl_Position = u_ModelViewProjectionMatrix * vec4(worldSpacePosition, 1.0);

#elif defined(LIGHT_GRID)

	const vec2 positions[] = vec2[4](
		vec2(-1.0, -1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0,  1.0),
		vec2( 1.0, -1.0)
	);

	vec2 position = positions[gl_VertexID];
	var_ViewDir = (u_ViewForward + u_ViewLeft * -position.x) + u_ViewUp * position.y;
	gl_Position = vec4(position, 0.0, 1.0);

#endif
}
