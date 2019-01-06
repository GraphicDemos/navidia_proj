/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  SDK\DEMOS\Direct3D9\src\WaterInteraction\
File:  PA_Water.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
2/7/2005 - Greg James - Porting from DX8 =)
3/17/01 - Greg James - The class is getting a bit bloated & could use a good parent class.

-------------------------------------------------------------------------------|--------------------*/

#include <crtdbg.h>
#include <assert.h>
#include <string>

#include "PA_Water.h"
#include "Media\Programs\D3D9_WaterInteraction\WaterInteractionConstants.h"

#include <ShaderManager.h>
#include <TextureDisplay2.h>
#include <D3DDeviceAndHWInfo.h>

using namespace std;

#define FILE_TexCoord_4_Offsets_03					"Media\\Programs\\D3D9_WaterInteraction\\TexCoord_4_Offsets_03.vsh"

#define FILE_EqualWeightCombine_PostMult_AR			"Media\\Programs\\D3D9_WaterInteraction\\EqualWeightCombine_PostMult_AR.psh"
#define FILE_WaterAnimCalc_Step1					"Media\\Programs\\D3D9_WaterInteraction\\WaterAnimCalc_Step1.psh"
#define FILE_WaterAnimCalc_Step2					"Media\\Programs\\D3D9_WaterInteraction\\WaterAnimCalc_Step2.psh"
#define FILE_Create_NormalMap_SrcAlpha_Scale		"Media\\Programs\\D3D9_WaterInteraction\\Create_NormalMap_SrcAlpha_Scale.psh"
#define FILE_Create_Dot3x2EMBMMap_SrcAlpha_Scale	"Media\\Programs\\D3D9_WaterInteraction\\Create_Dot3x2EMBMMap_SrcAlpha_Scale.psh"
#define FILE_Create_EMBMMap_SrcAlpha				"Media\\Programs\\D3D9_WaterInteraction\\Create_EMBMMap_SrcAlpha.psh"

#ifndef ASSERT_IF_FAILED
	#define ASSERT_IF_FAILED( hres )	\
	{									\
		if( FAILED(hres) )				\
		   assert( false );				\
	}
#endif

//-------------------------------------------------------------------------
PA_Water::PA_Water()
{
	m_bWireframe				= false;
	SetAllNull();
}

PA_Water::~PA_Water()
{
	Free();
}

void PA_Water::SetAllNull()
{
	m_ppShaderManager				= NULL;
	m_ppITextureDisplay				= NULL;
	m_pTextureDisplay				= NULL;
	m_ppInteriorBoundariesTexture	= NULL;
	m_pDropletTexture				= NULL;

	m_VSHI_TexCoordOffset				= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_EqualWeight_PostMult			= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_WaterAnimStep_1				= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_WaterAnimStep_2				= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_NormalMapCreate_Alpha		= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_Dot3x2EMBMMapCreate_Alpha	= ShaderManager::SM_INDEX_UNSET;
	m_PSHI_CreateEMBM_A					= ShaderManager::SM_INDEX_UNSET;

	m_pVertexBuffer			= NULL;
	m_pVertexDecl			= NULL;
	m_pBackbufferColor		= NULL;
	m_pBackbufferDepth		= NULL;
	m_pD3DDev				= NULL;

	for ( int i = 0; i < kMaxNumTargets; ++i )
	{
        m_pRTTTexture[i] = 0;
		m_pRTTSurface[i] = 0;
	}
}

HRESULT PA_Water::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT Format)
{
	string erstr;

    // check vertex shading support
    if(D3DSHADER_VERSION_MAJOR(pCaps->VertexShaderVersion) < 1)
	{
		erstr = "Device does not support vertex shaders!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}
    if(D3DSHADER_VERSION_MINOR(pCaps->VertexShaderVersion) < 1)
	{
		erstr = "Device does not support 1.1 vertex shaders!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}

    // check simultaneous texture support
    if(pCaps->MaxSimultaneousTextures < 4)
	{
		erstr = "Device does not support 4 simultaneous textures!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}

    // check pixel shader support
    if(D3DSHADER_VERSION_MAJOR(pCaps->PixelShaderVersion) < 1)
	{
		erstr = "Device does not support pixel shaders!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}
	if(D3DSHADER_VERSION_MINOR(pCaps->PixelShaderVersion) < 1)
	{
		erstr = "Device does not support 1.1 pixel shaders!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}
	if(pCaps->MaxTextureBlendStages < 8)
	{
		erstr = "Device does not support 8 register combiners!";
		FDebug("%s\n", erstr.c_str() );
		return E_FAIL;
	}

	return S_OK;
}

HRESULT  PA_Water::LoadTexture( IDirect3DDevice9 * pD3DDev, tstring filename, 
									IDirect3DTexture9 ** ppTex )
{		
	if( ppTex == NULL )
	{
		FDebug("Can't load into a null handle!\n");
		assert( false );
		return( E_FAIL );
	}
	if( pD3DDev == NULL )
	{
		assert( false );
		return( E_FAIL );
	}

	HRESULT hr = S_OK;

	hr = D3DXCreateTextureFromFileEx( pD3DDev,
			GetFilePath::GetFilePath( filename.c_str() ).c_str(),
			D3DX_DEFAULT,				// width
			D3DX_DEFAULT,				// height 
			0,							// mip
			0,							// usage
			D3DFMT_UNKNOWN,				// format
			D3DPOOL_DEFAULT,			// pool
			D3DX_FILTER_BOX,
			D3DX_FILTER_BOX,
			0,							// 0 for no color key
			NULL,						// image info
			NULL,						// palette
			ppTex );

	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't create texture\n", E_FAIL );
	return( hr );
}

// Function name   : PA_Water::LoadTextureMaps
// Description     : Loads textures used in updating the water
// Return type     : void
void PA_Water::LoadTextureMaps()
{
	HRESULT hr;

	// m_pDropletTexture is a simple droplet shadpe used to distort the water surface.
	SAFE_RELEASE( m_pDropletTexture );
	hr = LoadTexture( m_pD3DDev, m_tstrDropletFilename, & m_pDropletTexture );
	BREAK_IF( FAILED(hr) );

	// Could also load the interior boundary condition texture here too:
	// m_ppInteriorBoundariesTexture
}

HRESULT PA_Water::Free()
{
	SAFE_RELEASE( m_pVertexBuffer );			// sets pointers to null after delete
	SAFE_RELEASE( m_pDropletTexture );
	SAFE_RELEASE( m_pVertexDecl );
	m_Droplets.clear();

	FREE_GUARANTEED_ALLOC( m_ppITextureDisplay, m_pTextureDisplay );
	m_ppITextureDisplay = NULL;

    for (int i = 0; i < kMaxNumTargets; ++i)
    {
        SAFE_RELEASE(m_pRTTSurface[i]);
        SAFE_RELEASE(m_pRTTTexture[i]);
    }
    SAFE_RELEASE( m_pBackbufferColor );
	SAFE_RELEASE( m_pBackbufferDepth );
	SAFE_RELEASE( m_pD3DDev );		// we AddRef()'d in Initialize
	SetAllNull();
	return S_OK;
}

HRESULT PA_Water::Initialize( IDirect3DDevice9 * pDev, int res_x, int res_y,
								tstring droplet_filename,
								DisplacementMapMode eMapMode,
								ShaderManager ** ppShaderManager,
								ITextureDisplay ** ppTextureDisplay )
{
	RET_VAL_IF( pDev == NULL, E_FAIL );
	RET_VAL_IF( ppShaderManager == NULL, E_FAIL );
	RET_VAL_IF( *ppShaderManager == NULL, E_FAIL );
	HRESULT hr;
    int	i;
	Free();	
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();			// released on Free()

	m_ppShaderManager = ppShaderManager;

	TextureDisplay2 ** ppTD2;
	GUARANTEE_ALLOCATED( ppTextureDisplay, ppTD2, m_pTextureDisplay, TextureDisplay2, Initialize(m_pD3DDev) );
	m_ppITextureDisplay = (ITextureDisplay**)((void**)ppTD2);

	MSG_AND_RET_VAL_IF( m_ppITextureDisplay == NULL, TEXT("PA_Water::Initialize pptd is null\n"), E_FAIL );
	MSG_AND_RET_VAL_IF( *m_ppITextureDisplay == NULL, TEXT("PA_Water::Initialize *pptd is null\n"), E_FAIL );
	ITextureDisplay * pTD;
	pTD = *m_ppITextureDisplay;
	pTD->AddTexture( &m_TD_Fullscreen, NULL, FRECT( 0.0f, 0.0f, 1.0f, 1.0f ) );
	pTD->AddTexture( &m_TD_UpperRight, NULL, FRECT( 0.5f, 0.0f, 1.0f, 0.5f ) );
	pTD->AddTexture( &m_TD_UpperLeft, NULL, FRECT( 0.0f, 0.0f, 0.5f, 0.5f ) );
	pTD->AddTexture( &m_TD_LowerRight, NULL, FRECT( 0.5f, 0.5f, 1.0f, 1.0f ) );
	pTD->AddTexture( &m_TD_LowerLeft, NULL, FRECT( 0.0f, 0.5f, 0.5f, 1.0f ) );
	pTD->AddTexture( &m_TD_Droplet, &m_pDropletTexture, FRECT( 0.0f, 1.0f, 0.0f, 1.0f ) );

	m_tstrDropletFilename = droplet_filename;
	m_Droplets.clear();
	SetRenderResultToScreen( false );

	// See class header for what each variable represents!		
	m_bReset					= true;
	m_bAnimate					= true;
	m_bSingleStep				= false;
	m_bWrap						= true;
	m_bCreateNormalMap			= true;
	m_bDgbDrawOutputToScreen	= false;
	m_bApplyInteriorBoundaries	= true;
	m_eDiagnosticDisplayMode	= ALL_TOGETHER;
	m_eDisplacementMapMode		= eMapMode;
	m_nFlipState				= 0;
	m_fNrmlSTScale				= 0.8f;
	m_fScrollU					= 0.0f;
	m_fScrollV					= 0.0f;
	m_fDropletMinSize			= 0.25f;
	m_fDropletMaxSize			= 0.35f;
	m_fWindY					= 0.00f;
	m_fWindX					= 0.00f;

	// create the vertex shaders
	ShaderManager * pSM = *m_ppShaderManager;
	hr = pSM->LoadAndAssembleShader( TEXT( FILE_TexCoord_4_Offsets_03 ), SM_SHADERTYPE_VERTEX, &m_VSHI_TexCoordOffset );

	// pixel shaders
	hr = pSM->LoadAndAssembleShader( TEXT( FILE_EqualWeightCombine_PostMult_AR ), SM_SHADERTYPE_PIXEL, &m_PSHI_EqualWeight_PostMult );

	hr = pSM->LoadAndAssembleShader( TEXT( FILE_WaterAnimCalc_Step1 ), SM_SHADERTYPE_PIXEL, &m_PSHI_WaterAnimStep_1 );

	hr = pSM->LoadAndAssembleShader( TEXT( FILE_WaterAnimCalc_Step2 ), SM_SHADERTYPE_PIXEL, &m_PSHI_WaterAnimStep_2 );

	// pixel shader for creating normal maps with scalable red & green components for the s and t axis
	hr = pSM->LoadAndAssembleShader( TEXT( FILE_Create_NormalMap_SrcAlpha_Scale ), SM_SHADERTYPE_PIXEL, &m_PSHI_NormalMapCreate_Alpha );

	hr = pSM->LoadAndAssembleShader( TEXT( FILE_Create_Dot3x2EMBMMap_SrcAlpha_Scale ), SM_SHADERTYPE_PIXEL, &m_PSHI_Dot3x2EMBMMapCreate_Alpha );

	hr = pSM->LoadAndAssembleShader( TEXT( FILE_Create_EMBMMap_SrcAlpha ), SM_SHADERTYPE_PIXEL, &m_PSHI_CreateEMBM_A );


    // get a pointer to the current back-buffer (so we can restore it later)
	// Need to do this before CreateTextureRenderTargets() because that function
	//  will attempt a switch.

	m_pD3DDev->GetRenderTarget( 0, &m_pBackbufferColor );
	m_pD3DDev->GetDepthStencilSurface( &m_pBackbufferDepth );

	// Set depth clear flags depending on whether or not the depth buffer has stencil
	D3DDeviceAndHWInfo hwi;
	hwi.Initialize( m_pD3DDev );
	m_dwZClearFlags = hwi.GetDepthClearFlags();

	m_Const1	= D3DXVECTOR4( 0.0f, 0.5f,  1.0f, 2.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1, (float*)&m_Const1,	1 );

	LoadTextureMaps();
	hr = CreateTextureRenderTargets( res_x, res_y );
	ASSERT_IF_FAILED(hr);

	// create vertex buffer 
	hr = m_pD3DDev->CreateVertexBuffer( 4 * sizeof(QuadVertex), 
									D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, 
									D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL );
	ASSERT_IF_FAILED(hr);
	if (FAILED(hr))
		return hr;

	QuadVertex      *pBuff;

	if (m_pVertexBuffer)
	{
		hr = m_pVertexBuffer->Lock(0, 4 * sizeof(QuadVertex),(void**)&pBuff, 0);
		ASSERT_IF_FAILED(hr);
		if (FAILED(hr))
		{
			FDebug("Couldn't lock buffer!\n");
			return hr;
		}
		float uv_base;
		float uv_max;
		uv_base = 0.0f;
		uv_max  = 1.0f; 
        for (i = 0; i < 4; ++i)
        {
            pBuff->Position = D3DXVECTOR3((i==0 || i==3) ? -1.0f : 1.0f,
                                          (i<2)          ? -1.0f : 1.0f,
                                          0.0f);
		    pBuff->Tex      = D3DXVECTOR2((i==0 || i==3) ? uv_base : uv_max , 
                                          (i<2)          ? uv_max : uv_base );
		    pBuff++;
        }
        m_pVertexBuffer->Unlock();
    }

	//@@ move to separate class/file/library!! =) 
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0, 12,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};
	hr = m_pD3DDev->CreateVertexDeclaration( decl, &m_pVertexDecl );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("PA_Water Couldn't create vertex decl\n"), E_FAIL );

	// Create a matrix to transform the quad created above to exactly
	//  cover the render target.  This will map pixels of a source 
	//  texture one-to-one to pixels of a render target of same resolution.
	D3DXMATRIX matWorld;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	D3DXMATRIX matViewProj;
	D3DXVECTOR3 const vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
	D3DXVECTOR3 const vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 const vUp       = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	// Set World, View, Projection, and combination matrices.
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
	D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);
    D3DXMatrixMultiply(&matViewProj, &matView, &matProj);
    // draw a single quad to texture:
    // the quad covers the whole "screen" exactly
	D3DXMatrixScaling( & matWorld, 2.0f, 2.0f, 1.0f);
	D3DXMatrixMultiply( & m_mWorldViewProj, &matWorld, &matViewProj);
    D3DXMatrixTranspose( & m_mWorldViewProj, &m_mWorldViewProj);
	m_nResX = res_x;
	m_nResY = res_y;
    CreateUVOffsets( m_nResX, m_nResY );
	// Clear initial textures, and set texture index variables to that
	//	for the first time step
	m_nTex_HeightSrc    = 0;
	m_pTex_HeightSrc	= m_pRTTTexture[m_nTex_HeightSrc];
	m_nForceStepOne		= 1;
	m_nTex_HeightTarg	= 2;
	// Could also use the old height source as the place to keep
	//  the normal map.  This would save one texture, but be unfriendly
	//  to the rendering of each step of the calculation.
	m_nTex_DisplTarg		= 3;

	// Clear initial textures to 0x80 == gray, which is 0 velocity, and
	//  effectively 0 position (equilib position is set in pixel shader 
	//  constant)
	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightTarg] );
	ASSERT_IF_FAILED( hr );
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0x80808080, 1.0f, 0 );

	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightSrc] );
	ASSERT_IF_FAILED( hr );
	hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0x80808080, 1.0f, 0 );
	RenderTarget_Restore();
	SetDefaultParameters();		// set defaults for physical params
    return hr;
}

HRESULT PA_Water::CreateTextureRenderTargets( int width, int height )
{
    HRESULT         hr;
	assert( width > 0 );
	assert( height > 0 );
	assert( m_pBackbufferColor != NULL );
	assert( m_pBackbufferDepth != NULL );

	int i;
	for( i=0; i < kMaxNumTargets; i++ )
	{
		if( m_pRTTSurface[i] != NULL )
		{
            SAFE_RELEASE(m_pRTTSurface[i]);
			m_pRTTSurface[i] = NULL;
		}
		if( m_pRTTTexture[i] != NULL )
		{            
            SAFE_RELEASE(m_pRTTTexture[i]);
			m_pRTTTexture[i] = NULL;
		}
	}
	string erstr;
    // create new textures just like the current texture
    for( i = 0; i < kMaxNumTargets; i++ )
    {
        hr = m_pD3DDev->CreateTexture( width, height, 1, 
                                      D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                      D3DPOOL_DEFAULT, &m_pRTTTexture[i], NULL );
        if (FAILED(hr))
        {
		    erstr = "Can't CreateTexture!\n";
			FDebug("%s\n", erstr.c_str() );
            assert(false);
            return E_FAIL;
        }
        hr = m_pRTTTexture[i]->GetSurfaceLevel(0, &m_pRTTSurface[i]);
        if (FAILED(hr))
        {
		    erstr = "Can't Get to top-level texture!\n";
			FDebug("%s\n", erstr.c_str() );
            assert(false);
            return E_FAIL;
        }
        // set our render target to the new and shiny textures without depth
        hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[i] );
        if (FAILED(hr))
        {
		    erstr = "Can't SetRenderTarget to new surface!\n";
			FDebug("%s\n", erstr.c_str() );
            assert(false);
            return E_FAIL;
        }
    }

    // switch back to conventional back-buffer
    hr = m_pD3DDev->SetRenderTarget( 0, m_pBackbufferColor );
	MSG_IF( FAILED(hr), TEXT("srt #01\n"));
	hr = m_pD3DDev->SetDepthStencilSurface( m_pBackbufferDepth );
    if (FAILED(hr))
    {
		erstr = "Can't SetRenderTarget to original back-buffer surfaces!\n";
		FDebug("%s\n", erstr.c_str() );
        assert(false);
        return E_FAIL;
    }
    return S_OK;
}

void PA_Water::DisplayParameters()
{
	FDebug("\n");
	FDebug("Simulation parameters:\n");
	FDebug(" m_fPosAtten =          %f\n", m_fPosAtten );
	FDebug(" m_fEqRestore_factor =  %f\n", m_fEqRestore_factor );
	FDebug(" m_fBlurDist =          %f\n", m_fBlurDist );
	FDebug(" m_fVelFactor =         %f\n", m_fVelFactor );
	FDebug(" m_fForceFactor =       %f\n", m_fForceFactor );
	FDebug(" m_fNrmlSTScale =       %f\n", m_fNrmlSTScale );
	FDebug(" m_fDropletFreq =       %f\n", m_fDropletFreq );
	FDebug("\n");
}

bool PA_Water::Keyboard( DWORD dwKey, UINT nFlags, bool bDown )
{
	// Return true if keyboard key handled.
	char charcode;
	// increments for values per key press
	float eqinc = 0.003f;
	float blurinc = 0.01f;
	float normalinc = 0.01f;
	float patteninc = 0.0015f;
	// limits on values 
	float	blur_limit = 50.0f;
	const float	windf		= 0.000390625f;
	const float	windlimit	= 3.0f;
	if( bDown )
	{
		// Use only capital letters here - these are VK_ codes!
		switch( dwKey )
		{
		case 'O':
			DisplayParameters();
			break;
		case VK_HOME:
			m_bReset = true;
			FDebug("Resetting simulation!\n");
			break;
		case VK_RIGHT:
			break;
		case VK_LEFT:
			break;
		case VK_UP:
			m_fDropletFreq += 0.05f;
			if( m_fDropletFreq > 1.0f )
				m_fDropletFreq = 1.0f;
			FDebug("Droplet Frequency: %f\n", m_fDropletFreq );
			return(true);
			break;
		case VK_DOWN:
			m_fDropletFreq -= 0.05f;
			if( m_fDropletFreq < 0.0f )
				m_fDropletFreq = 0.0f;
			FDebug("Droplet Frequency: %f\n", m_fDropletFreq );
			return(true);
			break;
		case VK_RETURN:
			// single step
			m_bAnimate = false;
			m_bSingleStep = !m_bSingleStep;
			return(true);
			break;
		case VK_SPACE:
			// start or stop the animation
			m_bAnimate = !m_bAnimate;
			return(true);
			break;
		case 'F':
			m_fPosAtten -= patteninc;
			FDebug("m_fPosAtten:  %f\n", m_fPosAtten );
			break;
		case 'G':
			m_fPosAtten += patteninc;
			FDebug("m_fPosAtten:  %f\n", m_fPosAtten );
			break;
		case 'I':
		case 'L':
			m_bApplyInteriorBoundaries = !m_bApplyInteriorBoundaries;
			FDebug("Apply interior boundaries: %s\n", m_bApplyInteriorBoundaries ? "TRUE" : "FALSE");
			return(true);
			break;
		case 'Z':
			if( m_fEqRestore_factor >= eqinc )
				m_fEqRestore_factor -= eqinc;
			else
				m_fEqRestore_factor = 0.0f;
			FDebug("m_fEqRestore_factor: %f\n", m_fEqRestore_factor );
			return(true);
			break;
		case 'X':
			if( m_fEqRestore_factor <= 1.0f - eqinc )
				m_fEqRestore_factor += eqinc;
			else
				m_fEqRestore_factor = 1.0f;
			FDebug("m_fEqRestore_factor: %f\n", m_fEqRestore_factor );
			return(true);
			break;
		case 'C':
			m_fBlurDist -= blurinc;
			if( m_fBlurDist < 0.0f )
				m_fBlurDist = 0.0f;
			FDebug("m_fBlurDist: %f\n", m_fBlurDist );
			CreateUVBlurOffsets();
			return(true);
			break;
		case 'V':
			m_fBlurDist += blurinc;
			if( m_fBlurDist > blur_limit )
				m_fBlurDist = blur_limit;
			FDebug("m_fBlurDist: %f\n", m_fBlurDist );
			CreateUVBlurOffsets();
			return(true);
			break;
		case 'B':
			m_bWrap = !m_bWrap;
			FDebug("Wrap mode:  %d\n", m_bWrap );
			return(true);
			break;
		case 'Y':
			m_fNrmlSTScale -= normalinc;
			if( m_fNrmlSTScale < 0.0f )
				m_fNrmlSTScale = 0.0f;
			FDebug("m_fNrmlSTScale  Displacement map scale: %f\n", m_fNrmlSTScale );
			return(true);
			break;
		case 'U':
			m_fNrmlSTScale += normalinc;
			if( m_fNrmlSTScale > 1.0f )
				m_fNrmlSTScale = 1.0f;
			FDebug("m_fNrmlSTScale  Displacement map scale: %f\n", m_fNrmlSTScale );
			return(true);
			break;
		case '1' :
			m_eDiagnosticDisplayMode = FULLSCREEN_STEP_1_CALC;
			FDebug("displaying neighbor calc = force texture\n");
			return(true);
			break;
		case '2':
			m_eDiagnosticDisplayMode = FULLSCREEN_DISPLACEMENT_MAP;
			FDebug("displaying result = velocity\n");
			return(true);
			break;
		case '3' :
			m_eDiagnosticDisplayMode = ALL_TOGETHER;
			FDebug("displaying all surfs\n");
			return(true);
			break;
		case '4':
			m_eDiagnosticDisplayMode = FULLSCREEN_FINALOUT;
			FDebug("displaying finalout = height field\n");
			return(true);
			break;
		case '`':
		case '~':
			FDebug("Loading new texture maps\n");
			LoadTextureMaps();
			return(true);
			break;
		case 'A':
			m_fWindY -= windf;
			if( m_fWindY < - windlimit )
				m_fWindY = - windlimit;
			FDebug("m_fWindY    %f\n", m_fWindY );
			CreateUVOffsets( m_nResX, m_nResY );
			return(true);
			break;
		case 'S':
			m_fWindY += windf;
			if( m_fWindY >  windlimit )
				m_fWindY =  windlimit;
			FDebug("m_fWindY    %f\n", m_fWindY );
			CreateUVOffsets( m_nResX, m_nResY );
			return(true);
			break;
		case 'W':
			m_bWireframe = !m_bWireframe;
			return(false);
			break;
		default:
			charcode = MapVirtualKey( dwKey, 2 ); // VK to char code
			switch( charcode )
			{
			case '<':
			case ',':
				m_fVelFactor *= 0.98f;
				FDebug("m_fVelFactor  (mass):  %f\n", m_fVelFactor );
				return(true);
				break;
			case '>':
			case '.':
				m_fVelFactor *= 1.02f;
				FDebug("m_fVelFactor  (mass):  %f\n", m_fVelFactor );
				return(true);
				break;
			case '[':
			case '{':
				m_fForceFactor *= 0.98f;
				FDebug("m_fForceFactor:  %f\n", m_fForceFactor );
				return(true);
				break;
			case ']':
			case '}':
				m_fForceFactor *= 1.02f;
				FDebug("m_fForceFactor:  %f\n", m_fForceFactor );
				return(true);
				break;
			};
		};
	}
	return( false );
}

HRESULT		PA_Water::RenderTarget_SetToCurrentStateTexture()
{
	assert( m_pD3DDev != NULL );
	assert( m_pRTTSurface[ m_nTex_HeightTarg ] != NULL );
	HRESULT hr;
	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[ m_nTex_HeightTarg ] );
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	ASSERT_IF_FAILED(hr);
	return( hr );
}

HRESULT		PA_Water::RenderTarget_Restore()
{
	// set back to normal backbuffer that was found when 
	//  this class was Initialized.
	assert( m_pD3DDev != NULL );
	assert( m_pBackbufferColor != NULL );
	HRESULT hr;
	hr = m_pD3DDev->SetRenderTarget( 0, m_pBackbufferColor );
	hr = m_pD3DDev->SetDepthStencilSurface( m_pBackbufferDepth );
	ASSERT_IF_FAILED(hr);
	return( hr );
}

void PA_Water::SetInitialRenderStates()
{
	int i;
	SetOffsets_ZeroOffsets();
	// Setup constants
	m_pD3DDev->SetVertexShaderConstantF(CV_ZERO,   D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
	m_pD3DDev->SetVertexShaderConstantF(CV_ONE,    D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
    // set up render state: disable all except texture stage 0 (see below)
    for(i = 0; i < 4; i++ )
    {
		m_pD3DDev->SetTextureStageState( i,D3DTSS_TEXTURETRANSFORMFLAGS,	 D3DTTFF_DISABLE );
		m_pD3DDev->SetTextureStageState( i,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i );
		m_pD3DDev->SetTextureStageState( i, D3DTSS_RESULTARG, D3DTA_CURRENT );
    }
    m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	// Set wrap to zero - It will wrap anyway and avoid the problems of
	//   setting cylindrical wrapping (D3DRS_WRAP0, D3DWRAP_U == cylindrical)
	//   for texture coords > 1.0f
	// For wrapped textures, you should use only the D3DTSS_ADDRESSU/V/W
	//   states, unless you are doing cube environment mapping.
    m_pD3DDev->SetRenderState( D3DRS_WRAP0, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP1, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP2, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP3, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP4, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP5, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP6, 0 );
    m_pD3DDev->SetRenderState( D3DRS_WRAP7, 0 );

	// Disable culling
    m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	FALSE );
    m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_FALSE  );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,			FALSE );
	m_pD3DDev->SetRenderState( D3DRS_DITHERENABLE,		FALSE );
	m_pD3DDev->SetRenderState( D3DRS_ALPHATESTENABLE,	FALSE );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_ONE  );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ZERO );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		FALSE );
	m_pD3DDev->SetRenderState( D3DRS_SPECULARENABLE,	FALSE );
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		FALSE );
	m_pD3DDev->SetRenderState( D3DRS_LIGHTING,			FALSE );
	m_pD3DDev->SetRenderState( D3DRS_COLORVERTEX,		FALSE );
	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
}

HRESULT PA_Water::Tick()
{	
	// Render a new frame, updating the texture render targets
	// At the end, this function resets the color & backbuffer targets to
	//   what they were upon entry to Tick().
	HRESULT hr;
	SetInitialRenderStates();

	// Get a pointer to the current render targets so we can restore them later
	m_pD3DDev->GetRenderTarget( 0, &m_pBackbufferColor );
	m_pD3DDev->GetDepthStencilSurface( &m_pBackbufferDepth );

	// Select offsets of zero
	// Do this if not doing a time step, but still displaying results
	SetOffsets_ZeroOffsets();

    // set render state 
	RET_VAL_IF( m_ppShaderManager == NULL, E_FAIL );
	RET_VAL_IF( *m_ppShaderManager == NULL, E_FAIL );
	ShaderManager * pSM = *m_ppShaderManager;
	hr = pSM->SetShader( m_VSHI_TexCoordOffset );
	ASSERT_IF_FAILED(hr);

	hr = m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof( QuadVertex ) );
	ASSERT_IF_FAILED(hr);
	hr = m_pD3DDev->SetVertexDeclaration( m_pVertexDecl );
	ASSERT_IF_FAILED(hr);

	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, &m_mWorldViewProj(0, 0), 4);

	if( m_bReset == true )
	{
		m_bReset = false;
		m_nFlipState = 0;
		FDebug("Water sim reset to initial conditions\n");
	}

	if( ((m_bAnimate == true) && ( m_bSingleStep != true ))
		|| ( m_bSingleStep == true ) )
	{
		// Do one time step of the water simulation
		DoSingleTimeStep_Optimized();
		m_bSingleStep = false;
	}

	if( m_bDgbDrawOutputToScreen == true )
	{
		Diag_RenderResultToScreen();			
	}
	else
	{
		// Do not display textures for the user to see.
		// Restore the default render targets in case the app is doing more rendering to the window
		hr = m_pD3DDev->SetRenderTarget( 0, m_pBackbufferColor );
		hr = m_pD3DDev->SetDepthStencilSurface( m_pBackbufferDepth );
	}

	m_pBackbufferColor->Release();
	m_pBackbufferDepth->Release();
	return( hr );
}

// Function name   : PA_Water::Diag_RenderResultToScreen
// Description     : Renders the textures to the screen, using m_ppITextureDisplay class
// This is not needed to run the simulation, but it useful for debugging what is going on.
// Return type     : 
void	PA_Water::Diag_RenderResultToScreen()
{
	RET_IF( m_ppShaderManager == NULL );
	RET_IF( *m_ppShaderManager == NULL );
	MSG_AND_RET_IF( m_ppITextureDisplay == NULL, "PA_Water  Can't render results to screen without m_ppITextureDisplay\n" );
	HRESULT hr;
	ITextureDisplay * pTD = *m_ppITextureDisplay;
	MSG_AND_RET_IF( pTD == NULL, TEXT("PA_Water  Diag_RenderResultToScreen TextureDisplay * is null!\n") );
	// Set render state to opaque to display each texture to the screen
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

	hr = m_pD3DDev->SetRenderTarget( 0, m_pBackbufferColor );
	hr = m_pD3DDev->SetDepthStencilSurface( m_pBackbufferDepth );
	ASSERT_IF_FAILED( hr );
	// No need to clear - We overdraw the whole backbuffer
	// No z test
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE  ); 
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	false );
	// turn off pixel shading
	m_pD3DDev->SetPixelShader( NULL );

	if( m_bWireframe )
	{
		// Wireframe to show that all information is in textures =)
		hr = m_pD3DDev->Clear( 0, NULL, D3DCLEAR_TARGET | m_dwZClearFlags,
								D3DCOLOR_XRGB( 0,0,0 ), 1.0, 0);
		ASSERT_IF_FAILED(hr);
		m_pD3DDev->SetRenderState(D3DRS_FILLMODE,	D3DFILL_WIREFRAME );
		// chances are the texture will be all dark, so render in solid color
		m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,  0xFFFFFFFF );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TFACTOR	);
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
	}
	else
	{
		// display texture color.
		m_pD3DDev->SetRenderState(D3DRS_FILLMODE,	D3DFILL_SOLID );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE	);
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
	}

	// Now the business of drawing the textures
	FRECT r;
	switch( m_eDiagnosticDisplayMode )
	{
	case FULLSCREEN_FINALOUT :
		// Draw quad over full display
		pTD->SetTexture( m_TD_Fullscreen, &m_pRTTTexture[m_nTex_HeightTarg] );
		hr = pTD->Render( m_TD_Fullscreen );
		break;
	case FULLSCREEN_DISPLACEMENT_MAP :
		// Draw quad over full display
		pTD->SetTexture( m_TD_Fullscreen, &m_pRTTTexture[m_nTex_DisplTarg] );
		hr = pTD->Render( m_TD_Fullscreen );
		break;
	case FULLSCREEN_STEP_1_CALC:
		pTD->SetTexture( m_TD_Fullscreen, &m_pRTTTexture[m_nForceStepOne] );
		hr = pTD->Render( m_TD_Fullscreen );
		break;
	case ALL_TOGETHER :
		// Draw all textures together, each to 1/4 of the display
		// Upper left is the height map
		pTD->SetTexture( m_TD_UpperLeft, &m_pRTTTexture[m_nTex_HeightTarg] );
		pTD->SetTexture( m_TD_UpperRight, &m_pRTTTexture[m_nForceStepOne] );
		pTD->SetTexture( m_TD_LowerRight, &m_pRTTTexture[m_nTex_DisplTarg] );
		pTD->SetTexture( m_TD_LowerLeft, &m_pRTTTexture[m_nTex_DisplTarg] );
		pTD->Render( m_TD_UpperLeft );
		pTD->Render( m_TD_UpperRight );
		pTD->Render( m_TD_LowerRight );
		pTD->Render( m_TD_LowerLeft );
		break;
	};

	// restore the vertex stream
	hr = m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof( QuadVertex ) );
	ASSERT_IF_FAILED(hr);
	hr = m_pD3DDev->SetVertexDeclaration( m_pVertexDecl );
	ASSERT_IF_FAILED(hr);
	// restore vertex shader
	(*m_ppShaderManager)->SetShader( m_VSHI_TexCoordOffset );
}

void	PA_Water::SetScrollAmount( float u, float v )
{
	// set amount to scroll the prev texture when rendering into the new.	
	m_fScrollU = u;
	m_fScrollV = v;	
}

void	PA_Water::SetDisplacementMapSTScale( float STScale )
{
	m_fNrmlSTScale = STScale;
}

void	PA_Water::SetBlurDistance( float fac )
{
	m_fBlurDist  = fac;
	CreateUVBlurOffsets();
}

void	PA_Water::SetEqRestoreFac( float fac )
{
	m_fEqRestore_factor = fac;
}

void	PA_Water::SetVelocityApplyFac( float fac )
{
	m_fVelFactor = fac;
}

void	PA_Water::SetBlendFac( float fac )
{
	m_fForceFactor = fac;
}


// Function name   : PA_Water::DrawInteriorBoundaryObjects
// Description     : Draws position constraints into the water animation texture.
// Position is kept in the blue and alpha channels, so only those channels are rendered to.
// This function should only be called in the appropriate place as the PA_Water simulation is
//  computing the next time step.
// The constraints are applied through ordinary rendering to the render target textures.
// You can render anything you like into the position.  You could also write another 
//  function to render into the force or velocity for different types of boundary conditions.
// Alpha blending is used to control the strength of the boundary condition.  
// The water droplet effect is created in a similar manner by rendering small perturbations
//  into the texture.  These perturbations spread out into waves.
// Return type     : void
void	PA_Water::DrawInteriorBoundaryObjects()
{
	HRESULT hr;
	BREAK_AND_RET_IF( m_pD3DDev == NULL );
	BREAK_AND_RET_IF( m_ppShaderManager == NULL );
	BREAK_AND_RET_IF( *m_ppShaderManager == NULL );
	RET_IF( m_ppInteriorBoundariesTexture == NULL );
	RET_IF( *m_ppInteriorBoundariesTexture == NULL );

	ShaderManager * pSM = *m_ppShaderManager;
	int i;
	// Just use one fixed object for the logo texture
	// constraint.  No reason you can't add more, or make the constraints move around.
	const int numobjects = 1;
	int ind = 0;
	float xmin[numobjects];
	float xmax[numobjects];
	float ymin[numobjects];
	float ymax[numobjects];
	DWORD color = 0x00000000;		// ARGB
	xmin[ind] = 0.0f;
	xmax[ind] = 1.0f;
	ymin[ind] = 0.0f;
	ymax[ind] = 1.0f;
	ind++;

	// Calc things from the data above to use in rendering
	float x_center[ numobjects ];
	float y_center[ numobjects ];
	float x_size[ numobjects ];
	float y_size[ numobjects ];

	for( i=0; i < numobjects; i++ )
	{
		x_center[i] = (xmin[i] + xmax[i])/2.0f;
		y_center[i] = (ymin[i] + ymax[i])/2.0f;
	
		x_size[i] = (xmax[i] - xmin[i]);
		y_size[i] = (ymax[i] - ymin[i]);
	}
	// Draw into the velocity texture:
	hr = pSM->SetShader( m_VSHI_TexCoordOffset );
	ASSERT_IF_FAILED(hr);
	// Set render mode to use source texture
	// Alpha determines fixed vs. free areas
	hr = m_pD3DDev->SetPixelShader( NULL );
	m_pD3DDev->SetTexture( 0, *m_ppInteriorBoundariesTexture );
	//  Additive blend
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA		);
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA	);

	// m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, color );	// ARGB
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_TFACTOR	);	// blue only
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_SELECTARG1	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE		);
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );

	// Set render color mask to only update position (blue, alpha) component
	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE,	D3DCOLORWRITEENABLE_BLUE |
														D3DCOLORWRITEENABLE_ALPHA );
	float x,y;
	for( i=0; i < numobjects; i++ )
	{
		// shift x,y into -1 to 1 range
		x = (x_center[i] - 0.5f)*2.0f;
		y = (y_center[i] - 0.5f)*2.0f;

		// alter matrix to place the droplet
		D3DXMATRIX matWorld;
		D3DXMATRIX matView;
		D3DXMATRIX matProj;
		D3DXMATRIX matViewProj;
		D3DXMATRIX matWorldViewProj;
		D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
		D3DXVECTOR3 vLookatPt = D3DXVECTOR3( x,    y,     0.0f );
		D3DXVECTOR3 vUp       = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
		// Set World, View, Projection, and combination matrices.
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
		// 2.0f cause translation extends from -1 to 1
		D3DXMatrixOrthoLH(&matProj, 2.0f, 2.0f, 0.2f, 20.0f);	// x,y, zmin zmax
		D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

		// draw a single quad to texture:
		// the quad covers the whole "screen" exactly
		D3DXMatrixScaling(&matWorld, x_size[i], y_size[i], 1.0f);
		D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
		D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
		m_pD3DDev->SetVertexShaderConstantF(CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4);
		// Draw the square object
		hr = m_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		ASSERT_IF_FAILED(hr);
	}
	// reset world view proj matrix
	m_pD3DDev->SetVertexShaderConstantF(CV_WORLDVIEWPROJ_0, &m_mWorldViewProj(0, 0), 4);
	// Set render color mask back to updating everything
	hr = m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED |
									D3DCOLORWRITEENABLE_GREEN |	D3DCOLORWRITEENABLE_BLUE |
									D3DCOLORWRITEENABLE_ALPHA );
}

void	PA_Water::AddDroplet( float x, float y, float scale )
{
	// Adds a droplet to the rendering queue.
	// These are then rendered when DrawDroplets() is called
	// Coords are from 0 to 1.0 across texture surface
	// Size of droplet is determined at draw time - Could add 
	//   another variable to override this
	D3DXVECTOR3	pos( x, y, scale );
	m_Droplets.push_back( pos );
}

void	PA_Water::DrawDroplets()
{
	RET_IF( m_pD3DDev == NULL );

	// This must be called during DoSingleTimeStep_Optimized()
	// It uses the render state color write mask to update only the velocity (green) channel
	//   of the simulation texture.
	// input x,y in range 0.0 to 1.0 to cover whole target
	HRESULT hr;
	int		i;
	float	x,y;

	// Rendering is happening in middle of physics calc.
	// Render target is already set appropriately in DoSingleTimeStep_Optimized()
	// Use the legacy SetTextureStageState pipeline with a 
	//  color mask to render only to the green 'velocity'
	//  channel of the texture.
	m_pD3DDev->SetPixelShader( NULL );
	// Texture 0 is the droplet texture
	// Target is the intermediate force accumulation texture.
	// The droplet texture is applied additively to the green channel only
	hr = m_pD3DDev->SetTexture( 0, m_pDropletTexture );
	ASSERT_IF_FAILED(hr);
	// Additive blend
	// Affect only the green velocity component 
	hr = m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_GREEN );
	ASSERT_IF_FAILED(hr);

	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE, false );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_TFACTOR	);	// blue only

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );

	// If have TextureDisplay class use it to minimize matrix multiplies
	//  and save CPU
	if( m_ppITextureDisplay != NULL )
	{
		RET_IF( m_ppITextureDisplay == NULL );
		ITextureDisplay * pTD = *m_ppITextureDisplay;
		RET_IF( pTD == NULL );
		float dx, dy, scale;
		FRECT r;
		//## Change to create a mesh of separate rectangles and make only 1 draw call
		for( i=0; i < m_Droplets.size(); i++)
		{
			// coords in m_Droplets are stored in 0,0 to 1,1 range
			scale = m_Droplets[i].z;
			dx =  scale / 2.0f;
			dy =  scale / 2.0f;
			r.left		= m_Droplets[i].x - dx;
			r.right		= m_Droplets[i].x + dx;
			r.bottom	= m_Droplets[i].y + dy;
			r.top		= m_Droplets[i].y - dy;
			// FDebug("size: %6.6f  left %f   right %f  top %f  bottom %f\n", scale, r.left, r.right, r.top, r.bottom );
			// FDebug("   x: %6.6f     y: %6.6f\n", m_Droplets[i].x, m_Droplets[i].y );
			pTD->SetTextureRect( m_TD_Droplet, FRECT( r.left, r.top, r.right, r.bottom ) );
			pTD->Render( m_TD_Droplet );
		}
		// restore the vertex stream
		hr = m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof(QuadVertex) );
		ASSERT_IF_FAILED(hr);
		hr = m_pD3DDev->SetVertexDeclaration( m_pVertexDecl );
		// restore vertex shader
		RET_IF( m_ppShaderManager == NULL );
		RET_IF( *m_ppShaderManager == NULL );
		ShaderManager * pSM = *m_ppShaderManager;
		pSM->SetShader( m_VSHI_TexCoordOffset );
	}

	// Clear the array of droplets that we've just drawn
	m_Droplets.clear();
	// Reset the color mask operation so that all colors will 
	//  be rendered to in future ops.
	hr = m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED |
									D3DCOLORWRITEENABLE_GREEN |	D3DCOLORWRITEENABLE_BLUE |
									D3DCOLORWRITEENABLE_ALPHA );
	ASSERT_IF_FAILED(hr);
	// Turn off alpha blending
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, false );
}

void	PA_Water::Do_CreateEMBMMapSteps()
{
	// Should be called only during Tick() as part of rendering process.
	HRESULT hr = S_OK;
	D3DXVECTOR4 offset( 0.0f, 0.0f, 0.0f, 0.0f);

	// Set render target first, so the source texture is never simultaneously 
	//   a source and target
	m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[ m_nTex_DisplTarg ] );
	m_pD3DDev->SetDepthStencilSurface( NULL );
	ASSERT_IF_FAILED( hr );

	hr = m_pD3DDev->SetTexture( 0,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 1,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 2,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 3,	m_pRTTTexture[m_nTex_HeightTarg] );
	ASSERT_IF_FAILED(hr);

	float pix_masks[4] = { m_fNrmlSTScale, m_fNrmlSTScale, m_fNrmlSTScale, m_fNrmlSTScale };

	m_pD3DDev->SetPixelShaderConstantF( PCN_EMBM_SCALE, (float*) &pix_masks, 1 );

	RET_IF( m_ppShaderManager == NULL );
	RET_IF( *m_ppShaderManager == NULL );
	ShaderManager * pSM = *m_ppShaderManager;

	hr = pSM->SetShader( m_PSHI_CreateEMBM_A );
	ASSERT_IF_FAILED(hr);
	hr = pSM->SetShader( m_VSHI_TexCoordOffset );
	ASSERT_IF_FAILED(hr);

	// use nearest neighbor offsets
	SetOffsets_NearestNeighbors();

	hr = m_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	ASSERT_IF_FAILED(hr);
}

void	PA_Water::Do_CreateDot3x2EMBMMapSteps()
{
	// Should use nearest-neighbor sampling
	// Should be called only during Tick() as part of rendering process.
	HRESULT hr = S_OK;
	D3DXVECTOR4 offset( 0.0f, 0.0f, 0.0f, 0.0f);
	// Set render target first, so the source texture is never simultaneously 
	//   a source and target
	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[ m_nTex_DisplTarg ] );	
	ASSERT_IF_FAILED( hr );
	m_pD3DDev->SetDepthStencilSurface( NULL );
	hr = m_pD3DDev->SetTexture( 0,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 1,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 2,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 3,	m_pRTTTexture[m_nTex_HeightTarg] );
	ASSERT_IF_FAILED(hr);

	// Set constants for red & green scale factors which also act 
	//  as essential color masks to update only the appropriate 
	//  components
	// Red mask first
	float pix_masks[4] = { m_fNrmlSTScale, 0.0f, 0.0f, 0.0f };
	m_pD3DDev->SetPixelShaderConstantF( PCN_RED_MASK, (float*)&pix_masks, 1 );
	// Now green mask & scale:
	pix_masks[0] = 0.0f;
	pix_masks[1] = m_fNrmlSTScale;
	m_pD3DDev->SetPixelShaderConstantF( PCN_GREEN_MASK, (float*)&pix_masks, 1 );
	
	RET_IF( m_ppShaderManager == NULL );
	RET_IF( *m_ppShaderManager == NULL );
	ShaderManager * pSM = *m_ppShaderManager;
	pSM->SetShader( m_PSHI_Dot3x2EMBMMapCreate_Alpha );
	pSM->SetShader( m_VSHI_TexCoordOffset );
	// use nearest neighbor offsets
	SetOffsets_NearestNeighbors();
	hr = m_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	ASSERT_IF_FAILED(hr);
}

void	PA_Water::Do_CreateNormalMapSteps()
{
	// Should be called only during Tick() as part of rendering process.
	HRESULT hr = S_OK;
	D3DXVECTOR4 offset( 0.0f, 0.0f, 0.0f, 0.0f);
	// Set render target first, so the source texture is never simultaneously 
	//   a source and target
	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[ m_nTex_DisplTarg ] );
	ASSERT_IF_FAILED( hr );
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	hr = m_pD3DDev->SetTexture( 0,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 1,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 2,	m_pRTTTexture[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetTexture( 3,	m_pRTTTexture[m_nTex_HeightTarg] );
	ASSERT_IF_FAILED(hr);

	// Set constants for red & green scale factors (also essential color masks)
	// Red mask first
	float pix_masks[4] = { m_fNrmlSTScale, 0.0f, 0.0f, 0.0f };
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_RED_MASK, (float*)&pix_masks, 1 );
	ASSERT_IF_FAILED(hr);
	// Now green mask & scale:
	pix_masks[0] = 0.0f;
	pix_masks[1] = m_fNrmlSTScale;
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_GREEN_MASK, (float*)&pix_masks, 1 );
	ASSERT_IF_FAILED(hr);

	RET_IF( m_ppShaderManager == NULL );
	RET_IF( *m_ppShaderManager == NULL );
	ShaderManager * pSM = *m_ppShaderManager;
	pSM->SetShader( m_PSHI_NormalMapCreate_Alpha );
	pSM->SetShader( m_VSHI_TexCoordOffset );

	// use nearest neighbor offsets
	SetOffsets_NearestNeighbors();
	hr = m_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	ASSERT_IF_FAILED(hr);
}

HRESULT PA_Water::DoSingleTimeStep_Optimized()
{
	HRESULT hr;
	int i;
	int tmp;

	RET_VAL_IF( m_ppShaderManager == NULL, E_FAIL );
	RET_VAL_IF( *m_ppShaderManager == NULL, E_FAIL );
	ShaderManager * pSM = *m_ppShaderManager;
	// Uses fewer render-to-texture passes and fewer intermediate textures
	//  to run the water simulation than the old CA_Water class did.
	// Swap texture source & target indices & pointers
	//  0 = start from initial loaded texture
	//  1/2 = flip flop back and forth between targets & sources

	switch( m_nFlipState )
	{
	case 0:
		// These are also set in the initialize
		m_nTex_HeightSrc    = 0;
		m_pTex_HeightSrc	= m_pRTTTexture[m_nTex_HeightSrc];
		m_nForceStepOne		= 1;
		m_nTex_HeightTarg	= 2;
		// Could also use the old height source as the place to keep
		//  the normal map.  This would save one texture, but be unfriendly
		//  to the rendering of each step of the calculation.
		m_nTex_DisplTarg		= 3;
		// Clear initial textures to 0x80 == gray, which is 0 velocity, and
		//  effectively 0 position (equilib position is set in pixel shader 
		//  constant)
		hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightTarg] );
		hr = m_pD3DDev->SetDepthStencilSurface( NULL );
		ASSERT_IF_FAILED( hr );
		hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0x80808080, 1.0f, 0 );
		hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightSrc] );
		hr = m_pD3DDev->SetDepthStencilSurface( NULL );
		ASSERT_IF_FAILED( hr );
		hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0x80808080, 1.0f, 0 );
		break;
	case 1:
		// Swap heigh source and target textures
		tmp = m_nTex_HeightTarg;
		m_nTex_HeightTarg	= m_nTex_HeightSrc;
		m_nTex_HeightSrc	= tmp;
		m_pTex_HeightSrc	= m_pRTTTexture[m_nTex_HeightSrc];
		m_nForceStepOne		= 1;
		m_nTex_DisplTarg	= 3;
		break;
	default:
		FDebug("Bad flip state encountered!\n");
		assert( false );
		break;
	}

    // variable for writing to constant memory which uv-offsets to use
    D3DXVECTOR4     offset(0.0f, 1.0f, 0.0f, 0.0f);
	m_pD3DDev->SetVertexShaderConstantF( CV_ONOFF_1, (float*)&offset,    1);

	//  Pixel shader computes a factor to bring the positions back to
	//   some equilibrium value.  Set the factor used to apply this 
	//   force to the total force from the neighbors.  
	//  Setting m_fEqRestore_factor to zero will zero out this equilib
	//   restoring force and the values will tend to saturate at either
	//   all 1.0 or all 0.0
    D3DXVECTOR4     tmp_vec( m_fEqRestore_factor, m_fEqRestore_factor, m_fEqRestore_factor, m_fEqRestore_factor );
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_EQ_REST_FAC, (float*) &tmp_vec, 1 );
	ASSERT_IF_FAILED(hr);

	// no need to clear temporary target
	// Render to texture as solid
	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID );
	DWORD wrapval = m_bWrap ?  D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
    for( i = 0; i < 4; i++ )
    {
		m_pD3DDev->SetSamplerState( i, D3DSAMP_ADDRESSU, wrapval );
		m_pD3DDev->SetSamplerState( i, D3DSAMP_ADDRESSV, wrapval );
		m_pD3DDev->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pD3DDev->SetSamplerState( i, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
	}

	// Set constant at which to apply force to velocity
	float pix_force_mult[4] = { 0.0f, m_fForceFactor, 0.0f, 0.0f };	// only green component!
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_MULTFACTOR_1, (float*)&pix_force_mult, 1 );
	ASSERT_IF_FAILED(hr);
	// Set constant at which to apply velocity to height
	float pix_vel_mult[4] = { 0.0f, 0.0f, m_fVelFactor, m_fVelFactor };		// only blue component
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_MULTFACTOR_2, (float*)&pix_vel_mult, 1 );
	ASSERT_IF_FAILED(hr);
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

	//  blur positions to smooth noise & generaly dampen things
	//  degree of blur is controlled by magnitude of 4 neighbor texel
	//   offsets with bilinear on
	//  Also replicate blue to alpha to ensure the two fields hold the
	//   same value.
	//  This step also takes care of translating by a given scroll
	//   amount, as when used by the WaterCoupler class to blend
	//   from a tiled texture to a detailed texture.
	// First, make sure we have properly computed blur & scroll values
	CreateUVBlurOffsets();

	// Set textures to NULL so a texture is never simultaneously a source 
	//  and target
	m_pD3DDev->SetTexture( 0, NULL );
	m_pD3DDev->SetTexture( 1, NULL );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );
	// At first I was thinking to skip the blur pass if the dist is
	//  set to zero for no blurring.  The lack of blur gives sharp
	//  checkerboard noise in the simulation, so it is good to always
	//  blur just a little bit.

	if( m_fBlurDist > 0.0f )
	{
		// only blur if taking the samples will really, in fact, blur!
		hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightTarg] );
		hr = m_pD3DDev->SetDepthStencilSurface( NULL );
		ASSERT_IF_FAILED( hr );
		m_pD3DDev->SetTexture( 0, m_pTex_HeightSrc );
		m_pD3DDev->SetTexture( 1, m_pTex_HeightSrc );
		m_pD3DDev->SetTexture( 2, m_pTex_HeightSrc );
		m_pD3DDev->SetTexture( 3, m_pTex_HeightSrc );

		hr = pSM->SetShader( m_PSHI_EqualWeight_PostMult );
		ASSERT_IF_FAILED( hr );
		// Select texture coordinate offsets for blurring
		SetOffsets_Blur();
		hr = m_pD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
		ASSERT_IF_FAILED(hr);
		// Swap height targ & src
		tmp = m_nTex_HeightTarg;
		m_nTex_HeightTarg = m_nTex_HeightSrc;
		m_nTex_HeightSrc = tmp;
	}

	m_pTex_HeightSrc	= m_pRTTTexture[m_nTex_HeightSrc];

	// With smoothed result texture selected as render target,
	//   render new excitation droplets at a certain frequency, up to
	//   one new drop per time step
	// No reason you can't call more AddDroplet() per time step
	// This should not change the texture filter mode
	float rnd_freq;
	rnd_freq = (float)rand()/((float)RAND_MAX);
	if( m_fDropletFreq > rnd_freq )
	{
		float scale = m_fDropletMinSize;
		scale += ( m_fDropletMaxSize - m_fDropletMinSize ) * ((float)rand()/((float)RAND_MAX));
		// Use these to restrict the droplet so it never crosses the texture
		//  border.  This is a bad idea in general, as it will make tiling more
		//  obvious.
		//		float x = scale / 2.0f + (1.0f - scale ) * (float)rand()/((float)RAND_MAX);
		//		float y = scale / 2.0f + (1.0f - scale ) * (float)rand()/((float)RAND_MAX);
		// Use these to not restrict placement based on size -- 
		// Feel free to render noise displacements that cross the edges, provided
		//  you add a copy to the opposite edge to preserve tiling and not make
		//  high-frequency artifacts along the texture edge.
		float x = (float)rand()/((float)RAND_MAX);
		float y = (float)rand()/((float)RAND_MAX);
		// Add a droplet to the temporary storage.
		// They will all be drawn in the call DrawDroplets() 
		AddDroplet( x, y, scale );
		// Now check if the droplet hits the texture edge, and if 
		//  so, add other droplets to the other edges to preserve
		//  tiling and not give rise to high frequency differences
		//  along the edges.
		if( x < scale / 2.0f )
		{
			AddDroplet( x + 1.0f, y, scale );
			if( y < scale / 2.0f )
			{
				AddDroplet( x,			y + 1.0f, scale );
				AddDroplet( x + 1.0f,	y + 1.0f, scale );
			}
			else if( y > 1.0f - scale / 2.0f )
			{
				AddDroplet( x,		  y - 1.0f, scale );
				AddDroplet( x + 1.0f, y - 1.0f, scale );
			}
		}
		else if( x > 1.0f - scale / 2.0f )
		{
			AddDroplet( x - 1.0f, y, scale );

			if( y < scale / 2.0f )
			{
				AddDroplet( x,		  y + 1.0f, scale );
				AddDroplet( x - 1.0f, y + 1.0f, scale );
			}
			else if( y > 1.0f - scale / 2.0f )
			{
				AddDroplet( x,		  y - 1.0f, scale );
				AddDroplet( x - 1.0f, y - 1.0f, scale );
			}
		}
		else
		{
			if( y < scale / 2.0f )
				AddDroplet( x, y + 1.0f, scale );
			else if( y > 1.0f - scale / 2.0f )
				AddDroplet( x, y - 1.0f, scale );
		}
	}

	DrawDroplets();
	//  Render first 3 components of force from three neighbors
	//  Offsets selected are 1 center texel for center height
	//    and 3 of the 4 nearest neighbors.  Texture selected
	//    is same for all stages as we're turning height difference
	//    of nearest neightbor texels into a force value.
	// Alphablend must be false
	m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nForceStepOne] );
	m_pD3DDev->SetDepthStencilSurface( NULL );

	pSM->SetShader( m_PSHI_WaterAnimStep_1 );

	// Select offsets for first step of accumulating force
	//   from neighbors
	SetOffsets_Step1();

	// No alpha blending - calculated value written opaque to the
	//  render target
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

	// Set same texture to all inputs, so that texels and their
	//  neighbors are sampled into the pixel shader.

	hr = m_pD3DDev->SetTexture( 0, m_pTex_HeightSrc );
	hr = m_pD3DDev->SetTexture( 1, m_pTex_HeightSrc );
	hr = m_pD3DDev->SetTexture( 2, m_pTex_HeightSrc );
	hr = m_pD3DDev->SetTexture( 3, m_pTex_HeightSrc );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	m_pD3DDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
	m_pD3DDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );

	// Draw the quad with texture coords set to sample neighboring texels
	//  in order to compute the force acting at each texel
	m_pD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

	// Part 2 of the neighbor force calculation
	// Add in the force contribution from the 4th neighbor
	// We didn't have enough texture lookups to do this in the first pass
	// Render to the final force texture 
	hr = m_pD3DDev->SetRenderTarget( 0, m_pRTTSurface[m_nTex_HeightTarg] );
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	ASSERT_IF_FAILED( hr );

	hr = pSM->SetShader( m_PSHI_WaterAnimStep_2 );
	ASSERT_IF_FAILED( hr );

	// Set offsets for step two.
	//  Here the force from the last neighbor point is calculated
	//  and the velocity & heights are updated.
	SetOffsets_Step2();

	// Set constant to attenuate position a small amount at each time step
	//   This is to dissipate energy and prevent the system from running
	//   away to saturation
	tmp_vec = D3DXVECTOR4( 0.0f, 0.0f, m_fPosAtten, m_fPosAtten );
	hr = m_pD3DDev->SetPixelShaderConstantF( PCN_POSITIONMASK, (float*) &tmp_vec, 1);
	ASSERT_IF_FAILED(hr);

	// t0 holds center texel position in blue and alpha, AND the partial
	//   force result from the first step in the red component
	// t1 holds the 4th neighbor sample point, and we care only about
	//   the height which is in blue and alpha.
	// t0 = center position (b,a), partial force result (r), velocity (g)
	// t1 = 4th neighbor point
	m_pD3DDev->SetTexture( 0, m_pRTTTexture[m_nForceStepOne] );
	m_pD3DDev->SetTexture( 1, m_pTex_HeightSrc );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );

	hr = m_pD3DDev->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
	// Apply position or other constraints to the height or velocity
	// Draw fixed objects within the texture
	if( m_bApplyInteriorBoundaries )
	{
		DrawInteriorBoundaryObjects();
	}

	// If selected, create a displacement map from the water height
	//  data
	switch( m_eDisplacementMapMode )
	{
	case	DM_NONE	:
		// Don't create a map!	
		break;
	case	DM_NORMAL_MAP	:
		Do_CreateNormalMapSteps();
		break;
	case	DM_DOT3X2_MAP	:
		Do_CreateDot3x2EMBMMapSteps();
		break;
	case	DM_EMBM_MAP		:
		Do_CreateEMBMMapSteps();
		break;
	default:
		FDebug("unknown displacement map mode!\n");
		assert( false );
		break;
	}

	// Flip the state variable for the next round of rendering
	switch( m_nFlipState )
	{
	case 0:
		m_nFlipState = 1;
		break;
	case 1:
		m_nFlipState = 1;
		break;
	default:
		FDebug("unknown flip state!!\n");
		assert( false );
		break;
	}
	return( hr );
}

IDirect3DTexture9 *		PA_Water::GetStateTexture()
{
	assert( m_pRTTTexture[m_nTex_HeightTarg] != NULL );
	return( m_pRTTTexture[m_nTex_HeightTarg] );
}

IDirect3DTexture9 *		PA_Water::GetPrevStateTexture()
{
	assert( m_pRTTTexture[ m_nTex_HeightSrc ] != NULL );
	return( m_pRTTTexture[ m_nTex_HeightSrc ] );
}

IDirect3DTexture9 *		PA_Water::GetOutputTexture()
{
	assert( m_pRTTTexture[m_nTex_DisplTarg] != NULL );
	return( m_pRTTTexture[m_nTex_DisplTarg] );
}

void PA_Water::SetOffsets( int set )
{
	// Set the vertex shader texture coordinate offset vectors to 
	//  one of the computed sets.
	m_pD3DDev->SetVertexShaderConstantF( CV_T0_OFFSET, & ( m_Offsets[set].t[0].x ), 4 );
}

void PA_Water::SetOffsets_ZeroOffsets()
{
	SetOffsets( 0 );
}
void PA_Water::SetOffsets_Step1()
{
	SetOffsets( 1 );
}
void PA_Water::SetOffsets_Step2()
{
	SetOffsets( 2 );
}
void PA_Water::SetOffsets_Blur()
{
	SetOffsets( 3 );
}
void PA_Water::SetOffsets_NearestNeighbors()
{
	SetOffsets( 4 );
}

void PA_Water::CreateUVBlurOffsets()
{
	int set;
	float scale;
	int i;
	float th, tw;
	tw = m_fPerTexelWidth;
	th = m_fPerTexelHeight;
	float woff, hoff;
	woff =  m_fPerTexelWidth	/ 2.0f;
	hoff =  m_fPerTexelHeight	/ 2.0f;

	// Offsets 3
	// used in blurring to smooth the water
	set = 3;
	scale = m_fBlurDist;
	woff += m_fScrollU;		// Add scrolling amount
	hoff += m_fScrollV;		// This is used in coupled water to scroll a texture
							//  as it moves in relation to another
	m_Offsets[set].t[0] = D3DXVECTOR4(  0.0f, -th	,	0.0f, 0.0f );
	m_Offsets[set].t[1] = D3DXVECTOR4(  tw	,  0.0f	,	0.0f, 0.0f );
	m_Offsets[set].t[2] = D3DXVECTOR4(  0.0f,  th	,	0.0f, 0.0f );
	m_Offsets[set].t[3] = D3DXVECTOR4( -tw	,  0.0f	,	0.0f, 0.0f );

	for( i=0; i<4; i++ )
	{
		m_Offsets[set].t[i].x *= scale; m_Offsets[set].t[i].y *= scale;		
		m_Offsets[set].t[i].x += woff; 	m_Offsets[set].t[i].y += hoff;		
	}
}

void PA_Water::CreateUVOffsets( int texture_width, int texture_height )
{
	// texture_width:
	//		The texture width in pixels for the textures
	//		on which the simulation is running.
	// texture_height:
	//		texture height in pixels.

	// This creates several sets of texture coordinate offsets
	// Each set is applied in a vertex shader to create several
	//  sets of texture coordinates for each vertex.
	//
	// In a situation where a source texture is mapped one-to-one
	//  to pixels of a render target, these offsets are used to
	//  point each texture stage sample to a neighbor of the 
	//  pixel being rendered.
	// In this way, with four texture stages, up to four texture
	//  samples (point, bilinear, etc.) can be delivered to a 
	//  pixel shader in a single pass.
	// Each pixel rendered will sample the same pattern of local
	//  texel neighbors.

	// Various sets of vectors are used throughout the simulation.
	// One set has no offset, and this causes the same sample to
	//  be delivered in all four stages.
	// One set points to the four nearest neighbors.
	// Other sets have some offsets set to zero and some pointing
	//  to neighbors, for accumulating results over more than one
	//  pass
	//
	// All you need to know for this case, is that each vector is
	//  the texture coordinate offset to some other texel from 
	//  the current pixel being rendered.
	// Because D3D samples from the texel corner and not the 
	//  texel center (as in OpenGL) a half-texel offset is added
	//  to all vectors so that samples are taken from the expected
	//  place.
    m_fPerTexelWidth  = 1.0f / ((float) texture_width );
    m_fPerTexelHeight = 1.0f / ((float) texture_height );

	// Half-texel width and height offsets for sampling from the
	//  exact center of a texel.  Without these, D3D will interpret
	//  a vector offset of (0,0) as sampling from a texel corner
	//  instead of a texel center.
	float woff, hoff;
	int i;
	woff =  m_fPerTexelWidth	/ 2.0f;
	hoff =  m_fPerTexelHeight	/ 2.0f;
	float tw, th;
	tw = m_fPerTexelWidth;
	th = m_fPerTexelHeight;
	int set;
	float scale;		// for scaling offsets outward from center
	scale = 1.0f;

	// Offsets set 0 - all zeros
	// This will be used to deliver the same texture sample to all
	//	texture stages
	set = 0;
	m_Offsets[set].t[0] = D3DXVECTOR4( 0.0f + woff, 0.0f + hoff, 0.0f, 0.0f );
	m_Offsets[set].t[1] = m_Offsets[set].t[0];
	m_Offsets[set].t[2] = m_Offsets[set].t[0];
	m_Offsets[set].t[3] = m_Offsets[set].t[0];
	// Offset set 1:  For use with neighbor force pixel shader 1
	//
	// Texel offsets are (0,0) (+1,0) (-1,0) and (0,+1)
	//  so each pixel rendered samples its corresponding texel and
	//  three neighbors.
	set = 1;
	float nfs = 1.5f;
	scale = nfs;
	// These place samples diagonaly outward from the center
	m_Offsets[set].t[0] = D3DXVECTOR4(  0.0f,  0.0f	,	0.0f, 0.0f );
	m_Offsets[set].t[1] = D3DXVECTOR4( -tw	, -th	,	0.0f, 0.0f );
	m_Offsets[set].t[2] = D3DXVECTOR4(  tw	, -th	,	0.0f, 0.0f );
	m_Offsets[set].t[3] = D3DXVECTOR4(  tw	,  th	,	0.0f, 0.0f );

	for( i=0; i<4; i++ )
	{
		m_Offsets[set].t[i].x *= scale; m_Offsets[set].t[i].y *= scale;
		m_Offsets[set].t[i].x +=  m_fWindX;
		m_Offsets[set].t[i].y +=  m_fWindY;
		m_Offsets[set].t[i].x += woff; 	m_Offsets[set].t[i].y += hoff;		
	}
	// Offset set 2:  for use with neighbor force pixel shader 2
	// Samples with (0,0) and (0,-v) offsets to grab a texel and
	//  it's 4th neighbor sample
	// This completes a pattern of sampling center texel and it's
	//   4 nearest neighbors to run the height-based water simulation
	// 3rd offset must be (0,0) to sample texel center from partial result
	//   texture.
	set = 2;
	// These place samples diagonaly outward from the center
	scale = nfs;		// distance in texels along each axis
	m_Offsets[set].t[0] = D3DXVECTOR4(  0.0f	,  0.0f		,	0.0f, 0.0f );
	m_Offsets[set].t[1] = D3DXVECTOR4( -tw		,  th		,	0.0f, 0.0f );
	m_Offsets[set].t[2] = D3DXVECTOR4(  0.0f	,  0.0f		,	0.0f, 0.0f );
	m_Offsets[set].t[3] = D3DXVECTOR4(  0.0f	,  0.0f		,	0.0f, 0.0f );

	for( i=0; i<4; i++ )
	{
		m_Offsets[set].t[i].x *= scale; m_Offsets[set].t[i].y *= scale;
		if( i==1 )
		{
			m_Offsets[set].t[i].x +=  m_fWindX;
			m_Offsets[set].t[i].y +=  m_fWindY;
		}
		m_Offsets[set].t[i].x += woff; 	m_Offsets[set].t[i].y += hoff;		
	}
	// Offsets 3 are set in separate function
	// offsets 3 are used in blurring
	// Offsets 4
	// Nearest neighbor offsets:
	set = 4;
	scale = 1.0f;
	m_Offsets[set].t[0] = D3DXVECTOR4(  -tw	,  0.0f		,	0.0f, 0.0f );
	m_Offsets[set].t[1] = D3DXVECTOR4(   tw	,  0.0f		,	0.0f, 0.0f );
	m_Offsets[set].t[2] = D3DXVECTOR4(  0.0f,  th		,	0.0f, 0.0f );
	m_Offsets[set].t[3] = D3DXVECTOR4(  0.0f, -th		,	0.0f, 0.0f );
	for( i=0; i<4; i++ )
	{
		m_Offsets[set].t[i].x *= scale; m_Offsets[set].t[i].y *= scale;		
		m_Offsets[set].t[i].x += woff; 	m_Offsets[set].t[i].y += hoff;		
	}
	CreateUVBlurOffsets();
}

// Function name   : PA_Water::SetRenderResultToScreen
// Description     : For diagnostic purposes - Sets whether or not to render textures to the screen
//  so you can see what they contain.
// Return type     : void 
// Argument        : bool true_to_render
void PA_Water::SetRenderResultToScreen( bool true_to_render )
{
	m_bDgbDrawOutputToScreen = true_to_render;
}

void PA_Water::SetDefaultParameters()
{
	// Values for the constants & scale factors which give good results.
	// Feel free to experiment with these.
	// Several keyboard controls are mapped to affect the values.  See the Keyboard(..) function.
	m_fDropletFreq		= 0.1750f;
	m_fPosAtten			= 0.980f;
	m_fEqRestore_factor	= 0.17f;
	m_fVelFactor		= 0.50f;
	m_fForceFactor		= 0.50f;
	m_fBlurDist			= 0.1100f;
	m_fNrmlSTScale		= 0.230000f;

	m_bCreateNormalMap = true;
	m_eDiagnosticDisplayMode = ALL_TOGETHER;
	CreateUVBlurOffsets();
}
