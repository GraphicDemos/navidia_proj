#include "ReadBackTexture.h"

ReadBackTexture::ReadBackTexture(IDirect3DDevice9* pd3dDevice, UINT width, UINT height, D3DFORMAT format)
{
	pOldSurface = NULL;
	pOldZ = NULL;
	pRT = NULL;
	pRTTex = NULL;
	pRO = NULL;
	pROTex = NULL;
	m_pd3dDevice = pd3dDevice;

	pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &pRTTex, NULL);
	HRESULT hr = pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, format, D3DPOOL_SYSTEMMEM, &pROTex, NULL);
	pRTTex->GetSurfaceLevel(0, &pRT);
	pROTex->GetSurfaceLevel(0, &pRO);
}

ReadBackTexture::~ReadBackTexture()
{
	SAFE_RELEASE(pRT);
	SAFE_RELEASE(pRTTex);
	SAFE_RELEASE(pRO);
	SAFE_RELEASE(pROTex);
}

HRESULT ReadBackTexture::SetRT(int index)
{
	if(pOldSurface)
		return E_FAIL;

	m_RTIndex = index;

	m_pd3dDevice->GetRenderTarget(m_RTIndex, &pOldSurface);
	m_pd3dDevice->GetDepthStencilSurface(&pOldZ);
	m_pd3dDevice->SetDepthStencilSurface(NULL);
	return  m_pd3dDevice->SetRenderTarget(m_RTIndex, pRT);
}

IDirect3DSurface9* ReadBackTexture::GetData()
{
	m_pd3dDevice->GetRenderTargetData(pRT, pRO);
	return pRO;
}

HRESULT ReadBackTexture::ResetRT()
{
	m_pd3dDevice->SetRenderTarget(m_RTIndex, pOldSurface);
	if(m_RTIndex == 0)
		m_pd3dDevice->SetDepthStencilSurface(pOldZ);
	SAFE_RELEASE(pOldSurface);
	SAFE_RELEASE(pOldZ);
	return S_OK;
}