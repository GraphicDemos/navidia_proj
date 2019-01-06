/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderTargetsPS30_8bpc_MRT.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Render targets used for thickness rendering with ps.3.0, 8-bits-per-color-channel render targets, and
a floating point render target texture.

These surfaces are used with Multiple Render Target (MRT) rendering as exposed in DX9.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_ThicknessRenderTargetsPS30_8bpc_MRT_H
#define H_ThicknessRenderTargetsPS30_8bpc_MRT_H

class ThicknessRenderTargetsPS30_8bpc_MRT
{
public:
	// Render target texture / surface pairs
	// m_pTexOrdinaryShading is used to render the ordinary scene during the MRT rendering.
	// A render target texture is used instead of the ordinary backbuffer for AA compatability 
	//  between render targets and because you shouldn't do MRT rendering when one surface is in
	//  the swap chain and another surface is not.
	IDirect3DTexture9 *		m_pTexOrdinaryShading;			// D3DFMT_A8R8G8B8
	IDirect3DSurface9 *		m_pSurfOrdinaryShading;			// Surface corresponding to m_pTexOccludersDepth 
	IDirect3DSurface9 *		m_pSurfOrdinaryShadingDepth;	// Depth buffer

	// m_pTexOccludersDepth is used to render the RGB-encoded depth of scene objects during the MRT rendering
	IDirect3DTexture9 *		m_pTexOccludersDepth;		// D3DFMT_A8R8G8B8
	IDirect3DSurface9 *		m_pSurfOccludersDepth;		// Surface corresponding to m_pTexOccludersDepth 

	// Used to acumulate the sum of front and back face depths for the volume objects
	IDirect3DTexture9 *		m_pTexFPFaceDepthSum;
	IDirect3DSurface9 *		m_pSurfFPFaceDepthSum;

	// The ordinary back buffers in the D3D swap chain
	IDirect3DSurface9 *		m_pDefaultBackbufferColor;
	IDirect3DSurface9 *		m_pDefaultBackbufferDepth;

	IDirect3DDevice9 *		m_pD3DDev;

	D3DXVECTOR4				m_HalfTexelSize;
	DWORD					m_dwDepthClearFlags;

	HRESULT		Initialize( IDirect3DDevice9 * pD3DDev, UINT x_resolution, UINT y_resolution );
	HRESULT		Free();

	HRESULT		SetToBuffersForOrdinarySceneAndDepth();
	HRESULT		SetToFlipChainBackbuffers();
	HRESULT		SetToOccludersDepth();
	HRESULT		SetToFaceDepthSum();

	DWORD		GetDepthClearFlags()		{ return( m_dwDepthClearFlags ); };

	ThicknessRenderTargetsPS30_8bpc_MRT();
	~ThicknessRenderTargetsPS30_8bpc_MRT();
	void	SetAllNull()
	{
		m_pTexOrdinaryShading		= NULL;
		m_pSurfOrdinaryShading		= NULL;
		m_pSurfOrdinaryShadingDepth = NULL;
		m_pTexOccludersDepth		= NULL;
		m_pSurfOccludersDepth		= NULL;
		m_pTexFPFaceDepthSum		= NULL;
		m_pSurfFPFaceDepthSum		= NULL;
		m_pDefaultBackbufferColor	= NULL;
		m_pDefaultBackbufferDepth	= NULL;
		m_pD3DDev					= NULL;
	}
};

#endif

