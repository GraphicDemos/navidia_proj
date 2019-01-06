#ifndef __GLSL_CG_DATA_TYPES
# define half2 vec2
# define half3 half3
# define half4 half4
#endif

// Uniforms for vertex light, world position, worldview matrix
uniform mat4      worldViewMatrix;
uniform mat4      projMatrix;
uniform half4     eyePosition;
uniform half4     lightVector;
uniform half4     lightPos;

// uniforms for color of lights/objects/clearing
uniform half4     clearColor;
uniform half3     ambientColor;
uniform half3     diffuse;
uniform half      Kd;
uniform half      Ks;
uniform half3     lightColor;
uniform half3     specColor;
uniform half      specPower;

// Common to both vertex and fragment programs
uniform half      Time;
uniform half2     Offsets;
uniform half      Speed;
uniform int       Samples;
uniform int		  UseYUV;

// Uniforms for the maximum Texture and Window Size
uniform half2     winsize;
uniform half2     texsize;

// Uniform variables (vertex displacement, vertex angles)
uniform half      displacement;
uniform half      currentAngle;
uniform half2     BlurCenter;

// Uniforms to control ProcAmps and other post processing
uniform half      brightness;
uniform half      hue;
uniform half      contrast;
uniform half      saturation;

uniform float OverExplosure;		// range 0.0 to 10.0, steps of 0.1
uniform half FrameJitter;		// range 0.0 to 6.0, steps of 0.1 
uniform half MaxFrameJitter;	// range 0.0 to 10.0, steps of 0.1
uniform half DustAmount;		// range 0.0 to 4.0, steps of 1
uniform half GrainThickness;	// range 0.1 to 4.0, steps of 0.1
uniform half ScratchesAmount;	// range 0.0 to 3.0, steps of 1

// maximum of 32 varying halfs
varying half4 Dust01Coords;
varying half4 Dust23Coords;

varying half2 SceneCoord;
varying half2 TvCoords;
varying half2 NoiseCoords;
varying half2 Line0VertCoords;
varying half2 Line1VertCoords;
varying half2 Line2VertCoords;
varying float OverExp;

void main(void)
{
    half4 v = half4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  
    gl_Position = gl_ModelViewProjectionMatrix * v;

		// some pseudo-random numbers
	float Random0 = mod(Time, 7.000);
	float Random1 = mod(Time, 3.300);
	float Random2 = mod(Time, 0.910);
	float Random3 = mod(Time, 0.110);
	float Random4 = mod(Time, 0.070);
	float Random5 = mod(Time, 0.013);

	half2 inverseWindowSize = half2(1.0, 1.0) / winsize;

	// compute vertical frame jitter
	float frameJitterY =  40 * MaxFrameJitter * Random2 * Random0 * Random3;
	if (frameJitterY < (6 - FrameJitter) * 10) { frameJitterY = 0; }
 	frameJitterY *=inverseWindowSize.y;

	// add jitter to the original coords.
	half2 normTexCoord = gl_MultiTexCoord0.xy / winsize;
	SceneCoord.xy =  half4(normTexCoord, gl_MultiTexCoord0.z, 1).xy + half4(0, frameJitterY, 0, 0).xy;
	SceneCoord.xy *= winsize;

	// compute over exposure amount
//	OverExp = OverExposure * Random3;
	
	// pass original screen coords (border rendering)
	TvCoords = half4(normTexCoord, gl_MultiTexCoord0.z, 1);

	// compute noise coords.
	half2 NoiseCoordsTmp = (winsize / (GrainThickness * half2(128.0, 128.0))) * normTexCoord;
	NoiseCoordsTmp      += half2(100 * Random3 * Random1 - Random0, Random4 + Random1 * Random2);
	NoiseCoords          = NoiseCoordsTmp.xyxy;

	// dust section (turns on or off particular dust texture)
	if (DustAmount > 0)
	{
		Dust01Coords.xy = 2.0 * gl_Position.xy + 200 * float2(Random1 * Random4, mod(Time,0.03) );
	}
	else
	{
		Dust01Coords.xy = 0;
	}

	if (DustAmount > 1)
	{
		Dust01Coords.zw = 2.3 * gl_Position.yx - 210 * half2(Random4 * 0.45, Random5 * 2);
	}
	else
	{
		Dust01Coords.zw = 0;
	}

	if (DustAmount > 2)
	{
		Dust23Coords.xy = 1.4 * gl_Position.xy + half2(700, +100) * half2(Random2 * Random4, Random2);
	}
	else
	{
		Dust23Coords.xy = 0;
	}

	if (DustAmount > 3)
	{
		Dust23Coords.zw = 1.7 * gl_Position.yx + half2(-100, 130) * half2(Random2 * Random4, Random1 * Random4);
	}
	else
	{
		Dust23Coords.zw = 0;
	}
	
	// vert lines section
	Line0VertCoords   = 0.5 * gl_Position.xx * winsize.xx * 0.3;
	Line1VertCoords = Line0VertCoords;
	Line2VertCoords = Line0VertCoords;

	// first line
	if (ScratchesAmount > 0)
	{
		Line0VertCoords.x += 0.15 - ((Random1 - Random3 + Random2) * 0.1) * winsize.x;
	}
	else
	{
		Line0VertCoords.x = -1;
	}

	// second line
	if (ScratchesAmount > 1)
	{
		Line1VertCoords.x += 0.55 + ((Random0 - Random2 + Random4) * 0.1) * winsize.x;
	}
	else
	{
		Line1VertCoords.x = -1;
	}

	// third line
	if (ScratchesAmount > 2)
	{
		Line2VertCoords.x += 0.31 + ((Random1 - Random2 + Random4) * 0.2) * winsize.x;
	}
	else
	{
		Line2VertCoords.x = -1;
	}

    gl_FrontColor  = gl_Color;
}

