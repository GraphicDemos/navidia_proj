// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/standard_fs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

varying vec2 pixelPos;

uniform samplerRECT chunk;

void main()
{
  gl_FragColor = texRECT(chunk, pixelPos);
}
