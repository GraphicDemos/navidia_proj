/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  DiffuseAndRGBDepthEncode.vsh

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

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_TEXTURE   v2

#define TEMP		r2

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

// Set fog to 1.0 for no fog
mov oFog, c[CV_CONSTS_1].z

//--------------------------------------------
// Directional light + ambient
	// negative light vector for vector from vertex to light 
dp3		TEMP, -c[CV_DIRLIGHT1_DIR], V_NORMAL
	// clamp <0 to 0
max		TEMP, TEMP, c[CV_CONSTS_1].xxxx
	// mult by light color
mul		TEMP, TEMP, c[CV_DIRLIGHT1_COLOR]
	// Add ambient
add		oD0, TEMP, c[CV_OBJ_AMBIENT_COL]
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
mov TEX_RGCRDO.xy, r3				// output texture coord for red-green color ramps

// Generate texture coordinate for the blue ramp to encode the lowest bits
// Blue should complete one full ramp for every bit increment of green.
mul r6.xyz, r6, c[CV_RAMPSCALE].zzzz

// Calculate y coordinate based on dither control constant to enable or disable dithering.
// The y=0 line of the blue texture has no dithering, so setting y coordinate to zero will disable the dithering

dp3 r6.y, r1, c[CV_DITHER_CONTROL]	// set CV_DITHER_CONTROL to 0,0,0,0 for no dither
mov TEX_BCRDO, r6					// output texture coord for blue color ramp

//-------------------------------------------
//  Pass through the ordinary texture coords
mov oT0, V_TEXTURE
mov oT1, V_TEXTURE
