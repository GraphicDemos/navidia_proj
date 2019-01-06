/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  RenderToTextureBase.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:


******************************************************************************/

// #define NVMSG_SAFE_ARRAY_DELETE
// #define NVMSG_SAFE_DELETE
// #define NVMSG_SAFE_RELEASE

#include "NV_D3DCommonDX9PCH.h"

#if 0
	#define TRACE0 FDebug
#else
	#define TRACE0 NullFunc
#endif

//------------------------------------------

RenderToTextureBase::RenderToTextureBase()
{
	TRACE0("RenderToTextureBase::RenderToTextureBase()\n");
	SetToNull();
}

RenderToTextureBase::~RenderToTextureBase()
{
	TRACE0("RenderToTextureBase::~RenderToTextureBase()\n");
	RenderToTextureBase::Free();
}

//------------------------------------------


void	RenderToTextureBase::SetToNull()
{
	m_ppRTTSurface				= NULL;
	m_ppRTTTexture				= NULL;
	m_pRTTDepthBuffer			= NULL;

	m_pDefaultBackbufferColor	= NULL; 
	m_pDefaultBackbufferDepth	= NULL; 


	m_nNumSurfaces	= 0;
	m_pD3DDev		= NULL;
	m_nRTTWidth		= 0;
	m_nRTTHeight	= 0;

	m_pXShaderManager	= NULL;
	m_ppShaderManager	= NULL;
}


HRESULT RenderToTextureBase::Free()
{
	HRESULT hr = S_OK;


	// m_pXShaderManager is not NULL only if created by this class
	SAFE_DELETE( m_pXShaderManager );

	// make sure surfaces aren't the current render targets 
	SetRenderTargetsToDefault();

	FreeTextureRenderTargets();

	SAFE_RELEASE( m_pDefaultBackbufferColor );
	SAFE_RELEASE( m_pDefaultBackbufferDepth ); 

	SAFE_RELEASE( m_pD3DDev );

	SetToNull();
	return( hr );
}





HRESULT RenderToTextureBase::Initialize( IDirect3DDevice9 * pDev,
										ShaderManager ** ppShaderManager,
										int width, int height,
										int num_surfaces,
										bool create_depth_buffer )
{

	// If pShaderManager is NULL, a new one will be created
	HRESULT hr = S_OK;

	RenderToTextureBase::Free();

	FAIL_IF_NULL( pDev );

	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();


	// remember the current back buffer color & depth as the
	//  default back buffers
	GrabCurrentBackbuffersAsDefault();


	bool create_shadermanager = false;
	if( ppShaderManager == NULL )
	{
		create_shadermanager = true;
	}
	else if( *ppShaderManager == NULL )
	{
		create_shadermanager = true;
	}

	if( create_shadermanager )
	{
		m_pXShaderManager = new ShaderManager();
		FAIL_IF_NULL( m_pXShaderManager );

		m_pXShaderManager->Initialize( m_pD3DDev );
		m_ppShaderManager = & m_pXShaderManager;
	}
	else
	{
		m_ppShaderManager = ppShaderManager;
		SAFE_DELETE( m_pXShaderManager );
	}


	CreateTextureRenderTargets( width, height, num_surfaces, create_depth_buffer );

	return( hr );
}


void RenderToTextureBase::FreeTextureRenderTargets()
{
	int i;

	SetRenderTargetsToDefault();

	if( m_pD3DDev != NULL )
	{
		int nstages;

//@@@@ check caps for # texture stages
		nstages = 4;

		for( i=0; i < nstages; i++ )
		{
			m_pD3DDev->SetTexture( i, NULL );
		}
	}

	if( m_ppRTTSurface != NULL && m_ppRTTTexture != NULL )
	{
		for( i=0; i < m_nNumSurfaces; i++ )
		{
			SAFE_RELEASE( m_ppRTTTexture[i] );
			SAFE_RELEASE( m_ppRTTSurface[i] );
		}

		SAFE_ARRAY_DELETE( m_ppRTTTexture );
		SAFE_ARRAY_DELETE( m_ppRTTSurface );
	}

	SAFE_RELEASE( m_pRTTDepthBuffer );

	m_nNumSurfaces = 0;
}


HRESULT	RenderToTextureBase::CreateTextureRenderTargets( int width,
														int height,
														int num_surfaces,
														bool create_depth_buffer,
														bool use_stencil )
{
	// if bool create_depth_buffer, then it creates a single depth buffer
	//  to share among the render targets
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	CHECK_BOUNDS_HR( width,			1, 4096 );
	CHECK_BOUNDS_HR( height,		1, 4096 );
	CHECK_BOUNDS_HR( num_surfaces,	1, 1000 );	// you're fired if you create 1000 textures

	FreeTextureRenderTargets();

	m_ppRTTTexture = new LPDIRECT3DTEXTURE9[ num_surfaces ]; 
	FAIL_IF_NULL( m_ppRTTTexture );

	m_ppRTTSurface = new LPDIRECT3DSURFACE9[ num_surfaces ];
	FAIL_IF_NULL( m_ppRTTSurface );

    // Create textures and grab their surface pointers
	int i;
    for( i = 0; i < num_surfaces; i++ )
    {
        hr = m_pD3DDev->CreateTexture( width, height, 1, 
                                      D3DUSAGE_RENDERTARGET,
									  D3DFMT_A8R8G8B8, 
                                      D3DPOOL_DEFAULT,
									  & m_ppRTTTexture[i],
									  NULL );
        if (FAILED(hr))
        {
			FDebug("Can't CreateTexture!\n");
            assert(false);
            return E_FAIL;
        }

        hr = m_ppRTTTexture[i]->GetSurfaceLevel(0, & m_ppRTTSurface[i]);
        if (FAILED(hr))
        {
			FDebug("Can't Get to top-level texture!\n");
            assert(false);
            return E_FAIL;
        }
    }


	//////////////////////////////////////////////////
	// Create a depth buffer to use with the textures

	if( create_depth_buffer )
	{
		if( use_stencil )
		{
			m_DepthFormat = D3DFMT_D24S8;
			m_dwDepthClearFlags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
		}
		else
		{
			m_DepthFormat = D3DFMT_D24X8;
			m_dwDepthClearFlags = D3DCLEAR_ZBUFFER;
		}

		hr = m_pD3DDev->CreateDepthStencilSurface( width, height,
									m_DepthFormat,
									D3DMULTISAMPLE_NONE,
									0,				// multisample qual
									TRUE,			// discard
									& m_pRTTDepthBuffer,
									NULL );
		if (FAILED(hr))
		{
			FDebug("Can't CreateDepthStencilSurface()!\n");
			assert(false);
			return E_FAIL;
		}
	}


	m_nRTTWidth		= width;
	m_nRTTHeight	= height;
	m_nNumSurfaces	= num_surfaces;


	return( hr );
}



HRESULT	RenderToTextureBase::GrabCurrentBackbuffersAsDefault()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pDefaultBackbufferColor );
	SAFE_RELEASE( m_pDefaultBackbufferDepth );
	// Get..() increments ref counts
	hr = m_pD3DDev->GetRenderTarget( 0,	&m_pDefaultBackbufferColor );
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	hr = m_pD3DDev->GetDepthStencilSurface( &m_pDefaultBackbufferDepth );
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	return( hr );
}

HRESULT	RenderToTextureBase::SetDefaultBuffersToThese(	IDirect3DSurface9 * pColor,
														IDirect3DSurface9 * pDepth )
{
	SAFE_RELEASE( m_pDefaultBackbufferColor );
	if( pColor != NULL )
	{
		pColor->AddRef();
	}
	m_pDefaultBackbufferColor = pColor;

	SAFE_RELEASE( m_pDefaultBackbufferDepth );
	if( pDepth != NULL )
	{
		pDepth->AddRef();
	}
	m_pDefaultBackbufferDepth = pDepth;

	return( S_OK );
}

void	RenderToTextureBase::GetDefaultBackbuffers( IDirect3DSurface9 ** ppColor,
													IDirect3DSurface9 ** ppDepth )
{
	*ppColor = m_pDefaultBackbufferColor;
	*ppDepth = m_pDefaultBackbufferDepth;
}



HRESULT	RenderToTextureBase::SetRenderTargetsToDefault( bool set_color,
														bool set_depth )
{
	// if arg is false, corresponding target is set to NULL

	HRESULT hr = E_FAIL;
	if( m_pD3DDev != NULL )
	{
		IDirect3DSurface9 * pColor = NULL;
		IDirect3DSurface9 * pDepth = NULL;

		if( set_color )
			pColor = m_pDefaultBackbufferColor;
		if( set_depth )
			pDepth = m_pDefaultBackbufferDepth;
		hr = m_pD3DDev->SetRenderTarget( 0,	pColor );
		hr = m_pD3DDev->SetDepthStencilSurface( m_pDefaultBackbufferDepth );
	}
	return( hr );
}


HRESULT	RenderToTextureBase::SetRenderTargetsToDefault()
{
	HRESULT hr;
	hr = SetRenderTargetsToDefault( true, true );
	return( hr );
}


HRESULT		RenderToTextureBase::SetRenderTargets( int color_index, int depth_index )
{
	// use index = -1 to set to NULL
	HRESULT hr = E_FAIL;
	IDirect3DSurface9 * pColor = NULL;
	IDirect3DSurface9 * pDepth = NULL;
	GetTexAndSurf( color_index, NULL, &pColor );

	if( depth_index >= 0 && depth_index < GetNumSurfaces() )
	{
		pDepth = m_pRTTDepthBuffer;
	}
	hr = m_pD3DDev->SetRenderTarget( 0, pColor );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
	hr = m_pD3DDev->SetDepthStencilSurface( pDepth );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
	return( hr );
}


