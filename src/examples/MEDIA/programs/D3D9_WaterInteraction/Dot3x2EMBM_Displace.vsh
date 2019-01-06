/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  Dot3x2EMBM_Displace.vsh

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

This vertex shader establishes per-vertex data for computing a more flexible
version of DX6-style Environment Mapped Bump Mapping (EMBM).  In this case
the 2x2 EMBM rotation matrix may be specified per-vertex and is written to
vertex texture coordinates by this shader.  Using the DX8 API SetTextureStageState()
call D3DTSS_BUMPENVMAT00 this matrix may only be specified per rendering call,
so would be the same for all vertices.

Rather than use that 'fixed function' method, this shader writes texture
coordinate data to two vertex texture coordinates.  A pixel shader may then
use the interpolated texture coordinates and a displacement map (similar to
a normal map, but with a constant 1.0 in the blue channel) to achieve EMBM-like
displacements from a base texture coordinate.

The base texture coordinate is written to the texture coordinate .Z values, with
the base U coordinate written to the first texture coordinate .Z and the base 
V coordinate written to the second texture coordinate .Z value.
The 2x2 EMBM matrix is written to the .X and .Y values of the two texture 
coordinates. 

In this way, the magnitude and direction of the displacements can vary with
distance from the viewer, or any other vertex shader computation.

The pixel shader should use the texm3x2tex operation (consuming one more
instruction slot than the texbem instruction) to perform the 2x2 displacement
map rotation and add this displacement to the base coordinate.  It does this
as follows:

t0:  displacement map texture coords
t1:  m1, m2, base_u
t2:  m3, m4, base_v
displacement map:  red = du;  green = dv; blue = 1.0

tex t0				// fetch displacement map value
texm3x2pad  t1, t0	// t1 coord DOT t0(RGB) for U coordinate of environment look-up
texm3x2tex  t2, t0  // t2 coord DOT t0(RGB) for V coordinate of environment look-up
so texture t2 - the environment map -- is accessed at (u,v) where:

u = du * m1  + dv * m2  + base_u * 1.0
v = du * m3  + dv * m4  + base_v * 1.0
See how the 2x2 rotation is computed in the first two multiplies of the dot-products.

In practice, the displacement map may contain biased-scaled values (as in a normal map)
so the actual pixel shader instructions are:

tex t0
texm3x2pad  t1,  t0_bx2
texm3x2tex  t2,  t0_bx2

t2 then contains the bump mapped reflection.

For questions, please contact devsupport@nvidia.com or Greg James, gjames@nvidia.com

*** this one reduces magnitude of 'basis' vectors with distance, in order to
  shrink the texture displacements (size of waves) off in the distance.
  Could also use mip-mapping of height map and displacement map to do this, but
  that would require many extra render-to-texture operations.

*** This shader was put together quickly, so there is probably good room for
  optimization.

This shader has about 30 instructions
Shader requires:
c[CV_CONSTS_1].xyzw  =  0.0,  0.5,  1.0, 2.0
c[CV_CONSTS_2].xyzw  =  0.0,  0.5, -1.0, 1.0

-------------------------------------------------------------------------------|--------------------*/

#include "WaterInteractionConstants.h"

// name the required vertex inputs
// This format is compat with the GeometryVB_Dot3 vertex type
//  which is the Dot3Vertex type, though for this shader the
//  texture space basis vectors are not used.

#define V_POSITION    v0
#define V_NORMAL      v1
#define V_DIFFUSE     v2
#define V_TEXTURE     v3

// Name the output texture coordinates
// Re-map them if you want to use the texm3x2 in a different pixel shader
//   instruction slot
#define O_TEX_DISPLACEMENT_MAP	oT0
#define O_TEX_MAT_AND_BASEU		oT2
#define O_TEX_MAT_AND_BASEV		oT3

// Name temporary registers
#define WORLD_VERTEX	r0
#define EYE_XY			r3
#define EYE_VECTOR      r4
#define TEMP            r5
#define BASIS_1         r7
#define BASIS_2         r8
#define BSC				r10

vs.1.1
dcl_position	V_POSITION
dcl_color		V_DIFFUSE
dcl_texcoord	V_TEXTURE
dcl_normal		V_NORMAL

// Transform position to clip space and output it
dp4 oPos.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

// V_POSITION is input in world space
// ** This is unconventional, so you may want
//    to add an object-to-world space transform
// 
// The CV_EYE_OBJSPC location is the eye point
//   in world space translated to compensate for 
//   translation of the water object being currently
//   rendered.  Effectively, it is the eye position in 
//   object space, though a full world-to-object space
//   transform is not performed.  This is a simplification
//   possible because the water plane does not rotate or move.

// Compute vector from vertex to eye
add EYE_VECTOR, -V_POSITION, c[CV_EYE_OBJSPC]

// Now we want to create a normalized vector on the
//  water plane that points from the eye to the vertex.
//  This vector will help to establish the 2x2 EMBM 
//  rotation matrix at each vertex.
//  This shader assumes the water is in the xy plane,
//  so to get this vector we set the .z component to
//  zero.
//  Values of 1.0 and 0.0 are stored in CV_CONSTS_1
//  so c[CV_CONSTS_1].zzxx = (1,1,0,0)

// Make vector that is:  (EYE_VECTOR.x, EYE_VECTOR.y, 0, 0)

mul EYE_XY,  EYE_VECTOR, c[CV_CONSTS_1].zzxx

// Normalize the (x,y,0) vector so we get a direction
//  in x,y plane to the vertex

dp3		EYE_XY.w,	EYE_XY, EYE_XY           // 3 instruction normalization
rsq		EYE_XY.w,	EYE_XY.w
mul		EYE_XY.xyz, EYE_XY, EYE_XY.w

// Compute vector orthogonal to the x,y eye vector 
//  in the x,y plane
// Cross product between EYE_XY and 0,0,1 to 
//  find the orthogonal vector.
// Since EYE_XY = (x,y,0) the cross product reduces
//  to:   ( EYE_XY.y, -EYE_XY.x, 0 )
//  
// We can do this easily with the constant c[CV_CONSTS_2].xyzw  =  0.0, 0.5, -1.0, 1.0
//
// Also, write 0.5 to the basis z component.
//
// The z coord is the un-perturbed texture coord for lookup
//   into the sky texture map (2D).  This coord will
//   be perturbed per-pixel in 2D to arrive at the final
//   texture coordinate for accessing the sky texture.
// Setting this to 0.5 says 'start at center of texture'
//   A vector will be added to it later on in this vertex shader
//   to move the coordinate out from the center.
//
// We want to end up with the following in the basis vectors:
//   .x and .y form two elements of the 2x2 EMBM style rotation matrix
//   Pixel shader will do dot products between these texture 
//   coords and a red-green du, dv displacement map (values not in 
//   two's-complement as they must be for EMBM) so as to rotate
//   the displacements.
//
// BASIS_1.x =  EYE_XY.x
// BASIS_1.y =  EYE_XY.y
// BASIS_1.z =  0.5
// 
// BASIS_2.x =  EYE_XY.y
// BASIS_2.y = -EYE_XY.x
// BASIS_2.z =  0.5

//		BASIS_1 = 	( x,y,0,0 )	 * 	( 1, 1,0,0 )	  +  (0, 0, 0.5, 0)
mad		BASIS_1,	EYE_XY,		c[CV_CONSTS_2].wwxx,	c[CV_CONSTS_2].xxyx

//	 BASIS_2 = 	( y,x,0,0 )	 * 	( 1,-1,0,0 )	  +  (0, 0, 0.5, 0)
mad  BASIS_2,	EYE_XY.yxzz, c[CV_CONSTS_2].wzxx,	c[CV_CONSTS_2].xxyx

// Normalize 3D vertex-to-eye vector
// This will be used to determine the base texture coordinate
//  into the environment map by computing a reflection.
// The reflection is off the xy water plane bouncing up into
//  a sky dome.  The sky dome is rendered to the 2D texture which
//  is a very wide-angle view of the sky dome.  The top of the sky
//  dome is in the center of the 2D texture, and the horizon lies in 
//  a circle around the edge of the 2D environment texture.

dp3		EYE_VECTOR.w,	EYE_VECTOR,		EYE_VECTOR
rsq		EYE_VECTOR.w,	EYE_VECTOR.w
mul		EYE_VECTOR.xyz,	EYE_VECTOR,		EYE_VECTOR.w

// make sure eye3.z is positive.  
// This is so things don't go bad when viewed from underneath

max  EYE_VECTOR.z,  EYE_VECTOR.z, -EYE_VECTOR.z

// Compute a term for attenuation based on normalized eyevector Z.
// As distance from vertex-to-eye increases, the z component
//  decreases because the normalized vector has a greater xy
//  xy component.
// We can use this decreasing z with distance to attenuate
//  the scale of the bump map displacements with distance.
//
// 1 - EYE_VECTOR.z

add  TEMP, c[CV_ONE], -EYE_VECTOR.zzzz

// Change scale of displacements
// This is arbitrary and numbers/amounts selected to just 
//   look good.
// TEMP controls magnitude of displacements away from the center

mul BSC, TEMP, TEMP
mul BSC, BSC, BSC
mul BSC, BSC, c[CV_CONSTS_1].w			// mult by two

// reduce 'attenuation' of displacement vectors
//  1.0 = displacement goes to zero,
//  0.7 = displacement goes to minimum of 0.3 
//  0.4 = displacement goes to minimum of 0.6

mul BSC, BSC, c[CV_DISPSCALE]
add BSC, c[CV_ONE], -BSC

// get rid of negative values - clamp negatives to 0

sge BSC.w, BSC.x, c[CV_CONSTS_1].x
mul BSC, BSC, BSC.w

// Scale the xy rotation matrix by the BSC scale factor
// Leave .z = 0.5 value untouched.

mul BASIS_1.xy, BASIS_1, BSC
mul BASIS_2.xy, BASIS_2, BSC

mul TEMP, TEMP, TEMP
mul TEMP, TEMP, c[CV_CONSTS_1].y

//mov oD0, TEMP			// Option for diagnosis -- 
						// display the result as color
// Determine base texture coord.
// Write this to z.
// This is the coordinate at which the sample will be taken
//  if the displacement map has a displacement vector of zero.
// These .z components are used in the pixel shader texm3x2tex
//  operation where they are multiplied by a value of 1.0 blue
//  to yield a texture coordinate value for each axis (u and v)
//  The .xy values of the texture coords are the DX6-style EMBM
//  matrix written per-vertex, and serve to rotate the du, dv
//  displacement map vector.  The texm3x2 operation performs the
//  2x2 rotation in x,y dot red,green and adds this displacement
//  to the .z * blue base coordinate.
//
// BASIS_1.z BASIS_2.z are  0.5  -- ie starting from the center
// The EYE_XY determines direction to move our from the center
//  TEMP determines length to move out from center.
// The result is the output texture coordinates u,v for an 
//  approximated reflection off a plane into the wide-angle 
//  rendering of the sky dome, as though the eye-to-vertex vector
//  reflected off a flat water surface and into the sky dome.

mad  O_TEX_MAT_AND_BASEU.z,  TEMP, EYE_XY.x, BASIS_1.z
mad  O_TEX_MAT_AND_BASEV.z,  TEMP, EYE_XY.y, BASIS_2.z

// Scale down the x,y displacement multipliers
// This scales down the eventual pixel-shader computed
//  displacements from the base texture coord.

mul O_TEX_MAT_AND_BASEU.xy, BASIS_1, c[CV_OFFSETSCALE]
mul O_TEX_MAT_AND_BASEV.xy, BASIS_2, c[CV_OFFSETSCALE]

//  Now eye vector for dot3 quasi-Fresnel term
//  Bias and scale the normalized 3D eye vector from the
//   [-1,1] range to the [0,1] range
//  This is used in a pixel shader dot-product with the
//   displacement map to vary transparency with angle
//   of incidence to the bump mapped surface.

mad  oD0, EYE_VECTOR, c[CV_CONSTS_1].y, c[CV_CONSTS_1].y

// Texture coordinate for accessing the displacement map
// This is just a simple mapping so that adjacent quads of
//   the water plane will tile the texture across the
//   water.

mov O_TEX_DISPLACEMENT_MAP.xy, V_TEXTURE

