#include "EffectFactory.h"
#include <DXUT.h>
EffectFactory::EffectFactory(IDirect3DDevice9* pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;
	m_pEffect = NULL;
}

HRESULT EffectFactory::CreateEffect(LPCWSTR EffectPath)
{
	HRESULT hr = S_OK;
	V_RETURN(D3DXCreateEffectFromFile(m_pd3dDevice, EffectPath, NULL, NULL, NULL, NULL, &m_pEffect, NULL));
	return hr;
}

EffectFactory::~EffectFactory()
{
	SAFE_RELEASE(m_pEffect);
	TextureObject texture;
	while(!m_TextureStack.empty())
	{
		texture = m_TextureStack.top();
		SAFE_RELEASE(texture.pTexture);
		SAFE_DELETE(texture.name);
		m_TextureStack.pop();
	}
}

HRESULT EffectFactory::GetTexture(char* name, IDirect3DTexture9** ppTex)
{
	HRESULT hr = E_FAIL;
	for(unsigned int i = 0; i < m_TextureStack.size(); ++i)
	{
		if(strcmp(name, m_TextureStack.c[i].name) == 0)
		{
			*ppTex = m_TextureStack.c[i].pTexture;
			hr = S_OK;
			break;
		}
	}
	return hr;
}
HRESULT EffectFactory::AddTexture(LPCWSTR TexturePath, char* TextureName)
{
	HRESULT hr;
	TextureObject Texture;
	Texture.pTexture = NULL;
	Texture.name = new char[strlen(TextureName) + 1];
	strcpy(Texture.name, TextureName);

	V_RETURN(D3DXCreateTextureFromFile(m_pd3dDevice, TexturePath, &Texture.pTexture));
	V_RETURN(m_pEffect->SetTexture(Texture.name, Texture.pTexture));

	m_TextureStack.push(Texture);
	return hr;
}


HRESULT EffectFactory::UpdateMatrices(D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	HRESULT hr = S_OK;
	V_RETURN(m_pEffect->SetMatrix("World", world));
	V_RETURN(m_pEffect->SetMatrix("View", view));
	V_RETURN(m_pEffect->SetMatrix("Projection", projection));
	return hr;
}

HRESULT EffectFactory::Render(double fTime)
{
	return S_OK;
}
