#include <DXUT.h>
#include "GrassObject.h"
#include "shared\GetFilePath.h"
#include "EffectFactory.h"
#include "shared\GetFilePath.h"


GrassObject::GrassObject(IDirect3DDevice9* pd3dDevice, std::stack<D3DXMATRIX>* worldPosStack, LPCWSTR texture, LPCWSTR normalmap) : EffectFactory(pd3dDevice)
{
	CreateEffect(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\AntiAliasingWithTransparency\\GrassObject.fx")).c_str());
	AddTexture(texture, "GrassTex");
	AddTexture(normalmap, "NormalMap");
	IDirect3DTexture9* pTex;
	GetTexture("GrassTex", &pTex);
	D3DSURFACE_DESC desc;
	pTex->GetLevelDesc(0, &desc);
	float ratio = (float)desc.Height / (float)desc.Width;

#pragma pack (push, 1)
	D3DVERTEXELEMENT9 QuadDecl[7] =
	{
	{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{1, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	{1, 16,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
	{1, 32,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
	{1, 48,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},
	D3DDECL_END()
	};

	struct QUADVERTEX
	{
		float x, y, z;
		float u, v;
	};
#pragma pack (pop)

	QUADVERTEX vertices[] = {
	{-1.0f, ratio*1.0f, 0.0f, 0.0f, 0.0f},
	{1.0f,  ratio*1.0f, 0.0f, 1.0f, 0.0f},	
	{1.0f,  ratio*-1.0f, 0.0f, 1.0f, 1.0f}, 
	{-1.0f, ratio*-1.0f, 0.0f, 0.0f, 1.0f},

	{0.0f, ratio*1.0f, -1.0f, 0.0f, 0.0f},
	{0.0f, ratio*1.0f, 1.0f, 1.0f, 0.0f},	
	{0.0f, ratio*-1.0f, 1.0f, 1.0f, 1.0f}, 
	{0.0f, ratio*-1.0f, -1.0f, 0.0f, 1.0f},

	{-1.0f, ratio*1.0f, -1.0f, 0.0f, 0.0f},
	{1.0f,  ratio*1.0f, 1.0f, 1.0f, 0.0f},	
	{1.0f,  ratio*-1.0f, 1.0f, 1.0f, 1.0f}, 
	{-1.0f, ratio*-1.0f, -1.0f, 0.0f, 1.0f},

	{-1.0f, ratio*1.0f, -1.0f, 0.0f, 0.0f},
	{1.0f,  ratio*1.0f, 1.0f, 1.0f, 0.0f},	
	{1.0f,  ratio*-1.0f, 1.0f, 1.0f, 1.0f}, 
	{-1.0f, ratio*-1.0f, -1.0f, 0.0f, 1.0f},
	};	

	WORD indices[24] = {
	 0,  1,  2,  3,  0,  2, 
	 4,  5,  6,  7,  4,  6, 
	 8,  9, 10, 11,  8, 10,
	12, 13, 14, 15, 12, 13
	};

	int stacksize = 0;
	D3DXMATRIX* world = NULL;
	if(!worldPosStack)
	{
		stacksize = 1;
		world = new D3DXMATRIX[1];
		D3DXMatrixIdentity(world);
	}
	else
	{
		stacksize = (int)worldPosStack->size();
		world = new D3DXMATRIX[stacksize];
		for(int i = 0; i < stacksize; ++i)
		{
			world[i] = worldPosStack->top();
			//Translate up 1 ratio in the direction of the normal
			world[i]._41 += ratio * world[i]._21;
			world[i]._42 += ratio * world[i]._22;
			world[i]._43 += ratio * world[i]._23;
			worldPosStack->pop();
		}
	}

	m_pIG = new InstancedGeometry(m_pd3dDevice, stacksize, QuadDecl, 7, vertices, 16*sizeof(QUADVERTEX), indices, 24*sizeof(WORD), world, stacksize*sizeof(D3DXMATRIX));
	delete world;
}


HRESULT GrassObject::Render(LPCSTR tech, float alphaScale, double fTime)
{
	HRESULT hr = S_OK;
	V_RETURN(m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 1));
	m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0);
	m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);

	//Setup effect
	V_RETURN(m_pEffect->SetTechnique(tech));
	V_RETURN(m_pEffect->SetFloat("xshift", (float)fTime));
	V_RETURN(m_pEffect->SetFloat("alphascale", alphaScale));
	m_pIG->Set();

	UINT iPass, iPasses;
	m_pEffect->Begin(&iPasses, 0);
	for(iPass = 0; iPass < iPasses; iPass++)
	{
		m_pEffect->BeginPass(iPass);
		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 16, 0, 8);
		m_pEffect->EndPass();
	}
	m_pEffect->End();

	m_pIG->ReSet();
	m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 0);
	return hr;
}

GrassObject::~GrassObject()
{
	SAFE_DELETE(m_pIG);
}
