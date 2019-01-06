// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/Separable-Symmetric_fs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

// Note that the calling code will #define the KERNELSIZE macro for us
//   before loading this code....

varying vec4 filterRange;

uniform samplerRECT chunk;
uniform half duoweights[KERNELSIZE];	// The overall kernel weight of pixel pairs
uniform vec4 offsets[KERNELSIZE];		// pre-computed trash

void main()
{
	half4 accum = 0;

	for(int i = 0;i < KERNELSIZE;i++)
	{
		vec4 px = filterRange + offsets[i];
		accum += (texRECT(chunk, px.xy) + texRECT(chunk, px.zw)) * duoweights[i];
	}

	gl_FragColor = accum;
}
