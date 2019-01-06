/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  Mesh.h

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

#ifndef H_NVD3DMESH_H
#define H_NVD3DMESH_H

#include "NV_D3DMesh\MeshVertex.h"
#include "NV_D3DMesh_decl.h"

class DECLSPEC_NV_D3D_MESH_API Mesh
{
public:
	MeshVertex *		m_pVertices;
	DWORD				m_dwNumVertices;
	DWORD *				m_pIndices;
	DWORD				m_dwNumIndices;

	D3DPRIMITIVETYPE	m_PrimType;
	bool				m_bIsValid;

	HRESULT Allocate( DWORD num_vertices, DWORD num_indices );
	HRESULT AllocateVertices( DWORD num_vertices );
	HRESULT AllocateIndices( DWORD num_indices );
	HRESULT AllocateResizeVertices( DWORD total_num_vertices );		// copies old vertices into new array, up to the new size
	HRESULT AllocateResizeIndices( DWORD total_num_indices );		

	void	Free();
	void	FreeVertices();
	void	FreeIndices();

	DWORD	GetNumVertices() const	{ return( m_dwNumVertices ); };
	DWORD	GetNumIndices() const	{ return( m_dwNumIndices );	};
	DWORD	GetNumTriangles() const;
	HRESULT GetTriangleIndices( DWORD triangle, DWORD * pVertIndex1, DWORD * pVertIndex2, DWORD * pVertIndex3 ) const;

	// A few simple geometry processing functions
	// See MeshGeoCreator and MeshProcessor for more complicated functions
	void	FlipWinding();
	void	FlipNormals();
	void	Translate( float x, float y, float z );					// translate all vertex positions
	void	Scale( float x, float y, float z );
	void	Transform( D3DXMATRIX * pMat );							// transform positions & normals
	void    Transform( Mesh * pOutMesh, D3DXMATRIX * pMat ) const;	// leaves 'this' unaffected
	void	SetVertexColor( DWORD ARGBColor );
	void	CalculateTriangleCenter( DWORD triangle, D3DXVECTOR3 * pOutCenter ) const;
	
	MeshVertex * GetVertexPtr( UINT vertex_index )
	{
		return( &(m_pVertices[vertex_index]) );
	}
	D3DXVECTOR3 GetVertexPosition( UINT vertex_index )
	{
		return( m_pVertices[vertex_index].pos );
	}
	D3DXVECTOR3 GetVertexNormal( UINT vertex_index )
	{
		return( m_pVertices[vertex_index].nrm );
	}
	D3DXVECTOR2 GetVertexTexcoord( UINT vertex_index )
	{
		return( m_pVertices[vertex_index].t0 );
	}
	DWORD		GetVertexColor( UINT vertex_index )
	{
		return( m_pVertices[vertex_index].diffuse );
	}

	Mesh()				{ SetAllNull();	}
	virtual ~Mesh()		{ Free(); SetAllNull(); }

protected:
	void SetAllNull()
	{
		m_pVertices		= NULL;
		m_pIndices		= NULL;
		m_dwNumVertices = 0;
		m_dwNumIndices	= 0;
	}
};

#endif

