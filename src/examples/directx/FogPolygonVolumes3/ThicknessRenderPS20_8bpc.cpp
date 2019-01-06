/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderPS20_8bpc.cpp

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
#include "ThicknessRenderPS20_8bpc.h"
#include "ThicknessRenderProperties.h"
#include "ThicknessRenderProperties8BPC.h"
#include "ThicknessRenderTargetsPS20_8bpc.h"

#include "FogTombDemo.h"
#include "FogTombShaders8BPC.h"

#include "MEDIA\programs\D3D9_FogPolygonVolumes3\Constants.h"

void ThicknessRenderPS20_8bpc::SRS_DiffuseDirectional( IDirect3DDevice9 * pDev )
{
	RET_IF( pDev == NULL );
	pDev->SetRenderState( D3DRS_ZENABLE,		D3DZB_TRUE );
	pDev->SetRenderState( D3DRS_ZWRITEENABLE,	true );
	pDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_LESSEQUAL );

	pDev->SetRenderState( D3DRS_STENCILENABLE,		false );
	pDev->SetRenderState( D3DRS_SPECULARENABLE,		false );
	pDev->SetRenderState( D3DRS_FOGENABLE,			false );
	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	pDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );

	pDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		                                               D3DCOLORWRITEENABLE_RED |
													   D3DCOLORWRITEENABLE_GREEN | 
													   D3DCOLORWRITEENABLE_BLUE );

	// ( diffuse + tfactor * ) base texture
	pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );
	pDev->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	m_pD3DDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
	m_pD3DDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );
	m_pD3DDev->SetTextureStageState( 4, D3DTSS_TEXCOORDINDEX, 4 );
	m_pD3DDev->SetTextureStageState( 5, D3DTSS_TEXCOORDINDEX, 5 );
	m_pD3DDev->SetTextureStageState( 6, D3DTSS_TEXCOORDINDEX, 6 );

	int tcc;
	for( tcc=0; tcc < 3; tcc++ )
	{
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );

		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );		// no trilinear
	}
}

HRESULT ThicknessRenderPS20_8bpc::SetToRenderOrdinaryScene( ThicknessRenderTargetsPS20_8bpc * pTargets,
															FogTombShaders8bpc * pShaders )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );

	hr = pTargets->SetToDefaultBackbuffers();

	// Clear backbuffer
	// There is no need to D3DCLEAR_TARGET if you are redrawing the entire screen every time.
	hr = m_pD3DDev->Clear( 0, NULL,
							D3DCLEAR_TARGET | pTargets->m_dwDepthClearFlags,
							0x80808080,
							1.0, 0);
	BREAK_AND_RET_VAL_IF_FAILED(hr);

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	pSM->SetShader( pShaders->m_VSHI_DiffuseDirectional );
	m_pD3DDev->SetPixelShader( 0 );

	SRS_DiffuseDirectional( m_pD3DDev );

	D3DXVECTOR4 light_dir( 1.0f, -1.0f, 1.0f, 1.0f );
	D3DXVec4Normalize( &light_dir, &light_dir );
	m_pD3DDev->SetVertexShaderConstantF( CV_DIRLIGHT1_DIR, light_dir, 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1, D3DXVECTOR4( 0.0f, 0.5f, 1.0f, 2.0f ), 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_DIRLIGHT1_COLOR, D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ), 1 );

	m_pD3DDev->SetTexture( 0, NULL );
	m_pD3DDev->SetTexture( 1, NULL );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );

	return( hr );
}


HRESULT ThicknessRenderPS20_8bpc::SetToRenderOccludersDepth( ThicknessRenderTargetsPS20_8bpc * pTargets,
															 FogTombShaders8bpc * pShaders,
															 bool bDither )
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( pTargets == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( pShaders == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );

	hr = pTargets->SetToOccludersDepth();

	// Clear front solid object depth color texture and shared depth buffer
	// Clear to white since that is the farthest possible depth value
	m_pD3DDev->Clear( 0, NULL,
						D3DCLEAR_TARGET | pTargets->GetDepthClearFlags(),
						0xFFFFFFFF, 1.0, 0 );

	// Front facing tris only
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,				D3DCMP_LESSEQUAL );
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_SPECULARENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,			false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		                                               D3DCOLORWRITEENABLE_RED |
													   D3DCOLORWRITEENABLE_GREEN | 
													   D3DCOLORWRITEENABLE_BLUE );

	// Sets point filtering for RGB depth encoding.  D3DTEXF_LINEAR filtering would 
	//  corrupt the RGB-encoded depth information.
	int tcc;
	for( tcc=0; tcc < 3; tcc++ )
	{
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );

		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );
	}

	// Set textures for endocding high precision depth as an a8r8g8b8 color
	m_pD3DDev->SetTexture( 0, *(m_RenderProperties.m_ppTexRedGreenRamp) );
	m_pD3DDev->SetTexture( 1, *(m_RenderProperties.m_ppTexBlueRamp) );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );

	// dithering of RGB-encoded depth information on or off
	// The constant is used to generate widely varying texture coordinate .y value based on screen space position and depth.
	if( bDither )
		m_pD3DDev->SetVertexShaderConstantF( CV_DITHER_CONTROL, D3DXVECTOR4( 9.0f, 23.7f, 33.3f, -3.0f ), 1 );
	else
		m_pD3DDev->SetVertexShaderConstantF( CV_DITHER_CONTROL, D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), 1 );

	float fDS = 1.0f;
	D3DXVECTOR4 vec;
	float red, green, blue;
	red		= fDS * m_RenderProperties.m_fScale;		// for red texture encoding ramps
	green	= red * m_RenderProperties.m_fGrnRamp;		// for green texture encoding ramps
	blue	= red * m_RenderProperties.m_fBluRamp;
	vec = D3DXVECTOR4( red, green, blue, 0.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_RAMPSCALE, vec, 1 );

	m_pD3DDev->SetVertexShaderConstantF( CV_NORMALIZEWDEPTH, m_RenderProperties.m_NormalizeWDepth, 1 );

	vec = D3DXVECTOR4( m_RenderProperties.m_fBitReduce, 0.0f, 0.0f, 0.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_BITREDUCE, vec, 1 );

	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1,	D3DXVECTOR4( 0.0f, 0.5f, 1.0f, 2.0f ), 1 );

	D3DXVECTOR4 * pVec;
	pVec = & ( pTargets->m_HalfTexelSize );
	m_pD3DDev->SetVertexShaderConstantF( CV_HALF_TEXEL_SIZE, (float*) pVec, 1 );

	m_pD3DDev->SetVertexShaderConstantF( CV_RGB_TEXADDR_WEIGHTS, m_RenderProperties.m_vTexAddrWeights, 1 );

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	pSM->SetShader( pShaders->m_VSHI_DepthToTexcrdForRGB );
	pSM->SetShader( pShaders->m_PSHI_DepthToRGBEncode );

	return( hr );
}


HRESULT ThicknessRenderPS20_8bpc::SetToRenderFrontFaceDepths( ThicknessRenderTargetsPS20_8bpc * pTargets,
																FogTombShaders8bpc * pShaders )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( m_pD3DDev );
	
	// Render front and back face depth acumulation textures
	// This sums the front face depths to one texture, and sums the
	//  backface depths to another texture.  The previously created
	//  solid object depth texture is sampled to handle solid objects
	//  intersecting the fog volumes.

	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_ALWAYS );

	// Additive blending
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,		D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_BLENDOP,		D3DBLENDOP_ADD );

	pTargets->SetToFrontFacesDepth();

	// Set texture 3 to previously rendered solid object depth texture
	const int proj_tex_stage = 3;

	IDirect3DTexture9 * pTexOccludersDepth;
	pTexOccludersDepth = pTargets->m_pTexOccludersDepth;
	m_pD3DDev->SetTexture( proj_tex_stage, pTexOccludersDepth );

	m_pD3DDev->Clear( 0, NULL,
						D3DCLEAR_TARGET,
						0x00, 1.0, 0 );

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	pSM->SetShader( pShaders->m_VSHI_DepthToTexcrdForRGB_TC4Proj );
	pSM->SetShader( pShaders->m_PSHI_DepthToRGBAndCompare_20 );

	m_pD3DDev->SetPixelShaderConstantF( CP_RGB_TEXADDR_WEIGHTS, m_RenderProperties.m_vTexAddrWeights, 1 );
	return( hr );
}

// 
// Call SetToRenderFrontFaceDepths(..) before calling this function if you want to use the 
//  shaders given in a FogTombShaders8bpc class.
// 
// You should set the cull mode to render only the back faces of volume objects
HRESULT ThicknessRenderPS20_8bpc::SetToRenderBackFaceDepths( ThicknessRenderTargetsPS20_8bpc * pTargets )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	FAIL_IF_NULL( pTargets );

	pTargets->SetToBackFacesDepth();
	m_pD3DDev->Clear( 0, NULL,
						D3DCLEAR_TARGET,
						0x00, 1.0, 0 );

	return( hr );
}


// Use this single pass difference and decode step with Pixel Shaders 2.0
//  in place of the two pass approach using RenderP13_SubtractFrontDepthsFromBackDepths
//  and RenderP13_ConvertDifferenceToFogColor
// Also blends fog color to scene rather than rendering it to texture
HRESULT ThicknessRenderPS20_8bpc::SetToRenderFogSubtractConvertAndBlend( ThicknessRenderTargetsPS20_8bpc * pTargets,
																		 FogTombShaders8bpc * pShaders,
																		 IDirect3DTexture9 * pThicknessToColorTexture )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pThicknessToColorTexture );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( m_pD3DDev );

	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_FALSE );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );

	// Point sample back face texture
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	// Point saple front face texture
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	// bilinear sample fog color ramp texture
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	// Clamp so brightest fog doesn't wrap back to zero
    m_pD3DDev->SetSamplerState( 2, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    m_pD3DDev->SetSamplerState( 2, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );


	pTargets->SetToDefaultBackbuffers();

	IDirect3DTexture9 *		pTexFrontFaceDepths;
	pTexFrontFaceDepths = pTargets->m_pTexFrontFacesDepth;

	IDirect3DTexture9 *		pTexBackFaceDepths;
	pTexBackFaceDepths = pTargets->m_pTexBackFacesDepth;

	m_pD3DDev->SetTexture( 0, pTexBackFaceDepths );			// back faces; positive depths
	m_pD3DDev->SetTexture( 1, pTexFrontFaceDepths );		// front faces; negative depths
	m_pD3DDev->SetTexture( 2, pThicknessToColorTexture );

	m_pD3DDev->SetPixelShaderConstantF( CP_RGB_TEXADDR_WEIGHTS, m_RenderProperties.m_vTexAddrWeights, 1 );

	// Does not require custom vertex shader for TextureDisplay like the 
	//  vs/ps 1.1 - 1.3 versions do.  Texture address is calculated from
	//  a pixel shader constant rather than a constant texture coordinate.
	
	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	pSM->SetShader( pShaders->m_PSHI_SubtractRGBAndGetFogColor_20 );

	// Default settings are to render with:
	// src + (1-src) * dest
	// You could use only additive blending (src+dest), but the fog would tend to not
	//  obscure things behind it as much.
	m_pD3DDev->SetRenderState( D3DRS_BLENDOP,	m_RenderProperties.m_dwVolumeColorToScreenBlendOp );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,	m_RenderProperties.m_dwVolumeColorToScreenSrcBlend );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,	m_RenderProperties.m_dwVolumeColorToScreenDestBlend );

	return( hr );
}

