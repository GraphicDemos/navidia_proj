#include <DXUT.h>
#include "AntiAliasingWithTransparency.h"
#include "ReadBackTexture.h"
#include <d3dx9mesh.h>
#include <time.h>
#include <stack>

AntiAliasingWithTransparency::AntiAliasingWithTransparency(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	m_pWeed = NULL;
	m_pd3dDevice = pd3dDevice;

	//Setup the terrain object (Shaders and textures)
	m_pTerrain = new TerrainObject(pd3dDevice);


	//We will render out the geometry of our mesh so that we can place each instance of the grass object
	//This consists of rendering out per grass object (defined by the alpha channel of our coverage map) the normal and tangent

	int x, y;
	m_pTerrain->ReturnCoverageSize(&x, &y);

	//Need three RTs for position, normal, and tangent
	ReadBackTexture posmap(pd3dDevice, x, y, D3DFMT_A32B32G32R32F);
	ReadBackTexture normmap(pd3dDevice, x, y, D3DFMT_A32B32G32R32F);
	ReadBackTexture tangentmap(pd3dDevice, x, y, D3DFMT_A32B32G32R32F);

	pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE1, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE2, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	V(posmap.SetRT(0));
	V(normmap.SetRT(1));
	V(tangentmap.SetRT(2));
 
	pd3dDevice->BeginScene();
	m_pTerrain->RenderGeometry();
	pd3dDevice->EndScene();

	V(posmap.ResetRT());
	V(normmap.ResetRT());
	V(tangentmap.ResetRT());
	pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE1, 0x00000000);
	pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE2, 0x00000000);

	D3DLOCKED_RECT posrect, normrect, tangentrect;
	IDirect3DSurface9* posdata = posmap.GetData();
	IDirect3DSurface9* normdata = normmap.GetData();
	IDirect3DSurface9* tangentdata = tangentmap.GetData();

	V(posdata->LockRect(&posrect, NULL, D3DLOCK_READONLY));
	V(normdata->LockRect(&normrect, NULL, D3DLOCK_READONLY));
	V(tangentdata->LockRect(&tangentrect, NULL, D3DLOCK_READONLY));

	std::stack<D3DXMATRIX> WorldStack0;
	std::stack<D3DXMATRIX> WorldStack1;
	std::stack<D3DXMATRIX> WorldStack2;

	D3DXVECTOR4 pospixel, normal, tangent;
	for(int i = 0; i < x * y; ++i)
	{
		pospixel = ((D3DXVECTOR4*)posrect.pBits)[i];
		float coverage = pospixel.w;
		if(coverage != 0.0f)
		{
			normal = ((D3DXVECTOR4*)normrect.pBits)[i];
			tangent = ((D3DXVECTOR4*)tangentrect.pBits)[i];

			D3DXVECTOR3 binormal;
			//Generate a binormal
			D3DXVec3Cross(&binormal, &D3DXVECTOR3(normal.x, normal.y, normal.z), &D3DXVECTOR3(tangent.x, tangent.y, tangent.z));

			D3DXMATRIX world;
			world._11 = binormal.x;
			world._12 = binormal.y;
			world._13 = binormal.z;
			world._14 = 0.0f;

			world._21 = normal.x;
			world._22 = normal.y;
			world._23 = normal.z;
			world._24 = 0.0f;

			world._31 = tangent.x;
			world._32 = tangent.y;
			world._33 = tangent.z;
			world._34 = 0.0f;

			D3DXMATRIX randrot;
			D3DXMatrixRotationY(&randrot, (float)rand());
			D3DXMatrixMultiply(&world, &world, &randrot);

			world._41 = pospixel.x;
			world._42 = pospixel.y;
			world._43 = pospixel.z;

			//Throw our random wind variable here
			world._44  = ((rand() % 2) * 2 - 1) * ((float)rand()) / RAND_MAX;

	 		if(pospixel.w > 0.25f && pospixel.w < 0.50f)
				WorldStack0.push(world);
			else if(pospixel.w > 0.50f && pospixel.w < 0.75f)
				WorldStack1.push(world);
			else
				WorldStack2.push(world);
		}
	}
	V(posdata->UnlockRect());
	V(normdata->UnlockRect());
	V(tangentdata->UnlockRect());

	//Setup the grass object (Shaders and textures)
	m_pWeed = new GrassObject(pd3dDevice, &WorldStack0, GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass01.dds")).c_str(), GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass01_normal.dds")).c_str());
	m_pWeed2 = new GrassObject(pd3dDevice, &WorldStack1, GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass02.dds")).c_str(), GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass02_normal.dds")).c_str());
	m_pWeed3 = new GrassObject(pd3dDevice, &WorldStack2, GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass03.dds")).c_str(), GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass03_normal.dds")).c_str());
	m_CloseUp = new GrassObject(pd3dDevice, NULL, GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass02.dds")).c_str(), GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/grass02_normal.dds")).c_str());
}

HRESULT AntiAliasingWithTransparency::UpdateMatrices(D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	HRESULT hr = S_OK;
	m_pWeed->UpdateMatrices(world, view, projection);
	m_pWeed2->UpdateMatrices(world, view, projection);
	m_pWeed3->UpdateMatrices(world, view, projection);
	m_pTerrain->UpdateMatrices(world, view, projection);
	return hr;
}

HRESULT AntiAliasingWithTransparency::RenderAlphaTested(D3DFORMAT AAFmt, bool alphaTestEnable, bool alphaBlendEnable, float alphaScale,double fTime, bool bCloseUp)
{
	HRESULT hr = S_OK;
	if(alphaTestEnable)
	{
		m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTestEnable);
		m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x0000000F);
	}
	else if(alphaBlendEnable)
	{
		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlendEnable);
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	}

	//Per primitive MSAA and SSAA are turned on by setting D3DRS_ADAPTIVETESS_Y to either:
	//(D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C'))
	//(D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'))
	//Setting D3DRS_ADAPTIVETESS_Y back to D3DFMT_UNKNOWN turns both modes off
	m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, AAFmt);

	LPCSTR technique;

	if(!bCloseUp)
	{
		technique = "InstancedGrassObject";

		m_pTerrain->RenderBack();
		m_pWeed->Render(technique, alphaScale, fTime);
		m_pWeed2->Render(technique, alphaScale, fTime);
		m_pWeed3->Render(technique, alphaScale, fTime);
	}
	else
	{
		technique = "CloseUp";
		m_CloseUp->Render(technique, alphaScale);
	}
	
	m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	return hr;
}

HRESULT AntiAliasingWithTransparency::RenderOpaque(D3DCULL cullmode)
{
	HRESULT hr = S_OK;
	m_pTerrain->SetFront(cullmode);
	m_pTerrain->RenderFront(); 
	return hr;
}


AntiAliasingWithTransparency::~AntiAliasingWithTransparency()
{
	SAFE_DELETE(m_pTerrain);
	SAFE_DELETE(m_pWeed);
	SAFE_DELETE(m_pWeed2);
	SAFE_DELETE(m_pWeed3);
	SAFE_DELETE(m_CloseUp);
}