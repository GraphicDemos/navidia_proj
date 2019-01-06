/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  WC_TexCoordGen_Dbg.vsh

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/

#include "WaterInteractionConstants.h"

#define V_POSITION		v0
#define V_DIFFUSE		v1
#define WORLD_POS		r0

vs.1.1
dcl_position	V_POSITION
dcl_color		V_DIFFUSE


// Scale & adjust position as an option for rendering
//  the blending object to world space as opposed to
//  rendering it into a texture.  For ordinary use
//  in blending textures together these steps are not needed
//
// c[CV_CONSTS_1] = ( 0.0f, 0.5f, 1.0f, 2.0f );
//
// translate from [0,1] to [-.5,.5]
// then scale to tile world space size

mov r0, V_POSITION
add	r0.xy, V_POSITION, - c[CV_CONSTS_1].y
mul r0.xy, r0, c[CV_TILE_SIZE] 



// Transform vertex-position to clip-space
// Could be made simpler, since the incoming quad is just 0,0 to 1.0
//  and all that needs to be done is shift it to -1,1 range.

dp4 oPos.x, r0, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, r0, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, r0, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, r0, c[CV_WORLDVIEWPROJ_3]



// Vertex coord 0,0 used to correspond to tex coord 0,0,
// but we've moved the vertex position so the center of the 
//  tile is in the center of the objectWe've moved the vertices so the center Object center 

sub oT0, V_POSITION, c[CV_TEXCOORD_BASE]


// mov oT0, v0


mov r0, V_DIFFUSE		// color to output



