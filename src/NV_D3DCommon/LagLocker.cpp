/*---------------------------------------------------------------------------------
Path: SDK\LIBS\src\NV_D3DCommon
File: LagLocker.cpp

See LagLocker.h for comments

---------------------------------------------------------------------------------*/

#include <stdio.h>						// for sprintf() for error reporting
#include <assert.h>
#include <tchar.h>
#include "NV_D3DCommon\LagLocker.h"


//---------------------------------------------------------------------------------

#define LMSG( m )	OutputDebugString( TEXT(m) )			// LMSG always outputs text for DEBUG and RELEASE builds
															// LDEBUG outputs text only for DEBUG builds
#ifdef _DEBUG
	#define LDEBUG	OutputDebugString
#else
	#define LDEBUG	LLNullMsgFunc				// not a DEBUG build, so use the nothing function
#endif

// #define TRACEMSG0		LMsg		// define this to LMsg in order to output text
#define TRACEMSG0		NullMsg		// or define to NullMsg to compile to nothing


//----------------------------------------------------------------------------------

inline void LLNullMsgFunc( TCHAR * p )		{ *p; }		// func that does nothing

void LMsg( TCHAR * szFormat, ... )
{	
	TCHAR buffer[2048];
	va_list args;
	va_start( args, szFormat );
	_vsntprintf( buffer, 2048, szFormat, args );
	va_end( args );
	buffer[2048-1] = '\0';			// terminate in case of overflow
	OutputDebugString ( buffer );
}

inline void NullMsg( TCHAR * szFormat, ... )	{ *szFormat; };


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if( (p) != NULL ) { (p)->Release(); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE( p )  { if((p) != NULL ) { delete(p); (p) = NULL; } }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(p)  { if(p) { delete [] (p);  p=NULL; } }
#endif

#ifndef FAIL_IF_NULL
#define FAIL_IF_NULL( x )	{ if( (x)==NULL ) { return(E_FAIL); } }
#endif

#ifndef MSG_AND_RET_VAL_IF
#define MSG_AND_RET_VAL_IF( expr, msg, retval )			\
{														\
	if( expr )	{ LMSG(msg); return( retval ); }		\
}
#endif


//-------------------------------------------------------------------------------


HRESULT LagLocker::Free()
{
	HRESULT hr = S_OK;
	FreeSurfaces();
	SAFE_RELEASE( m_pD3DDev );
	m_Mode = UNINITIALIZED;
	return( hr );
}

// You can Initialize( .., N) to sync after N frames (up to 3 is all you should ever need), but then
//  call SetNumFramesToSyncAfter() to set the nubmer of frames to any value less than N.  Going to lower
//  numbers will cause more immediate locks (not more frequent locks) and may have a greater impact on performance.
HRESULT	LagLocker::Initialize( IDirect3DDevice9 * pDev, LagLockerMode mode, int max_number_of_frames_to_sync_after )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	if( max_number_of_frames_to_sync_after > 3 )
		max_number_of_frames_to_sync_after = 3;
	m_nInitLockInterval	= max_number_of_frames_to_sync_after;
	m_nLockInterval		= max_number_of_frames_to_sync_after;
	m_bEnabled			= true;

	hr = SetMode( mode );
	return( hr );
}


HRESULT	LagLocker::SetMode( LagLockerMode mode )
{
	HRESULT hr = S_OK;

	// If we're switching to a different mode, free thing we no longer need from the old mode
	if( mode != m_Mode )
	{
		if( m_Mode == TEXTURE_LOCK )
		{
			FreeSurfaces();
		}
	}

	switch( mode )
	{
	case TEXTURE_LOCK :
		hr = CreateSurfaces( m_pD3DDev, m_nInitLockInterval + 1 );		// syn after 0 frames needs 1 texture
		m_Mode = mode;	
		break;

	case EVENT_QUERY :
		LMsg(TEXT("SetMode() EVENT_QUERRY mode not yet supported!\n"));
		assert( false );
		m_Mode = UNINITIALIZED;
		return( E_FAIL );
		break;
	case NVCPL :
		LMsg(TEXT("SetMode() NVCPL mode not yet supported!\n"));
		assert( false );
		m_Mode = UNINITIALIZED;
		return( E_FAIL );
		break;
	}

	return( hr );
}

LagLocker::LagLockerMode LagLocker::GetMode()
{
	return( m_Mode );
}

void LagLocker::Enable()
{
	m_bEnabled = true;
}
void LagLocker::Disable()
{
	m_bEnabled = false;
}


HRESULT LagLocker::OnDeviceLost()
{
	LMsg(TEXT("LagLocker::OnDeviceLost() not yet implemented!  Please just destroy and re-create the LagLocker\n"));
	assert( false );
	return( E_FAIL );
}


// sync_after_n_frames must be <= the sync_after_n_frames supplied to Initialize
// 0 causes a lock after each frame rendered, allowing no frames to be buffered by the driver
// 1 causes a lock one frame after each frame rendered, allowing up to 1 frame to be buffered by the driver
HRESULT	LagLocker::SetNumFramesToSyncAfter( int sync_after_n_frames )
{
	HRESULT hr = S_OK;
	if( sync_after_n_frames >= m_nNumSurfaces )
	{
		LMSG("LagLocker: you must set num frames to less than the number supplied to Initialize\n");
		return(E_FAIL);
	}
	m_nLockInterval = sync_after_n_frames;
	ResetLockCount();
	return( hr );
}

void LagLocker::ResetLockCount()
{
	m_nSurfToUpdate = 0;
	m_nSurfToLock	= m_nLockInterval;
}

void LagLocker::IncrementLockCount()
{
	int tmp;
	switch( m_nLockInterval )
	{
	case 0 :
		m_nSurfToUpdate = m_nSurfToLock;
		break;
	case 1 :
		tmp = m_nSurfToUpdate;
		m_nSurfToUpdate = m_nSurfToLock;
		m_nSurfToLock	= tmp;
		break;
	default :
		m_nSurfToUpdate = (m_nSurfToUpdate+1) % m_nNumSurfaces;
		m_nSurfToLock	= (m_nSurfToLock+1) % m_nNumSurfaces;
		break;
	}
}


HRESULT LagLocker::CreateSurfaces( IDirect3DDevice9 * pDev, int num_surf )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	hr = FreeSurfaces();
	MSG_AND_RET_VAL_IF( FAILED(hr), "CreateSurfaces() FreeSurfaces() failed!\n", hr );

	if( num_surf < 0 )
		return( E_FAIL );
	if( num_surf == 0 )
		return( S_OK );
	if( num_surf > 5 )
	{
		LMSG("LagLocker() more than 5 surfaces requested.  I doubt this is correct.  Failing!\n");
		return( E_FAIL );
	}

	m_nNumSurfaces = num_surf;
	m_pTexSurfPairs = new TexSurfPair[ num_surf ];
	FAIL_IF_NULL( m_pTexSurfPairs );

	D3DFORMAT	format;
	const int surf_resolution = 16;
	format = D3DFMT_A8R8G8B8;

	int i;
	for( i=0; i < num_surf; i++ )
	{
		m_pTexSurfPairs[i].SetToNull();

		// Create a render-target texture to use in forcing synchronization between CPU and GPU
		hr = m_pD3DDev->CreateTexture( surf_resolution, surf_resolution, 1, 
										D3DUSAGE_RENDERTARGET,
										format,
										D3DPOOL_DEFAULT, 
										&(m_pTexSurfPairs[i].m_pRTTTexture), NULL );
		if( FAILED(hr) )
		{
			TCHAR buf[200];
			_stprintf( buf, TEXT("LagLocker: Couldn't create texture %d\n"), i );
			OutputDebugString( buf );
			m_pTexSurfPairs[i].m_pRTTTexture = NULL;
			assert( false );
			return( E_FAIL );
		}

		// Get the surface associated with the render target texture
		hr = m_pTexSurfPairs[i].m_pRTTTexture->GetSurfaceLevel( 0, &(m_pTexSurfPairs[i].m_pRTTSurface) );
		MSG_AND_RET_VAL_IF( FAILED(hr), "LagLocker:  Couldn't get surface from the texture!\n", hr );
		FAIL_IF_NULL( m_pTexSurfPairs[i].m_pRTTSurface );

		// Create a system memory texture that the render-target texture will be copied to 
		//  in order to force synchronization
		hr = m_pD3DDev->CreateTexture( surf_resolution, surf_resolution, 1, D3DUSAGE_DYNAMIC, 
											format, D3DPOOL_SYSTEMMEM, 
											&(m_pTexSurfPairs[i].m_pDynTexture), NULL );
		
		hr = m_pTexSurfPairs[i].m_pDynTexture->GetSurfaceLevel( 0, &(m_pTexSurfPairs[i].m_pDynSurface ) );
		MSG_AND_RET_VAL_IF( FAILED(hr), "LagLocker:  Couldn't get surface from the texture!\n", hr );
		FAIL_IF_NULL( m_pTexSurfPairs[i].m_pDynSurface );
	}

	ResetLockCount();
	return( hr );
}

HRESULT LagLocker::FreeSurfaces()
{
	HRESULT hr = S_OK;
	int i;
	for( i=0; i < m_nNumSurfaces; i++ )
	{
		SAFE_RELEASE( m_pTexSurfPairs[i].m_pDynSurface );
		SAFE_RELEASE( m_pTexSurfPairs[i].m_pDynTexture );
		SAFE_RELEASE( m_pTexSurfPairs[i].m_pRTTSurface );
		SAFE_RELEASE( m_pTexSurfPairs[i].m_pRTTTexture );
	}
	SAFE_ARRAY_DELETE( m_pTexSurfPairs );
	return( hr );
}


// Fills out m_b<>ModeSupported variables according to capabilities of the device
HRESULT LagLocker::FindSupportedModes( IDirect3DDevice9 * pDev )
{
	FMsg(TEXT("LagLocker::FindSupportedModes NOT IMPLEMENTED"));
	return( E_FAIL );
	HRESULT hr = S_OK;
	m_bTextureModeSupported			= true;
	m_bNVCPLModeSupported			= false;
	m_bEventQuerryModeSupported		= false;
	FAIL_IF_NULL( pDev );

	D3DCAPS9 caps;
	pDev->GetDeviceCaps( &caps );
	return( hr );
}

// Call this after BeginScene() to let LagLocker know you are starting to render
HRESULT	LagLocker::BeginFrame()
{
	HRESULT hr = S_OK;
	if( m_bEnabled )
	{
		if( m_Mode == TEXTURE_LOCK )
		{
			hr = BeginFrame_Texture();
		}
		else if( m_Mode == UNINITIALIZED )
		{
			// do nothing
		}
		else
		{
			LMSG("LagLocker::BeginFrame() m_Mode not supported!\n");
			return( S_OK );
		}
	}

	return( hr );
}

// Call this before EndScene() to let LagLocker know that you are done rendering your frame
HRESULT	LagLocker::EndFrame()
{
	HRESULT hr = S_OK;
	if( m_bEnabled )
	{
		if( m_Mode == TEXTURE_LOCK )
		{
			hr = EndFrame_Texture();
			IncrementLockCount();
		}
		else if( m_Mode == UNINITIALIZED )
		{
			// do nothing
		}
		else
		{
			LMSG("LagLocker::EndFrame() m_Mode not supported!\n");
			return( S_OK );
		}
	}
	return( hr );
}


HRESULT LagLocker::BeginFrame_Texture()
{
	HRESULT hr = S_OK;

	IDirect3DSurface9	*	surf_to_update;
	surf_to_update = m_pTexSurfPairs[m_nSurfToUpdate].m_pRTTSurface;
	FAIL_IF_NULL( surf_to_update );

	DWORD color = rand();

	// update the surface so the driver marks it as changed
	hr = m_pD3DDev->ColorFill( surf_to_update, NULL, color );	// ARGB
	MSG_AND_RET_VAL_IF( FAILED(hr), "LagLocker::BeginFrame_Texture() Couldn't fill surface!\n", hr );
	TRACEMSG0(TEXT("wrote val to surf[%d] :     %u\n"), m_nSurfToUpdate, color );

	return( hr );
}


//---------------------------------------------------------------------
// Forces synchronization after a certain number of frames have passed
// It does this by reading a GPU texture back to the CPU
//
//---------------------------------------------------------------------
HRESULT LagLocker::EndFrame_Texture()
{
	HRESULT hr = S_OK;
	IDirect3DSurface9 *	pRTTSource;
	IDirect3DSurface9	*	pDynDest;

	pRTTSource	= m_pTexSurfPairs[m_nSurfToLock].m_pRTTSurface;
	pDynDest	= m_pTexSurfPairs[m_nSurfToLock].m_pDynSurface;

	// Copy the rendertarget into the system memory surface
	hr = m_pD3DDev->GetRenderTargetData( pRTTSource, pDynDest );		// ..( renderTarget, destSurf ) 
	MSG_AND_RET_VAL_IF( FAILED(hr), "LagLocker::EndFrame_Texture() Couldn't GetRenderTargetData(..)!\n", hr );

	return( hr );
}
