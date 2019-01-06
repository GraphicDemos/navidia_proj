#pragma once
//--------------------------------------------------------------------------------------
// File: nv_skybox.cpp
//
// A little wrapper that makes and renders a skybox mesh.
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <DXUT.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

class nv_SkyBox
{
	class WorldBoxVertex
	{
	public:
		WorldBoxVertex(const D3DXVECTOR3& vecPosition, const D3DXVECTOR3& vecTexture)
			: mPosition(vecPosition)
			, mTexture(vecTexture)
		{};

		D3DXVECTOR3 mPosition;
		D3DXVECTOR3 mTexture;
	};
	#define WORLDBOX_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0))

public:

	nv_SkyBox()
	{
		m_bInit = false;
		m_CubeMap = NULL;
		m_pWorldBoxVertices = NULL;
		m_pWorldBoxIndices = NULL;
	}

	HRESULT Init(LPDIRECT3DDEVICE9 d3dDevice)
	{
		HRESULT hr;

		// Create world-cube buffers
		V(d3dDevice->CreateVertexBuffer(24 * sizeof(WorldBoxVertex), D3DUSAGE_WRITEONLY, WORLDBOX_FVF, D3DPOOL_MANAGED, &m_pWorldBoxVertices, NULL));
		V(d3dDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pWorldBoxIndices, NULL));

		WorldBoxVertex  *pVertices = NULL;
		WORD            *pIndices  = NULL;

		V(m_pWorldBoxVertices->Lock(0, 24*sizeof(WorldBoxVertex),(void**)&pVertices, 0));
		V(m_pWorldBoxIndices->Lock(0, 36*sizeof(WORD),(void**)&pIndices, 0));

		CreateCube(pVertices, pIndices);

		m_pWorldBoxVertices->Unlock();
		m_pWorldBoxIndices->Unlock();
		return S_OK;
	}

	void Destroy()
	{
		SAFE_RELEASE(m_pWorldBoxVertices);
		SAFE_RELEASE(m_pWorldBoxIndices);
		SAFE_RELEASE(m_CubeMap);
	}

	void SetCubeMap(LPDIRECT3DCUBETEXTURE9 cubemap)
	{
		m_CubeMap = cubemap;
	}

	HRESULT Render(LPDIRECT3DDEVICE9 d3dDevice, D3DXMATRIX &mWorld)
	{
		HRESULT     hr = S_OK;

		D3DXMATRIX oldWorld,oldView,oldProj;

		hr = d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		hr = d3dDevice->SetPixelShader(0);    
		hr = d3dDevice->SetVertexShader(0);

		// note: setting FVF clobbers our current vertex declaration, so be sure 
		// that the caller re-establishes the vertex declaration on return!
		hr = d3dDevice->SetFVF( WORLDBOX_FVF );

		hr = d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
		hr = d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		hr = d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR,0);
		hr = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
		hr = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);

		hr = d3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		hr = d3dDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);

		hr = d3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
		hr = d3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE | D3DTTFF_COUNT3 );    
	    
		hr = d3dDevice->SetStreamSource(0, m_pWorldBoxVertices, 0, sizeof(WorldBoxVertex));
		hr = d3dDevice->SetIndices(m_pWorldBoxIndices);

		hr = d3dDevice->SetTexture(0, m_CubeMap);
		hr = d3dDevice->SetTexture(1, NULL);

		d3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
		d3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		d3dDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);
		

		D3DXMATRIX id, matProj;
		D3DXMatrixIdentity(&id);
		D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(90.0f), 1.0f, 0.1f, 8000.0f);
		d3dDevice->SetTransform(D3DTS_WORLD, &mWorld);
		d3dDevice->SetTransform(D3DTS_VIEW, &id);
		d3dDevice->SetTransform(D3DTS_PROJECTION, &matProj); 

		hr = d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);

		hr = d3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);    

		return hr;
	}

	HRESULT CreateCube(WorldBoxVertex* pVertices, WORD* pIndices)
	{
		// Set up the vertices for the Floor. Note: to prevent tiling problems,
		// the u/v coords are knocked slightly inwards.
		float const kEpsMult = 1.9999f;
		for (int ix = 0; ix < 2; ++ix)
			for (int iy = 0; iy < 2; ++iy)
				for (int iz = 0; iz < 2; ++iz)
				{
					float const x  = 4000.0f*(static_cast<float>(ix) - 0.5f);
					float const y  = 4000.0f*(static_cast<float>(iy) - 0.5f);
					float const z  = 4000.0f*(static_cast<float>(iz) - 0.5f);

					for (int iv = 0; iv < 3; ++iv)
					{
						const float tx = ((iv == 0) ? 2.0f : kEpsMult) * (static_cast<float>(ix) - 0.5f);
						const float ty = ((iv == 1) ? 2.0f : kEpsMult) * (static_cast<float>(iy) - 0.5f);
						const float tz = ((iv == 2) ? 2.0f : kEpsMult) * (static_cast<float>(iz) - 0.5f);

						*pVertices++ = WorldBoxVertex(D3DXVECTOR3(x, y, z), D3DXVECTOR3(tx, ty, tz));
					}
				}
		// Set up the indices for the cube
		// no offset in x-coords
		*pIndices++ =  0;     *pIndices++ =  6;     *pIndices++ =  3;
		*pIndices++ =  3;     *pIndices++ =  6;     *pIndices++ =  9;
		*pIndices++ = 12;     *pIndices++ = 15;     *pIndices++ = 21;
		*pIndices++ = 12;     *pIndices++ = 21;     *pIndices++ = 18;
	    
		// no offset in y-coords
		*pIndices++ =  9+1;   *pIndices++ =  6+1;   *pIndices++ = 21+1;
		*pIndices++ =  6+1;   *pIndices++ = 18+1;   *pIndices++ = 21+1;
		*pIndices++ =  0+1;   *pIndices++ = 15+1;   *pIndices++ = 12+1;
		*pIndices++ =  0+1;   *pIndices++ =  3+1;   *pIndices++ = 15+1;
	    
		// no offset in z-coords
		*pIndices++ =  3+2;   *pIndices++ =  9+2;   *pIndices++ = 15+2;
		*pIndices++ =  9+2;   *pIndices++ = 21+2;   *pIndices++ = 15+2;
		*pIndices++ =  0+2;   *pIndices++ = 18+2;   *pIndices++ =  6+2;
		*pIndices++ =  0+2;   *pIndices++ = 12+2;   *pIndices++ = 18+2;

		return S_OK;
	}


	bool m_bInit;
    LPDIRECT3DVERTEXBUFFER9 m_pWorldBoxVertices;
    LPDIRECT3DINDEXBUFFER9  m_pWorldBoxIndices;
	LPDIRECT3DCUBETEXTURE9 m_CubeMap;
};