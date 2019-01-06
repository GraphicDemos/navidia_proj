/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_TwoSidedPolys\
File:  TwoSided.vsh

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
This shader will correctly light a single-surface ribbon object where both sides of any given face are
potentialy visible to the viewer and/or light.

The shader will reverse the direction of each vertex normal depending on the camera position relative
 to the vertex.  In this way, faces where the light is on the opposite side from the camera will not be 
 illuminated, or can use a completely different lighting calculation.
For this example, those faces with the light on the opposite side are lit as though the object were a thin
 scattering material.  The ordinary Lamberian light value is squared for a sharper falloff for these backfaces.
 This squaring is just an arbitrary guess, and a more realistic model could be used in its place.
The result, for this example, is a thin "leaf" object where the effect of a thin membrane is accomplished by
 blending two textures.  One represents light bouncing off a surface (light on same side as camera), and the
 other texture represents light scattering through the surface (light on opposite side).
Values for lighting these textures are output to the  diffuse and specular colors for use in a pixel shader
 or the standard SetTextureStage..() pipeline setup.

-------------------------------------------------------------------------------|--------------------*/

#include "TwoSidedConstants.h"

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_DIFFUSE   v2
#define V_TEXTURE   v3

// convenient names for temporary registers
#define VEC_VERT_TO_EYE		r0
#define VEC_VERT_TO_LIGHT	r1
#define ALIGNED				r2
#define EYE_LOCAL			r3
#define EYE_VECTOR			r5
#define TEMP				r6
#define COR_NORMAL			r7

vs.1.1

dcl_position	V_POSITION
dcl_normal		V_NORMAL
dcl_color		V_DIFFUSE
dcl_texcoord	V_TEXTURE

// Transform position to clip space and output it
dp4 oPos.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

// Use camera position, light position, and vertex position to determine correct orientation of the
// normal vector -- ie flip the normal if the one-sided object's face is pointing the wrong way.
// Camera position was transformed to object space in the shader setup.  Light position was also
// transformed to object space.  This avoids having to transform the normals to world space.

// Make vec from vertex to camera
sub  VEC_VERT_TO_EYE,   c[CV_EYE_POS_OSPACE],   V_POSITION

// Dot the v to eye and the normal to see if they point
//  in the same direction or opposite

dp3 ALIGNED, V_NORMAL, VEC_VERT_TO_EYE

// If aligned is positive, no correction is needed. 
// If aligned is negative, we need to flip the normal
// Do this with tricky SGE logic stuff

sge ALIGNED.x, ALIGNED, c[CV_ZERO]    // if aligned.x >= 0 set x component to one
add ALIGNED.y, c[CV_ONE], -ALIGNED.x  // set y component to logical opposite of x

mul COR_NORMAL,-V_NORMAL, ALIGNED.x
mad COR_NORMAL, V_NORMAL, ALIGNED.y, COR_NORMAL   // corrected normal for one-sided object
 
// Point lighting
// Normalized vector from vertex to the light:

add  VEC_VERT_TO_LIGHT, -c[CV_LIGHT_POS_OSPACE], V_POSITION

dp3 TEMP.w, VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT
rsq TEMP.w, TEMP.w		
mul VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT, TEMP.w

// Dot the vectors for the diffuse component from the point light
// Point light is not attenuated

dp3 r4, VEC_VERT_TO_LIGHT, COR_NORMAL

// Use lit instruction to clamp for negative values of the dp3
//  r5 will have diffuse light value in y component

lit r5, r4

// Add ambient to light value and output to diffuse color

add oD0, r5.y, c[CV_LIGHT_CONST].x  // x holds ambient

// another lit for the light value from opposite normal
// Put it in specular color

lit r5, -r4
mul r5.y, r5.y, r5.y		// square it for more attenuation

mad oD1, r5.y, c[CV_LIGHT_CONST].z, c[CV_LIGHT_CONST].y  ; diffuse * atten + ambient
mov oT0, V_TEXTURE
mov oT1, V_TEXTURE

