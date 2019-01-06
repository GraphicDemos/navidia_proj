// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/LABBlit_fs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

varying vec2 pixelPos;

uniform sampler2D grid;

uniform samplerRECT chunk;
uniform vec2 chunkPos;
uniform bool hasAlpha;

// Blits a L*a*b* image as RGB

// R -> L*
// G -> a*
// B -> b*
// A -> A

const mat3 XYZConv = 
{
vec3(3.240479,  -1.537150, -0.498535),
vec3(-0.969256,	1.875992,  0.041556),
vec3(0.055648,  -0.204043, 1.057311),
};

const vec3 XYZn = {0.950456, 1, 1.088754};
const vec3 LabScale = {255.0f/500.0f,0,-255.0f/200.0f};

void main()
{
	vec4 lab = texRECT(chunk, pixelPos);

	float p = (lab.x * 100.0f + 16.0f) / 116.0f;
	vec3 xyz = XYZn * pow(p.xxx + (lab.yxz - 0.5)*LabScale, 3);

	if(hasAlpha)
	{
		// Get the grid pixel
		vec3 gridc = texture2D(grid, (pixelPos + chunkPos).xy/15).rgb;
		gl_FragColor = vec4(lerp(gridc, clamp(pow(xyz*XYZConv, 1.0/2.2),0,1), lab.a), 1);
	}
	else
		gl_FragColor = vec4(pow(xyz*XYZConv, 1.0/2.2), 1);
}
