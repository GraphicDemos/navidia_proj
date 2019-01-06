/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_Membrane\
File:  Membrane_vs11.vsh

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

#include "MembraneConstants.h"

#define V_POS		v0
#define V_NRM		v1
#define V_TEX		v2
#define V_COL		v3

vs.1.1

dcl_position	V_POS
dcl_normal		V_NRM
dcl_texcoord	V_TEX

// transform position
dp4 oPos.x, V_POS, c[MAT_WVP_0]
dp4 oPos.y, V_POS, c[MAT_WVP_1]
dp4 oPos.z, V_POS, c[MAT_WVP_2]
dp4 oPos.w, V_POS, c[MAT_WVP_3]

// vector eye position (in object space) to the vertex
add r0, c[EYE_POS_OBJ_SPACE], -V_POS
// normalize it
dp3 r0.w, r0, r0
rsq r0.w, r0.w
mul r0, r0, r0.w

// (normalized vector from vertex to eye) DOT (vertex normal)
// to compute the view angle to the mesh at the vertex
dp3 r1, r0, V_NRM
// absolute value to map -1 back to 1
max r1, r1, -r1

// Add an optional offset held in c20.  This can be used to animate the result
add oT0.x, r1, c[ TXCRD_OFFSET_0 ]

slt oT1, r1, r1			// set oT1 to zero
