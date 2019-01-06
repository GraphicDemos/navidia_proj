#ifndef BLOODMAP_H
#define BLOODMAP_H

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#include "dxstdafx.h"

class BloodMap
{
private:
	bool m_bTextureSelect;
	IDirect3DTexture9* m_ptextureA;
	IDirect3DTexture9* m_ptextureB;
	IDirect3DTexture9* m_pSeedTexture;

public:
	HRESULT Init(IDirect3DDevice9* pd3dDevice, const wchar_t* seedTexture);
	IDirect3DTexture9* pBloodTexture;
	IDirect3DSurface9* pBloodSurface;
	HRESULT Swap();
	HRESULT Release();
};

#endif