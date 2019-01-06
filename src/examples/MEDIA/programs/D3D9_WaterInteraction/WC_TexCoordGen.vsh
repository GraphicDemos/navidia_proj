/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  WC_TexCoordGen.vsh

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

#define V_POSITION    v0
#define V_DIFFUSE     v1


#define WORLD_POS		r0


vs.1.1

dcl_position	V_POSITION
dcl_color		V_DIFFUSE

// tile size is  sx, sy, 1.0, 1.0
//mov		r0, V_POSITION
//mul		r0.xy, V_POSITION, c[CV_TILE_SIZE]

mul		r0, V_POSITION, c[CV_TILE_SIZE]


// Transform vertex-position to clip-space
// Could be made simpler, since the incoming quad is just 0,0 to 1.0
//  and all that needs to be done is shift it to -1,1 range.

dp4 oPos.x, r0, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, r0, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, r0, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, r0, c[CV_WORLDVIEWPROJ_3]


// Generate texture coords from position.  
// Need to invert v texture coord axis

mov		r1,		V_POSITION

// y = 1 - y
add		r1.y,	c[CV_CONSTS_1].z, -r1

// Translate tex coord to match difference in position
//  of tiled & detail objects.
// No scaling for now, so the size of texels in world space
//	 must match between the detail and tiled textures.

sub		oT0, r1, c[CV_TEXCOORD_BASE]

mov oD0, V_DIFFUSE.zzzz			// color to output, replicate blue to all

// Test - move constant white into output color
// mov		oD0, c[CV_CONSTS_1].zzzz


