// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GrayBlit_fs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

varying vec2 pixelPos;		// Position of this pixel, within the chunk

uniform sampler2D grid;

uniform samplerRECT chunk;
uniform vec2 chunkPos;		// Position of the chunk being rendered
uniform bool hasAlpha;		// True if the image uses teh alpha component

void main()
{
	vec4 c = texRECT(chunk, pixelPos);

	if(hasAlpha)
	{
		// Get the grid pixel. Dye it purple to distinguish it from the rest of the gray.
		vec3 gridc = vec3(0.9, 0.8, 1.0) * texture2D(grid, (pixelPos + chunkPos).xy/15).rgb;
		gl_FragColor = vec4(lerp(gridc, c.rrr, c.a), 1);
	}
	else
		gl_FragColor = c;
}
