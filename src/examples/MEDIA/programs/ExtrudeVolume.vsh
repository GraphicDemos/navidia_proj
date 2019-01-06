
#include "Constants_StencilShadow.h"

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_DIFFUSE   v2
#define V_TEXTURE   v3


#define VEC_VERT_TO_EYE      r0
#define VEC_VERT_TO_LIGHT    r1

#define ALIGNED       r2

#define EYE_LOCAL     r3
#define EYE_VECTOR    r5

#define TEMP          r6
#define COR_NORMAL    r7


vs.1.1
dcl_position	v0
dcl_normal		v1
dcl_color		v2
dcl_texcoord0	v3

// no fogging
// fog = 1.0 = no fog
mov oFog, c[CV_ONE].x
mov oD1, c[CV_ZERO].xyz

//----------------------------------------------------------
// Normalize vector from vertex to the light:
add  VEC_VERT_TO_LIGHT, -c[CV_LIGHT_POS_OSPACE], V_POSITION
dp3 TEMP.w, VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT
rsq TEMP.w, TEMP.w		
mul VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT, TEMP.w

// N dot L for diffuse component
// Point light is not attenuated
dp3 r4, VEC_VERT_TO_LIGHT, -V_NORMAL

//---------------------------------------------------------
// Inset the position along the normal vector direction
// This moves the shadow volume points inside the model
//  slightly to minimize poping of shadowed areas as
//  each facet comes in and out of shadow.

mul r7, V_NORMAL, c[CV_FATNESS_SCALE]
add r7, V_POSITION, r7
mov r7.w, V_POSITION.w

//---------------------------------------------------------
// vector from light to vertex

mul r3, VEC_VERT_TO_LIGHT, c[CV_SHDVOL_DIST]

// If r4 < 0 then vertex faces away from light, so 
//  move it along the direction to the light to extrude the
//  shadow volume.

slt  r5, r4,  c[CV_ZERO].z			// r5 = N dot L  <   0, this is when we fatten

//---------------------------------------------------------
// Inset only front facing points
//mul r7,   V_NORMAL, c[CV_FATNESS_SCALE]
//sge r6,   r4,  c[CV_ZERO].z
//mad r7,   r6, r7,   V_POSITION
//mov r7.w, V_POSITION.w

//---------------------------------------------------------
// extrude back-facing shadow volume points 

mad r2, r3, r5, r7				// r7 ~ V_POSITION

// transform to hclip space
dp4 oPos.x, r2, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, r2, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, r2, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, r2, c[CV_WORLDVIEWPROJ_3]

// Use lit instruction to clamp for negative values of the dp3
//  r5 will have diffuse light value in y component

lit r6, r4

// Light color to output diffuse color.  Const.X holds ambient
//add oD0,		r6.y, c[CV_LIGHT_CONST].x
mov oD0.xyzw,	c[CV_FACTORS].x
mov oD0.xy,		c[CV_ZERO].xy

mov oT0, V_TEXTURE



