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
uniform sampler2D u_ScreenDepthMap; // colormap
uniform sampler2D u_ScreenImageMap; // lightmap
uniform vec4 u_ViewInfo; // znear, zfar, 0, 0
uniform vec2 u_ScreenInfo; // width, height

uniform vec4 u_SSAOSettings; // aocap, strength, aoMultiplier, lightmap
uniform vec4 u_SSAOSettings2; // noise, aorange, depthTolerance, 0

in vec2 var_ScreenTex;

out vec4 out_Color;

//
// AO Shader by Monsterovich :D
//

vec2 camerarange = vec2(u_ViewInfo.x, u_ViewInfo.y);

float readDepth( in vec2 coord ) {
	return (2.0 * camerarange.x) / (camerarange.y + camerarange.x - texture2D( u_ScreenDepthMap, coord ).x * (camerarange.y - camerarange.x));	
}

float compareDepths( in float depth1, in float depth2 ) {
	float diff = sqrt( clamp(1.0-(depth1-depth2) / ( u_SSAOSettings2.y /* aorange */ / (u_ViewInfo.y - u_ViewInfo.x)),0.0,1.0) );
	float ao = min(u_SSAOSettings.x /* aocap */,max(0.0,depth1-depth2-u_SSAOSettings2.z /* depthTolerance */) * u_SSAOSettings.z /* aoMultiplier */) * diff;
	return ao;
}

vec2 rand(vec2 coord) //generating random noise
{
	float noiseX = (fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453));
	float noiseY = (fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453));
	return vec2(noiseX,noiseY)*0.004;
}

void main(void)
{
	float depth = readDepth( var_ScreenTex );
	vec2 noise = rand( var_ScreenTex );

	float d;
 
	float pw = 1.0 / u_ScreenInfo.x;
	float ph = 1.0 / u_ScreenInfo.y;
	if (u_SSAOSettings2.x > 0) // apply random noise?
	{
		pw /= clamp(depth,0.05,1.0)+(noise.x*(1.0-noise.x));
		ph /= clamp(depth,0.05,1.0)+(noise.y*(1.0-noise.y));
	}
 
	float aoCap = u_SSAOSettings.x;
	float ao = 0.0;
	float aoScale = 1.0;

	int i;
	for (i = 0; i < 4; i++)
	{
		d = readDepth( vec2(var_ScreenTex.x+pw,var_ScreenTex.y+ph));
		ao += compareDepths(depth,d) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw,var_ScreenTex.y+ph));
		ao += compareDepths(depth,d) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x+pw,var_ScreenTex.y-ph));
		ao += compareDepths(depth,d) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw,var_ScreenTex.y-ph));
		ao += compareDepths(depth,d) / aoScale;

		pw *= 2.0;
		ph *= 2.0;
		aoScale *= 1.2;
	}

	ao /= 16.0;
	ao *= u_SSAOSettings.y;
 
	float done = (1.1 - ao);
	if (u_SSAOSettings.w > 1)
	{
		float orig = texture2D(u_ScreenImageMap,var_ScreenTex).x;
		done *= (1.0 - orig) * u_SSAOSettings.w;
	}

	out_Color = vec4(done, done, done, 0.0); 
}
