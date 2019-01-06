/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombShaders8BPC.cpp

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

#include "dxstdafx.h"

#include <NV_D3DCommon\NV_D3DCommonDX9.h>
#include "FogTombShaders8BPC.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

// Defines are used to make it clear what assets this module attempts to load
#define VSH_Diffuse_Directional				TEXT("D3D9_FogPolygonVolumes3\\DiffuseDirectional.vsh")
#define VSH_DepthToTexcrdForRGB				TEXT("D3D9_FogPolygonVolumes3\\DepthToTexcrdForRGB.vsh")
#define VSH_DepthToTexcrdForRGB_TC4Proj		TEXT("D3D9_FogPolygonVolumes3\\DepthToTexcrdForRGB_TC4Proj.vsh")
#define VSH_RGBDifferencetoFogColor_11		TEXT("D3D9_FogPolygonVolumes3\\RGBDifferenceToFogColor_11.vsh")

#define PSH_DepthToRGBEncode				TEXT("D3D9_FogPolygonVolumes3\\DepthToRGBEncode.psh")
#define PSH_DepthToRGBAndCompare_20			TEXT("D3D9_FogPolygonVolumes3\\DepthToRGBAndCompare_20.psh")
#define PSH_SubtractRGBAndGetFogColor_20	TEXT("D3D9_FogPolygonVolumes3\\SubtractRGBAndGetFogColor_20.psh")



FogTombShaders8bpc::FogTombShaders8bpc()
{
	SetAllNull();
}

FogTombShaders8bpc::~FogTombShaders8bpc()
{
	FreeShaders();
	SetAllNull();
}

ShaderManager * FogTombShaders8bpc::GetShaderManager()
{
	BREAK_AND_RET_VAL_IF( m_ppShaderManager == NULL, NULL );
	BREAK_AND_RET_VAL_IF( *m_ppShaderManager == NULL, NULL );
	return( *m_ppShaderManager );
}

HRESULT FogTombShaders8bpc::FreeShaders()
{
	HRESULT hr = S_OK;
	return( hr );
}

HRESULT FogTombShaders8bpc::LoadShaders13( ShaderManager ** ppShaderManager )
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( ppShaderManager == NULL, E_FAIL );
	ShaderManager * pSM = *ppShaderManager;
	BREAK_AND_RET_VAL_IF( pSM == NULL, E_FAIL );
	m_ppShaderManager = ppShaderManager;

	FMsg("LoadShaders13 not implemented\n");
	assert( false );

	return( hr );
}

HRESULT FogTombShaders8bpc::LoadShaders20( ShaderManager ** ppShaderManager )
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( ppShaderManager == NULL, E_FAIL );
	ShaderManager * pSM = *ppShaderManager;
	BREAK_AND_RET_VAL_IF( pSM == NULL, E_FAIL );
	m_ppShaderManager = ppShaderManager;

	// Load vertex & pixel shaders
	hr = pSM->LoadAndAssembleShader(
			VSH_Diffuse_Directional,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_DiffuseDirectional	 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			VSH_DepthToTexcrdForRGB,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_DepthToTexcrdForRGB );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			VSH_DepthToTexcrdForRGB_TC4Proj,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_DepthToTexcrdForRGB_TC4Proj );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			PSH_DepthToRGBEncode,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_DepthToRGBEncode );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			VSH_RGBDifferencetoFogColor_11,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_RGBDifferencetoFogColor_11 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	//---------------------------------------------------------------
	// ps.2.0 shaders:

	hr = pSM->LoadAndAssembleShader(
			PSH_DepthToRGBAndCompare_20,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_DepthToRGBAndCompare_20 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			PSH_SubtractRGBAndGetFogColor_20,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_SubtractRGBAndGetFogColor_20 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );


	return( hr );
}


