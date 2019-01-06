/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  DepthToTexcrdForRGB.nvv

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Requires:
CV_CONSTS_1 = 0.0, 0.5, 1.0, 2.0


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
dp4 r1.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 r1.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 r1.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 r1.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

mov oPos, r1


//////////////////////////////////////////////
// Generate a fog value:
// Fill fog with 1.0 for no fog
// D3D was oFog.x, D3D9 is oFog

mov oFog, c[CV_CONSTS_1].z

//////////////////////////////////////////////
// Generate texture coordinates according to 
//  z depth of the object (before w divide, so
//  z intervals are regular & linear).
//
// Texture coordinates address ramp textures with
//  1st coordinate addressing 2D ramp for red & green
//  and 2nd coordinate addressing 1D ramp for blue.
// This encodes depth as an RGB value.
//
// red =	tex coord 0 .x = high bits
// green =	tex coord 0 .y = mid bits
// blue =	tex coord 1    = low bits


// Create a 'normalized' value from the w depth so that it lies
//  in the range [0,1] from the near to the far clip plane.
// r6.w = 1

mad r6, r1.w, c[CV_NORMALIZEWDEPTH].xxxz, c[CV_NORMALIZEWDEPTH].yyyw

mul oT0.x, r6, c[CV_RAMPSCALE].xxxx

// Green forms the middle precision bits of the depth value
// For green, multiply depth value by a scale factor which
//  affects how many green bands of color will repeat per
//  red band.
// For example, if red and green values from 0 to 32 are used
//  in each color ramp, then the green ramp should repeat 32
//  times as much as the red ramp, which is one full green ramp
//  per increment of red color.

mul oT0.y, r6, c[CV_RAMPSCALE].yyyy


// The same is done for the last texture coordinate, which will
//  be used to generate the blue ramps.  Blue should complete
//  one full ramp for every increment of green.

mul r6.xyz, r6, c[CV_RAMPSCALE].zzzz

// Calculate y coordinate based on dither control constant
//  to enable or disable dithering.
// The y=0 line of the blue texture has no dithering, so setting y
//  coordinate to zero will disable the dithering

dp3 r6.y, r1, c[CV_DITHER_CONTROL]	// set CV_DITHER_CONTROL to 0,0,0,0 for no dither

mov oT1, r6

//-------------------------------------
// diagnostics
//mov oD0, r1.w
//mov oD0, V_NORMAL
//mov oT0, c[CV_NORMALIZEWDEPTH]
