/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  Dot3Vertex_transform.vsh

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A simple shader to transform the vertex location, output
the color and the texture coordinates

-------------------------------------------------------------------------------|--------------------*/

#include "WaterInteractionConstants.h"

#define V_POSITION v0
#define V_NORMAL v1
#define V_DIFFUSE v2
#define V_TEXTURE v3

vs.1.1
dcl_position	V_POSITION
dcl_color		V_DIFFUSE
dcl_texcoord	V_TEXTURE
dcl_normal		V_NORMAL

// Transform position to clip space and output it
dp4 oPos.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

mov oT0, V_TEXTURE
mov oD0, V_DIFFUSE









