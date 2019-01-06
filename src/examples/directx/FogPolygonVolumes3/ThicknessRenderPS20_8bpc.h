/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderPS20_8bpc.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
This class contains functions to set the state and shaders for rendering polygon objects as thick
volumes of material.  This class is specific to rendering using ps.2.0 hardware and 8-bit-per-color-
channel render target textures.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_THICKNESSRENDERPS20_8BPC_H
#define H_THICKNESSRENDERPS20_8BPC_H

#include "FogTombDemo.h"
#include "FogTombScene.h"
#include "ThicknessRenderProperties.h"
#include "ThicknessRenderProperties8BPC.h"


class ThicknessRenderProperties;
class ThicknessRenderProperties8BPC;


class ThicknessRenderPS20_8bpc
{
public:
	ThicknessRenderProperties8BPC	m_RenderProperties;
	IDirect3DDevice9 *	m_pD3DDev;

	// Render solid objects with ordinary diffuse color shading.  The fog will be applied on top of
	// this ordinary rendering.
	virtual HRESULT	SetToRenderOrdinaryScene( ThicknessRenderTargetsPS20_8bpc * pTargets,
												FogTombShaders8bpc * pShaders );
	virtual void	SRS_DiffuseDirectional( IDirect3DDevice9 * pDev );


	// Render the depth to the nearest visible solid objects that occlude or intersect the fog volumes
	virtual HRESULT SetToRenderOccludersDepth( ThicknessRenderTargetsPS20_8bpc * pTargets,
												FogTombShaders8bpc * pShaders,
												bool bDither );

	virtual HRESULT SetToRenderFrontFaceDepths( ThicknessRenderTargetsPS20_8bpc * pTargets,
												FogTombShaders8bpc * pShaders );
	virtual HRESULT SetToRenderBackFaceDepths( ThicknessRenderTargetsPS20_8bpc * pTargets );

	virtual HRESULT SetToRenderFogSubtractConvertAndBlend( ThicknessRenderTargetsPS20_8bpc * pTargets,
															FogTombShaders8bpc * pShaders,
															IDirect3DTexture9 * pThicknessToColorTexture );

	ThicknessRenderPS20_8bpc()
	{
		m_pD3DDev				= NULL;
	}
};


#endif
