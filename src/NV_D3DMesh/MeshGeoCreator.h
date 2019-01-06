/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshGeoCreator.h

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

#ifndef H_D3DMESHGEOCREATOR_H
#define H_D3DMESHGEOCREATOR_H

#include <windows.h>

//--------------------------------------------------------
// Forward decls of things defined elsewhere
class Mesh;
struct D3DXVECTOR3;
struct D3DXVECTOR4;
struct D3DXVECTOR2;
//--------------------------------------------------------
#include "NV_D3DMesh_decl.h"

class DECLSPEC_NV_D3D_MESH_API MeshGeoCreator
{
public:
	//----------------------------------------------------------------------------------
	// Functions to duplicate objects or add more vertices and indices to objects
	HRESULT	InitClone( Mesh * pMesh, const Mesh * pMeshToClone );		// initialize pMesh to a copy of pMeshToClone
	HRESULT InitAddClone( Mesh * pMesh, const Mesh * pMeshToClone );	// add mesh verts and tris to existing mesh

	HRESULT InitArray( Mesh * pMesh, const Mesh * pInputMesh, int nNumInstances, D3DXMATRIX & matEachInstance );

	//----------------------------------------------------------------------------------
	// Functions to create geometry
	HRESULT InitSphere( Mesh * pMesh, float radius, UINT num_latitude_lines, UINT num_longitude_lines );
	HRESULT InitSphere( Mesh * pMesh, float radius, UINT num_latitude_lines, UINT num_longitude_lines,
						const D3DXVECTOR3 & axis1, 
						const D3DXVECTOR3 & axis2, 
						const D3DXVECTOR3 & axis3	);

	HRESULT InitSphereFromBox( Mesh * pMesh, float radius,
								const D3DXVECTOR3 & axis1, UINT num_axis1_subdiv,
								const D3DXVECTOR3 & axis2, UINT num_axis2_subdiv,
								const D3DXVECTOR3 & axis3, UINT num_axis3_subdiv  );

	HRESULT InitTesselatedPlane( Mesh * pMesh, 
									const D3DXVECTOR3 & base_point, const D3DXVECTOR2 & base_uv,
									const D3DXVECTOR3 & axis1_point, const D3DXVECTOR2 & axis1_uv,
									const D3DXVECTOR3 & axis2_point, const D3DXVECTOR2 & axis2_uv,
									UINT n_subdiv_axis1, UINT n_subdiv_axis2 );

	HRESULT	InitTesselatedPlane( Mesh * pMesh, 
									const D3DXVECTOR3 & width_direction, float width, UINT n_subdiv_width,
									const D3DXVECTOR3 & height_direction, float height, UINT n_subdiv_height );

	HRESULT InitTesselatedBox( Mesh * pMesh, const D3DXVECTOR3 & base_point,
								const D3DXVECTOR3 & axis1, UINT num_axis1_subdiv,
								const D3DXVECTOR3 & axis2, UINT num_axis2_subdiv,
								const D3DXVECTOR3 & axis3, UINT num_axis3_subdiv );

	HRESULT InitBlock( Mesh * out_pMesh, const D3DXVECTOR3 & min_corner, const D3DXVECTOR3 & max_corner );
	HRESULT InitBlockCS( Mesh * out_pMesh, const D3DXVECTOR3 & center, const D3DXVECTOR3 & size );
	HRESULT InitTesselatedBlockCS( Mesh * out_pMesh, const D3DXVECTOR3 & center, const D3DXVECTOR3 & size,
									int n_x_subdiv, int n_y_subdiv, int n_z_subdiv );

	HRESULT InitSpiral( Mesh * pMesh, 
						const D3DXVECTOR3 & spiral_axis, float axis_length, int n_subdiv_length,
						const D3DXVECTOR3 & base_axis,   float base_length, int n_subdiv_base,
						float n_twists );

	// A cylinder where the center of each end cap are at the two end_cap points
	// The two end cap points define the cylinder's axis.
	// If bSmoothCorners is false, then two vertices will be created for each point
	//  on each end's corners.  These vertices will have separate normals so that 
	//  shading is not smooth across the corner.
	HRESULT InitCylinder( Mesh * pMesh, const D3DXVECTOR3 & end_cap1_center,
							const D3DXVECTOR3 & end_cap2_center,
							float radius, UINT num_sides,
							UINT num_cap1_subdiv, UINT num_length_subdiv,
							UINT num_cap2_subdiv,
							bool bSmoothCorners = false );

	// Creates a torus with seams of overlapping vertices so that cylindrical
	//  texture coordinate wrapping is not needed.
	// radius is the radius along the center line of the torus cross section
	HRESULT InitTorus( Mesh * pMesh, const D3DXVECTOR3 & axis, float radius, UINT num_sides, 
						float cross_section_radius, UINT num_cross_section_sides );

	//----------------------------------------------------------------------------------
	// Objects that are not closed hulls.
	HRESULT InitTriangle( Mesh * pMesh, D3DXVECTOR3 & pt1, D3DXVECTOR3 & pt2, D3DXVECTOR3 & pt3 );
	HRESULT InitDisc( Mesh * pMesh, D3DXVECTOR3 & center, D3DXVECTOR3 & perpendicular,
						float fRadius, int num_segments );

	//----------------------------------------------------------------------------------
	// Various test objects
	HRESULT InitTestObj_ColorBox( Mesh * pMesh );		// box from (-1,-1,-1) to (1,1,1)

	// helper functions for common tasks
	HRESULT	MakeTris_RegularGrid( Mesh * pMesh, UINT nwidth_verts, UINT nheight_verts );
	// Creates orthogonal basis given one input vector, pInputAxis
	// pOutVec1 CROSS pOutVec2 = pInputAxis
	// input does not have to be normalized
	HRESULT	MakeOrthogonalBasis( const D3DXVECTOR3 * pInputAxis, D3DXVECTOR3 * pOutVec1,
									D3DXVECTOR3 * pOutVec2 );
	HRESULT TransformMesh( Mesh * pMesh, const D3DXVECTOR3 & x_param_axis,
						   const D3DXVECTOR3 & y_param_axis, const D3DXVECTOR3 & z_param_axis );

public:
	MeshGeoCreator()	{};
	~MeshGeoCreator()	{};
};


#endif
