/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVB.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Mesh Vertex Buffer object.  This copies the CPU resident data of a Mesh into Direct3D hardware 
vertex and index buffers for efficient rendering.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_D3DMESH_MESHVB_H
#define H_D3DMESH_MESHVB_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"

#include "NV_D3DMesh_decl.h"

class DECLSPEC_NV_D3D_MESH_API MeshVB
{
public:
	enum VBUsage
	{
		DYNAMIC,			// use dynamic vertex buffer
		STATIC				// use static vertex buffer
	};
public:
	// main interface functions
	virtual HRESULT Free();
	virtual HRESULT CreateFromMesh( Mesh * pObj,
									IDirect3DDevice9*  pD3DDev,
									VBUsage dynamic_or_static = MeshVB::STATIC );
	virtual HRESULT UpdateFromMesh( Mesh * pObj );
	virtual HRESULT UpdateIndicesFromMesh( Mesh * pObj );
	virtual HRESULT UpdateIndicesFromMesh( Mesh * pMesh, IDirect3DIndexBuffer9 * pIndexBuffer );
	virtual HRESULT UpdateVerticesFromMesh( Mesh * pObj );					
	virtual size_t	GetSizeOfVertexInBytes();
	virtual HRESULT CreateVertexDeclaration();

	virtual HRESULT Draw();
	virtual HRESULT Draw( UINT start_index, UINT primitive_count );
	virtual HRESULT DrawAsPoints( UINT start_vertex =0, UINT num_verts = 0xFFFFFFFF );

	bool				IsValid();
	UINT				GetNumVertices()	{ return( m_uNumVertices ); };
	UINT				GetNumIndices()		{ return( m_uNumIndices ); };
	D3DPRIMITIVETYPE	GetPrimitiveType()	{ return( m_PrimType );				};
	IDirect3DVertexDeclaration9 * GetVertexDeclaration() { return( m_pVertexDeclaration ); };

protected:
	IDirect3DDevice9*	m_pD3DDev; 
	bool				m_bIsValid;
	D3DPRIMITIVETYPE	m_PrimType;
	bool				m_bDynamic;					// VertexBuffer created with USAGE_DYNAMIC?
	DWORD				m_dwIndexSizeInBytes;
    IDirect3DVertexDeclaration9 *	m_pVertexDeclaration;

	HRESULT CreateVertexBuffer( UINT Length,
								DWORD Usage,
								DWORD FVF,
								D3DPOOL Pool,
								IDirect3DVertexBuffer9** ppVertexBuffer,
								HANDLE* pHandle = NULL );

	HRESULT CreateIndexBuffer(	UINT Length,
								DWORD Usage,
								D3DFORMAT Format,
								D3DPOOL Pool,
								IDirect3DIndexBuffer9** ppIndexBuffer,
								HANDLE* pHandle = NULL );

public:
	IDirect3DVertexBuffer9 *	m_pVertexBuffer;
	UINT						m_uNumVertices;
	IDirect3DIndexBuffer9 *		m_pIndexBuffer;
	UINT						m_uNumIndices;

	MeshVB();
	virtual ~MeshVB();

	friend class ShadowVolumeMeshVB;
	friend class NVBModelLoader;
};

#endif

