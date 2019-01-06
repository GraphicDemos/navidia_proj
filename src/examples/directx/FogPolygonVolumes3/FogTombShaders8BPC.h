/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombShaders8BPC.h

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
target textures.  Shader handles exist for ps.2.0 and ps.1.3 hardware, but may not all be loaded, 
depending on the capabilities of the hardware.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_FOGTOMBSHADERS8BPC_H
#define H_FOGTOMBSHADERS8BPC_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"

class FogTombShaders8bpc
{
public:
	ShaderManager **	m_ppShaderManager;

	// Indices to ShaderManager shaders
	//
	SM_SHADER_INDEX		m_VSHI_DiffuseDirectional;				// ordinary scene lighting with diffuse directional light
	//-------------------------------------------------------------------------------------------------------
	// ps.1.3 shader path
	// These are shaders used with DX8 ps.1.3 hardware
	SM_SHADER_INDEX		m_VSHI_RGBDifferencetoFogColor_11;		// replacement shader for TextureDisplay shader
	// Converts RGB-encoded depth difference into a texture coordinate and looks up the fog color
	SM_SHADER_INDEX		m_PSHI_RGBDifferenceToFogColor_13;
	// Uses texcoords to encode depth as RGB value and compares to the occluder's depth read from a texture.
	// Outputs the lesser of the RGB-encoded depth and the occluder's depth.
	SM_SHADER_INDEX		m_PSHI_DepthToRGBAndCompare_13;
	// Samples rendered textures for front and back face depths and subtracts them, writing a biased-scaled
	//  difference value to an unsigned a8r8g8b8 texture render target.
	SM_SHADER_INDEX		m_PSHI_SubtractRGBEncodedDepth_13;
	//-------------------------------------------------------------------------------------------------------
	// ps_2_0 shader path
	// These are used with DX9 ps_2_0 hardware

	// Generates RGB-encoded depth value for the pixel rendered, compares
	//  it to an RGB-encoded depth value read from a texture and outputs
	//  the lesser of the two values.
	//  The RGB-encoded depth read from texture is the nearest solid object
	//  depth, and it is used to handle intersection of solids with the 
	//  fog volumes.
	SM_SHADER_INDEX		m_PSHI_DepthToRGBAndCompare_20;
	// Subtracts front depths from back depths, converts the difference
	//  to texture coordinate, and uses the coordinate to look up the
	//  fog color.
	SM_SHADER_INDEX		m_PSHI_SubtractRGBAndGetFogColor_20;
	//-------------------------------------------------------------------------------------------------------
	// General shaders used on both ps.1.3 and ps_2_0 hardware

	SM_SHADER_INDEX		m_PSHI_DepthToRGBEncode;

	// Calculates texture coordinates used to encode pixel depth
	//  as an RGB value.  A pixel shader uses the coordinates to
	//  sample RGB ramp textures and create an RGB-encoded depth
	//  value for each pixel of the triangle.
	SM_SHADER_INDEX		m_VSHI_DepthToTexcrdForRGB;

	// Same as above, but calculates a projected texture coordinate
	//  in texture unit 4.  This coordinate is used to map the screen-
	//  aligned fog calculation textures to arbitrary geometry in the 
	//  scene so that the texels of the screen-aligned texture always
	//  fall on their corresponding screen pixel, regardless of the
	//  shape or position of the geometry being rendered.
	SM_SHADER_INDEX		m_VSHI_DepthToTexcrdForRGB_TC4Proj;

	//-------------------------------------------------------------------------------------------------------
	
	// Loads shaders for ps.1.3 DX8 hardware
	virtual HRESULT LoadShaders13( ShaderManager ** ppShaderManager );
	// Loads shaders for ps.2.0 hardware
	virtual HRESULT LoadShaders20( ShaderManager ** ppShaderManager );
	virtual HRESULT	FreeShaders();

	ShaderManager * GetShaderManager();

	FogTombShaders8bpc();
	~FogTombShaders8bpc();
	void SetAllNull()
	{
		m_ppShaderManager		= NULL;
		m_VSHI_RGBDifferencetoFogColor_11		= SM_IDXUNSET;
		m_PSHI_RGBDifferenceToFogColor_13		= SM_IDXUNSET;
		m_PSHI_DepthToRGBAndCompare_13			= SM_IDXUNSET;
		m_PSHI_SubtractRGBEncodedDepth_13		= SM_IDXUNSET;
		m_PSHI_DepthToRGBAndCompare_20			= SM_IDXUNSET;
		m_PSHI_SubtractRGBAndGetFogColor_20		= SM_IDXUNSET;
		m_PSHI_DepthToRGBEncode					= SM_IDXUNSET;
		m_VSHI_DepthToTexcrdForRGB				= SM_IDXUNSET;
		m_VSHI_DepthToTexcrdForRGB_TC4Proj		= SM_IDXUNSET;
	}
};

#endif
