/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  RGBEncodeAndCompare.vsh

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Vertex shader for the RGBEncodeAndCompare_30.psh


Requires:
CV_CONSTS_1 = 0.0, 0.5, 1.0, 2.0

-------------------------------------------------------------------------------|--------------------*/

#include "Constants.h"

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_TEXTURE   v2

#define TEMP		r1

vs.3.0
dcl_position	V_POSITION
dcl_normal		V_NORMAL
dcl_texcoord	V_TEXTURE
dcl_position	o0
dcl_fog			o1
dcl_texcoord0	o2
dcl_texcoord4   o3
dcl_texcoord5   o4

// Transform position to clip space and output it
dp4 r1.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 r1.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 r1.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 r1.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

mov o0, r1

//--------------------------------------
// no fog
// D3D8 was oFog.x, D3D9 is oFog
mov o1, c[CV_CONSTS_1].z

//--------------------------------------------
// Compute texture coordinates for encoding pixel depth as an A8R8G8B8 value
// The generated texture coordinates are meant to address color ramp textures with
//  t1 coordinate addressing 2D ramp for red & green
//  t2 coordinate addressing 1D ramp for blue.
// red =	tex coord A .x = high bits of high precision depth
// green =	tex coord A .y = mid bits of high precision depth
// blue =	tex coord B    = low bits of high precision depth
//
// Create a 'normalized' value from the w depth so that it lies
//  in the range [0,1] from the near to the far clip plane.
// Also set r6.w = 1
mad r6, r1.w, c[CV_NORMALIZEWDEPTH].xxxz, c[CV_NORMALIZEWDEPTH].yyyw
mul r3, r6, c[CV_RAMPSCALE].xy
mov o3.xy, r3						// output texture coord for red-green color ramps

// Generate texture coordinate for the blue ramp to encode the lowest bits
// Blue should complete one full ramp for every bit increment of green.
mul r6.xyz, r6, c[CV_RAMPSCALE].zzzz

// Calculate y coordinate based on dither control constant to enable or disable dithering.
// The y=0 line of the blue texture has no dithering, so setting y coordinate to zero will disable the dithering

dp3 r6.y, r1, c[CV_DITHER_CONTROL]	// set CV_DITHER_CONTROL to 0,0,0,0 for no dither
mov o4, r6					// output texture coord for blue color ramp
//---------------------------------------------------
// Texture coords for projecting texture to exactly 
//  cover the screen.  Converts geometry screen
//  position to a texture coordinate from [0,1]
// Uses half-texel width and height offset to sample
//  from texel center, not upper-left corner of texels
//  OpenGL does not require this half-texel offset.
//
// Turn screen space position into texture coordinate
// Bias position from [-1,1] to [0,1] range by
//   multiplying x,y by 0.5*w and add 0.5*w
//   This is equivalent to (x+1)/2, (y+1)/2, etc.

// Multiply by 0.5
mul r1.xy, r1.xy, c[CV_CONSTS_1].yyyy

// Add w/2 to x,y to shift from (x/w,y/w) in the 
//  range [-1/2,1/2] to (x/w,y/w) in the range [0,1]
mad r1.xy, r1.wwww, c[CV_CONSTS_1].yyyy, r1.xy

// Invert y coordinate by setting y = 1-y
// But remeber, w!=1 so 1.0 really equals 1*w
// y = 1*w - y
add r1.y, r1.w, -r1.y

// Add half-texel offset to sample from texel centers, 
//  not texel corners
// Multiply by w because w != 1
mad r1.xy, r1.wwww, c[CV_HALF_TEXEL_SIZE], r1.xy

// Output the texture coordinate for mapping the screen-aligned texture
// You must use projective texturing when reading from the texture using this
// coordinate.  
// So, use either:   ps.2.0 texldp -- ie.  texldp  r0, t0, s0
// or:	m_pD3DDev->SetTextureStageState( 3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED | D3DTTFF_COUNT4 );
// Note:  The SetTextureStageState is only required if using fixed-function pixel shading.  
//        It is not required if using pixel shaders 1.1 or 1.3
mov o2, r1
