#ifndef ANTIALIASINGWITHTRANSPARENCY_H
#define ANTIALIASINGWITHTRANSPARENCY_H
#include "nvafx.h"
#include "shared\GetFilePath.h"
#include "TerrainObject.h"
#include "GrassObject.h"


//Our renderer
class AntiAliasingWithTransparency
{
private:
	//Global objects
	IDirect3DDevice9* m_pd3dDevice;

	//Resources
	GrassObject* m_pWeed;
	GrassObject* m_pWeed2;
	GrassObject* m_pWeed3;
	GrassObject* m_CloseUp;
	TerrainObject* m_pTerrain;

public:
	AntiAliasingWithTransparency(IDirect3DDevice9* pd3dDevice);
	~AntiAliasingWithTransparency();

	HRESULT RenderAlphaTested(D3DFORMAT AAFmt, bool alphaTestEnable, bool alphaBlendEnable, float alphaScale, double fTime, bool bCloseUp);
	HRESULT RenderOpaque(D3DCULL cullmode);
	HRESULT UpdateMatrices(D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* projection);
};

#endif ANTIALIASINGWITHTRANSPARENCY_H
