//-----------------------------------------------------------------------------
// Copyright NVIDIA Corporation 2004
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED 
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS 
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL 
// NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR 
// CONSEQUENTIAL DAMAGES WHATSOEVER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR 
// LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS 
// INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR 
// INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGES.
// 
// File: MotherShipManager.h
// Desc: Wraps up management of little rotating D3DXMesh objects.  Used by the spaceship
//			manager to collide with.
//-----------------------------------------------------------------------------

#pragma once

#include "dxstdafx.h"
#include "Asteroid.h"
#include <DXUT/SDKmisc.h>
/*
This is a silly little class that wraps up the rendering of a D3DXMesh.
*/
class MotherShip : public Asteroid
{
public:
	MotherShip() 
	{
		position = D3DXVECTOR3(0,0,0);

		float fscale = 1.f;
		scale = D3DXVECTOR3(fscale,fscale,fscale);

		float cg = 0.25f + 0.25f*(float)rand()/32768.0f;
		color = D3DXCOLOR(cg,cg,cg,1);	

		float x = 2.f*((float)rand()/32768.0f-0.5f);
		float y = 2.f*((float)rand()/32768.0f-0.5f);
		float z = 2.f*((float)rand()/32768.0f-0.5f);
		rotationAxis = D3DXVECTOR3(0,1+y*0.1f,0);
		D3DXVec3Normalize(&rotationAxis,&rotationAxis);

		radius = 70.f*fscale;
		radiusSq = radius*radius;

		D3DXQuaternionRotationYawPitchRoll(&rotation,0,0,0);

		speed = 0.5f + (float)rand()/32768.0f;;
	}
};

const D3DVERTEXELEMENT9 g_aMSVertDecl[] =
{
	{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
	{0, 36, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
	{0, 48, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

class MothershipManager
{
public:

	MothershipManager()
	{
		ships = NULL;
		numShips = 0;
	}

	int GetNumPolysRendered()
	{
		if(shipMesh)
			return numShips*shipMesh->GetNumFaces();
		return 0;
	}

	void SetWorldExtents(D3DXVECTOR3 &wmin,D3DXVECTOR3 &wmax)
	{
		worldMin = wmin;
		worldMax = wmax;
	}

	void Init(int num)
	{
		MotherShip *newships;
		if(num > 0)
		{
			newships = new MotherShip[num];
		}

		if(ships != NULL)
		{
			// preserve existing ships
			if(num > 0 && numShips > 0)
			{
				if(num > numShips)
				{
					int numToCopy = numShips;
					memcpy(newships,ships,numToCopy*sizeof(MotherShip));
					for(int iShip=numShips;iShip<num;iShip++)
					{
						newships[iShip].MakeRandomPosition(worldMin,worldMax);
					}
				}
				else
				{
					int numToCopy = num;
					memcpy(newships,ships,numToCopy*sizeof(MotherShip));
				}
			}

			delete[]ships;
		}
		else
		{
			for(int iShip=0;iShip<num;iShip++)
			{
				newships[iShip].MakeRandomPosition(worldMin,worldMax);
			}
		}
		ships = newships;
		numShips = num;
	}

	void Update(float deltaTime)
	{
		for(int iShip=0;iShip<numShips;iShip++)
		{
			D3DXQUATERNION rotAdjust;
			D3DXQuaternionRotationAxis(&rotAdjust,&ships[iShip].rotationAxis,deltaTime*ships[iShip].speed);
			D3DXQuaternionMultiply(&ships[iShip].rotation,&ships[iShip].rotation,&rotAdjust);
		}
	}

	D3DXMATRIX MakeMatrixFor(int iShip)
	{
		D3DXMATRIX scaling; D3DXMatrixScaling(&scaling,ships[iShip].scale.x,ships[iShip].scale.y,ships[iShip].scale.z);
		D3DXMATRIX rotation; D3DXMatrixRotationQuaternion(&rotation,&ships[iShip].rotation);
		D3DXMATRIX translation; D3DXMatrixTranslation(&translation,ships[iShip].position.x,ships[iShip].position.y,ships[iShip].position.z);
		D3DXMATRIX world = scaling*rotation*translation;
		return world;
	}

	D3DXCOLOR GetColorFor(int iShip)
	{
		return ships[iShip].color;
	}

	HRESULT Create(IDirect3DDevice9* pd3dDevice)
	{
		WCHAR str[MAX_PATH];
		HRESULT hr;

		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"models\\UFO-03\\ufo3.x" ) );
		V_RETURN( D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &shipMesh) );

		DWORD *rgdwAdjacency = NULL;

		// Make sure there are normals which are required for lighting
		if( !(shipMesh->GetFVF() & D3DFVF_NORMAL) )
		{
			ID3DXMesh* pTempMesh;
			V( shipMesh->CloneMeshFVF( shipMesh->GetOptions(), 
				shipMesh->GetFVF() | D3DFVF_NORMAL, 
				pd3dDevice, &pTempMesh ) );
			V( D3DXComputeNormals( pTempMesh, NULL ) );
			SAFE_RELEASE( shipMesh );
			shipMesh = pTempMesh;
		}

		rgdwAdjacency = new DWORD[shipMesh->GetNumFaces() * 3];
		if( rgdwAdjacency == NULL )
			return E_OUTOFMEMORY;

		// Optimize the mesh for this graphics card's vertex cache 
		// so when rendering the mesh's triangle list the vertices will 
		// cache hit more often so it won't have to re-execute the vertex shader 
		// on those vertices so it will improve perf.     
		V( shipMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency) );
		V( shipMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL) );
		delete []rgdwAdjacency;

		// Clone the mesh into one that uses our decl
		ID3DXMesh* pConformedMesh = NULL;
		V(shipMesh->CloneMesh(0,g_aMSVertDecl,pd3dDevice,&pConformedMesh));
		shipMesh->Release();
		shipMesh = pConformedMesh;

		// Update tangent and binorm info
		rgdwAdjacency = new DWORD[shipMesh->GetNumFaces() * 3];
		if( rgdwAdjacency == NULL )
			return E_OUTOFMEMORY;
		V( shipMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency) );
		V(D3DXComputeTangent( shipMesh, 0,0,0,0,rgdwAdjacency));
		delete []rgdwAdjacency;

		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"models\\UFO-03\\UFO-03_Metal-01_df.dds" ) );

		V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
			D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
			D3DX_DEFAULT, D3DX_DEFAULT, 0, 
			NULL, NULL, &shipTex1 ) );

		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"models\\UFO-03\\UFO-03_Metal-02_df.dds" ) );

		V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
			D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
			D3DX_DEFAULT, D3DX_DEFAULT, 0, 
			NULL, NULL, &shipTex2 ) );

		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"models\\UFO-03\\UFO-03-glow.dds" ) );

		V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
			D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
			D3DX_DEFAULT, D3DX_DEFAULT, 0, 
			NULL, NULL, &shipGlow ) );

		return S_OK;
	}

	HRESULT Destroy()
	{
		SAFE_RELEASE(shipMesh);
		SAFE_RELEASE(shipTex1);  // Mesh texture
		SAFE_RELEASE(shipTex2);  // Mesh texture
		SAFE_RELEASE(shipGlow);  // Mesh texture

		return S_OK;
	}

	// returns draw calls
	int Render(IDirect3DDevice9* pd3dDevice, LPD3DXEFFECT g_pEffect)
	{
		HRESULT hr;
		UINT iPass, cPasses;
		int drawcalls = 0;
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			V( g_pEffect->SetTechnique( "RenderMSNormal" ) );
			V(g_pEffect->SetVector("g_MaterialDiffuseColor",&D3DXVECTOR4(1,1,1,1)));
			V( g_pEffect->SetFloat("SpecExpon",120.0f));
			V( g_pEffect->SetFloat("Ks",0.6f));
			V( g_pEffect->SetFloat("Bumpy",1.5f));
			V( g_pEffect->SetFloat("Kd",1.0f));

			for(int iShip=0;iShip<numShips;iShip++)
			{
				D3DXMATRIXA16 matrix = MakeMatrixFor(iShip);
				V( g_pEffect->SetMatrix( "g_mWorld", &(matrix) ));
				V( g_pEffect->SetTexture( "g_MeshBumpTexture", shipGlow) );

				V( g_pEffect->Begin(&cPasses, 0) );
				for (iPass = 0; iPass < cPasses; iPass++)
				{
					V( g_pEffect->BeginPass(iPass) );
					V( g_pEffect->SetTexture( "g_MeshTexture", shipTex1) );
					V( g_pEffect->CommitChanges() );
					shipMesh->DrawSubset(0);
					V( g_pEffect->SetTexture( "g_MeshTexture", shipTex2) );
					V( g_pEffect->CommitChanges() );

					shipMesh->DrawSubset(1);
					drawcalls+=2;
					V( g_pEffect->EndPass() );
				}
				V( g_pEffect->End() );
			}

			pd3dDevice->EndScene();
		}	
		return drawcalls;
	}

public:
	LPD3DXMESH shipMesh;
	LPDIRECT3DTEXTURE9				shipTex1;  // Mesh texture
	LPDIRECT3DTEXTURE9				shipTex2;  // Mesh texture
	LPDIRECT3DTEXTURE9				shipGlow;  // Mesh texture

	D3DXVECTOR3 worldMin;
	D3DXVECTOR3 worldMax;
	MotherShip *ships;
	int numShips;
	int polysPerShip;
};
