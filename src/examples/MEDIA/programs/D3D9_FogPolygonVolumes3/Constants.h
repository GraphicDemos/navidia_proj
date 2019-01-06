/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_FogPolygonVolumes3\
File:  Constants.h

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

// Texture stage indices for RGB-encoding ramp textures.
// These are used in the MRT ps.3.0 path.
// You may want to move these to different texture slots depending on how many textures you
//   need for the ordinary diffuse color rendering.
#define TEX_RG		4
#define TEX_RGSAMP	s4
#define TEX_RGCRDO	oT4
#define TEX_RGCRDI	t4
#define TEX_RGDECL	dcl  t4.xy  dcl_2d s4
#define TEX_RGCRDI3 v4
#define TEX_RGDECL3 dcl_texcoord4  v4.xy  dcl_2d s4

#define TEX_B		5
#define TEX_BSAMP	s5
#define TEX_BCRDO	oT5
#define TEX_BCRDI	t5
#define TEX_BDECL	dcl  t5.xy  dcl_2d s5
#define TEX_BCRDI3	v5
#define TEX_BDECL3  dcl_texcoord5  v5.xy  dcl_2d s5

//===========================================
#define CP_RGB_TEXADDR_WEIGHTS	 4
#define CPN_RGB_TEXADDR_WEIGHTS	 c4

#define	CV_CONSTS_1				 0

#define CV_WORLDVIEWPROJ_0		 2
#define CV_WORLDVIEWPROJ_1		 3
#define CV_WORLDVIEWPROJ_2		 4
#define CV_WORLDVIEWPROJ_3		 5
#define CV_OBJ_AMBIENT_COL		20
#define CV_LIGHT_POS_OSPACE     21
#define CV_NDOTLTHRESH			25
#define CV_EXTRUSION_DIST		26
#define CV_VERTEX_INSET_SCALE	27
#define CV_BITREDUCE			30
#define CV_RAMPSCALE			31
#define	CV_RGB_TEXADDR_WEIGHTS	32
#define CV_DITHER_CONTROL		33
#define CV_NORMALIZEWDEPTH		34
#define CV_DIRLIGHT1_DIR		40
#define CV_DIRLIGHT1_COLOR		41
#define	CV_HALF_TEXEL_SIZE		50

