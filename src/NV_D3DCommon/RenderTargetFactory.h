/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  RenderTargetFactory.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Class for making D3D texture render targets, tracking the associated surfaces,
texture, and depth buffers.


-------------------------------------------------------------------------------|--------------------*/

#ifndef H_RENDERTARGETFACTORY_H
#define H_RENDERTARGETFACTORY_H

#include "NV_D3DCommon\TextureFactory.h"

// Describes one texture / surface group
// Associates render target surfaces with their parent textures
class DECLSPEC_NV_D3D_COMMON_API RenderTargetDesc
{
public:
	IDirect3DDevice9 * m_pD3DDev;			// not AddRef'd
	IDirect3DTexture9 **	m_ppRTTTexture;
	IDirect3DSurface9 **	m_ppRTTSurface;	// render target surface from the texture 

	int			m_nWidth;		// resolution of the surface
	int			m_nHeight;
	D3DFORMAT	m_SurfaceFormat;

	RenderTargetDesc()		{ SetAllNull(); };
	RenderTargetDesc( IDirect3DSurface9 ** ppSurf );
	~RenderTargetDesc()		{ SetAllNull(); };
	void SetAllNull()
	{
		m_pD3DDev			= NULL;
		m_ppRTTTexture		= NULL;
		m_ppRTTSurface		= NULL;
		m_nWidth			= 0;
		m_nHeight			= 0;
		m_SurfaceFormat		= D3DFMT_UNKNOWN;
	}
	IDirect3DTexture9 *	GetTextureP()
	{
		if( m_ppRTTTexture != NULL )
			return( *m_ppRTTTexture );
		return( NULL );
	}
	IDirect3DSurface9 *	GetSurfaceP()
	{
		if( m_ppRTTSurface != NULL )
			return( *m_ppRTTSurface );
		return( NULL );
	}
};

// Describes a depth render target
class DECLSPEC_NV_D3D_COMMON_API RenderTargetDepthDesc
{
public:
	IDirect3DDevice9 *		m_pD3DDev;			// not AddRef'd
	IDirect3DSurface9 **	m_ppRTTDepthBuffer;	// depth buffer surface
	
	int			m_nWidth;		// resolution of the surface
	int			m_nHeight;
	D3DFORMAT	m_DepthFormat;
	DWORD		m_dwDepthClearFlags;

	RenderTargetDepthDesc()			{ SetAllNull(); };
	RenderTargetDepthDesc( IDirect3DSurface9	** ppSurf );
	~RenderTargetDepthDesc()		{ SetAllNull(); };
	void SetAllNull()
	{
		m_pD3DDev				= NULL;
		m_ppRTTDepthBuffer		= NULL;
		m_nWidth				= 0;
		m_nHeight				= 0;
		m_DepthFormat			= D3DFMT_UNKNOWN;
		m_dwDepthClearFlags		= 0;
	}
	DWORD GetDepthClearFlags( DWORD format );

	IDirect3DSurface9 *	GetSurfaceP()
	{
		if( m_ppRTTDepthBuffer != NULL )
			return( *m_ppRTTDepthBuffer );
		return( NULL );
	}
};

// A color and depth target.  This is used to track the targets
//  so that the D3D device's active render targets can be set
//	back to what they were before being changed.
class DECLSPEC_NV_D3D_COMMON_API RenderTargetSet
{
public:
	RenderTargetDesc		**	m_ppColorTarget;
	RenderTargetDepthDesc	**	m_ppDepthTarget;

	HRESULT	SetAsCurrent( bool bSetColor=true, bool bSetDepth = true );

	RenderTargetSet()	{ SetToNull(); };
	~RenderTargetSet()	{ SetToNull(); };	
	void SetToNull()	{ m_ppColorTarget = NULL; m_ppDepthTarget = NULL; };
};



class DECLSPEC_NV_D3D_COMMON_API RenderTargetFactory : public TextureFactory
{
public : 
	RenderTargetFactory();
	virtual ~RenderTargetFactory();

	virtual HRESULT Initialize( GetFilePath::GetFilePathFunction file_path_callback = NULL );
	virtual HRESULT Free();

	RenderTargetDesc **  CreateRenderTarget( IDirect3DDevice9 * pDev,
											 int width, int height,
											 D3DFORMAT format );
	
	RenderTargetDepthDesc ** CreateDepthTarget( IDirect3DDevice9 * pDev,
												int width, int height,
												D3DFORMAT format );

		// Convenience functions to return the current render targets so they
		//  can be restored after using a different render target.
		// Each call will allocate a new descriptor and associated pointers
		//  to hold the information.
	RenderTargetDesc **			GetCurrentColorTarget( IDirect3DDevice9 * pDev );
	RenderTargetDepthDesc **	GetCurrentDepthTarget( IDirect3DDevice9 * pDev );
	RenderTargetSet				GetCurrentTargets( IDirect3DDevice9	* pDev );
		
		// Sets the device's render targets to those pointed to in the descriptions
	HRESULT		SetRenderTargets( IDirect3DDevice9 * pDev,
									RenderTargetDesc ** pColorDesc,
									RenderTargetDepthDesc ** ppDepthDesc,
									bool bVerbose = false );


		// Diagnostic / debug display functions
	void	DbgTextRenderTargetDesc( RenderTargetDesc ** ppCol );
	void	DbgTextRenderTargetDesc( RenderTargetDepthDesc ** ppDepth );


protected:
	virtual void	SetAllNull()
	{
		TextureFactory::SetAllNull();
	};


	vector< RenderTargetDesc ** >		m_vppRTDTexSurfPairs;
	vector< RenderTargetDepthDesc ** >	m_vppRTDDepthSurfaces;

};


#endif				// H_RENDERTARGETFACTORY_H
