/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceAndHWInfo.cpp

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
#include <shared/UtilityFunctions.h>

#define SHADER_VERSION_UNSET	-1.0f

D3DDeviceAndHWInfo::D3DDeviceAndHWInfo()
{
	SetToNull();
}

D3DDeviceAndHWInfo::~D3DDeviceAndHWInfo()
{
	Free();
}

/////////////////////////////////////////

void D3DDeviceAndHWInfo::SetToNull()
{
	m_pD3DDev = NULL;
	m_bHasStencil = false;;
	m_fMaxPixelShaderVersion	= SHADER_VERSION_UNSET;
	m_fMaxVertexShaderVersion	= SHADER_VERSION_UNSET;
	m_dwDepthClearFlags	= 0;
	m_pcUsageStr		= NULL;
}


HRESULT D3DDeviceAndHWInfo::Free()
{
	SAFE_RELEASE( m_pD3DDev );
	SAFE_DELETE( m_pcUsageStr );

	SetToNull();
	return( S_OK );
}


HRESULT D3DDeviceAndHWInfo::Initialize( IDirect3DDevice9 * pDev, bool verbose )
{
	FAIL_IF_NULL( pDev );
	HRESULT hr = S_OK;
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();
	hr = GetDepthStencilInfo( m_pD3DDev, verbose );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
	hr = GetHWShaderInfo( m_pD3DDev, verbose );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
	return( hr );
}


HRESULT D3DDeviceAndHWInfo::GetHWShaderInfo( IDirect3DDevice9 * pDev,
											 bool verbose )
{
	FAIL_IF_NULL( pDev );
	HRESULT hr = S_OK;
	D3DCAPS9	caps;
	pDev->GetDeviceCaps( & caps );

	m_fMaxPixelShaderVersion =  (float) (D3DSHADER_VERSION_MAJOR( caps.PixelShaderVersion ));
	m_fMaxPixelShaderVersion += 0.1f * D3DSHADER_VERSION_MINOR( caps.PixelShaderVersion );
	m_fMaxVertexShaderVersion = (float) (D3DSHADER_VERSION_MAJOR( caps.VertexShaderVersion ));
	m_fMaxVertexShaderVersion += 0.1f * D3DSHADER_VERSION_MINOR( caps.VertexShaderVersion );

	if( verbose )
	{
		FMsg(TEXT("Vertex shader version: %3.3f\n"), m_fMaxVertexShaderVersion );
		FMsg(TEXT("Pixel  shader version: %3.3f\n"), m_fMaxPixelShaderVersion );
	}
	return( hr );
}

HRESULT D3DDeviceAndHWInfo::GetHWShaderInfo(	float * pfMaxVertexShaderVersion,
												float * pfMaxPixelShaderVersion		)
{
	HRESULT hr = S_OK;
	if( m_fMaxVertexShaderVersion == SHADER_VERSION_UNSET )
	{
		hr = GetHWShaderInfo( m_pD3DDev, false );
	}
	if( pfMaxVertexShaderVersion != NULL )
	{
		*pfMaxVertexShaderVersion = m_fMaxVertexShaderVersion;
	}
	if( pfMaxPixelShaderVersion != NULL )
	{
		*pfMaxPixelShaderVersion = m_fMaxPixelShaderVersion;
	}
	return( hr );
}




HRESULT D3DDeviceAndHWInfo::GetDepthStencilInfo( IDirect3DDevice9 * pDev,
												 bool verbose )
{
	// fill in info about stencil
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	D3DSURFACE_DESC desc;
	hr = GetDepthBufferDesc( & desc, pDev );
	if( FAILED(hr) ) return( hr );

	m_bHasStencil =		( desc.Format == D3DFMT_D15S1)		
					||	( desc.Format == D3DFMT_D24S8)		
					||	( desc.Format == D3DFMT_D24X4S4)	
					||  ( desc.Format == D3DFMT_D24FS8 )
					;

	if( m_bHasStencil )
	{
		m_dwDepthClearFlags = D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER;
	}
	else
	{
		m_dwDepthClearFlags = D3DCLEAR_ZBUFFER;
	}

	if( verbose )
	{

		char fmt[128];
		
		switch( desc.Format )
		{
		case D3DFMT_D15S1 :
			strcpy( fmt, "D3DFMT_D15S1");
			break;
		case D3DFMT_D24S8 :
			strcpy( fmt, "D3DFMT_D24S8");
			break;
		case D3DFMT_D24X4S4 :
			strcpy( fmt, "D3DFMT_D24X4S4");
			break;
		case D3DFMT_D24FS8 :
			strcpy( fmt, "D3DFMT_D24FS8");
			break;
		default :
			strcpy( fmt, "Unknown");
			break;
		}

		if( m_bHasStencil )
			FMsg(TEXT("Device has stencil:  %s\n"), fmt );
		else
			FMsg(TEXT("Device does not have stencil: %u\n"), desc.Format );
		
	}
	return( hr );
}

bool	D3DDeviceAndHWInfo::DoesFormatHaveStencil( D3DFORMAT format )
{
	bool bHasStencil = false;
	switch( format )
	{
	case D3DFMT_D15S1 :
	case D3DFMT_D24S8 :
	case D3DFMT_D24X4S4 :
	case D3DFMT_D24FS8 :
		bHasStencil = true;
		break;
	}
	return( bHasStencil );
}

HRESULT D3DDeviceAndHWInfo::GetDepthBufferDesc( D3DSURFACE_DESC * pDesc,
												IDirect3DDevice9 * pDev,
												bool bVerbose )
{
	bVerbose;
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	FAIL_IF_NULL( pDesc );
	IDirect3DSurface9 * pDepthSurf = NULL;

	hr = pDev->GetDepthStencilSurface( &pDepthSurf );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't get depth stencil surface!\n", hr );

	hr = pDepthSurf->GetDesc( pDesc );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't get depth surface desc!\n", hr );

	pDepthSurf->Release();
	return( S_OK );
}


HRESULT D3DDeviceAndHWInfo::GetColorBufferDesc( D3DSURFACE_DESC * pOutDesc,
											    IDirect3DDevice9 * pDev,
											    bool bVerbose )
{
	bVerbose;
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOutDesc );
	FAIL_IF_NULL( pDev );
	IDirect3DSurface9 * pColorSurf = NULL;

	hr = pDev->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pColorSurf );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't GetBackBuffer()\n", hr );

	if( pColorSurf != NULL )
	{
		hr = pColorSurf->GetDesc( pOutDesc );
		pColorSurf->Release();
	}
	return( hr );
}

HRESULT D3DDeviceAndHWInfo::GetAdapterDisplayMode( IDirect3DDevice9 * pDev, 
												   UINT Adapter, 
												   D3DDISPLAYMODE * pOutMode )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	FAIL_IF_NULL( pOutMode );

	IDirect3D9 * pD3D9;
	pDev->GetDirect3D( &pD3D9 );
	FAIL_IF_NULL( pD3D9 );
	hr = pD3D9->GetAdapterDisplayMode( Adapter, pOutMode );
	SAFE_RELEASE( pD3D9 );

	return( hr );
}

HRESULT D3DDeviceAndHWInfo::GetDeviceInfo( IDirect3DDevice9 * pDev,
											bool bVerbose )
{
	bVerbose;
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	FMsg(TEXT("D3DDeviceAndHWInfo::GetDeviceInfo not implemented\n"));
	assert( false );
	return( hr );
}


// This function checks for FULL support of non-pow2 texturing.
// Return  true only if:
// D3DPTEXTURECAPS_NONPOW2CONDITIONAL = false
// and D3DPTEXTURECAPS_POW2 = false
// If D3DPTEXTURECAPS_NONPOW2CONDITIONAL is true, then non-pow2 textures are supported with
//  certain restrictions.  See the DXSDK docs.
//---------------------------------------------------------------------------------------------
// 4/19/2004 - 60.72 GeForce FX reports POW2 = true and NONWPOW2CONDITIONAL = true.  Ignore the 
//  POW2 flag which technicaly means that only pow2 textures are supported.
bool D3DDeviceAndHWInfo::IsNonPow2TexturingFullySupported( IDirect3DDevice9 * pDev, bool bVerbose )
{
	if( pDev == NULL )
		return( false );
	D3DCAPS9	caps;
	pDev->GetDeviceCaps( & caps );
	bool bFullySupported;
	// must NOT have D3DPTEXTURECAPS_NONPOW2CONDITIONAL
	// must NOT have D3DPTEXTURECAPS_POW2
	bool bNP2c = (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL ) != 0;
	bool bPow2 = (caps.TextureCaps & D3DPTEXTURECAPS_POW2 ) != 0;
	if( (!bNP2c) && (!bPow2) )
	{
		bFullySupported = true;
	}
	else
	{
		bFullySupported = false;
	}
	if( bVerbose )
	{
		FMsg(TEXT("D3DPTEXTURECAPS_NONPOW2CONDITIONAL = %s\n"), bNP2c ? TEXT("TRUE") : TEXT("FALSE") );
		FMsg(TEXT("D3DPTEXTURECAPS_POW2               = %s\n"), bPow2 ? TEXT("TRUE") : TEXT("FALSE") );
		FMsg(TEXT("Non-pow2 texturing fully supported: %s\n"), bFullySupported ? TEXT("TRUE") : TEXT("FALSE") );
	}
	return( bFullySupported );
}

const TCHAR * D3DDeviceAndHWInfo::_GetStrD3DFORMAT( D3DFORMAT format )
{
	return( GetStrD3DFORMAT( format ));
}

const TCHAR * D3DDeviceAndHWInfo::_GetStrD3DRESOURCETYPE( D3DRESOURCETYPE restype )
{
	return( GetStrD3DRESOURCETYPE( restype ));
}

// If the supplied pBuf buffer is not large enough, an error string will be returned
void D3DDeviceAndHWInfo::_GetStrUsage( DWORD usage, TCHAR * pBuf, size_t buf_size )
{
	GetStrUsage( usage, pBuf, buf_size );
}

const TCHAR * D3DDeviceAndHWInfo::_GetStrD3DPOOL( D3DPOOL pool )
{
	return( GetStrD3DPOOL( pool ));
}

const TCHAR * D3DDeviceAndHWInfo::_GetStrD3DMULTISAMPLE_TYPE( D3DMULTISAMPLE_TYPE multisampletype )
{
	return( GetStrD3DMULTISAMPLE_TYPE( multisampletype ));
}

const TCHAR * D3DDeviceAndHWInfo::_GetStrRenderState( D3DRENDERSTATETYPE renderstate )
{
	return( GetStrRenderState( renderstate ) );
}


HRESULT	D3DDeviceAndHWInfo::ReportSurfaceDesc( const D3DSURFACE_DESC * pDesc )
{
	HRESULT hr = S_OK;
	if( pDesc == NULL )
	{
		FMsg("  ReportSurfaceDesc() pointer is NULL!\n");
		hr = E_FAIL;
		return( hr );
	}

	// prints via OutputDebugString()
	//@ add support for printing to log file

	TCHAR * indent = TEXT("  ");
	TCHAR pUsage[1024];
	GetStrUsage( pDesc->Usage, pUsage, 1024 );

	FMsg("ReportSurfaceDesc:\n");
	FMsg("%sFormat          = %s\n", indent, GetStrD3DFORMAT( pDesc->Format ) );
	FMsg("%sResource Type   = %s\n", indent, GetStrD3DRESOURCETYPE( pDesc->Type ) );
	FMsg("%sUsage           = %s\n", indent, pUsage );         
	FMsg("%sPool            = %s\n", indent, GetStrD3DPOOL( pDesc->Pool ) );
	FMsg("%sMultiSampleType = %s\n", indent, GetStrD3DMULTISAMPLE_TYPE( pDesc->MultiSampleType ) );
	FMsg("%sMultiSampleQual = %u\n", indent, pDesc->MultiSampleQuality );
	FMsg("%sWidth           = %u\n", indent, pDesc->Width );
	FMsg("%sHeight          = %u\n", indent, pDesc->Height );

	return( hr );
}



