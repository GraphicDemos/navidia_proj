// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/standard_vs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

varying vec2 pixelPos;

uniform vec2 chunkPos;

void main()
{
  gl_Position = gl_ProjectionMatrix * gl_Vertex;
  pixelPos = gl_Vertex.xy - chunkPos;
}
