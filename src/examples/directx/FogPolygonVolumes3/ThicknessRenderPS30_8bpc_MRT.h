/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderPS30_8bpc_MRT.h

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
volumes of material.  This class is specific to rendering using ps.3.0 hardware, an 8-bit-per-color-
channel render target texture for the scene object's depths, and a floating point render target
texture to acumulate the depths of the volume object's faces.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_THICKNESSRENDERPS30_8BPC_MRT_H
#define H_THICKNESSRENDERPS30_8BPC_MRT_H

#include "FogTombDemo.h"
#include "FogTombScene.h"
#include "ThicknessRenderProperties.h"
#include "ThicknessRenderProperties8BPC.h"


class ThicknessRenderProperties;
class ThicknessRenderProperties8BPC;
class ThicknessRenderTargetsPS30_8bpc_MRT;
class FogTombShaders8bpc_MRT;


class ThicknessRenderPS30_8bpc_MRT
{
public:
	ThicknessRenderProperties8BPC	m_RenderProperties;
	IDirect3DDevice9 *	m_pD3DDev;

	// Render ordinary scene to one MRT buffer, and the depth of objects encoded as an 
	// RGB value to another MRT buffer.
	virtual HRESULT	SetToRenderOrdinarySceneAndDepth( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
														FogTombShaders8bpc_MRT * pShaders,
														bool bDither );
	virtual void	SRS_OrdinarySceneAndDepth( IDirect3DDevice9 * pDev );

	virtual HRESULT SetToRenderVolumeObjectThickness( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
														FogTombShaders8bpc_MRT * pShaders );

	virtual HRESULT SetToRenderSceneAndVolumeColorToBackbuffer( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
																FogTombShaders8bpc_MRT * pShaders,
																IDirect3DTexture9 * pTexColorRamp );

	ThicknessRenderPS30_8bpc_MRT()
	{
		m_pD3DDev				= NULL;
	}
};


#endif
