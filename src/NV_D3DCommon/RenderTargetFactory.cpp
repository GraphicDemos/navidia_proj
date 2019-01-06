/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  RenderTargetFactory.cpp

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

RenderTargetDesc::RenderTargetDesc( IDirect3DSurface9 ** ppSurf )
{
	SetAllNull();
	if( ppSurf == NULL )
		return;
	if( *ppSurf == NULL )
		return;

	D3DSURFACE_DESC desc;
	HRESULT hr;

	hr = (*ppSurf)->GetDesc( & desc );
	if( FAILED(hr) )
	{
		FDebug("Couldn't get surface desc!\n");
		return;
	}	

	m_nWidth			= desc.Width;
	m_nHeight			= desc.Height;
	m_ppRTTSurface		= ppSurf;
	m_ppRTTTexture		= NULL;
	m_SurfaceFormat		= desc.Format;

	// m_pD3DDev is not addref'd, so do this cheesy Release()
	hr = (*ppSurf)->GetDevice( & m_pD3DDev );
	if( FAILED(hr) )
	{
		FDebug("Couldn't get depth surface's device!\n");
		return;
	}
	m_pD3DDev->Release();		// keep the pointer value though!

}



///////////////////////////////////////////////////////

RenderTargetDepthDesc::RenderTargetDepthDesc( IDirect3DSurface9 ** ppSurf )
{
	SetAllNull();
	if( ppSurf == NULL )
		return;
	if( *ppSurf == NULL )
		return;
	
	D3DSURFACE_DESC desc;
	HRESULT hr;
	hr = (*ppSurf)->GetDesc( & desc );
	if( FAILED(hr) )
	{
		FDebug("Couldn't get surface desc!\n");
		return;
	}	

	m_nWidth			= desc.Width;
	m_nHeight			= desc.Height;
	m_ppRTTDepthBuffer	= ppSurf;
	m_DepthFormat		= desc.Format;

	m_dwDepthClearFlags	= GetDepthClearFlags( m_DepthFormat );

	// m_pD3DDev is not addref'd, so do this cheesy Release()
	hr = (*ppSurf)->GetDevice( & m_pD3DDev );
	if( FAILED(hr) )
	{
		FDebug("Couldn't get depth surface's device!\n");
		return;
	}
	m_pD3DDev->Release();		// keep the pointer value though!

}


DWORD RenderTargetDepthDesc::GetDepthClearFlags( DWORD format )
{
	DWORD surf_clear_flags;

	switch( format )
	{
	case D3DFMT_D24S8 :
	case D3DFMT_D15S1 :
	case D3DFMT_D24FS8 :
		surf_clear_flags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
		break;

	case D3DFMT_D24X8 :
	case D3DFMT_D16_LOCKABLE :
	case D3DFMT_D32 :
	case D3DFMT_D16 :
	case D3DFMT_D32F_LOCKABLE :
		surf_clear_flags = D3DCLEAR_ZBUFFER;
		break;

	default :
		FMsg("RenderTargetDepthDesc::GetDepthClearFlags() FAILED : Unrecognized depth-stencil surface format! %u\n", format );  
		assert( false );
		surf_clear_flags = 0;
		break;
	}

	return( surf_clear_flags );
}

//------------------------------------------------------------------------------

HRESULT	RenderTargetSet::SetAsCurrent( bool bSetColor, bool bSetDepth )
{
	HRESULT hr = S_OK;
	if( bSetColor == true )
	{
		FAIL_IF_NULL( m_ppColorTarget );
		FAIL_IF_NULL( *m_ppColorTarget );
		FAIL_IF_NULL( (*m_ppColorTarget)->m_pD3DDev );
		hr = (*m_ppColorTarget)->m_pD3DDev->SetRenderTarget( 0, (*m_ppColorTarget)->GetSurfaceP() );
		DBG_ONLY( MSG_AND_RET_VAL_IF( FAILED(hr), "RenderTargetSet::SetAsCurrent failed to set color\n", hr ));
	}
	if( bSetDepth == true )
	{
		FAIL_IF_NULL( m_ppDepthTarget );
		FAIL_IF_NULL( *m_ppDepthTarget );
		FAIL_IF_NULL( (*m_ppDepthTarget)->m_pD3DDev );
		hr = (*m_ppDepthTarget)->m_pD3DDev->SetDepthStencilSurface( (*m_ppDepthTarget)->GetSurfaceP() );
		DBG_ONLY( MSG_AND_RET_VAL_IF( FAILED(hr), "RenderTargetSet::SetAsCurrent failed to set depthstencil\n", hr ));
	}
	return( hr );
}
//------------------------------------------------------------------------------


RenderTargetFactory::RenderTargetFactory()
{
	SetAllNull();	// called by virtual base 
}

RenderTargetFactory::~RenderTargetFactory()
{
	Free();			// 
	SetAllNull();	// called by virtual base
}


HRESULT RenderTargetFactory::Free()
{
	HRESULT hr = S_OK;
	hr = TextureFactory::Free();
	
	UINT i;

	// delete description objects, setting pointer to NULL
	// delete pointers, setting the handle to NULL
	RenderTargetDesc	** ppDesc;
	for( i=0; i < m_vppRTDTexSurfPairs.size(); i++ )
	{
		ppDesc = m_vppRTDTexSurfPairs.at(i);

		if( ppDesc != NULL )
		{
			ppDesc = & (*ppDesc);
			SAFE_DELETE( (*ppDesc) );	// delete the description object, setting pointer to NULL
			SAFE_DELETE( ppDesc );		// delete the pointer
		}
		m_vppRTDTexSurfPairs.at(i) = NULL;
	}
	m_vppRTDTexSurfPairs.clear();


	// release depth surfaces
	// and delete pointers to surfaces
	// delete depth surface descriptions
	RenderTargetDepthDesc	*  pDDesc;
	RenderTargetDepthDesc	** ppDDesc;
	for( i=0; i < m_vppRTDDepthSurfaces.size(); i++ )
	{
		ppDDesc = m_vppRTDDepthSurfaces.at(i);

		if( ppDDesc != NULL )
		{
			pDDesc = *ppDDesc;
			if( pDDesc != NULL )
			{
				if( pDDesc->m_ppRTTDepthBuffer != NULL )
				{
					// release the surface
					// set pointer to NULL
					SAFE_RELEASE( *(pDDesc->m_ppRTTDepthBuffer) );
					// delete the pointer to the surface
					SAFE_DELETE( pDDesc->m_ppRTTDepthBuffer );
				}
			}
			SAFE_DELETE( *ppDDesc );	// delete description object
			SAFE_DELETE( ppDDesc );		// delete pointer to desc object

			m_vppRTDDepthSurfaces.at(i) = NULL;
		}
	}

	return( hr );
}


HRESULT RenderTargetFactory::Initialize( GetFilePath::GetFilePathFunction file_path_callback )
{
	HRESULT hr = S_OK;

	hr = TextureFactory::Initialize( file_path_callback );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	return( hr );
}


void	RenderTargetFactory::DbgTextRenderTargetDesc( RenderTargetDesc ** ppCol )
{
	char ind[32] = "  ";

	if( ppCol == NULL )
	{
		FMsg("%sColor desc handle is NULL\n", ind );
		return;
	}
	if( *ppCol == NULL )
	{
		FMsg("%sColor desc pointer is NULL\n", ind );
		return;	
	}

	FMsg("%sm_nWidth        = %d\n", ind,	(*ppCol)->m_nWidth );
	FMsg("%sm_nHeight       = %d\n", ind,	(*ppCol)->m_nHeight );
	FMsg("%sm_SurfaceFormat = 0x%8x\n", ind,	(*ppCol)->m_SurfaceFormat );
	FMsg("%sm_ppRTTSurface  = 0x%8x\n", ind,	(*ppCol)->m_ppRTTSurface );
	FMsg("%sm_ppRTTTexture  = 0x%8x\n", ind,	(*ppCol)->m_ppRTTTexture );
	FMsg("%sm_pD3DDev       = 0x%8x\n", ind,	(*ppCol)->m_pD3DDev );
}

void	RenderTargetFactory::DbgTextRenderTargetDesc( RenderTargetDepthDesc ** ppDepth )
{
	char ind[32] = "  ";

	if( ppDepth == NULL )
	{
		FMsg("%sDepth desc handle is NULL\n", ind );
		return;
	}
	if( *ppDepth == NULL )
	{
		FMsg("%sDepth desc pointer is NULL\n", ind );
		return;	
	}

	FMsg("%sm_nWidth            = %d\n", ind,	(*ppDepth)->m_nWidth );
	FMsg("%sm_nHeight           = %d\n", ind,	(*ppDepth)->m_nHeight );
	FMsg("%sm_DepthFormat       = 0x%8x\n", ind,	(*ppDepth)->m_DepthFormat );
	FMsg("%sm_dwDepthClearFlags = 0x%8x\n", ind,   (*ppDepth)->m_dwDepthClearFlags );
	FMsg("%sm_ppRTTDepthBuffer  = 0x%8x\n", ind,	(*ppDepth)->m_ppRTTDepthBuffer );
	FMsg("%sm_pD3DDev           = 0x%8x\n", ind,	(*ppDepth)->m_pD3DDev );
}



RenderTargetDesc **	RenderTargetFactory::GetCurrentColorTarget( IDirect3DDevice9 * pDev )
{
	// A new RenderTargetDesc is created each time this is called

	RET_NULL_IF_NULL( pDev );
	HRESULT hr;

	// Create surface pointer
	IDirect3DSurface9	** ppSurf = new (IDirect3DSurface9*);
	RET_NULL_IF_NULL( ppSurf );

	// Get device's current color target
	hr = pDev->GetRenderTarget( 0, ppSurf );
	if( FAILED(hr) )
	{
		delete ppSurf;
		FDebug("Couldn't get color render target!\n");
		assert( false );
		return( NULL );
	}

	// make a pointer to a surface description
	RenderTargetDesc ** ppDesc = new (RenderTargetDesc*);
	RET_NULL_IF_NULL( ppDesc );

	// Make a surface description
	// Information is taken from the surface pointer, if it exists
	*ppDesc = new RenderTargetDesc( ppSurf );
	if( *ppDesc == NULL )
	{
		*ppSurf = NULL;
		delete ppSurf;
		SAFE_DELETE( ppDesc );
		return( NULL );
	}

	m_vppRTDTexSurfPairs.push_back( ppDesc );

	// Add surface to the list of surface pointers to be released and deleted
	AddSurfacePtrToDelete( ppSurf );
	
	return( ppDesc );
}

RenderTargetDepthDesc ** RenderTargetFactory::GetCurrentDepthTarget( IDirect3DDevice9 * pDev )
{
	// A new RenderTargetDepthDesc is created each time this is called

	RET_NULL_IF_NULL( pDev );

	// create a surface pointer
	IDirect3DSurface9	** ppSurf = new (IDirect3DSurface9*);
	RET_NULL_IF_NULL( ppSurf );

	// Get the device's current target.
	// This will increment the refcount, requiring a Release() later

	HRESULT hr;
	hr = pDev->GetDepthStencilSurface( ppSurf );
	if( FAILED(hr) )
	{
		*ppSurf = NULL;
		delete ppSurf;
		return( NULL );
	}

	// make a pointer to a surface description
	RenderTargetDepthDesc ** ppDesc = new (RenderTargetDepthDesc*);
	RET_NULL_IF_NULL( ppDesc );

	// Make a surface description
	// Information is taken from the surface pointer, if it exists
	*ppDesc = new RenderTargetDepthDesc( ppSurf );
	if( *ppDesc == NULL )
	{
		*ppSurf = NULL;
		delete ppSurf;
		SAFE_DELETE( ppDesc );
		return( NULL );
	}

	m_vppRTDDepthSurfaces.push_back( ppDesc );

//	FMsg("\n******************** GetCurrentDepthTarget\n");
//	DbgTextRenderTargetDesc( ppDesc );
	
	return( ppDesc );
}


RenderTargetSet	RenderTargetFactory::GetCurrentTargets( IDirect3DDevice9	* pDev )
{
	// New target descriptions are allocated every time you call this
	
	RenderTargetSet	target_set;
	target_set.m_ppColorTarget = GetCurrentColorTarget( pDev );
	target_set.m_ppDepthTarget	= GetCurrentDepthTarget( pDev );

	return( target_set );
}


	// Sets the device's render targets to those pointed to in the descriptions
HRESULT	RenderTargetFactory::SetRenderTargets( IDirect3DDevice9 * pDev,
												RenderTargetDesc ** ppColorDesc,
												RenderTargetDepthDesc ** ppDepthDesc,
												bool bVerbose )
{
	// Fail if the set render target fails, but not if
	//  either pointer is NULL
	FAIL_IF_NULL( pDev );
	HRESULT hr = S_OK;

	if( bVerbose )
	{
		FMsg("RenderTargetFactory::SetRenderTargets(..)\n");

		FMsg("Attempting to set color target:\n");
		DbgTextRenderTargetDesc( ppColorDesc );
	}

	if( ppColorDesc == NULL )
		goto TryDepth;
	if( *ppColorDesc == NULL )
		goto TryDepth;
	if( (*ppColorDesc)->m_ppRTTSurface == NULL )
		goto TryDepth;
	IDirect3DSurface9 * pSurf;
	pSurf = *((*ppColorDesc)->m_ppRTTSurface);
	if( pSurf == NULL )
		goto TryDepth;

	hr = pDev->SetRenderTarget( 0, pSurf ); 
	BREAK_AND_RET_VAL_IF_FAILED(hr);

TryDepth:
	if( bVerbose )
	{
		FMsg("Attempting to set depth target:\n");
		DbgTextRenderTargetDesc( ppDepthDesc );
	}

	if( ppDepthDesc == NULL )
		return( hr );
	if( *ppDepthDesc == NULL )
		return( hr );
	if( (*ppDepthDesc)->m_ppRTTDepthBuffer == NULL )
		return( hr );
	pSurf = *((*ppDepthDesc)->m_ppRTTDepthBuffer);
	if( pSurf == NULL )
		return( hr );

#ifdef NV_USING_D3D9
	hr = pDev->SetDepthStencilSurface( pSurf );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
#endif
#ifdef NV_USING_D3D8
	hr = pDev->SetRenderTarget( NULL, pSurf );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
#endif

	return( hr );
}




RenderTargetDesc ** RenderTargetFactory::CreateRenderTarget( IDirect3DDevice9 * pDev,
																int width, int height,
																D3DFORMAT format )
{
	RET_NULL_IF_NULL( pDev );

	IDirect3DTexture9 ** ppTex;
	ppTex = CreateTexture( pDev, width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT );

	RET_NULL_IF_NULL( ppTex );
	RET_NULL_IF_NULL( *ppTex );

	// Create pointer to a description object
	RenderTargetDesc ** ppDesc = new (RenderTargetDesc*);	// allocate pointer
	RET_NULL_IF_NULL( ppDesc );

	*ppDesc = new RenderTargetDesc;		// allocate description class
	if( *ppDesc == NULL )
	{
		SAFE_DELETE( ppDesc );	// delete pointer
		return( NULL );
	}

	RenderTargetDesc * pDesc;
	pDesc = *ppDesc;

	m_vppRTDTexSurfPairs.push_back( ppDesc );

	pDesc->m_nWidth			= width;
	pDesc->m_nHeight		= height;
	pDesc->m_pD3DDev		= pDev;
	pDesc->m_ppRTTTexture	= ppTex;
	pDesc->m_ppRTTSurface	= GetTextureSurface( *ppTex, 0 );
	pDesc->m_SurfaceFormat	= format;

//	IDirect3DSurface9 ** ppSurf;
//	ppSurf = GetTextureSurface( *ppTex, 0 );
//	FMsg("ppSurf  = 0x%8x\n", ppSurf );
//	FMsg("*ppSurf = 0x%8x\n", *ppSurf );

	if( pDesc->m_ppRTTSurface == NULL )
	{
		SAFE_DELETE( *ppDesc );
		SAFE_DELETE( ppDesc );
		return( NULL );
	}
	
	return( ppDesc );
}

RenderTargetDepthDesc ** RenderTargetFactory::CreateDepthTarget( IDirect3DDevice9 * pDev,
																int width, int height,
																D3DFORMAT format )
{
	RET_NULL_IF_NULL( pDev );
	HRESULT hr = S_OK;

	DWORD surf_clear_flags;

//@@@@ this code is duplicated - put it in D3DDeviceAndHWInfo ??

	switch( format )
	{
	case D3DFMT_D24S8 :
	case D3DFMT_D15S1 :
	case D3DFMT_D24FS8 :
		surf_clear_flags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
		break;

	case D3DFMT_D24X8 :
	case D3DFMT_D16_LOCKABLE :
	case D3DFMT_D32 :
	case D3DFMT_D16 :
#ifdef NV_USING_D3D9
	case D3DFMT_D32F_LOCKABLE :
#endif

		surf_clear_flags = D3DCLEAR_ZBUFFER;
		break;

	default :
		FMsg("RenderTargetFactory::CreateDepthTarget() FAILED : Unrecognized depth-stencil surface format! %u\n", format );  
		assert( false );
		return( NULL );
		break;
	}

	// create a surface pointer and allocate the surface

	IDirect3DSurface9	** ppSurf = new (IDirect3DSurface9*);
	RET_NULL_IF_NULL( ppSurf );

	hr = pDev->CreateDepthStencilSurface( width, height,
								format,
								D3DMULTISAMPLE_NONE,
								0,				// multisample qual
								TRUE,			// discard //@@@ option to keep around?
								ppSurf,
								NULL );
	if( FAILED(hr) )
	{
		FDebug("RenderTargetFactory:: Can't CreateDepthStencilSurface()!\n");
		assert(false);
		return( NULL );
	}

	// make a pointer to a surface description
	RenderTargetDepthDesc ** ppDesc = new (RenderTargetDepthDesc*);
	RET_NULL_IF_NULL( ppDesc );

	// make a surface description
	*ppDesc = new RenderTargetDepthDesc;
	if( *ppDesc == NULL )
	{
		SAFE_DELETE( ppDesc );
		return( NULL );
	}

	RenderTargetDepthDesc * pDesc = *ppDesc;

	m_vppRTDDepthSurfaces.push_back( ppDesc );

	pDesc->m_nWidth				= width;
	pDesc->m_nHeight			= height;
	pDesc->m_pD3DDev			= pDev;
	pDesc->m_ppRTTDepthBuffer	= ppSurf;
	pDesc->m_dwDepthClearFlags	= surf_clear_flags;
	pDesc->m_DepthFormat		= format;

	return( ppDesc );
}

