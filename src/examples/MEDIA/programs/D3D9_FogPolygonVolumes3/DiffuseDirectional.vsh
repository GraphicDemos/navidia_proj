/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  DiffuseDirectional.nvv

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
CV_CONSTS_1 = 0.0, 0.5, 1.0, 2.0

CV_DIRLIGHT1 = directional light vector from light in object space


-------------------------------------------------------------------------------|--------------------*/

#include "Constants.h"

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_TEXTURE   v2

#define TEMP		r1

vs.1.1
dcl_position	V_POSITION
dcl_normal		V_NORMAL
dcl_texcoord	V_TEXTURE

// Transform position to clip space and output it
dp4 oPos.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

// Generate a fog value:
// Fill fog with 1.0 for no fog
mov oFog, c[CV_CONSTS_1].z

// Directional light + ambient
// negative light vector for vector from vertex to light 
dp3		TEMP, -c[CV_DIRLIGHT1_DIR], V_NORMAL

// clamp <0 to 0
max		TEMP, TEMP, c[CV_CONSTS_1].xxxx

// mult by light color
mul		TEMP, TEMP, c[CV_DIRLIGHT1_COLOR]

// Add ambient
add		oD0, TEMP, c[CV_OBJ_AMBIENT_COL]

//  Pass through the texture coords
mov oT0, V_TEXTURE
mov oT1, V_TEXTURE
mov oT2, V_TEXTURE
mov oT3, V_TEXTURE

