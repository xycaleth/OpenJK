uniform vec3 u_ViewOrigin;

out vec3 var_ViewDir;

void main()
{
	const vec2 positions[] = vec2[4](
		vec2(-1.0, -1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0,  1.0),
		vec2( 1.0, -1.0)
	);

	var_ViewDir = vec3(0.0, 0.0, -1.0);
	gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}