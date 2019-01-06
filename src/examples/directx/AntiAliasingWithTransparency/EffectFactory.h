#ifndef EFFECTFACTORY_H
#define EFFECTFACTORY_H
#include "nvafx.h"
#include <stack>

//Class for easy management of effects

struct TextureObject
{
	IDirect3DTexture9* pTexture;
	char* name;
};

class my_stack : public std::stack< TextureObject > {
public:
	using std::stack<TextureObject>::c; // expose the container
};

class EffectFactory
{
private:
	my_stack m_TextureStack;
protected:
	ID3DXEffect* m_pEffect;
	IDirect3DDevice9* m_pd3dDevice;
	HRESULT CreateEffect(LPCWSTR EffectPath);
	HRESULT GetTexture(char* name, IDirect3DTexture9** ppTex);

public:
	EffectFactory(IDirect3DDevice9* pd3dDevice);
	~EffectFactory();
	HRESULT AddTexture(LPCWSTR TexturePath, char* TextureName);

	virtual HRESULT UpdateMatrices(D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* projection);
	virtual HRESULT Render(double fTime = 0.0f);

};

#endif