/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshBeingProcessed.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

	For some mesh processing operations such as re-ordering vertices or re-ordering
triangles, it is fastest to store and process an intermediate representation.
This representation can then be reduced to an ordinary Mesh when the processing
is finished.

-------------------------------------------------------------------------------|--------------------*/


#ifndef H_MESHBEINGPROCESSED_H
#define H_MESHBEINGPROCESSED_H


class Mesh;

#include "NV_D3DMesh_decl.h"
class DECLSPEC_NV_D3D_MESH_API MeshBeingProcessed
{
public:
	UINT	*	m_pVertIndex;					// Vertex index redirection table.  Triangles that normally reference
												// vertices 1,2,3 in the vertex array, now reference entries 1,2,3 in 
												// this array to retrieve the vertex indices.
	UINT	*	m_pOrigVertPos;					// Which position each vertex originaly occupied.
	UINT		m_uNumVertIndex;				// number of UINTs in m_pVertIndex

	HRESULT InitFromMesh( const Mesh * pMesh );
	HRESULT Free();

	HRESULT	GetProcessedMesh( Mesh * pOutMesh );		// pOutMesh is overwritten to contain the final mesh

	HRESULT SwapVertices( UINT index1, UINT index2 );	// A fast vertex swap function.  Use this for reordering vertices, 
														// for example to move data if a vertex is removed from the mesh.
														// Preserves all triangles.  This function swaps the vertex data and
														// updates two entries of a vertex index redirection table (m_pVertIndex)
														// so that triangles use the correct vertex data.  The triangle vertex
														// indices are not changed until GetProcessedMesh() is called.
	
	// Changes triangle vertex indices so that every occurrence of dwVertexToChange in the triangle
	//  indices will become dwVertexItBecomes.  This does not eliminated the unused vertices.
	HRESULT MergeVertices( DWORD dwVertexItBecomes, DWORD dwVertexToChange );


	MeshBeingProcessed();
	MeshBeingProcessed( const Mesh * pMesh );
	~MeshBeingProcessed();
	void SetAllNull();

protected:
	Mesh * m_pMesh;			// allocated by MeshBeingProcessed class

	// Allocate a temporary array.  The pointer is not tracked by any
	// class data members and must be freed after use.
	HRESULT	AllocateArray( UINT ** ppArray, UINT num_elements );
	HRESULT AllocateResizeArray( UINT ** ppArray, UINT num_elements );
};


#endif		// H_MESHBEINGPROCESSED_H

