#ifndef READBACKTEXTURE_H
#define READBACKTEXTURE_H
#include "nvafx.h"

class ReadBackTexture
{
private:
	IDirect3DDevice9* m_pd3dDevice;
	IDirect3DSurface9* pOldSurface;
	IDirect3DSurface9* pOldZ;
	IDirect3DSurface9* pRT;
	IDirect3DTexture9* pRTTex;
	IDirect3DSurface9* pRO;
	IDirect3DTexture9* pROTex;
	int m_RTIndex;

public:
	ReadBackTexture(IDirect3DDevice9* pd3dDevice, UINT width, UINT height, D3DFORMAT format);
	~ReadBackTexture();
	HRESULT SetRT(int index);
	HRESULT ResetRT();
	IDirect3DSurface9* GetData();
	
};

#endif
