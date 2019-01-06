// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/Separable-Symmetric_vs.glsl#1 $
//
// (C)2005 NVIDIA Corporation
//
// Photoshop will search the folder "C:\GPUFilter", if it exists, or the local
//	directory for this shader. If it does not exist on disk, an internal version will be used.
//	Installing and modifying the disk version of this shader allows the user to alter the
//	behavior and definition of the overall filter.
//

varying vec4 filterRange;

uniform vec2 chunkPos;
uniform vec2 filterSize;

uniform float kernellen;

void main()
{
  gl_Position = gl_ProjectionMatrix * gl_Vertex;

  filterRange = vec4(1,1,-1,-1) * normalize(filterSize).xyxy * kernellen + (gl_Vertex.xy - chunkPos).xyxy;
}
