#include "BloodMap.h"

HRESULT BloodMap::Init(IDirect3DDevice9* pd3dDevice, const wchar_t* seedTexture)
{
	Release();
	pd3dDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_ptextureA, NULL);
	pd3dDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_ptextureB, NULL);

	D3DXCreateTextureFromFile(pd3dDevice, seedTexture, &m_pSeedTexture);
	//Seed first texture
	pBloodTexture = m_pSeedTexture;
	//Set target to texture A
	m_ptextureA->GetSurfaceLevel(0, &pBloodSurface);
	m_bTextureSelect = false;

	return S_OK;
}

HRESULT BloodMap::Swap()
{
	//Release old rendertarget
	SAFE_RELEASE(pBloodSurface);

	//Swap to new rendertarget
	if(m_bTextureSelect)
	{
		m_ptextureA->GetSurfaceLevel(0, &pBloodSurface);
		pBloodTexture = m_ptextureB;
	}
	else
	{
		m_ptextureB->GetSurfaceLevel(0, &pBloodSurface);
		pBloodTexture = m_ptextureA;
	}
	m_bTextureSelect = !m_bTextureSelect;
	return S_OK;
}

HRESULT BloodMap::Release()
{
	SAFE_RELEASE(m_ptextureA);
	SAFE_RELEASE(m_ptextureB);
	SAFE_RELEASE(pBloodSurface);
	SAFE_RELEASE(m_pSeedTexture);
	return S_OK;
}