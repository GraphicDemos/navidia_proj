

#ifndef H_QUADVB_H
#define H_QUADVB_H

#include "NV_D3DMeshDX9PCH.h"
#include "NV_D3DMesh_decl.h"

class DECLSPEC_NV_D3D_MESH_API QuadVB
{
public:
	IDirect3DVertexBuffer9 *	m_pVB;
	IDirect3DIndexBuffer9 *		m_pIB;

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev, float left, float right, float top, float bottom, float depth = 0.0f );
	HRESULT Initialize( IDirect3DDevice9 * pDev, D3DXVECTOR3 & pt1, D3DXVECTOR3 & pt2, D3DXVECTOR3 & pt3, D3DXVECTOR3 & pt4 );
	HRESULT Render();
	HRESULT DrawIndexedPrimitive();

	QuadVB();
	~QuadVB();
	void SetAllNull()
	{
		m_pVB		= NULL;
		m_pIB		= NULL;
		m_pDecl		= NULL;
		m_pD3DDev	= NULL;
	};

protected:
	IDirect3DVertexDeclaration9 *	m_pDecl;
	IDirect3DDevice9 *	m_pD3DDev;
};

#endif
