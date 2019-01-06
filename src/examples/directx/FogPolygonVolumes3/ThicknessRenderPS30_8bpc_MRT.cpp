/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderPS30_8bpc_MRT_MRT.cpp

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
#include "ThicknessRenderPS30_8bpc_MRT.h"
#include "ThicknessRenderProperties.h"
#include "ThicknessRenderProperties8BPC.h"
#include "ThicknessRenderTargetsPS30_8bpc_MRT.h"

#include "FogTombDemo.h"
#include "FogTombShaders8bpc_MRT.h"

#include "..\MEDIA\programs\D3D9_FogPolygonVolumes3\Constants.h"

void ThicknessRenderPS30_8bpc_MRT::SRS_OrdinarySceneAndDepth( IDirect3DDevice9 * pDev )
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

	int tcc;
	for( tcc=0; tcc < 2; tcc++ )
	{
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );

		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( tcc, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );		// no trilinear
	}
	
	// Set the two stages for RGB-encoding to use point sampling
	m_pD3DDev->SetSamplerState( TEX_RG, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
	m_pD3DDev->SetSamplerState( TEX_RG, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
	m_pD3DDev->SetSamplerState( TEX_RG, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( TEX_RG, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( TEX_RG, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );

	m_pD3DDev->SetSamplerState( TEX_B, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
	m_pD3DDev->SetSamplerState( TEX_B, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
	m_pD3DDev->SetSamplerState( TEX_B, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( TEX_B, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( TEX_B, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );

}

HRESULT ThicknessRenderPS30_8bpc_MRT::SetToRenderOrdinarySceneAndDepth( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
																		FogTombShaders8bpc_MRT * pShaders,
																		bool bDither )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( m_pD3DDev );

	SRS_OrdinarySceneAndDepth( m_pD3DDev );

	// Set source textures to NULL before setting render targets.  This is to prevent a 
	//  render target texture from being both a source and target at the same time.
	// This is needed for textures 0 and 1 which are the render target textures bound
	//  in the last step of the volume object rendering.
	m_pD3DDev->SetTexture( 0, NULL );
	m_pD3DDev->SetTexture( 1, NULL );

	// Set to use MRT rendering.  One a8r8g8b8 buffer for the ordinary shading, and another a8r8g8b8
	//  buffer to hold the RGB-encoded depth of the solid objects in the scene.
	hr = pTargets->SetToBuffersForOrdinarySceneAndDepth();

	// No need to clear the color buffers
	hr = m_pD3DDev->Clear( 0, NULL, 
							D3DCLEAR_ZBUFFER,
							0xFFFFFFFF,
							1.0f, 0 );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	m_pD3DDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
	m_pD3DDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );
	m_pD3DDev->SetTextureStageState( 4, D3DTSS_TEXCOORDINDEX, 4 );
	m_pD3DDev->SetTextureStageState( 5, D3DTSS_TEXCOORDINDEX, 5 );
	m_pD3DDev->SetTextureStageState( 6, D3DTSS_TEXCOORDINDEX, 6 );

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	FAIL_IF_NULL( pSM );
	pSM->SetShader( pShaders->m_VSHI_DiffuseAndRGBDepthEncode );
	pSM->SetShader( pShaders->m_PSHI_DiffuseAndRGBDepthEncode );

	D3DXVECTOR4 light_dir( 1.0f, -1.0f, 1.0f, 1.0f );
	D3DXVec4Normalize( &light_dir, &light_dir );
	m_pD3DDev->SetVertexShaderConstantF( CV_DIRLIGHT1_DIR, light_dir, 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1, D3DXVECTOR4( 0.0f, 0.5f, 1.0f, 2.0f ), 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_DIRLIGHT1_COLOR, D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ), 1 );

	m_pD3DDev->SetTexture( 0, NULL );		// Set this yourself when rendering the ordinary objects
	m_pD3DDev->SetTexture( TEX_RG, *(m_RenderProperties.m_ppTexRedGreenRamp) );
	m_pD3DDev->SetTexture( TEX_B,  *(m_RenderProperties.m_ppTexBlueRamp) );

	// Set constants for RGB-encoding of depth information
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

	return( hr );
}

// Set render target to a floating point surface.
// You should have called SetToRenderOrdinarySceneAndDepth(..) before this function in order
//  to set the proper shader constant values.
HRESULT ThicknessRenderPS30_8bpc_MRT::SetToRenderVolumeObjectThickness( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
																		FogTombShaders8bpc_MRT * pShaders )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( m_pD3DDev );

	hr = pTargets->SetToFaceDepthSum();

	m_pD3DDev->Clear( 0, NULL,						// count, rects*
						D3DCLEAR_TARGET,			// no z buffer, so do not clear Z
						0x00, 1.0, 0 );				// clear to black

	// Set t0 texture to the render target texture into which we rendered the depth of solid scene objects
	//  in the previous step.  This is done in order to handle intersections between the volume objects
	//  and the ordinary scene objects.  It also handles cases where scene objects occlude the volume objects.
	hr = m_pD3DDev->SetTexture( 0, pTargets->m_pTexOccludersDepth );
	
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_ALWAYS );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );

	// Additive blending
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,		D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_BLENDOP,		D3DBLENDOP_ADD );

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	pSM->SetShader( pShaders->m_VSHI_RGBEncodeAndCompare_30 );
	pSM->SetShader( pShaders->m_PSHI_RGBEncodeAndCompare_30 );

	m_pD3DDev->SetPixelShaderConstantF( CP_RGB_TEXADDR_WEIGHTS, m_RenderProperties.m_vTexAddrWeights, 1 );

	return( S_OK );
}


HRESULT ThicknessRenderPS30_8bpc_MRT::SetToRenderSceneAndVolumeColorToBackbuffer( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
																				  FogTombShaders8bpc_MRT * pShaders,
																				  IDirect3DTexture9 * pTexColorRamp )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( pTexColorRamp );
	FAIL_IF_NULL( m_pD3DDev );

	// Render ordinary scene and fog volume color to the backbuffer
	// Z buffer is not needed
	pTargets->SetToFlipChainBackbuffers();
	m_pD3DDev->SetDepthStencilSurface( NULL );

	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,				D3DZB_FALSE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,			false );

	m_pD3DDev->SetTexture( 0, pTargets->m_pTexOrdinaryShading );
	m_pD3DDev->SetTexture( 1, pTargets->m_pTexFPFaceDepthSum );
	m_pD3DDev->SetTexture( 2, pTexColorRamp );

	ShaderManager * pSM;
	pSM = pShaders->GetShaderManager();
	FAIL_IF_NULL( pSM );
	pSM->SetShader( pShaders->m_PSHI_RGBThicknessToFogColorAndSceneBlend );

	m_pD3DDev->SetPixelShaderConstantF( CP_RGB_TEXADDR_WEIGHTS, m_RenderProperties.m_vTexAddrWeights, 1 );

	// point sample the rendering of the ordinary scene
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );

	// point sample the thickness texture
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );

	// bilinear sample the color ramp texture
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_NONE  );

	return( hr );
}

