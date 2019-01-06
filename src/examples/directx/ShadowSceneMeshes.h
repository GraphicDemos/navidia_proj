/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\StencilShadow\
File:  ShadowSceneMeshes.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/

#ifndef H_SHADOWSCENEMESHES_H
#define H_SHADOWSCENEMESHES_H

#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include "NV_D3DCommon\TGroup.h"
#include "NV_D3DMesh\ShadowVolumeMesh.h"
#include "NV_D3DMesh\ShadowVolumeMeshVB.h"
#include "NV_D3DMesh\MeshVB.h"

// struct to hold objects, matrices, rotation axes, speed, etc.

class SceneNonCaster
{
public:
	Mesh *			m_pMesh;
	MeshVB *		m_pMeshVB;
	D3DXMATRIX		m_matWorld;

	SceneNonCaster()
	{
		m_pMesh = new Mesh;
		m_pMeshVB = new MeshVB;
		D3DXMatrixIdentity( &m_matWorld );
	};
	~SceneNonCaster()
	{
		SAFE_DELETE( m_pMesh );
		SAFE_DELETE( m_pMeshVB );
	}
};


class SceneObject
{
public:
	ShadowVolumeMeshVB * pObj;
	ShadowVolumeMesh *	m_pSVMesh;
	D3DXMATRIX			Matrix;
	float				fShadowInset;
	float				fExDist;

	SceneObject()
	{
		pObj		= NULL;
		m_pSVMesh	= NULL;
		D3DXMatrixIdentity( &Matrix );
		fShadowInset	= 0.0f;
		fExDist			= 0.0f;
	};
	~SceneObject()
	{
	};
};

class SceneCaster
{
public:
	ShadowVolumeMeshVB *	m_pSVMeshVB;
	ShadowVolumeMesh *		m_pSVMesh;
	D3DXMATRIX				m_matWorld;
	float					fShadowInset;
	float					fExDist;

	SceneCaster()
	{
		m_pSVMeshVB	= new ShadowVolumeMeshVB;
		m_pSVMesh	= new ShadowVolumeMesh;
		D3DXMatrixIdentity( &m_matWorld );
		fShadowInset	= 0.0f;
		fExDist			= 0.0f;
	};
	~SceneCaster()
	{
		SAFE_DELETE( m_pSVMeshVB );
		SAFE_DELETE( m_pSVMesh );
	};
};



class ShadowSceneMeshes
{
protected:
	IDirect3DDevice9 *		m_pD3DDev;
	HRESULT		CreateSVMeshVBs( IDirect3DDevice9 * pDev, TGroup< SceneCaster > * pGrp );
	HRESULT		CreateBigScene( TGroup<SceneCaster> * pGrp );
	HRESULT		CreateNonCasters( TGroup<SceneNonCaster> * pGrp );
	
public:
	TGroup< SceneCaster >		m_BigSceneCasters;
	TGroup< SceneNonCaster >	m_NonCasters;		// ordinary scene objects

	HRESULT		CreateShadowVolumeMeshes();
	HRESULT		CreateDeviceVBs( IDirect3DDevice9 * pDev );
	HRESULT		FreeDeviceVBs();

	ShadowSceneMeshes()
	{
		m_pD3DDev = NULL;
	};
	~ShadowSceneMeshes()
	{
		FreeDeviceVBs();
		SAFE_RELEASE( m_pD3DDev );
	}
};

#endif		// H_SHADOWSCENEMESHES_H
