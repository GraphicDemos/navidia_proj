/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderTargetsPS20_8bpc.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Render targets used for thickness rendering with ps.2.0 and 8-bits-per-color-channel render targets.
This is for GeForce FX generation hardware.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_THICKNESSRENDERTARGETSPS20_8BPC_H
#define H_THICKNESSRENDERTARGETSPS20_8BPC_H

class ThicknessRenderTargetsPS20_8bpc
{
public:
	IDirect3DTexture9 *		m_pTexOccludersDepth;
	IDirect3DSurface9 *		m_pSurfOccludersDepth;

	IDirect3DTexture9 *		m_pTexFrontFacesDepth;
	IDirect3DSurface9 *		m_pSurfFrontFacesDepth;
	IDirect3DTexture9 *		m_pTexBackFacesDepth;
	IDirect3DSurface9 *		m_pSurfBackFacesDepth;

	IDirect3DSurface9 *		m_pDefaultBackbufferColor;
	IDirect3DSurface9 *		m_pDefaultBackbufferDepth;

	IDirect3DDevice9 *		m_pD3DDev;

	D3DXVECTOR4				m_HalfTexelSize;
	DWORD					m_dwDepthClearFlags;

	HRESULT		Initialize( IDirect3DDevice9 * pD3DDev, UINT x_resolution, UINT y_resolution );
	HRESULT		Free();

	HRESULT		SetToDefaultBackbuffers();
	HRESULT		SetToOccludersDepth();
	HRESULT		SetToFrontFacesDepth();
	HRESULT		SetToBackFacesDepth();

	DWORD		GetDepthClearFlags()		{ return( m_dwDepthClearFlags ); };

	ThicknessRenderTargetsPS20_8bpc();
	~ThicknessRenderTargetsPS20_8bpc();
	void	SetAllNull()
	{
		m_pTexOccludersDepth		= NULL;
		m_pSurfOccludersDepth		= NULL;
		m_pTexFrontFacesDepth		= NULL;
		m_pSurfFrontFacesDepth		= NULL;
		m_pTexBackFacesDepth		= NULL;
		m_pSurfBackFacesDepth		= NULL;
		m_pDefaultBackbufferColor	= NULL;
		m_pDefaultBackbufferDepth	= NULL;
		m_pD3DDev					= NULL;
	}
};

#endif
