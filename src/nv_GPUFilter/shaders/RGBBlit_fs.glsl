// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/RGBBlit_fs.glsl#1 $
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

void main()
{
	vec4 c = texRECT(chunk, pixelPos);
	
	if(hasAlpha)
	{
		// Get the grid pixel
		vec3 gridc = texture2D(grid, (pixelPos + chunkPos).xy/15).rgb;
		gl_FragColor = vec4(lerp(gridc,c.rgb,c.a),1);
	}
	else
		gl_FragColor = c;
}
