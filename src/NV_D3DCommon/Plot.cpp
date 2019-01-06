/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  Plot.cpp

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

#include "NV_D3DCommonDX9PCH.h"
#include "NV_D3DMesh\NV_D3DMeshDX9PCH.h"

Plot::Plot()
{
	SetAllNull();
}

Plot::~Plot()
{
	Free();
	SetAllNull();
}

HRESULT Plot::Free()
{
	HRESULT hr = S_OK;
	size_t i;
	for( i=0; i < m_vPlots.size(); i++ )
	{
		SAFE_DELETE( m_vPlots.at(i) );
	}
	SAFE_RELEASE( m_pD3DDev );
	return( hr );
}

HRESULT Plot::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();
	return( hr );
}

//-------------------------------------------------------------------
// plot_axis_min_val and _max_val determine the scale of the plot.
// If these are not specified, the plot is scaled to the range of the 
//  pVecData
//@ only 1 type of plot is supported so far
//-------------------------------------------------------------------
HRESULT Plot::CreatePlot( PLOT_ID * out_ID, PlotType type,
							vector< float > * pVecData, 
							D3DCOLOR dwColor,
							FRECT fRectPlotBounds,
							float plot_axis_min_val,
							float plot_axis_max_val )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( out_ID );
	FAIL_IF_NULL( pVecData );
	FAIL_IF_NULL( m_pD3DDev );
	RET_VAL_IF( pVecData->size() < 1, E_FAIL );

	PlotDesc * pD = new PlotDesc;
	FAIL_IF_NULL( pD );

	size_t i;
	if( plot_axis_min_val == 0.0f && plot_axis_max_val == 0.0f )
	{
		plot_axis_min_val = pVecData->at(0);
		plot_axis_max_val = pVecData->at(0);
		for( i=1; i < pVecData->size(); i++ )
		{	
			if( pVecData->at(i) < plot_axis_min_val )
				plot_axis_min_val = pVecData->at(i);
			if( pVecData->at(i) > plot_axis_max_val )
				plot_axis_max_val = pVecData->at(i);
		}
	}

	Mesh mesh;
	MeshGeoCreator gc;
	float l, r, t, b;
	// translate window coords to D3D HCLIP coords((-1,-1 to (1,1))
	l = fRectPlotBounds.left * 2.0f - 1.0f;
	r = fRectPlotBounds.right * 2.0f - 1.0f;
	t = 1.0f - fRectPlotBounds.top * 2.0f ;
	b = 1.0f - fRectPlotBounds.bottom * 2.0f;
	gc.InitTesselatedPlane( &mesh, D3DXVECTOR3( l, b, 0.5f ), D3DXVECTOR2( 0.0f, 0.0f ),
							D3DXVECTOR3( r, b, 0.5f ), D3DXVECTOR2( 1.0f, 0.0f ),
							D3DXVECTOR3( l, t, 0.5f ), D3DXVECTOR2( 0.0f, 1.0f ),
							0, 0 );
	mesh.SetVertexColor( 0x00808080 );
	pD->m_pBackground = new MeshVB;
	FAIL_IF_NULL( pD->m_pBackground );
	pD->m_pBackground->CreateFromMesh( &mesh, m_pD3DDev );

	// Create plot bars
	mesh.Free();
	Mesh mesh2;
	float lx, incx, y, dval;
	incx = ( r - l ) / ((float)pVecData->size());
	for( i=0; i < pVecData->size(); i++ )
	{
		dval = pVecData->at(i);
		y = ( pVecData->at(i) - plot_axis_min_val ) / (plot_axis_max_val - plot_axis_min_val);
		y = y * (t - b);		// t-b is positive.  Coords are in D3D HCLIP space
		y = b + y;
		lx = l + (float)(incx * i);
		gc.InitTesselatedPlane( &mesh2, D3DXVECTOR3( lx, b, 0.5f ), D3DXVECTOR2( 0.0f, 0.0f ),
								D3DXVECTOR3( lx+incx, b, 0.5f ), D3DXVECTOR2( 1.0f, 0.0f ),
								D3DXVECTOR3( lx, y, 0.5f ), D3DXVECTOR2( 0.0f, 1.0f ),
								0, 0 );
		gc.InitAddClone( &mesh, &mesh2 );
	}
	mesh.SetVertexColor( dwColor );

	pD->m_pPlot = new MeshVB;
	FAIL_IF_NULL( pD->m_pPlot );
	pD->m_pPlot->CreateFromMesh( &mesh, m_pD3DDev, MeshVB::DYNAMIC );

	pD->m_dwColor = dwColor;
    pD->m_frBounds = fRectPlotBounds;

	m_vPlots.push_back( pD );
	*out_ID = (DWORD)(m_vPlots.size()-1);
	return( hr );
}

HRESULT Plot::DeletePlot( const PLOT_ID & in_ID )
{
	HRESULT hr = S_OK;
	if( in_ID < m_vPlots.size() )
	{
		SAFE_DELETE( m_vPlots.at( in_ID ) );
		m_vPlots.at( in_ID ) = NULL;
	}
	return( hr );
}

HRESULT Plot::SetColor( const PLOT_ID & in_ID, D3DCOLOR dwColor )
{
	HRESULT hr = S_OK;
	return( hr );
}

HRESULT Plot::SetData( const PLOT_ID & in_ID, vector< float > * pVecData )
{
	HRESULT hr = S_OK;
	return( hr );
}

// Render all the plots
HRESULT Plot::Render()
{
	HRESULT hr = S_OK;
	size_t i;
	for( i=0; i < m_vPlots.size(); i++ )
	{
		Render( (PLOT_ID)i );
	}
	return( hr );
}

HRESULT Plot::Render( const PLOT_ID & in_ID, bool bSetPixelState, bool bSetVertexState )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	RET_VAL_IF( in_ID >= m_vPlots.size(), E_FAIL );
	PlotDesc * pD;
	pD = m_vPlots.at(in_ID);
	FAIL_IF_NULL( pD );

	if( bSetVertexState )
	{
		m_pD3DDev->SetVertexShader( NULL );
		D3DXMATRIX matIdentity;
		D3DXMatrixIdentity( &matIdentity );
		m_pD3DDev->SetTransform( D3DTS_WORLD,		&matIdentity );
		m_pD3DDev->SetTransform( D3DTS_VIEW,		&matIdentity );
		m_pD3DDev->SetTransform( D3DTS_PROJECTION,	&matIdentity );
	}
	if( bSetPixelState )
	{
		m_pD3DDev->SetPixelShader( NULL );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE );
		m_pD3DDev->SetRenderState( D3DRS_LIGHTING,				false );
		m_pD3DDev->SetRenderState( D3DRS_COLORVERTEX,			true );

		m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	}

	DWORD dwZFunc;
	DWORD dwZWrite;
	DWORD dwCull;
	m_pD3DDev->GetRenderState( D3DRS_ZFUNC,			&dwZFunc );
	m_pD3DDev->GetRenderState( D3DRS_ZWRITEENABLE,	&dwZWrite );
	m_pD3DDev->GetRenderState( D3DRS_CULLMODE,		&dwCull );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_ALWAYS );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );

	if( pD->m_pBackground != NULL )
	{
		pD->m_pBackground->Draw();
	}
	if( pD->m_pPlot != NULL )
	{
		pD->m_pPlot->Draw();
	}

	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			dwZFunc );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	dwZWrite );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		dwCull );
	return( hr );
}

HRESULT Plot::SetStateForRendering( const PLOT_ID & in_ID, bool bSetPixelState, bool bSetVertexState )
{
	HRESULT hr = S_OK;
	return( hr );
}

//----------------------------------------

Plot::PlotDesc::PlotDesc()
{
	SetAllNull();
}

Plot::PlotDesc::~PlotDesc()
{
	SAFE_DELETE( m_pPlot );
	SAFE_DELETE( m_pBorder );
	SAFE_DELETE( m_pBackground );
	SAFE_DELETE( m_pPlotState );
	SAFE_DELETE( m_pBorderState );
	SAFE_DELETE( m_pBackgroundState );
}


