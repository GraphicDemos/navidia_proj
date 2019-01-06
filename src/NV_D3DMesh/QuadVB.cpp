/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  QuadVB.cpp

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


#include "NV_D3DMeshDX9PCH.h"

QuadVB::QuadVB()
{
	SetAllNull();
}

QuadVB::~QuadVB()
{
	Free();
	SetAllNull();
}
//---------------------------------
HRESULT QuadVB::Free()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pVB );
	SAFE_RELEASE( m_pIB );
	SAFE_RELEASE( m_pDecl );
	SAFE_RELEASE( m_pD3DDev );
	return( hr );
}

// Specify coordinates in HCLIP space, so for a quad covering the full
// screen, you would use ( pDev, -1.0f, 1.0f, 1.0f, -1.0f )
HRESULT QuadVB::Initialize( IDirect3DDevice9 * pDev, 
							float left, 
							float right, 
							float top, 
							float bottom, 
							float depth )
{
	HRESULT hr = S_OK;
	D3DXVECTOR3 pt1, pt2, pt3, pt4;
	pt1 = D3DXVECTOR3( left, top,		depth );
	pt2 = D3DXVECTOR3( left, bottom,	depth );
	pt3 = D3DXVECTOR3( right, bottom,	depth );
	pt4 = D3DXVECTOR3( right, top,		depth );

	hr = Initialize( pDev, pt1, pt2, pt3, pt4 );
	return( hr );
}

// Specify points in counter-clockwise order.
HRESULT QuadVB::Initialize( IDirect3DDevice9 * pDev, 
							D3DXVECTOR3 & pt1, 
							D3DXVECTOR3 & pt2, 
							D3DXVECTOR3 & pt3, 
							D3DXVECTOR3 & pt4 )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	QuadVBVertex v;
	QuadVBVertex * pV;
	m_pDecl = v.CreateDeclaration( m_pD3DDev );
	MSG_AND_RET_VAL_IF( m_pDecl == NULL, "couldn't create QuadVBVertex decl\n", E_FAIL );

	hr = m_pD3DDev->CreateVertexBuffer( 4 * sizeof(QuadVBVertex), D3DUSAGE_WRITEONLY,
										v.GetFVF(), D3DPOOL_DEFAULT, &m_pVB, NULL );
	RET_VAL_IF( FAILED(hr), hr );

	hr = m_pD3DDev->CreateIndexBuffer( 6 * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
										D3DPOOL_DEFAULT, &m_pIB, NULL );
	RET_VAL_IF( FAILED(hr), hr );

	hr = m_pVB->Lock( 0, 0, (void**)&pV, NULL );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), "QuadVB VB lock failed\n", hr );
	pV[0].pos = pt1;
	pV[1].pos = pt2;
	pV[2].pos = pt3;
	pV[3].pos = pt4;
	m_pVB->Unlock();

	short * pShort;
	hr = m_pIB->Lock( 0, 0, (void**)&pShort, NULL );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), "QuadVB IB lock failed\n", hr );
	pShort[0] = 0;
	pShort[1] = 1;
	pShort[2] = 2;
	pShort[3] = 2;
	pShort[4] = 3;
	pShort[5] = 0;
	m_pIB->Unlock();

	return( hr );
}

HRESULT QuadVB::Render()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	m_pD3DDev->SetVertexShader( NULL );
	QuadVBVertex v;
	m_pD3DDev->SetFVF( v.GetFVF() );
	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity( &matIdentity );
	m_pD3DDev->SetTransform( D3DTS_WORLD, &matIdentity );
	m_pD3DDev->SetTransform( D3DTS_VIEW, &matIdentity );
	m_pD3DDev->SetTransform( D3DTS_PROJECTION, &matIdentity );

	hr = DrawIndexedPrimitive();
	return( hr );
}

HRESULT QuadVB::DrawIndexedPrimitive()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	FAIL_IF_NULL( m_pVB );
	FAIL_IF_NULL( m_pIB );
	hr = m_pD3DDev->SetStreamSource( 0, m_pVB, 0, sizeof( QuadVBVertex ) );
	MSG_IF( FAILED(hr), "QuadVB::DrawIndexedPrimitive SetStreamSource failed!\n" );
	hr = m_pD3DDev->SetIndices( m_pIB );
	MSG_IF( FAILED(hr), "QuadVB::DrawIndexedPrimitive SetIndices failed!\n" );
	hr = m_pD3DDev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 
											2 ); // prim count
	MSG_IF( FAILED(hr), "QuadVB::DrawIndexedPrimitive DrawIndexedPrimitive failed!\n" );
	return( hr );
}
