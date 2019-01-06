
#include "nv_d3d9FullScreenQuad.h"
#include "nv_RainbowEffect.h"
#include "shared\GetFilePath.h"




nv_RainbowEffect::nv_RainbowEffect()
{
	m_pFullScreenQuad					= new nv_D3D9FullScreenQuad();	
	m_pRainbowEffect					= NULL; 
	m_hTechniqueRenderRainbowQuad		= NULL; 
	m_pRainbowLookupTextureScattering	= NULL;
	m_pCoronaLookupTexture				= NULL;
	
	m_pRenderTargetDepthBuffer			= NULL;
	m_pRenderTarget						= NULL;
	m_RenderTargetTexture_Moisture		= NULL;
	m_pRenderTargetTextureSurface		= NULL;

	
	m_ptheRealBackBuffer				= NULL; 
	m_ptheRealDepthBuffer				= NULL;


}
nv_RainbowEffect::~nv_RainbowEffect()
{
	SAFE_DELETE(m_pFullScreenQuad);
}

//standard d3d object interface functions
HRESULT nv_RainbowEffect::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pFullScreenQuad->RestoreDeviceObjects(pd3dDevice);

	if( m_pRainbowEffect )
	{
        m_pRainbowEffect->OnResetDevice();
		m_hTechniqueRenderRainbowQuad         = m_pRainbowEffect->GetTechniqueByName( "RainbowAndCorona" );
		m_pRainbowEffect->SetTechnique(m_hTechniqueRenderRainbowQuad);
	}

	//Load Textures/////////////////////////////////////////////
	HRESULT hr;

	tstring path;
	path = GetFilePath::GetFilePath(TEXT("MEDIA\\Textures\\2D\\Rainbow_Scatter_FakeWidet.tga"), false );
	if(path.empty())
		return -1;
	hr = D3DXCreateTextureFromFile(pd3dDevice, path.c_str(), &m_pRainbowLookupTextureScattering);
    if (FAILED(hr))
	{
		m_pRainbowLookupTextureScattering = NULL;
		return hr;
	}

	path = GetFilePath::GetFilePath(TEXT("MEDIA\\Textures\\2D\\rainbow_plot_i_vs_a_diffract_0_90_1024.tga"), false );
	if(path.empty())
		return -1;
	hr = D3DXCreateTextureFromFile(pd3dDevice, path.c_str(), &m_pCoronaLookupTexture);
    if (FAILED(hr))
	{
        m_pCoronaLookupTexture = NULL;
		return hr;
	}


	//Create Render Target and Texture for moisture////////////////////////////////
	const int DYNAMIC_TEXTURE_DIMENSIONS = 512;
	const D3DMULTISAMPLE_TYPE AA_TYPE =  D3DMULTISAMPLE_NONE ;
	const DWORD AA_QUALITY = 0;

	m_pRenderTargetDepthBuffer = NULL;
	m_pRenderTarget = NULL;

	if( FAILED( hr = pd3dDevice->CreateDepthStencilSurface(          
		DYNAMIC_TEXTURE_DIMENSIONS,
		DYNAMIC_TEXTURE_DIMENSIONS,
		D3DFMT_D16,
		AA_TYPE,
		AA_QUALITY,
		false,
		&m_pRenderTargetDepthBuffer,
		NULL
	)))
	{
		return hr;
	}
	if( FAILED( hr = pd3dDevice->CreateRenderTarget(          
		DYNAMIC_TEXTURE_DIMENSIONS,
		DYNAMIC_TEXTURE_DIMENSIONS,
		D3DFMT_A8R8G8B8,
		AA_TYPE, //depth stencil must have this format set as well!
		AA_QUALITY, //should really query for possible values
		false,
		&m_pRenderTarget,
		NULL
	)))
	{
		return hr;
	}

	if(FAILED(hr = D3DXCreateTexture( pd3dDevice, DYNAMIC_TEXTURE_DIMENSIONS, DYNAMIC_TEXTURE_DIMENSIONS, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_RenderTargetTexture_Moisture	)))
	{
		return hr;
	}

	m_RenderTargetTexture_Moisture->GetSurfaceLevel( 0 , &m_pRenderTargetTextureSurface );


	//Store a copy of the real back buffer info
	pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&m_ptheRealBackBuffer);
	pd3dDevice->GetDepthStencilSurface(&m_ptheRealDepthBuffer);

	m_pFullScreenQuad->SetUpForRenderTargetSurface( m_ptheRealBackBuffer );


	if(m_pRainbowEffect)
	{
		m_pRainbowEffect->SetTexture( "tRainbowLookup", m_pRainbowLookupTextureScattering);
		m_pRainbowEffect->SetTexture( "tCoronaLookup", m_pCoronaLookupTexture);
		m_pRainbowEffect->SetTexture( "tMoisture", m_RenderTargetTexture_Moisture);
	}
	return S_OK;
}
HRESULT nv_RainbowEffect::InvalidateDeviceObjects()
{
	m_pFullScreenQuad->InvalidateDeviceObjects();
	
	if( m_pRainbowEffect )
        m_pRainbowEffect->OnLostDevice();

	SAFE_RELEASE(m_pRainbowLookupTextureScattering);
	SAFE_RELEASE(m_pCoronaLookupTexture);
	
	SAFE_RELEASE(m_pRenderTargetTextureSurface);
	SAFE_RELEASE(m_RenderTargetTexture_Moisture);
	SAFE_RELEASE(m_pRenderTargetDepthBuffer);
	SAFE_RELEASE(m_pRenderTarget);

	SAFE_RELEASE(m_ptheRealDepthBuffer);
	SAFE_RELEASE(m_ptheRealBackBuffer);

	return S_OK;
}
HRESULT nv_RainbowEffect::Create( LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr;

	//load our fx file
	tstring path;

	path = GetFilePath::GetFilePath( TEXT("MEDIA\\programs\\RainbowFogbow.cso"), false );
	if(path.empty())
		return -1;
	hr = D3DXCreateEffectFromFile(pd3dDevice, path.c_str(), NULL, NULL, 0, NULL, &m_pRainbowEffect, NULL );
    if (FAILED(hr))
	{
		m_pRainbowEffect = NULL;
		return hr;
	}



	return S_OK;
}
HRESULT nv_RainbowEffect::Destroy()
{
	SAFE_RELEASE(m_pRainbowEffect);

	return S_OK;
}

HRESULT nv_RainbowEffect::BeginMoistureTextureRendering(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//set the render target for moisture rendering
	pd3dDevice->SetRenderTarget(0, m_pRenderTarget);
	pd3dDevice->SetDepthStencilSurface(m_pRenderTargetDepthBuffer);

	return S_OK;
}

HRESULT nv_RainbowEffect::EndMoistureTextureRendering(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//reset the original render target and depth buffers
	pd3dDevice->SetRenderTarget( 0, m_ptheRealBackBuffer);
	pd3dDevice->SetDepthStencilSurface( m_ptheRealDepthBuffer);
	//copy our rendertarget to a texture
	pd3dDevice->StretchRect(m_pRenderTarget,0,m_pRenderTargetTextureSurface,0,D3DTEXF_NONE);

	return S_OK;
}

HRESULT nv_RainbowEffect::RenderRainbow( LPDIRECT3DDEVICE9 pd3dDevice)
{

	UINT passes;
	m_pRainbowEffect->Begin( &passes , 0);
	for(UINT i = 0 ; i < passes ; ++i)
	{
		m_pRainbowEffect->BeginPass(i);
		m_pFullScreenQuad->Render(pd3dDevice);
		m_pRainbowEffect->EndPass();
	}
	m_pRainbowEffect->End();
	return S_OK;
}

void nv_RainbowEffect::SetLightDirection( D3DXVECTOR4* lightDir)
{
	m_pRainbowEffect->SetVector( "LightVec", lightDir );
}

void nv_RainbowEffect::SetDropletRadius( FLOAT radius)
{
	m_pRainbowEffect->SetFloat("dropletRadius", radius);
}

void nv_RainbowEffect::SetRainbowIntensity( FLOAT intensity)
{
	m_pRainbowEffect->SetFloat("rainbowIntensity", intensity);
}

void nv_RainbowEffect::SetViewMatrix(D3DXMATRIX* view)
{
	m_pRainbowEffect->SetMatrix( "View", view );
}

void nv_RainbowEffect::SetProjInvMatrix(D3DXMATRIX* projInv)
{
	m_pRainbowEffect->SetMatrix("ProjInv", projInv);
}
