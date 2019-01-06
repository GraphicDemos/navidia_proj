

#include "Constants_StencilShadow.h"

#define V_POSITION  v0
#define V_NORMAL    v1
#define V_DIFFUSE   v2
#define V_TEXTURE   v3

#define VEC_VERT_TO_EYE		r0
#define VEC_VERT_TO_LIGHT	r1
#define ALIGNED				r2
#define EYE_LOCAL			r3
#define EYE_VECTOR			r5
#define TEMP				r6
#define COR_NORMAL			r7

vs.1.1
dcl_position	v0
dcl_normal		v1
dcl_color		v2
dcl_texcoord0	v3

// Transform position to clip space and output it
dp4 oPos.x, V_POSITION, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POSITION, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POSITION, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POSITION, c[CV_WORLDVIEWPROJ_3]

// Generate a fog value:
//  Re-calc z component of position
dp4 r1, V_POSITION, c[CV_WORLDVIEWPROJ_2]

// scale by fog parameters:
//   c[CV_FOGPARAMS].x = fog start
//   c[CV_FOGPARAMS].y = fog end
//   c[CV_FOGPARAMS].z = range

// cameraspace depth (z) - fog start
add r1, r1, -c[CV_FOGPARAMS].x

// 1/range
rcp r1.y, c[CV_FOGPARAMS].z

// 1.0 - (z - fog start) * 1/range
// Because Fog = 1.0 means no fog, and fog = 0.0 means full fog

mad oFog, -r1.x, r1.y, c[CV_ONE].x

// Height-based fog from object space vertex position
mul r1, V_POSITION.x, c[CV_HEIGHT_FOG_PARAMS].x
mul oD1,  r1.x, r1.x

// Normalize vector from vertex to the light:
add  VEC_VERT_TO_LIGHT, -c[CV_LIGHT_POS_OSPACE], V_POSITION
dp3 TEMP.w, VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT
rsq TEMP.w, TEMP.w		
mul VEC_VERT_TO_LIGHT, VEC_VERT_TO_LIGHT, TEMP.w

// N dot L for diffuse component
// Point light is not attenuated

dp3 r4, VEC_VERT_TO_LIGHT, -V_NORMAL

// Use lit instruction to clamp for negative values of the dp3
//  r5 will have diffuse light value in y component

lit r5, r4

// Light color to output diffuse color.  Const.X holds ambient

add r5, r5.y, c[CV_LIGHT_CONST].x

// Do calc for color switch
mov r2, c[CV_COLOR]
mul r1, V_DIFFUSE, c[ CV_COLORSWITCH].x
mad r1, c[CV_COLORSWITCH].y, r2, r1

mul oD0, r5, r1
mov oT0, V_TEXTURE



