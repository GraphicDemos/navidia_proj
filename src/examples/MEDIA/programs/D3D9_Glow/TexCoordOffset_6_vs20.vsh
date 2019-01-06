/*********************************************************************NVMH4****
Path:  SDK\MEDIA\programs\D3D9_Glow
File:  TexCoord_6Offset.vsh

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:



******************************************************************************/

#include "Constants.h"

vs.2.0

dcl_position	v0
dcl_texcoord	v3


// output the vertex position in screen space
mov oPos, v0

// Offset the texture coords according to constant values
add oT0,  v3, c[CV_TEXCOORD_OFFSET_0]
add oT1,  v3, c[CV_TEXCOORD_OFFSET_1]
add oT2,  v3, c[CV_TEXCOORD_OFFSET_2]
add oT3,  v3, c[CV_TEXCOORD_OFFSET_3]
add oT4,  v3, c[CV_TEXCOORD_OFFSET_4]
add oT5,  v3, c[CV_TEXCOORD_OFFSET_5]
