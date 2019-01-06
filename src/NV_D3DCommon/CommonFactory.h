

#ifndef H_NVD3DCOMMONFACTORY_H
#define H_NVD3DCOMMONFACTORY_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include <vector>
using namespace std;

class CommonFactory
{
public:
	TextureFactory **		CreateTextureFactory( IDirect3DDevice9 * pDev );
	RenderTargetFactory **	CreateRenderTargetFactory( IDirect3DDevice9 * pDev );
	ShaderManager **		CreateShaderManager( IDirect3DDevice9 * pDev );
	ITextureDisplay **		CreateTextureDisplay( IDirect3DDevice9 * pDev );
	D3DDeviceStateFactory ** CreateStateFactory( IDirect3DDevice9 * pDev );

	HRESULT Free();
	CommonFactory();
	~CommonFactory();

protected:
	vector< TextureFactory ** >			m_vTF;
	vector< RenderTargetFactory ** >	m_vRTF;
	vector< ShaderManager ** >			m_vSM;
	vector< TextureDisplay2 ** >		m_vTD;
	vector< D3DDeviceStateFactory ** >	m_vSF;
};

#endif
