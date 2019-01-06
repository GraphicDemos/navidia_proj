/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TextureDisplay2.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the header file for comments.

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DCommonDX9PCH.h"
#include "NV_D3DMesh\NV_D3DMesh.h"

#if 0
#define TRACE0	FMsg
#else
#define TRACE0	NullFunc
#endif

TextureDisplay2::TextureDisplay2()
{
	TRACE0("TextureDisplay2::TextureDisplay2()\n");
	SetAllNull();
}

TextureDisplay2::~TextureDisplay2()
{
	TRACE0("TextureDisplay2::~TextureDisplay2()\n");
	Free();
	SetAllNull();
}

HRESULT TextureDisplay2::Free()
{
	TRACE0("TextureDisplay2::Free()\n");
	HRESULT hr = S_OK;
	FreeDisplayables();
	SAFE_DELETE( m_pAllQuads );
	SAFE_DELETE( m_pAllQuadsVB );
	SAFE_RELEASE( m_pD3DDev );
	return( hr );
}

HRESULT TextureDisplay2::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	Free();
	TRACE0("TextureDisplay2::Initialize()\n");
	FAIL_IF_NULL( pDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_pAllQuads = new Mesh;
	FAIL_IF_NULL( m_pAllQuads );
	m_pAllQuadsVB = new MeshVB;
	FAIL_IF_NULL( m_pAllQuadsVB );

	hr = ResizeDisplayables( 4 );		// reserve some space
	return( hr );
}


HRESULT	TextureDisplay2::ResizeDisplayables( UINT num )
{
	HRESULT hr = S_OK;
	m_pDisplayables = (Displayable*) realloc( m_pDisplayables, sizeof( Displayable ) * num );
	FAIL_IF_NULL( m_pDisplayables );
	for( UINT i=m_uNumDisplayables; i < num; i++ )
	{
		m_pDisplayables[i].m_uPrimCount = 0;
		m_pDisplayables[i].m_uStartIndex = 0;
		m_pDisplayables[i].m_ppTexture = NULL;
	}
	m_uNumDisplayables = num;
	return( hr );
}

HRESULT TextureDisplay2::FreeDisplayables()
{
	HRESULT hr = S_OK;
	if( m_pDisplayables != NULL )
	{
		free( m_pDisplayables );
		m_pDisplayables = NULL;
		m_uNumDisplayables = 0;
	}
	return( hr );
}


// Windows coords are [0,0] in upper left corner to [1,1] in lower right corner
// D3D homogeneous clip space (HCLIP) coords are from [-1,-1] in the lower left to [1,1] in the upper right
// This function maps windows coords to HCLIP space coords 
void TextureDisplay2::MapWindowsCoordsToHCLIP( const FRECT & in_fRect, FRECT * pOutRect )
{
	RET_IF( pOutRect == NULL );
	pOutRect->left	= in_fRect.left * 2.0f - 1.0f;
	pOutRect->right	= in_fRect.right * 2.0f - 1.0f;
	pOutRect->top	= 1.0f - in_fRect.top * 2.0f;
	pOutRect->bottom = 1.0f - in_fRect.bottom * 2.0f;
}

void TextureDisplay2::MapHCLIPCoordsToWindowsCoords( const FRECT & in_fRect, FRECT * pOutRect )
{
	RET_IF( pOutRect == NULL );
	pOutRect->left	= ( in_fRect.left + 1.0f ) / 2.0f;
	pOutRect->right	= ( in_fRect.right + 1.0f ) / 2.0f;
	pOutRect->top	= ( 1.0f - in_fRect.top ) / 2.0f;
	pOutRect->bottom = ( 1.0f - in_fRect.bottom ) / 2.0f;
}

//---------------------------------------------------------------------------------------
// in_fRect specifies the coordinates of the rect in Window's coordinates with (0,0) corresponding to the 
//   upper left of the render target and (1,1) corresponding to the lower right.
//---------------------------------------------------------------------------------------
HRESULT TextureDisplay2::AddTexture( TD_TEXID * out_ID, IDirect3DTexture9 ** in_ppTex, const FRECT & in_fRect )
{
	FAIL_IF_NULL( m_pAllQuads );
	FAIL_IF_NULL( m_pAllQuadsVB );
	HRESULT hr = S_OK;
	UINT i, tricount;

	// find a free entry
	tricount = 0;
	i = 0;
	if( m_pDisplayables != NULL )
	{
		while( m_pDisplayables[i].m_uPrimCount != 0 && 
				i < m_uNumDisplayables					)
		{
			tricount += m_pDisplayables[i].m_uPrimCount;
			i++;
		}
	}
	if( i >= m_uNumDisplayables )
	{
		hr = ResizeDisplayables( m_uNumDisplayables + 5 );
		MSG_AND_RET_VAL_IF( FAILED(hr), "AddTexture couldn't resize displayables\n", hr );
		MSG_AND_RET_VAL_IF( i >= m_uNumDisplayables, "Didn't allocate enough displayables\n", hr );
	}

	*out_ID = i;
	m_pDisplayables[i].m_uPrimCount = 2;				// must always be 2
	m_pDisplayables[i].m_uStartIndex = tricount * 3;

//	FMsg("m_pDisplayables[%u] prim count = %u  start = %u\n", i, m_pDisplayables[i].m_uPrimCount, m_pDisplayables[i].m_uStartIndex );
		
	// Set the rectangle and texture handle for the ID we've selected
	SetTextureRect( *out_ID, in_fRect );
	SetTexture( *out_ID, in_ppTex );

	return( hr );
}


HRESULT TextureDisplay2::RemoveTexture( const TD_TEXID & in_ID )
{
	HRESULT hr = S_OK;

	return( hr );
}

HRESULT TextureDisplay2::SetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** in_ppTex )
{
	HRESULT hr = S_OK;
	RET_VAL_IF( in_ID >= m_uNumDisplayables, E_FAIL );
	RET_VAL_IF( m_pDisplayables[in_ID].m_uPrimCount == 0, E_FAIL );

	m_pDisplayables[in_ID].m_ppTexture = in_ppTex;
	return( hr );
}

HRESULT TextureDisplay2::GetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** out_ppTex )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( out_ppTex );
	if( in_ID < m_uNumDisplayables )
	{
		*out_ppTex = *(m_pDisplayables[in_ID].m_ppTexture);
	}
	else
	{
		*out_ppTex = NULL;
		hr = E_FAIL;
	}
	return( hr );
}

IDirect3DTexture9 * TextureDisplay2::GetTextureP( const TD_TEXID & in_TD )
{
	IDirect3DTexture9 * pTex;
	GetTexture( in_TD, &pTex );
	return( pTex );
}

//---------------------------------------------------------------------------------------
// in_fRect specifies the coordinates of the rect in Window's coordinates with (0,0) corresponding to the 
//   upper left of the render target and (1,1) corresponding to the lower right.
//---------------------------------------------------------------------------------------
HRESULT TextureDisplay2::SetTextureRect( const TD_TEXID & in_ID, const FRECT & in_fRect )
{
	HRESULT hr = S_OK;
	UINT i;
	i = in_ID;
	RET_VAL_IF( i >= m_uNumDisplayables, E_FAIL );
	RET_VAL_IF( m_pDisplayables[i].m_uPrimCount == 0, E_FAIL );

	if( m_pAllQuads->GetNumIndices() < m_pDisplayables[i].m_uStartIndex + 6 )
	{
		m_pAllQuads->AllocateResizeIndices( m_pDisplayables[i].m_uStartIndex + 6 );
		m_pAllQuads->AllocateResizeVertices( i * 4 + 4 );		// 4 verts for every Displayable
		m_pAllQuads->m_PrimType = D3DPT_TRIANGLELIST;
		m_pAllQuads->m_bIsValid = true;
		m_pAllQuadsVB->CreateFromMesh( m_pAllQuads, m_pD3DDev, MeshVB::DYNAMIC );
	}

	FRECT hclip_rect;
	MapWindowsCoordsToHCLIP( in_fRect, & hclip_rect );
	float z = 0.5f;

	// set the verts & inds to make the box
	UINT ind;
	ind = i*4;
	m_pAllQuads->m_pVertices[ ind   ].pos = D3DXVECTOR3( hclip_rect.left, hclip_rect.bottom, z );		
	m_pAllQuads->m_pVertices[ ind+1 ].pos = D3DXVECTOR3( hclip_rect.right, hclip_rect.bottom, z );
	m_pAllQuads->m_pVertices[ ind+2 ].pos = D3DXVECTOR3( hclip_rect.left, hclip_rect.top, z );
	m_pAllQuads->m_pVertices[ ind+3 ].pos = D3DXVECTOR3( hclip_rect.right, hclip_rect.top, z );

	// texture coords from [0,0] in upper left to [1,1] in lower right
	m_pAllQuads->m_pVertices[ ind   ].t0  = D3DXVECTOR2( 0.0f, 1.0f );
	m_pAllQuads->m_pVertices[ ind+1 ].t0  = D3DXVECTOR2( 1.0f, 1.0f );
	m_pAllQuads->m_pVertices[ ind+2 ].t0  = D3DXVECTOR2( 0.0f, 0.0f );
	m_pAllQuads->m_pVertices[ ind+3 ].t0  = D3DXVECTOR2( 1.0f, 0.0f );

	UINT tind;
	tind = m_pDisplayables[i].m_uStartIndex;
	m_pAllQuads->m_pIndices[ tind++ ] = ind;	// 1st triangle
	m_pAllQuads->m_pIndices[ tind++ ] = ind+1;
	m_pAllQuads->m_pIndices[ tind++ ] = ind+2;

	m_pAllQuads->m_pIndices[ tind++ ] = ind+2;	// 2nd triangle
	m_pAllQuads->m_pIndices[ tind++ ] = ind+1;
	m_pAllQuads->m_pIndices[ tind++ ] = ind+3;

	m_pAllQuads->m_bIsValid = true;

	// make a vertex buffer object from the system memory vertex data
	m_pAllQuadsVB->UpdateFromMesh( m_pAllQuads );

	return( hr );
}

HRESULT TextureDisplay2::GetTextureRect( const TD_TEXID & in_ID, FRECT * pfRect )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pfRect );
	FAIL_IF_NULL( m_pAllQuads );
	if( in_ID < m_uNumDisplayables )
	{
		UINT tri = m_pDisplayables[in_ID].m_uStartIndex / 3;
		DWORD i1, i2, i3;
		m_pAllQuads->GetTriangleIndices( tri, &i1, &i2, &i3 );
		D3DXVECTOR3 v1, v2, v3;
		v1 = m_pAllQuads->GetVertexPosition( i1 );
		v2 = m_pAllQuads->GetVertexPosition( i2 );
		v3 = m_pAllQuads->GetVertexPosition( i3 );
		FRECT hclip_rect;
		hclip_rect.left = v1.x;
		hclip_rect.right = v2.x;
		hclip_rect.top = v3.y;
		hclip_rect.bottom = v1.y;

		MapHCLIPCoordsToWindowsCoords( hclip_rect, pfRect );
	}
	else
	{
		hr = E_FAIL;
	}
	return( hr );
}

// Sets coordinates to use in the rectangle.  Left is the leftmost X coordinate.
// Top is the topmost Y coordinate.
HRESULT TextureDisplay2::SetTextureCoords( const TD_TEXID & in_ID, const FRECT & in_fRect )
{
	HRESULT hr = S_OK;
	RET_VAL_IF( in_ID >= m_uNumDisplayables, E_FAIL );
	RET_VAL_IF( m_pDisplayables[in_ID].m_uPrimCount == 0, E_FAIL );
	RET_VAL_IF( m_pAllQuads->GetNumIndices() < m_pDisplayables[in_ID].m_uStartIndex + 6, E_FAIL );

	// set the verts & inds to make the box
	UINT ind;
	ind = in_ID * 4;

	// texture coords from [0,0] in upper left to [1,1] in lower right
	m_pAllQuads->m_pVertices[ ind   ].t0  = D3DXVECTOR2( in_fRect.left, in_fRect.bottom );
	m_pAllQuads->m_pVertices[ ind+1 ].t0  = D3DXVECTOR2( in_fRect.right, in_fRect.bottom );
	m_pAllQuads->m_pVertices[ ind+2 ].t0  = D3DXVECTOR2( in_fRect.left, in_fRect.top );
	m_pAllQuads->m_pVertices[ ind+3 ].t0  = D3DXVECTOR2( in_fRect.right, in_fRect.top );

	// update the vertex buffer data from the system memory vertex data
	m_pAllQuadsVB->UpdateFromMesh( m_pAllQuads );
	return( hr );
}


HRESULT TextureDisplay2::SetRenderState( const TD_TEXID & in_ID )
{
	FAIL_IF_NULL( m_pD3DDev );
	FAIL_IF_NULL( m_pDisplayables );
	RET_VAL_IF( in_ID >= m_uNumDisplayables, E_FAIL );
	HRESULT hr = S_OK;

	m_pD3DDev->SetPixelShader( NULL );

	if( m_pDisplayables[in_ID].m_ppTexture != NULL )
	{
		m_pD3DDev->SetTexture( 0,	*(m_pDisplayables[in_ID].m_ppTexture) );

		// set texture stage states to output the texture
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	}
	else
	{
		m_pD3DDev->SetTexture( 0, NULL );
		// set texture stage states to output vertex color
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	}

	m_pD3DDev->SetRenderState( D3DRS_COLORVERTEX,	true );
	m_pD3DDev->SetRenderState( D3DRS_LIGHTING,		false );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,		false );
	return( hr );
}


HRESULT TextureDisplay2::SetStateForRendering( const TD_TEXID & in_ID, bool bSetPixelState, bool bSetVertexState )
{
	HRESULT hr = S_OK;
	if( bSetVertexState )
	{
		D3DXMATRIX matIdentity;
		D3DXMatrixIdentity( &matIdentity );
		m_pD3DDev->SetTransform( D3DTS_WORLD, &matIdentity );
		m_pD3DDev->SetTransform( D3DTS_VIEW, &matIdentity );
		m_pD3DDev->SetTransform( D3DTS_PROJECTION, &matIdentity );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,	D3DCULL_NONE );

		// replicate 1 texture coord to four texture stages
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
		m_pD3DDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0 );
		m_pD3DDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 0 );

		m_pD3DDev->SetVertexShader( NULL );
		m_pD3DDev->SetFVF( MESHVERTEX_FVF );
	}

	if( bSetPixelState )
	{
		SetRenderState( in_ID );
	}
	return( hr );
}


HRESULT TextureDisplay2::Render( const TD_TEXID & in_ID, 
								 bool bSetPixelState, bool bSetVertexState )
{
	FAIL_IF_NULL( m_pD3DDev );
	FAIL_IF_NULL( m_pAllQuadsVB );
	FAIL_IF_NULL( m_pDisplayables );
	HRESULT hr = S_OK;

	TD_TEXID id = in_ID;
	MSG_AND_RET_VAL_IF( m_uNumDisplayables < id, "TextureDisplay2 asked to render a TD_TEXID with too high a value!\n", E_FAIL );

	SetStateForRendering( in_ID, bSetPixelState, bSetVertexState );
	m_pAllQuadsVB->Draw( m_pDisplayables[id].m_uStartIndex, m_pDisplayables[id].m_uPrimCount );

	return( hr );
}

