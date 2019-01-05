/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;

out vec3 var_Normal;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;

void main()
{
	vec3 position  = attr_Position;
	gl_Position = u_ModelViewProjectionMatrix * u_ModelMatrix * vec4(position, 1.0);
	var_Normal = attr_Normal;
}

/*[Fragment]*/
in vec3 var_Normal;

out vec4 out_Color;

void main()
{
    vec3 N = normalize(var_Normal);
    out_Color.rgb = N * 0.5 + vec3(0.5); //diffuse.rgb * lightColor;
	out_Color.a = 1.0; //diffuse.a * var_Color.a;
}
