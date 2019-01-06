/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  ViewVector.cpp

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

#include "dxstdafx.h"
#include <shared/NV_Error.h>

// Given a viewport coordinate, view, and projection matrices, return a vector
//  in world space from the eye to the viewport pixel.  You can use this to 
//  turn a mouse click coordinate into a 3D vector pointing at whatever the
//  user clicked on.
// out_direction	is output direction vector, not normalized
// out_origin		is where out_direction vector originates (camera position)
// vp_x				viewport x coord in [-1,1] range (center of viewport is (0,0))
// vp_y				viewport y coord  "" 
// matView			standard scene view matrix
// matProj			standard scene projection matrix
void	GetRayFromViewportCoord( D3DXVECTOR3 * out_direction, D3DXVECTOR3 * out_origin,
								 float vp_x, float vp_y,
								 const D3DXMATRIX * pmatView,
								 const D3DXMATRIX * pmatProj )
{
	if( pmatView == NULL || pmatProj == NULL || out_direction == NULL || out_origin == NULL )
	{
		FDebug("matrix pointer is null!\n");
		assert( false );
		return;
	}
	D3DXMATRIX		matViewProj;
	D3DXMATRIX		matINV_ViewProj;
	D3DXMatrixMultiply( &matViewProj,   pmatView, pmatProj );
	D3DXMatrixInverse( &matINV_ViewProj, NULL,  & matViewProj );
	// Give mouse location a depth of 0.0f
	D3DXVECTOR3 mouse_pt( vp_x, vp_y, 0.0f );
	// resulting point is in world space
	D3DXVec3TransformCoord( & mouse_pt,  & mouse_pt, & matINV_ViewProj );
	// extract camera position from view matrix
	D3DXVECTOR4	cam_pos( 0.0f, 0.0f, 0.0f, 1.0f );
	D3DXMATRIX	viewrot;	
	D3DXMatrixInverse( &viewrot, NULL, pmatView );
	D3DXVec4Transform( &cam_pos, &cam_pos, &viewrot );
	// homogenize
	cam_pos.x = cam_pos.x / cam_pos.w;
	cam_pos.y = cam_pos.y / cam_pos.w;
	cam_pos.z = cam_pos.z / cam_pos.w;
	cam_pos.w = cam_pos.w / cam_pos.w;
	out_direction->x	= mouse_pt.x - cam_pos.x;
	out_direction->y	= mouse_pt.y - cam_pos.y;
	out_direction->z	= mouse_pt.z - cam_pos.z;
	out_origin->x		= cam_pos.x;
	out_origin->y		= cam_pos.y;
	out_origin->z		= cam_pos.z;
}
