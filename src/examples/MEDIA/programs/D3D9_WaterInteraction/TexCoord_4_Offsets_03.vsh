/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  TexCoord_4_Offsets_03.vsh

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

#define V_POS		v0
#define V_TEXCOORD	v1


vs.1.1

dcl_position	V_POS
dcl_texcoord	V_TEXCOORD

// Transform vertex-position to clip-space
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

// Add texture coordinate offsets to the single input
//  vertex texture coordinate.  This will cause
//  a pattern of texels (for example, the nearest 
//  neighbors) to be sampled into the pixel shader
//  texture sample registers t0 through t3.
// If the original texture coordinates are a one-to-one
//  mapping of texels in the source to pixels of the 
//  render target, then these offsets will sample the
//  same pattern of neighboring texels surrounding each
//  pixel rendered.

add oT0, c[ CV_T0_OFFSET ], V_TEXCOORD
add oT1, c[ CV_T1_OFFSET ], V_TEXCOORD
add oT2, c[ CV_T2_OFFSET ], V_TEXCOORD
add oT3, c[ CV_T3_OFFSET ], V_TEXCOORD
