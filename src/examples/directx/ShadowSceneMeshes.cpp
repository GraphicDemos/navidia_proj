/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\StencilShadow\
File:  ShadowSceneMeshes.cpp

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

#include "NV_D3DMesh\NV_D3DMesh.h"
#include "ShadowSceneMeshes.h"


HRESULT CreateTerrain( Mesh * pMesh );

//-------------------------------------------------


HRESULT ShadowSceneMeshes::CreateSVMeshVBs( IDirect3DDevice9 * pDev, TGroup< SceneCaster > * pGrp )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	FAIL_IF_NULL( pGrp );

	size_t i;
	ShadowVolumeMesh * pMesh;
	ShadowVolumeMeshVB ** ppSVMeshVB;
	for( i=0; i < pGrp->GetNumElements(); i++ )
	{
		pMesh = pGrp->GetElement(i)->m_pSVMesh;
		ppSVMeshVB = &(pGrp->GetElement(i)->m_pSVMeshVB);
		if( pMesh != NULL )
		{
			if( *ppSVMeshVB == NULL )
				(*ppSVMeshVB) = new ShadowVolumeMeshVB;
			FAIL_IF_NULL( *ppSVMeshVB );
			hr = (*ppSVMeshVB)->CreateFromShadowVolumeMesh( pMesh, m_pD3DDev );
			MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't create ShadowVolumeMeshVB!\n", hr );
		}
		else
		{
			pGrp->GetElement(i)->m_pSVMeshVB = NULL;
		}
	}
	return( hr );
}

HRESULT ShadowSceneMeshes::CreateDeviceVBs( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pD3DDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	hr = CreateSVMeshVBs( pDev, &m_BigSceneCasters );
	RET_VAL_IF( FAILED(hr), hr );

	size_t i;
	SceneNonCaster * pNC;
	for( i=0; i < m_NonCasters.GetNumElements(); i++ )
	{
		pNC = m_NonCasters.GetElement(i);
		if( pNC == NULL )
			continue;
		if( pNC->m_pMesh == NULL )
			continue;
		if( pNC->m_pMeshVB == NULL )
			pNC->m_pMeshVB = new MeshVB;
		pNC->m_pMeshVB->CreateFromMesh( pNC->m_pMesh, pDev );
	}
	return( hr );
}

HRESULT	ShadowSceneMeshes::FreeDeviceVBs()
{
	size_t i;
	for( i=0; i < m_BigSceneCasters.GetNumElements(); i++ )
	{
		SAFE_DELETE( m_BigSceneCasters.GetElement(i)->m_pSVMeshVB );
	}

	SceneNonCaster * pNC;
	for( i=0; i < m_NonCasters.GetNumElements();  i++ )
	{
		pNC = m_NonCasters.GetElement(i);
		if( pNC == NULL )
			continue;
		SAFE_DELETE( pNC->m_pMeshVB );
	}
	return( S_OK );
}


HRESULT CreateTerrain( Mesh * pMesh )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );

	float width;
	int nHDiv, nVDiv;		// number of divisions

	width = 25.0f;
	nHDiv = nVDiv = 55;

	// Init a flat grid of geometry (twist = 0.0f => flat):
	MeshGeoCreator gc;
	gc.InitTesselatedPlane( pMesh, D3DXVECTOR3( 1.0f, 0.0f, 0.0f ), width, nHDiv,
							D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), width, nVDiv );
	D3DXVECTOR3  vNoiseDir;	
	vNoiseDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );		// direction of noise perturbation

	// Make a fractalish landscape by adding noise of varying roughness
	// This is what generates the terrain geometry 

	const UINT uNumNoise = 3;
	GridNoiseComponent pNoise[ uNumNoise ];
	float f1, f2, f3;
	f1 = 10.0f;
	f2 = 5.0f;
	f3 = 1.0f;

	f1 = 12.0f;
	f2 = 5.0f;
	f3 = 1.0f;
	pNoise[0].amplitude = 1.7f;
	pNoise[0].freq_x = f1 / width;
	pNoise[0].freq_y = f1 / width;
	pNoise[0].freq_z = 0.0f;
	pNoise[0].seed = 1;

	pNoise[1].amplitude = 0.5f;
	pNoise[1].freq_x = f2 / width;
	pNoise[1].freq_y = f2 / width;
	pNoise[1].freq_z = 0.0f;
	pNoise[1].seed = 2;

	pNoise[2].amplitude = 0.25f;
	pNoise[2].freq_x = f3 / width;
	pNoise[2].freq_y = f3 / width;
	pNoise[2].freq_z = 0.0f;
	pNoise[2].seed = 3;

	MeshProcessor mp;
	mp.AddPositionNoise1D( pMesh, pMesh, vNoiseDir, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
							pNoise, uNumNoise );

	D3DXVECTOR3 min, max;
	mp.FindPositionMinMax( pMesh, &min, &max );
	pMesh->Translate( 0.0f, 0.0f, -min.z );

	mp.CalculateNormalsCCW( pMesh );

	pMesh->SetVertexColor( 0x00FFFFFF );
	return( hr );
}


HRESULT ShadowSceneMeshes::CreateNonCasters( TGroup<SceneNonCaster> * pGrp )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pGrp );
	pGrp->FreeElements();

	SceneNonCaster * pSNC;
	MeshGeoCreator gc;

	pSNC = pGrp->NewElement();
	FAIL_IF_NULL( pSNC );
	BREAK_AND_RET_VAL_IF( pSNC->m_pMesh == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( pSNC->m_pMeshVB == NULL, E_FAIL );
	CreateTerrain( pSNC->m_pMesh );

	return( hr );
}


HRESULT ShadowSceneMeshes::CreateBigScene( TGroup<SceneCaster> * pGrp )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pGrp );
	pGrp->FreeElements();

	MeshGeoCreator gc;
	SceneCaster * pObj;
	DWORD	red = 0x00FF3333;
	DWORD   grn = 0x0022FF22;
	DWORD   wht = 0x00D0D0D0;
	float zerof = 0.0f;

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity( &matIdentity );

	// If dot of adjacent face normals is greater than fWeldEdgeThresh, then 
	//  adjacent face vertices will be welded and no zero-area triangles inserted 
	float fWeldEdgeThresh =  0.8f;
	float fShdVolDist		=  1.0f;
	float fShadowInset	= -0.06f;;

	D3DXVECTOR3 trans;
	D3DXMATRIX mat1, mat2, mat3;

	// Z direction is up

	// A base
	pObj = pGrp->NewElement();	FAIL_IF_NULL( pObj );
	gc.InitBlockCS( pObj->m_pSVMesh, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 2.0f, 2.0f, 1.0f ) );
	trans = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	pObj->m_pSVMesh->Translate( trans.x, trans.y, trans.z );
	pObj->m_pSVMesh->SetVertexColor( wht );
	pObj->m_pSVMesh->BuildShadowVolume( fWeldEdgeThresh );	
	pObj->fShadowInset	= zerof;
	pObj->fExDist		= 2.0f * fShdVolDist;
	//-----------------------------------
	// tori on the base
	pObj = pGrp->NewElement();	FAIL_IF_NULL( pObj );
	Mesh m1;
	float rad, csrad, tz;
	rad = 1.2f;
	csrad = 0.07f;
	tz = rad + 0.3f;
	gc.InitTorus( &m1, D3DXVECTOR3( 1.0f, 0.0f, 0.0f ), rad, 30, csrad, 10 );
	gc.InitClone( pObj->m_pSVMesh, &m1 );
	gc.InitTorus( &m1, D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), rad, 30, csrad, 10 );
	gc.InitAddClone( pObj->m_pSVMesh, &m1 );
	gc.InitTorus( &m1, D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), rad, 30, csrad, 10 );
	gc.InitAddClone( pObj->m_pSVMesh, &m1 );
	pObj->m_pSVMesh->SetVertexColor( wht );
	pObj->m_pSVMesh->BuildShadowVolume( fWeldEdgeThresh );	
	pObj->fShadowInset	= -0.04f;
	pObj->fExDist		= 5.0f * fShdVolDist;

	D3DXMatrixRotationX( &mat1, 3.14159f / 4.0f );
	D3DXMatrixRotationY( &mat2, 3.14159f / 5.0f );
	D3DXMatrixMultiply( &mat3, &mat1, &mat2 );
	D3DXMatrixMultiply( &mat3, &mat3, D3DXMatrixTranslation( &mat1, 0.0f, 0.0f, tz ) );
	pObj->m_matWorld = mat3;
	//-----------------------------------
	pObj = pGrp->NewElement();  FAIL_IF_NULL( pObj );
	float circrad, height;
	circrad = 3.0f;
	rad = 0.1f;
	height = 1.0f;
	gc.InitCylinder( &m1, D3DXVECTOR3( circrad, 0.0f, 0.0f ), D3DXVECTOR3( circrad, 0.0f, height ),
						rad, 12, 0, 0, 0 );
	gc.InitClone( pObj->m_pSVMesh, &m1 );
	int numcyls = 10;
	int n;
	for( n=1; n < numcyls; n++ )
	{
		D3DXMatrixRotationZ( &mat1, 3.14159f * 2.0f / numcyls );
		m1.Transform( &mat1 );	
		gc.InitAddClone( pObj->m_pSVMesh, &m1 );
	}
	pObj->m_pSVMesh->BuildShadowVolume( fWeldEdgeThresh );	
	pObj->fShadowInset	= -0.001f;
	pObj->fExDist		= 10.0f * fShdVolDist;
	//------------------------------------
	pObj = pGrp->NewElement();  FAIL_IF_NULL( pObj );
	gc.InitTorus( pObj->m_pSVMesh, D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), 
					circrad, numcyls, rad * 1.1f, 8 );
	pObj->m_pSVMesh->Translate( 0.0f, 0.0f, height );
	pObj->m_pSVMesh->BuildShadowVolume( fWeldEdgeThresh );	
	pObj->fShadowInset	= -0.001f;
	pObj->fExDist		= 10.0f * fShdVolDist;

	return( hr );
}

HRESULT ShadowSceneMeshes::CreateShadowVolumeMeshes()
{
	HRESULT hr = S_OK;
	hr = CreateBigScene( & m_BigSceneCasters );
	RET_VAL_IF( FAILED(hr), hr );

	hr = CreateNonCasters( & m_NonCasters );
	RET_VAL_IF( FAILED(hr), hr );

	return( hr );
}

