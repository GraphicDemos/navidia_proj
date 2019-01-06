/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshSectionStitcher.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A class to stitch together cross sections of vertices to make a mesh.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_MESHSECTIONSTITCHER_H
#define H_MESHSECTIONSTITCHER_H

#include <vector>
using namespace std;
#include "NV_D3DMesh\MeshVertex.h"
#include "NV_D3DMesh_decl.h"

class Mesh;
typedef vector< D3DXVECTOR4 >	CrossSection;
typedef vector< MeshVertex >	V_MeshVertex;


class DECLSPEC_NV_D3D_MESH_API MeshSectionStitcher
{
public:
	HRESULT InitExtrusion( Mesh * pMesh, 
						    D3DXVECTOR3 * pPos, D3DXVECTOR3 * pNrm, D3DCOLOR * pCol,
							D3DXVECTOR2 * pTexCoord,
							UINT num_verts_in_each_cross_section,
							UINT num_cross_sections,
							bool stitch_cross_sections_closed,
							bool stitch_first_cross_section_to_last );

	HRESULT InitExtrusion( Mesh * pMesh,
						    MeshVertex * pVertices, UINT num_vertices,
							UINT num_verts_in_each_cross_section,
							UINT num_cross_sections,
							bool stitch_cross_sections_closed,
							bool stitch_first_cross_section_to_last );

	HRESULT InitLathedObject( Mesh * pMesh,
							  MeshVertex * pInputVertices,  // one single cross section to sweep
							  UINT num_verts_in_cross_section,
							  D3DXVECTOR3 & lathe_axis,
							  float lathe_angle_start,	// in degrees
							  float lathe_angle_end,
							  UINT num_cross_sections,
							  bool close_cross_section_ends,
							  bool stitch_last_cross_section_to_first,
							  bool generate_texcoordx_on_input_cross_section );


	// Helper functions ---------------------------------------------------

	HRESULT AddToCrossSection( V_MeshVertex * pOut, const V_MeshVertex * pInputSections,
								UINT nvec_in_one_section, UINT n_sections,
								bool stitch_section_end,
								UINT subdiv_cross_section );

	HRESULT AddCrossSections( V_MeshVertex * pOut, const V_MeshVertex * pInputSections,
								UINT nvec_in_section, UINT n_sections,
								bool stitch_extrusion_ends, UINT add_cross_sections );

	// Interpolate 2D texcoord from start_coord to end_coord along
	//  the input pInputVertices array, writing to the vertices in 
	//  the array
	HRESULT InterpolateTexcoords( MeshVertex * pInputVertices,
								   UINT num_verts,
								   D3DXVECTOR2 & start_coord,
								   D3DXVECTOR2 & end_coord );

	HRESULT TesselateExtrusion( Mesh * pMesh,
								 UINT num_verts_in_each_cross_section,
								 UINT num_cross_sections,
								 bool stitch_cross_sections_closed,
								 bool stitch_first_cross_section_to_last );

	MeshSectionStitcher();
	~MeshSectionStitcher();
};

#endif
