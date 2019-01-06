/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  WaterInteractionDemo.cpp

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

#include "PA_Water.h"
#include "WaterCoupler.h"
#include "WaterInteractionDemo.h"
#include <ShaderManager.h>
#include <MouseUI.h>
#include <D3DDeviceAndHWInfo.h>
#include <MeshVBDot3.h>
#include <MeshVB.h>

#include "..\Media\Programs\D3D9_WaterInteraction\WaterInteractionConstants.h"

#define FILE_Dot3x2EMBM_Displace		"Media\\Programs\\D3D9_WaterInteraction\\Dot3x2EMBM_Displace.vsh"
#define FILE_Dot3Vertex_transform		"Media\\Programs\\D3D9_WaterInteraction\\Dot3Vertex_transform.vsh"
#define FILE_Dot3x2EMBM_DisplacePSH		"Media\\Programs\\D3D9_WaterInteraction\\Dot3x2EMBM_Displace.psh"

#define FILE_TOP_128_2					"Media\\textures\\2D\\top_128_2.tga"

#define INIT_BUMPSCALE		0.12f

// defined elsewhere
void	GetRayFromViewportCoord( D3DXVECTOR3 * out_direction, D3DXVECTOR3 * out_origin,
								 float vp_x, float vp_y,
								 const D3DXMATRIX * pmatView,
								 const D3DXMATRIX * pmatProj );

//---------------------------------------------------------

WaterInteractionDemo::WaterInteractionDemo()
:	m_eDisplayOption(DISPLAY_BLINN8BITSIGNED),
	m_pVertexBuffer(NULL),
    m_pIndexBuffer(NULL),
	m_pUI(NULL),
	m_bWireframe(false),
	m_fBumpScale(INIT_BUMPSCALE)
{
	m_pD3DDev = NULL;
	m_PSHI_Dot3x2EMBM_Displace	= ShaderManager::SM_INDEX_UNSET;
	m_VSHI_Dot3x2EMBM_Displace	= ShaderManager::SM_INDEX_UNSET;
	m_VSHI_Dot3Transform		= ShaderManager::SM_INDEX_UNSET;
	m_pShaderManager = NULL;
	m_pWaterCoupler = NULL;
	m_pSkyTexture = NULL;
}

WaterInteractionDemo::~WaterInteractionDemo()
{
	Free();	
}

HRESULT WaterInteractionDemo::ConfirmDevice( D3DCAPS9 * pCaps, DWORD dwBehavior, D3DFORMAT Format)
{
	if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
	{
		FMsg( TEXT("Device does not support cubemaps!"));
		return E_FAIL;
	}
	if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		FMsg( TEXT("Device does not support 3 element texture coordinates!"));
		return E_FAIL;
	}
	if (!(pCaps->MaxTextureBlendStages >= 4))
	{
		FMsg( TEXT("Not enough texture blend stages!"));
		return E_FAIL;
	}
	if(D3DSHADER_VERSION_MAJOR(pCaps->PixelShaderVersion) < 1)
	{
		FMsg( TEXT("Device does not support pixel shaders!"));
		return E_FAIL;
	}
	return S_OK;
}

HRESULT WaterInteractionDemo::Free()
{
	SAFE_DELETE( m_pShaderManager );
	SAFE_DELETE( m_pWaterCoupler );
	SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );
	SAFE_DELETE( m_pUI );
	SAFE_RELEASE( m_pSkyTexture );
	SAFE_RELEASE( m_pD3DDev );
	return S_OK;
}

HRESULT WaterInteractionDemo::Initialize( IDirect3DDevice9 * pDev )
{
	RET_VAL_IF( pDev == NULL, E_FAIL );
	HRESULT hr;
	vector<DWORD> Declaration, PatchDeclaration;
	m_pD3DDev = pDev;
	pDev->AddRef();

	m_bMouseIsDisplacingWater	= false;
	m_bMouseMoveMeansDrawing	= true;
	m_bDisplayBlenderObj		= false;
	m_nDisplayMode				= DM_REFLECTION;

	//initialize mouse UI
	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );
	rect.bottom = viewport.Height;
	rect.right  = viewport.Width;

	m_pUI = new MouseUI( (const RECT)rect );
	RET_VAL_IF( m_pUI == NULL, E_FAIL );

	// Must Reset BEFORE setting translatin sens factor
	// Send the matrix controller some input to start off in a good orientation
	m_pUI->Reset();
	m_pUI->Translate( 0.0f, 0.0f, 0.37f );
	m_pUI->Rotate( 0.0f, 0.0f, 0.7f + 3.14159f );
	m_pUI->Rotate( 0.0f, 1.1f, 0.0f );
	m_pUI->SetTranslationalSensitivityFactor( 0.1f );

	m_nSimulationStepsPerSecond	= 35;
	m_fLastProcTexUpdateTime	= - 2.0f / m_nSimulationStepsPerSecond;
	m_nLastMouseX				= 0;
	m_nLastMouseY				= 0;
	m_fDispSc					= 0.8f; 
	m_fTxCrdDispScale			= 0.3f;
	//  allocate class for creating dynamic normal maps
	m_pWaterCoupler = new WaterCoupler;
	RET_VAL_IF( m_pWaterCoupler == NULL, E_FAIL );

	m_water_desc.detail_res_x = 256;
	m_water_desc.detail_res_y = 256;
	m_water_desc.num_tiles_x = 16;
	m_water_desc.num_tiles_y = 16;
	m_water_desc.tile_width = 0.2f;
	m_water_desc.tile_height = 0.2f;
	m_water_desc.tile_res_x = 256;
	m_water_desc.tile_res_y = 256;
	m_pWaterCoupler->Initialize( pDev, m_water_desc );
	m_bShowProceduralMaps = false;

	m_pShaderManager = new ShaderManager;
	MSG_BREAK_AND_RET_VAL_IF( m_pShaderManager == NULL, TEXT("WaterInteractionDemo couldn't create shadermanager\n"), E_FAIL );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_Dot3x2EMBM_Displace ), SM_SHADERTYPE_VERTEX, 
												 &m_VSHI_Dot3x2EMBM_Displace );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") TEXT(FILE_Dot3x2EMBM_Displace) TEXT("\n"), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_Dot3x2EMBM_DisplacePSH ), SM_SHADERTYPE_PIXEL, 
												 &m_PSHI_Dot3x2EMBM_Displace );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") TEXT(FILE_Dot3x2EMBM_DisplacePSH) TEXT("\n"), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_Dot3Vertex_transform ), SM_SHADERTYPE_VERTEX, 
												 &m_VSHI_Dot3Transform );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") TEXT(FILE_Dot3Vertex_transform) TEXT("\n"), hr );

	// Load environment map texture
	hr = D3DXCreateTextureFromFileEx( pDev,
		GetFilePath::GetFilePath( TEXT( FILE_TOP_128_2 )).c_str(),
		D3DX_DEFAULT,				// width
		D3DX_DEFAULT,				// height 
		0,							// mip
		0,							// usage
		D3DFMT_UNKNOWN,				// format
		D3DPOOL_DEFAULT,			// pool
		D3DX_FILTER_BOX,
		D3DX_FILTER_BOX,
		0,							// 0 for no color key
		NULL,
		NULL,
		& m_pSkyTexture );

	if (FAILED(hr))
	{
		assert( false );
		return hr;
	}

	// Camera stuff
	m_vEyePt				= D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
	D3DXVECTOR3 vUp			= D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
	// View
	D3DXMatrixLookAtLH( &m_matView, &m_vEyePt, &vLookatPt, &vUp);
	// Projection
	D3DXMatrixPerspectiveFovLH( &m_matProj, D3DXToRadian(60.0f), 1.0f, 0.01f, 70.0f);
	D3DXMatrixMultiply( &m_matViewProj, &m_matView, &m_matProj );
	m_pD3DDev->SetRenderState(D3DRS_SPECULARENABLE, FALSE);

	// Try a clear with stencil to determine flags for 
	//  future clears
//	m_cClearColor = D3DCOLOR_XRGB( 0xAA, 0x00, 0x00 );
//	m_cClearColor = D3DCOLOR_XRGB( 0x22, 0x99, 0x22 );		// green
	m_cClearColor = D3DCOLOR_XRGB( 0x22, 0x22, 0x78 );		// dark blue
	D3DDeviceAndHWInfo hwi;
	hwi.Initialize( m_pD3DDev );
	m_dwZClearFlags = hwi.GetDepthClearFlags();

	return S_OK;
}

HRESULT WaterInteractionDemo::Tick( double fGlobalTimeInSeconds )
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pWaterCoupler == NULL, E_FAIL );

	// Update the procedural textures at a fixed rate independent of framerate.
	// This allows the demo framerate to be much higher than the dynamic texture update rate
	//  and provides an appropriate time scale for the procedural texture simulation relative
	//  to the demo's framerate.
	if( fGlobalTimeInSeconds - m_fLastProcTexUpdateTime > 1.0f / (float)m_nSimulationStepsPerSecond )
	{
		hr = m_pWaterCoupler->Tick();
		BREAK_IF( FAILED(hr) );
		m_fLastProcTexUpdateTime = fGlobalTimeInSeconds;
	}

	// Restore render state because it may have been polluted by water animation
	RestoreRenderState();

	// Option to show the procedural map textures without using them to render 
	//  a reflective water effect.
	if( m_bShowProceduralMaps )
	{
		if( m_pWaterCoupler->m_pCA_WaterDetail )
			m_pWaterCoupler->m_pCA_WaterDetail->Diag_RenderResultToScreen();
		// return here so we can skip rendering the reflective water
		return( hr );
	}


	// Clear color and z
	hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | m_dwZClearFlags,
							m_cClearColor, 1.0, 0);
	m_pD3DDev->SetRenderState( D3DRS_FILLMODE, m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

	switch( m_nDisplayMode )
	{
	case DM_REFLECTION	:
		SetRender_Reflection();
		break;
	case DM_NORMALMAP :
	case DM_WATERSTATETEXTURE :
		SetRender_NrmlMapDisplay();
		break;
	}
	// Draw the reflective water objects
	SubmitWaterObjects();

	return(hr);
}

void WaterInteractionDemo::SetNormalMap( IDirect3DTexture9 * pNormalMap )
{
	m_pD3DDev->SetTexture( 0, pNormalMap );
}

void WaterInteractionDemo::SetRender_Reflection()
{
	RET_IF( m_pD3DDev == NULL );
	RET_IF( m_pShaderManager == NULL );

	// General setup
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	m_pD3DDev->SetRenderState( D3DRS_WRAP0, 0 );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	D3DXVECTOR4 vec = D3DXVECTOR4( m_fDispSc, m_fDispSc, m_fDispSc, m_fDispSc ); 
	m_pD3DDev->SetVertexShaderConstantF( CV_DISPSCALE, (float*)&vec, 1 );

	m_pD3DDev->SetVertexShaderConstantF( CV_CALC_SXT, D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 0.0f), 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_BUMPSCALE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, m_fBumpScale), 1);

	vec = D3DXVECTOR4( m_fTxCrdDispScale, m_fTxCrdDispScale, m_fTxCrdDispScale, m_fTxCrdDispScale );
	m_pD3DDev->SetVertexShaderConstantF( CV_OFFSETSCALE, (float*)&vec, 1 );

	// Texture 0 is set elsewhere to be the normal map
	m_pD3DDev->SetTexture( 1, NULL);
	m_pD3DDev->SetTexture( 2, NULL);
	m_pD3DDev->SetSamplerState( 3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	m_pD3DDev->SetRenderState( D3DRS_WRAP3, 0 );
	m_pD3DDev->SetTexture( 3, m_pSkyTexture );

	// alpha blend for quasi-Fresnel term 
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_INVSRCALPHA );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );

	// Setup the vertex & pixel shaders
    m_pShaderManager->SetShader( m_VSHI_Dot3x2EMBM_Displace );	
	m_pShaderManager->SetShader( m_PSHI_Dot3x2EMBM_Displace );
}

void WaterInteractionDemo::SetRender_NrmlMapDisplay()
{
	RET_IF( m_pShaderManager == NULL );
	// Set render state to display Tex 0 on the objects.
	// No reflection calc, no fancy shading, just the texture 0
	//   as a decal texture.
	HRESULT hr;
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, false);

	// General setup
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_pD3DDev->SetRenderState( D3DRS_WRAP0, 0 );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Texture 0 is set elsewhere to be the normal map
	m_pD3DDev->SetTexture( 1, NULL );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );
	hr = m_pShaderManager->SetShader( m_VSHI_Dot3Transform );
	BREAK_IF( FAILED(hr) );

	m_pD3DDev->SetPixelShader( 0 );
	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, 0xFFFFFFFF );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_MODULATE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE 	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_TFACTOR 	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
}

void WaterInteractionDemo::SubmitWaterObjects()
{
	// Call one of the SetRender_..() functions before entry to this
	//  function, to establish the right render state.
	// This func sets object position, normal map texture, and
	//  submits geometry
	// Draws detail water object first, then tiled object
	//  so as to avoid problems with alpha blending
	// Z buffering should be on so that tiled water is NOT
	//  drawn underneath detail water
	assert( m_pWaterCoupler != NULL );
	HRESULT hr;
	WaterCoupler * pWC = m_pWaterCoupler;
	D3DXMATRIX matWorld;
	D3DXMATRIX matTemp;
	D3DXMATRIX matWorldViewProj;
	D3DXMATRIX matWVP_Tiled;
	D3DXMATRIX matWV_Tiled;
	D3DXMATRIX matWVP_Detail;
	D3DXMATRIX matWV_Detail;

	// Set matrices for tiled water object:
	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixMultiply( &m_matView,  &m_pUI->GetRotationMatrix(), &m_pUI->GetTranslationMatrix() );
	D3DXMatrixMultiply( &m_matViewProj, &m_matView, &m_matProj );
	D3DXMATRIX	matWorldView;
	D3DXMatrixMultiply( &matWorldView, &matWorld, &m_matView );
	D3DXMatrixMultiply( &matWorldViewProj, &matWorld, &m_matViewProj );
	
	// Transform & projection to clip space
	D3DXMatrixTranspose( &matWVP_Tiled, &matWorldViewProj);
	D3DXMatrixTranspose( &matWV_Tiled, &matWorldView);

	// Calculate eye position from inverse of WorldView matrix
	// Assuming object coords are in world space
	D3DXMATRIX  invWV;
	D3DXMatrixInverse( &invWV, NULL, &matWorldView );
	D3DXVECTOR4 outV, inV;
	inV = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f );
	D3DXVec4Transform( &outV, &inV, &invWV );
	// Homogenize and invert
	assert( outV.w != 0.0f );
	outV.x = - outV.x / outV.w;
	outV.y = - outV.y / outV.w;
	outV.z = - outV.z / outV.w;
	outV.w = outV.w / outV.w;
	outV.x = -outV.x;
	outV.y = -outV.y;
	// outV is now the eye location in obj space for the tiled water object
	//	FDebug("eye vec:  %f  %f  %f  %f\n", outV.x, outV.y, outV.z, outV.w );

	// Set matrices for the small detail normal map object
	// This object's texture is maintained by the WaterCoupler to
	//  blend seamlessly with the tiled normal map texture
	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixMultiply( &m_matView,  &m_pUI->GetRotationMatrix(), &m_pUI->GetTranslationMatrix() );
	D3DXMatrixMultiply( &m_matViewProj, &m_matView, &m_matProj );
	// Translate the detail object to it's proper place
	float x,y,z;
	m_pWaterCoupler->GetDetailObjCenter( &x, &y, &z );
	if( m_bDisplayBlenderObj )
	{
		// For debug - detail obj is centered about 0,0, but the 
		//   tile blending obj is from 0,0 to 1,1, so to display
		//   the blender object, you need to translate the center
		//   by half a tile width & height.
		x = x - m_water_desc.tile_width / 2.0f;
		y = y - m_water_desc.tile_height / 2.0f;
	}
	// Translate detail object to the right spot
	D3DXMatrixTranslation( &matTemp, x, y, z );
	D3DXVECTOR4  detailV;
	detailV = outV;
	detailV.x -= x;
	detailV.y -= y;
	detailV.z -= z;
	// This is now the eye location in obj space
	hr = m_pD3DDev->SetVertexShaderConstantF( CV_EYE_OBJSPC, (float*)&detailV, 1 );
	BREAK_IF( FAILED(hr) );

	D3DXMatrixMultiply( &matWorld, &matTemp, &matWorld );
	D3DXMatrixMultiply( &matWorldView, &matWorld, &m_matView );
	// Apply view * proj matrices to the world matrix
	D3DXMatrixMultiply( &matWorldViewProj, &matWorld, &m_matViewProj );
	// Transpose the matrix for setting as vertex shader constants
	// Alternate vshader math can remove the need for this transpose
	D3DXMatrixTranspose( &matWVP_Detail, &matWorldViewProj);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*)&matWVP_Detail(0, 0), 4);
	D3DXMatrixTranspose( &matWV_Detail, &matWorldView);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_0, (float*)&matWV_Detail(0, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_1, (float*)&matWV_Detail(1, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_2, (float*)&matWV_Detail(2, 0), 1);

	// If you want to rotate the cubemap, you need to apply the transform
	//  to the world matrix and submit that as the CV_BASISTRANSFORM_n
	//  matrix.
	// Here we do not, so just use the world matrix
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_0, (float*)&matWV_Detail(0, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_1, (float*)&matWV_Detail(1, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_2, (float*)&matWV_Detail(2, 0), 1);

	switch( m_nDisplayMode )
	{
	case DM_REFLECTION	:
	case DM_NORMALMAP :
		SetNormalMap( m_pWaterCoupler->GetDetailNormalMap() );
		break;
	case DM_WATERSTATETEXTURE :
		SetNormalMap( m_pWaterCoupler->m_pCA_WaterDetail->GetStateTexture() );
		break;
	}

	// Enable z bias to bring detail object in front of tiled water.
	float zbias_amt = -5.0f / (float)0xFFFFFF;		// assumes 24 bit zbuffer
	m_pD3DDev->SetRenderState( D3DRS_DEPTHBIAS, *((DWORD*)&zbias_amt) );
	if( m_bDisplayBlenderObj )
	{
		SetNormalMap( m_pWaterCoupler->GetTiledNormalMap() );
		m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, 0x00EEEEEE );
		m_pWaterCoupler->Dbg_RenderTiledToDetailedBlender();
	}
	else
	{
		D3DXVECTOR4 tmp( 1.0f, 1.0f, 1.0f, 1.0f );
		m_pD3DDev->SetVertexShaderConstantF( CV_TILE_SIZE, (float*)&tmp,    1);
		hr = pWC->m_pDetailWaterGeo->Draw();
		BREAK_IF( FAILED(hr) );
	}
	// Restore z bias
	float zero = 0.0f;
	m_pD3DDev->SetRenderState( D3DRS_DEPTHBIAS, *((DWORD*)&zero) );

	// Draw the reflective object with tiled normal map
	switch( m_nDisplayMode )
	{
	case DM_REFLECTION	:
	case DM_NORMALMAP :
		SetNormalMap( m_pWaterCoupler->GetTiledNormalMap() );
		break;
	case DM_WATERSTATETEXTURE :
		SetNormalMap( m_pWaterCoupler->m_pCA_WaterTiled->GetStateTexture() );
		break;
	}
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*)&matWVP_Tiled(0, 0), 4);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_0, (float*)&matWV_Tiled(0, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_1, (float*)&matWV_Tiled(1, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLD_2, (float*)&matWV_Tiled(2, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_0, (float*)&matWV_Tiled(0, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_1, (float*)&matWV_Tiled(1, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_BASISTRANSFORM_2, (float*)&matWV_Tiled(2, 0), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_EYE_OBJSPC, (float*)&outV, 1 );

	// Submit the geometry
	hr = pWC->m_pTiledWaterGeo->Draw();
	BREAK_IF( FAILED(hr) );
}

LRESULT WaterInteractionDemo::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int iMouseX, iMouseY;
	iMouseX = (short)LOWORD(lParam);
	iMouseY = (short)HIWORD(lParam);

	// if shift or tab are down, then mouse movement controls camera
	SHORT ks;
	ks = GetAsyncKeyState( VK_TAB );
	ks = ks | GetAsyncKeyState( VK_SHIFT );
	// if high bit is set, key is down
	if( ks >> (sizeof(ks)-1) == 1 )
		m_bMouseMoveMeansDrawing = false;
	else
		m_bMouseMoveMeansDrawing = true;

	D3DXVECTOR3 mouse_hit_water;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN :
		m_pUI->OnLButtonDown( iMouseX, iMouseY );
		if( m_bMouseMoveMeansDrawing )
			m_bMouseIsDisplacingWater = true;
		break;
	case WM_LBUTTONUP :
		m_pUI->OnLButtonUp( iMouseX, iMouseY );
		if( m_bMouseMoveMeansDrawing )
			m_bMouseIsDisplacingWater = false;
		break;
	case WM_MOUSEMOVE :
		if( m_bMouseIsDisplacingWater )
		{
			int dx, dy;
			GetMouseDelta( &dx, &dy, iMouseX, iMouseY );
			mouse_hit_water = GetMouseClickIntersectionWithWater( iMouseX, iMouseY );
			// Move the interaction point to the mouse hit point
			if( m_pWaterCoupler != NULL )
			{
				m_pWaterCoupler->SetDetailObjTarget( mouse_hit_water.x, mouse_hit_water.y );
				// Add a displacement to detailed texture 
				if( m_pWaterCoupler->m_pCA_WaterDetail != NULL )
				{
					m_pWaterCoupler->AddInteractionDisplacement();
				}
			}
			m_nLastMouseX = iMouseX;
			m_nLastMouseY = iMouseY;
		}
		else
		{
			m_nLastMouseX = iMouseX;
			m_nLastMouseY = iMouseY;
			m_pUI->OnMouseMove( iMouseX, iMouseY );
		}
		break;
	}
	return( 0 );
}


// Compute intersection of mouse cursur point and the water plane
// depending on the viewpoint held in the MouseUI
D3DXVECTOR3 WaterInteractionDemo::GetMouseClickIntersectionWithWater( int x_window_coord, int y_window_coord )
{
	BREAK_AND_RET_VAL_IF( m_pUI == NULL, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ));
	RECT winr;
	float fx, fy;
	winr = m_pUI->GetRECT();

	// Compute floating point window coord in [0,1] rnage
	// Invert y because windows mouse coords are invertex, with y=0 at the top
	fx =		((float)x_window_coord - (float)winr.left)/((float)winr.right - winr.left);
	fy = 1.0f - ((float)y_window_coord - (float)winr.top)/((float)winr.bottom - winr.top);
	// convert that to 3D viewport [-1,1] range so center of window is 0,0
	fx = fx * 2.0f - 1.0f;
	fy = fy * 2.0f - 1.0f;

	// Give the 2D mouse point a depth and transform it by the inverse view-projection matrix
	D3DXVECTOR3		dir, orig;
	GetRayFromViewportCoord( & dir, &orig, fx, fy, & m_matView, & m_matProj );
	m_vEyePtWorld = orig;
	m_vEyePickDir = dir;

	// Intersect pick vector with ground plane.  Plane is always z = 0
	// Construct plane from a point and normal
	D3DXPLANE	plane;
	D3DXPlaneFromPointNormal( &plane, & D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
								& D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) );
	// Intersect the plane & pick vector
	D3DXVECTOR3 hit, end;
	// intersection uses line start point and end point
	end = orig + dir;
	D3DXPlaneIntersectLine( &hit, &plane, &orig, &end );
	return( hit );
}

void WaterInteractionDemo::TryDrawDroplet( float x, float y )
{
	// Adds droplet to the queue for later rendering
	float fx,fy;
	RECT winr;
	float scale;
	if( m_pUI->IsInWindow( x,y ))
	{
		winr = m_pUI->GetRECT();
		fx = 1.0f - ((float)x - (float)winr.left)/((float)winr.right - winr.left);
		fy =        ((float)y - (float)winr.top)/((float)winr.bottom - winr.top);
		scale = 14.0f;
	}
}

void WaterInteractionDemo::RestoreRenderState()
{
	// call to reset render modes to those appropriate for
	//  rendering the reflective surface
	int i;
	for( i=0; i < 4; i++ )
	{
        m_pD3DDev->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	    m_pD3DDev->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	    m_pD3DDev->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT  );  // nearest level
	}
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
}

void WaterInteractionDemo::GetMouseDelta( int * dx, int * dy, int x, int y )
{
	*dx = - ( x - m_nLastMouseX );
	*dy = - ( y - m_nLastMouseY );
}

void WaterInteractionDemo::Keyboard( DWORD dwKey, UINT nFlags, bool bDown )
{
	bool result = false;
	bool send_to_water;

	switch( dwKey )
	{
	case VK_LEFT:
	case VK_RIGHT:
	case 'R':
	case 'T':
		send_to_water = false;
		break;
	default:
		send_to_water = true;
		break;
	}

	if( send_to_water )
	{
		result = m_pWaterCoupler->Keyboard( dwKey, nFlags, bDown );
		if( result == true )
		{
			return;			// no more keyboard processing!
		}
	}
	float dispinc = 0.02f;
	if( bDown )
	{
		switch( dwKey )
		{
		case 'R':
			m_fTxCrdDispScale -= 0.05f;
			FDebug("m_fTxCrdDispScale:  %f\n", m_fTxCrdDispScale );
			break;
		case 'T':
			m_fTxCrdDispScale += 0.05f;
			FDebug("m_fTxCrdDispScale:  %f\n", m_fTxCrdDispScale );
			break;
		case 'P':
			m_bDisplayBlenderObj = !m_bDisplayBlenderObj;
			FDebug("m_bDisplayBlenderObj = %s\n", m_bDisplayBlenderObj ? "TRUE" : "FALSE" );
			break;
		case 'A':
			break;
		case 'S':
			break;
		case 'M':
			m_nDisplayMode++;
			if( m_nDisplayMode >= DM_LAST )
			{
				m_nDisplayMode = 0;
			}
			FDebug("Display Mode: %d\n", m_nDisplayMode );
			break;
		case 'F':
			break;
		}
	}

	if( !bDown )
	{
		switch( dwKey )
		{
		case VK_NUMPAD8:
			m_pUI->Reset();
			m_pUI->Translate( 0.0f, 0.0f, -0.25f );
			m_pUI->OnLButtonDown( 50, 50 );
			m_pUI->OnMouseMove( 67, 57 );
			m_pUI->OnLButtonUp( 67, 57 );
			m_pUI->SetTranslationalSensitivityFactor( 0.1f );
            m_bWireframe = false;
			m_bShowProceduralMaps = false;
			break;
		case '5':
			ToggleShowProceduralMaps();
			break;
		}
	}
}

bool WaterInteractionDemo::ToggleShowProceduralMaps()
{
	m_bShowProceduralMaps = !m_bShowProceduralMaps;
	return( m_bShowProceduralMaps );
}

