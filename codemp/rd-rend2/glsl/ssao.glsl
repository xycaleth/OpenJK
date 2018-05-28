/*[Vertex]*/
in vec4 attr_Position;
in vec4 attr_TexCoord0;

out vec2 var_ScreenTex;

void main()
{
	gl_Position = attr_Position;
	var_ScreenTex = attr_TexCoord0.xy;
}

/*[Fragment]*/
uniform sampler2D u_ScreenDepthMap;
uniform sampler2D u_ScreenImageMap;
uniform vec4 u_ViewInfo; // znear, zfar, 0, 0
uniform vec2 u_ScreenInfo; // width, height

uniform vec4 u_SSAOSettings; // aocap, strength, aoMultiplier, 0

in vec2 var_ScreenTex;

out vec4 out_Color;

//
// AO Shader by Monsterovich :D
//

vec2 camerarange = vec2(u_ViewInfo.x, u_ViewInfo.y);

float readDepth( in vec2 coord ) {
	return (2.0 * camerarange.x) / (camerarange.y + camerarange.x - texture2D( u_ScreenDepthMap, coord ).x * (camerarange.y - camerarange.x));	
}

void main(void)
{
	float depth = readDepth( var_ScreenTex );
	float d;
 
	float pw = 1.0 / u_ScreenInfo.x;
	float ph = 1.0 / u_ScreenInfo.y;
 
	float aoCap = u_SSAOSettings.x;
	float ao = 0.0;
	float aoMultiplier=1000.0;
	float depthTolerance = 0.0001;

	int i;
	for (i = 0; i < 4; i++)
	{
		d = readDepth( vec2(var_ScreenTex.x+pw,var_ScreenTex.y+ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(var_ScreenTex.x-pw,var_ScreenTex.y+ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(var_ScreenTex.x+pw,var_ScreenTex.y-ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(var_ScreenTex.x-pw,var_ScreenTex.y-ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		pw *= 2.0;
		ph *= 2.0;
		if (u_SSAOSettings.z <= 0)
			aoMultiplier /= 2.0;
		else
			aoMultiplier /= u_SSAOSettings.z;
	}

	ao /= 16.0;
	ao *= u_SSAOSettings.y;
 
	float orig = texture2D(u_ScreenImageMap,var_ScreenTex).x;
	float done = (1.0 - ao) * orig;
	out_Color = vec4(done, done, done, 0.0); 
}
