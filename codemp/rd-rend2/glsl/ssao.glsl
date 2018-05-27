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
uniform vec4 u_ViewInfo; // zfar / znear, zfar, 0, 0
uniform vec2 u_ScreenInfo; // width, height

in vec2 var_ScreenTex;

out vec4 out_Color;

//
// AO Shader by Monsterovich :D
//

float zNear = 1.0 / (u_ViewInfo.x / u_ViewInfo.y);
float zFar = u_ViewInfo.y;

vec2 camerarange = vec2(zNear, zFar);
vec2 screensize = u_ScreenInfo;
vec2 texCoord = var_ScreenTex;
 
 
float readDepth( in vec2 coord ) {
	return (2.0 * camerarange.x) / (camerarange.y + camerarange.x - texture2D( u_ScreenDepthMap, coord ).x * (camerarange.y - camerarange.x));	
}

void main(void)
{	
	float depth = readDepth( texCoord );
	float d;
 
	float pw = 1.0 / screensize.x;
	float ph = 1.0 / screensize.y;
 
	float aoCap = 1.0;
	float ao = 0.0;
	float aoMultiplier=1000.0;
	float depthTolerance = 0.0001;

	int i;
	for (i = 0; i < 4; i++)
	{
		d = readDepth( vec2(texCoord.x+pw,texCoord.y+ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(texCoord.x-pw,texCoord.y+ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(texCoord.x+pw,texCoord.y-ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		d = readDepth( vec2(texCoord.x-pw,texCoord.y-ph));
		ao += min(aoCap,max(0.0,depth-d-depthTolerance) * aoMultiplier);

		pw *= 2.0;
		ph *= 2.0;
		aoMultiplier /= 2.0;
	}

	ao /= 16.0;
 
	float orig = texture2D(u_ScreenImageMap,texCoord).x;
	float done = (1.0 - ao) * orig;
	out_Color = vec4(done, done, done, 0.0); 
}
