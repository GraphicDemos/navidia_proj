/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceAndHWInfo.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Class to determine buffer formats, the presence of stencil, hardware shader
support, etc.

It can decode some D3D device and renderstate information to plain text strings.
It has utility functions for displaying surface properties in plain text.

Not intended to be frequently created and destroyed.  Make one and keep it around.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_D3DDEVICEANDHWINFO_H
#define H_D3DDEVICEANDHWINFO_H
#include "NV_D3DCommon_decl_.h"
class DECLSPEC_NV_D3D_COMMON_API D3DDeviceAndHWInfo
{
protected:
	IDirect3DDevice9 * m_pD3DDev;
	bool	m_bHasStencil;
	DWORD	m_dwDepthClearFlags;
	float	m_fMaxPixelShaderVersion;
	float	m_fMaxVertexShaderVersion;
	char *	m_pcUsageStr;
	void	SetToNull();

public:
	HRESULT Initialize( IDirect3DDevice9 * pDev, bool bVerbose = false );
	HRESULT Free();

	HRESULT	GetDepthStencilInfo( IDirect3DDevice9 * pDev, bool bVerbose = false );
	bool	DoesFormatHaveStencil( D3DFORMAT format );
	HRESULT GetHWShaderInfo( IDirect3DDevice9 * pDev, bool bVerbose = false );
	HRESULT GetHWShaderInfo( float * pfMaxVertexShaderVersion,
							 float * pfMaxPixelShaderVersion );

	DWORD	GetDepthClearFlags() { return( m_dwDepthClearFlags ); };

	HRESULT GetDepthBufferDesc( D3DSURFACE_DESC * pDesc, IDirect3DDevice9 * pDev, bool bVerbose = false );
	HRESULT GetColorBufferDesc( D3DSURFACE_DESC * pDesc, IDirect3DDevice9 * pDev, bool bVerbose = false );
	HRESULT GetAdapterDisplayMode( IDirect3DDevice9 * pDev, UINT Adapter, D3DDISPLAYMODE * pOutMode );

	HRESULT GetDeviceInfo( IDirect3DDevice9 * pDev, bool verbose = false );

	bool	IsNonPow2TexturingFullySupported( IDirect3DDevice9 * pDev, bool bVerbose = false );

	// Text output functions
	HRESULT	ReportSurfaceDesc( const D3DSURFACE_DESC * pDesc );

	// const char * is ok for these, as string literals are allocated
	//  before program begins and freed after program ends.
	// See UtilityFunctions.h for more of these string conversion functions
	const TCHAR * _GetStrD3DFORMAT(			D3DFORMAT format );
	const TCHAR * _GetStrD3DRESOURCETYPE(	D3DRESOURCETYPE restype );
	void		  _GetStrUsage(				DWORD usage, TCHAR * pBuf, size_t sz );
	const TCHAR * _GetStrD3DPOOL(			D3DPOOL pool );
	const TCHAR * _GetStrD3DMULTISAMPLE_TYPE( D3DMULTISAMPLE_TYPE multisampletype );
	const TCHAR * _GetStrRenderState(		D3DRENDERSTATETYPE renderstate );

public:
	D3DDeviceAndHWInfo();
	~D3DDeviceAndHWInfo();
};


#endif		// H_D3DDEVICEANDHWINFO_H
