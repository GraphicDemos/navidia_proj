/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombShaders8bpc_MRT.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Comments:
A collection of vertex and pixel shaders used to render ordinary polygon objects as thick volumes
of fog or translucent material.

This set of shaders is used for rendering volume objects using 8-bit-per-color-channel (8bpc) render
target textures and 16-bit floating point render target textures.  The shaders use multiple render
target (MRT) rendering and ps.3.0

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_FogTombShaders8bpc_MRT_H
#define H_FogTombShaders8bpc_MRT_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"

class FogTombShaders8bpc_MRT
{
public:
	ShaderManager **	m_ppShaderManager;

	// Indices to ShaderManager shaders
	SM_SHADER_INDEX		m_VSHI_DiffuseAndRGBDepthEncode;
	SM_SHADER_INDEX		m_VSHI_RGBEncodeAndCompare_30;

	SM_SHADER_INDEX		m_PSHI_DiffuseAndRGBDepthEncode;		// MRT to render scene and scene depth
	SM_SHADER_INDEX		m_PSHI_RGBEncodeAndCompare_30;
	SM_SHADER_INDEX		m_PSHI_RGBThicknessToFogColorAndSceneBlend;
	//-------------------------------------------------------------------------------------------------------
	// Loads shaders for ps.3.0 MRT fp blending hardware
	virtual HRESULT LoadShaders( ShaderManager ** ppShaderManager );
	virtual HRESULT	FreeShaders();

	ShaderManager * GetShaderManager();

	FogTombShaders8bpc_MRT();
	~FogTombShaders8bpc_MRT();
	void SetAllNull()
	{
		m_ppShaderManager		= NULL;
		m_VSHI_DiffuseAndRGBDepthEncode				= SM_IDXUNSET;
		m_VSHI_RGBEncodeAndCompare_30				= SM_IDXUNSET;

		m_PSHI_DiffuseAndRGBDepthEncode				= SM_IDXUNSET;
		m_PSHI_RGBEncodeAndCompare_30				= SM_IDXUNSET;
		m_PSHI_RGBThicknessToFogColorAndSceneBlend	= SM_IDXUNSET;
	}
};

#endif
