/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshProcessor.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Functions to process triangle mesh data.  Such functions are spread out between this class and the 
MeshBeingProcessed class, depending on the nature of the processing.  Functions which would be costly
for plain mesh data, such as reordering vertices, are moved to the MeshBeingProcessed class which holds
additional temporary data in order to speed up the operations.


-------------------------------------------------------------------------------|--------------------*/


#ifndef H_D3DMESHPROCESSOR_H
#define H_D3DMESHPROCESSOR_H

#include <shared/NoiseGrid3D.h>
#include "NV_D3DMesh_decl.h"

class Mesh;
class MeshBeingProcessed;
class MeshVertex;

typedef HRESULT (*PF_3DIN_3DOUT)( float, float, float, float*, float*, float* );
typedef HRESULT (*PF_MESHVERTEX)( MeshVertex * pVertex );


class DECLSPEC_NV_D3D_MESH_API MeshProcessor
{
public:

	HRESULT	FindPositionMinMax( const Mesh * pInMesh,
								float * pMinx, float * pMiny, float * pMinz,
								float * pMaxx, float * pMaxy, float * pMaxz );
	HRESULT FindPositionMinMax( const Mesh * pInMesh, D3DXVECTOR3 * pOutAABBMin, D3DXVECTOR3 * pOutAABBMax );
	HRESULT CalculateAABBCenter( const Mesh * pInMesh, D3DXVECTOR3 * pOutputCenter );

	HRESULT ApplyFunctionToPositions( Mesh * pOutMesh, Mesh * pInputMesh, PF_3DIN_3DOUT pFunc,
										UINT uStartVert = 0, UINT uNumVerts = 0xFFFFFFFF );
	HRESULT ApplyFunction( Mesh * pMesh, PF_MESHVERTEX pFunc, UINT uStartVert = 0, UINT uNumVerts = 0xFFFFFFFF );

	HRESULT ProjectVertsToSphere( Mesh * pMesh, float radius );

	// The Explode..() functions move each triangle outward along a vector from explosion_origin to
	//  the center of the triangle.  Triangles that share vertices will not appear to be separated
	//  from one another.
	HRESULT ExplodeFaces( Mesh * pMesh, float distance );		// explode from center of mesh's AABB
	HRESULT	ExplodeFaces( Mesh * pMesh, float distance, D3DXVECTOR3 explosion_origin );

	// Calculate and assign values to pMesh's normals by summing triangle face normals.
	// CCW assumes a counter-clockwise winding order to the triangle vertices
	HRESULT	CalculateNormalsCCW( Mesh * pMesh );
	HRESULT CalculateNormalsCW( Mesh * pMesh );
	HRESULT FlipNormals( Mesh * pMesh );		// flip all vertex normals
	HRESULT FlipWinding( Mesh * pMesh );		// flip the winding order for all triangles

	HRESULT SetColorFromVertexNormal( Mesh * pMesh );	// set each vertex's color to represent the vertex normal

	HRESULT MakeFaceted( Mesh * pMesh );		// generate unique vertices for every triangle

	// Adds noise to the positions pInputMesh, writing the perturbed positions to pOutMesh.
	// pOutMesh will be reallocated if it does not have as many vertices as pInputMesh
	// pOutMesh and pInputMesh can be the same mesh.
	// noise_dir      : The noise is always along the direction of this vector.
	// The noise magnitude is determined by the GridNoiseComponents
	// Mesh normals are not re-calculated
	HRESULT	AddPositionNoise1D( Mesh * pOutMesh, Mesh * pInputMesh, 
								D3DXVECTOR3 noise_dir,
								GridNoiseComponent * pNoiseComponents,
								UINT uNumNoiseComponents );

	HRESULT	AddPositionNoise1D( Mesh * pOutMesh, Mesh * pInputMesh, 
								D3DXVECTOR3 noise_dir,
								D3DXVECTOR3 input_pos_translation,
								GridNoiseComponent * pNoiseComponents,
								UINT uNumNoiseComponents );

	// Same as AddPositionNoise1D except the noise perturbation varies direction in 3D space.
	// The noise moves the vertex positions in different directions, unlike ..1D which moves
	// vertices in the same direction but with varying magnitude along that direction.
	HRESULT AddPositionNoise3D( Mesh * pOutMesh, Mesh * pInputMesh,
								D3DXVECTOR3 input_pos_translation,
								GridNoiseComponent * pNoiseComponents,
								UINT uNumNoiseComponents );

	// Scan a mesh's indices and remove any vertices that are not used.  This will 
	// remap vertices in the vertex array and will change the triangle vertex indices.
	HRESULT RemoveUnusedVertices( Mesh * pMesh );

	// If two vertices lie within all threshold amounts specified, then remove one
	//  of the vertices and remap triangles to use the other vertex.
	// If bLeaveUnusedVertsInPlace is true, then the vertex is not removed from the
	//  mesh's vertex array.  The vertex is left in place but is not referenced by
	//  any triangles.
	// The thresholds are in all axes of a parameter, so position threshold is applied
	//  to the vertice's difference in .x, .y, and .z.  Meaning the thresholds define
	//  a box in which the two vertices must lie in order to have one removed.
	// The threshold for the normal is 1-dimensional and is applied to the dotproduct
	//  (the cosine of the angle) between the two normals being compared.  The dotproduct
	//  must be higher than the threshold amount.  If fNormalDP3Thresh is less than -1.0f,
	//  then any two normals will be considered to be the same.
	HRESULT RemoveDegenerateVerts( Mesh * pMesh, float fPositionThresh, float fNormalDP3Thresh,
									float fTexcoordThresh, bool bLeaveUnusedVertsInPlace = false );

	// Delete the vertices and any triangles that use the vertices
	HRESULT RemoveVertices( Mesh * pMesh, UINT * pVerticesToRemove, UINT uNumVerticesInArray );

	float   CalculateTriangleArea( Mesh * pMesh, UINT vert_index_1, UINT vert_index_2, UINT vert_index_3 );
	// Remove triangles if their area is less than area_threshold.
	// Also removes degenerate triangles
	HRESULT RemoveZeroAreaTriangles( Mesh * pMesh, float area_threshold = 1e-4 );
	// Remove triangles where two or more vertex indices are the same.
	// This will leave zero-area triangles untouched.
	HRESULT RemoveDegenerateTriangles( Mesh * pMesh );

	HRESULT ListVertexPositions( Mesh * pMesh, UINT first = 0, UINT last = 0xFFFFFFFF );
	HRESULT ListTriangleIndices( Mesh * pMesh, UINT first = 0, UINT last = 0xFFFFFFFF );
	HRESULT ListMeshStats( Mesh * pMesh );

};


#endif
