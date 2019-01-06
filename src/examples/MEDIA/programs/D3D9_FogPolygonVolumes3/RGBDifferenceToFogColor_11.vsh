/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  RGBDifferenceToFogColor_11.nvv

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

#include "Constants.h"

#define V_POS		v0
#define V_TEX		v2


vs.1.1

dcl_position	v0
dcl_normal		v1
dcl_texcoord	v2
dcl_color		v3


// output position in HCLIP space.  no matrix transform
mov oPos, v0


// Output texture coords
// Ordinary texture coords to stage 0 and stage 3
//	These range from (0,0) to (1,1) and map the fog calculation
//  textures to screen pixels
//
// Special texture coords to stage 1 for use in dot product addr op
//	These decode the RGB-encoded depth to a floating point texture 
//	coordinate.

mov		oT0, V_TEX
mov		oT1, c[CV_RGB_TEXADDR_WEIGHTS]

mov		oT2, c[CV_CONSTS_1].xxxx		// zero
mov		oT3, V_TEX


