/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombShaders8bpc_MRT.cpp

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
#include "FogTombShaders8bpc_MRT.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

// Defines are used to make it clear what assets this module attempts to load
#define VSH_DiffuseAndRGBDepthEncode			TEXT("MEDIA\\programs\\D3D9_FogPolygonVolumes3\\DiffuseAndRGBDepthEncode.vsh")
#define	VSH_RGBEncodeAndCompare					TEXT("MEDIA\\programs\\D3D9_FogPolygonVolumes3\\RGBEncodeAndCompare.vsh")

#define	PSH_DiffuseAndRGBDepthEncode			TEXT("MEDIA\\programs\\D3D9_FogPolygonVolumes3\\DiffuseAndRGBDepthEncode.psh")
#define PSH_RGBEncodeAndCompare_30				TEXT("MEDIA\\programs\\D3D9_FogPolygonVolumes3\\RGBEncodeAndCompare_30.psh")
#define	PSH_RGBThicknessToFogColorAndSceneBlend	TEXT("MEDIA\\programs\\D3D9_FogPolygonVolumes3\\RGBThicknessToFogColorAndSceneBlend.psh")

FogTombShaders8bpc_MRT::FogTombShaders8bpc_MRT()
{
	SetAllNull();
}

FogTombShaders8bpc_MRT::~FogTombShaders8bpc_MRT()
{
	FreeShaders();
	SetAllNull();
}

ShaderManager * FogTombShaders8bpc_MRT::GetShaderManager()
{
	BREAK_AND_RET_VAL_IF( m_ppShaderManager == NULL, NULL );
	BREAK_AND_RET_VAL_IF( *m_ppShaderManager == NULL, NULL );
	return( *m_ppShaderManager );
}

HRESULT FogTombShaders8bpc_MRT::FreeShaders()
{
	HRESULT hr = S_OK;
	return( hr );
}



HRESULT FogTombShaders8bpc_MRT::LoadShaders( ShaderManager ** ppShaderManager )
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( ppShaderManager == NULL, E_FAIL );
	ShaderManager * pSM = *ppShaderManager;
	BREAK_AND_RET_VAL_IF( pSM == NULL, E_FAIL );
	m_ppShaderManager = ppShaderManager;

	hr = pSM->LoadAndAssembleShader(
			VSH_DiffuseAndRGBDepthEncode,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_DiffuseAndRGBDepthEncode );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			VSH_RGBEncodeAndCompare,
			SM_SHADERTYPE_VERTEX,
			& m_VSHI_RGBEncodeAndCompare_30 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );
	//--------------------------------
	hr = pSM->LoadAndAssembleShader(
			PSH_DiffuseAndRGBDepthEncode,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_DiffuseAndRGBDepthEncode );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			PSH_RGBEncodeAndCompare_30,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_RGBEncodeAndCompare_30 );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	hr = pSM->LoadAndAssembleShader(
			PSH_RGBThicknessToFogColorAndSceneBlend,
			SM_SHADERTYPE_PIXEL,
			& m_PSHI_RGBThicknessToFogColorAndSceneBlend );
	BREAK_AND_RET_VAL_IF_FAILED( hr );

	//--- Debug print shader text to console
/*
	FMsg( TEXT("shader text:\n") );
	tstring tstrShader, tstrBytes;
	tstrShader	= pSM->GetShaderText( m_PSHI_RGBThicknessToFogColorAndSceneBlend );
	FMsg( TEXT("shader:\n%s\n"), tstrShader.c_str() );
	tstrBytes	= pSM->GetShaderBytesText( m_PSHI_RGBThicknessToFogColorAndSceneBlend );
	FMsg( TEXT("bytes:\n%s\n"), tstrBytes.c_str() );
	FMsg( TEXT("\n"));
// */

	return( hr );
}


