#include "TerrainObject.h"
#include "shared\GetFilePath.h"
#include "shared\GetFilePath.h"
#include <NVMeshMender.h>

TerrainObject::TerrainObject(IDirect3DDevice9* pd3dDevice) : EffectFactory(pd3dDevice)
{
	HRESULT hr = S_OK;
	CreateEffect(GetFilePath::GetFilePath(TEXT("MEDIA/programs/AntiAliasingWithTransparency/TerrainObject.fx")).c_str());
	
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/ground_diffuse.dds")).c_str(), "DirtMap");
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/ground_grass.dds")).c_str(), "GrassMap");
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/ground_normal.dds")).c_str(), "NormalHeightMap");
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/ground_coverage.dds")).c_str(), "CoverageMap");
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/fence.dds")).c_str(), "Fence");
	AddTexture(GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/AntiAliasingWithTransparency/fence_bump.dds")).c_str(), "FenceBump");

	ID3DXMesh* pTempMesh;
	D3DXLoadMeshFromX(GetFilePath::GetFilePath(TEXT("MEDIA/models/AntiAliasingWithTransparency/groundplane.x")).c_str(), D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pTempMesh);

	D3DVERTEXELEMENT9 decl[6] = 
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD, 0},
		{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL, 0},
		{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TANGENT, 0},
		{0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_BINORMAL, 0},
			D3DDECL_END()
	};
	pTempMesh->CloneMesh(D3DXMESH_MANAGED, decl, m_pd3dDevice, &m_pMesh);
	SAFE_RELEASE(pTempMesh);
	D3DXComputeTangent(m_pMesh, NULL, NULL, NULL, true, NULL);
}




HRESULT TerrainObject::Render()
{
	HRESULT hr = S_OK;
	UINT iPass, iPasses;
	m_pEffect->Begin(&iPasses, 0);
	for(iPass = 0; iPass < iPasses; iPass++)
	{
		m_pEffect->BeginPass(iPass);
		m_pMesh->DrawSubset(0);
		m_pEffect->EndPass();
	}
	m_pEffect->End();
	return hr;
}


HRESULT TerrainObject::RenderGeometry()
{
	m_pEffect->SetTechnique("GraphGeometry");
	return Render();
}

HRESULT TerrainObject::RenderFront()
{
	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 1);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_cFront);
	m_pEffect->SetTechnique("TerrainObjectFront");
	HRESULT hr = Render();
	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 0);
	return hr;
}

HRESULT TerrainObject::RenderBack()
{
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_cBack);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 1);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	//m_pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, 1);
	m_pEffect->SetTechnique("TerrainObjectBack");
	HRESULT hr = Render();
	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 0);
	return hr;
}

HRESULT TerrainObject::ReturnCoverageSize(int* x, int* y)
{
	IDirect3DTexture9* pTex;
	GetTexture("CoverageMap", &pTex);
	D3DSURFACE_DESC desc;
	pTex->GetLevelDesc(0, &desc);
	*x = desc.Width;
	*y = desc.Height;
	return S_OK;
}

TerrainObject::~TerrainObject()
{
	SAFE_RELEASE(m_pMesh);
}

HRESULT TerrainObject::SetFront(D3DCULL front)
{
	m_cFront = front;
	if(front == D3DCULL_CW)
		m_cBack = D3DCULL_CCW;
	else if(front == D3DCULL_CCW)
		m_cBack = D3DCULL_CW;
	else
		m_cBack = m_cFront;

	return S_OK;
}