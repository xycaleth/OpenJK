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
uniform vec4 u_ViewInfo; // znear, zfar, 0, 0
uniform vec2 u_ScreenInfo; // width, height

uniform vec4 u_SSAOSettings; // aocap, 0, aoMultiplier, lightmap
uniform vec4 u_SSAOSettings2; // 0, aorange, depthTolerance, 0

in vec2 var_ScreenTex;

out vec4 out_Color;

//
// AO Shader by Monsterovich :D
//

float readDepth( in vec2 coord, in float znear, in float zfar ) {
	return (2.0 * znear) / (zfar + znear - texture( u_ScreenDepthMap, coord ).x * (zfar - znear));	
}

float compareDepths( in float depth1, in float depth2, in float znear, in float zfar  ) {
	float diff = sqrt( clamp(1.0-(depth1-depth2) / ( u_SSAOSettings2.y /* aorange */ / (zfar - znear)),0.0,1.0) );
	float ao = min(u_SSAOSettings.x /* aocap */,max(0.0,depth1-depth2-u_SSAOSettings2.z /* depthTolerance */) * u_SSAOSettings.z /* aoMultiplier */) * diff;
	return ao;
}

const float P1 = 0.70710678118; // sin,cos(pi/4)
const float P2x = 0.92387953251; // cos(pi/8)
const float P2y = 0.38268343236; // sin(pi/8)
const float P3x = P2y; // cos(3*pi/8)
const float P3y = P2x; // sin(3*pi/8)


void main(void)
{
	float depth = readDepth( var_ScreenTex, u_ViewInfo.x, u_ViewInfo.y );

	float d;
 
	float pw = 1.0 / u_ScreenInfo.x;
	float ph = 1.0 / u_ScreenInfo.y;
 
	float ao = 0.0;
	float aoScale = 1.0;

	int i;
	for (i = 0; i < 4; i++)
	{
		// This creates a circle, using precalculated sin/cos for performance reasons
		// pi / 8 (4 points)
		d = readDepth( vec2(var_ScreenTex.x+pw*P2x,var_ScreenTex.y+ph*P2y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw*P2x,var_ScreenTex.y+ph*P2y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x+pw*P2x,var_ScreenTex.y-ph*P2y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw*P2x,var_ScreenTex.y-ph*P2y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		// 3*pi / 8 (4 points)
		d = readDepth( vec2(var_ScreenTex.x+pw*P3x,var_ScreenTex.y+ph*P3y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw*P3x,var_ScreenTex.y+ph*P3y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x+pw*P3x,var_ScreenTex.y-ph*P3y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw*P3x,var_ScreenTex.y-ph*P3y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		// pi / 4 (4 points)
		d = readDepth( vec2(var_ScreenTex.x+pw/P1,var_ScreenTex.y+ph/P1), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw/P1,var_ScreenTex.y+ph/P1), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x+pw/P1,var_ScreenTex.y-ph/P1), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw/P1,var_ScreenTex.y-ph/P1), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		// up/down/left/right
		d = readDepth( vec2(var_ScreenTex.x+pw,var_ScreenTex.y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x-pw,var_ScreenTex.y), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x,var_ScreenTex.y-ph), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		d = readDepth( vec2(var_ScreenTex.x,var_ScreenTex.y+ph), u_ViewInfo.x, u_ViewInfo.y);
		ao += compareDepths(depth,d,u_ViewInfo.x,u_ViewInfo.y) / aoScale;

		pw *= 2.0;
		ph *= 2.0;
		aoScale *= 1.2;
	}

	ao /= 32.0;
 
	float done = (1.0 - ao) * u_SSAOSettings.w;
	out_Color = vec4(done, done, done, 0.0); 
}
