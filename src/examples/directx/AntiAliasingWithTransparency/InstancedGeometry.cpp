#include <DXUT.h>
#include "InstancedGeometry.h"

InstancedGeometry::InstancedGeometry(IDirect3DDevice9* pd3dDevice, int count, D3DVERTEXELEMENT9* pDecl, int DeclCount, void* pGeoVBData, DWORD GeoVBSize, void* pGeoIBData, DWORD GeoIBSize, void* pInstanceVBData, DWORD InstanceVBSize)
{
	iCount = count;
	m_pd3dDevice = pd3dDevice;
	m_pVertexElement = NULL;
	m_pDecl = NULL;
	pGeoVB = NULL;
	pGeoIB = NULL;
	pInstanceVB = NULL;

	HRESULT hr;

	m_pVertexElement = new D3DVERTEXELEMENT9[DeclCount];
	memcpy(m_pVertexElement, pDecl, DeclCount * sizeof(D3DVERTEXELEMENT9));
	V(m_pd3dDevice->CreateVertexDeclaration(m_pVertexElement, &m_pDecl));

	VOID* pData;

	V(m_pd3dDevice->CreateVertexBuffer(GeoVBSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pGeoVB, NULL ));
	pGeoVB->Lock(0, GeoVBSize, &pData, 0 );
	memcpy(pData, pGeoVBData, GeoVBSize);	
	pGeoVB->Unlock();

	V(m_pd3dDevice->CreateIndexBuffer(GeoIBSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pGeoIB, NULL));
	pGeoIB->Lock( 0, GeoIBSize, &pData, 0 );
	memcpy(pData, pGeoIBData, GeoIBSize);
	pGeoIB->Unlock();

	V(m_pd3dDevice->CreateVertexBuffer(InstanceVBSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pInstanceVB, NULL ));
	pInstanceVB->Lock(0, InstanceVBSize, &pData, 0 );
	memcpy(pData, pInstanceVBData, InstanceVBSize);	
	pInstanceVB->Unlock();
}

HRESULT InstancedGeometry::Set()
{
	HRESULT hr = S_OK;

	//if(!pGeoIB || !pGeoVB ||! pInstanceVB)
	//	return E_FAIL;
	V_RETURN(m_pd3dDevice->SetVertexDeclaration(m_pDecl));
	V_RETURN(m_pd3dDevice->SetIndices(pGeoIB));
	V_RETURN(m_pd3dDevice->SetStreamSource(0, pGeoVB, 0, D3DXGetDeclVertexSize(m_pVertexElement, 0)));
	
	V_RETURN(m_pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | iCount));
	V_RETURN(m_pd3dDevice->SetStreamSource(1, pInstanceVB, 0, D3DXGetDeclVertexSize(m_pVertexElement, 1)));
	V_RETURN(m_pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
	return hr;
}

HRESULT InstancedGeometry::ReSet()
{
	HRESULT hr = S_OK;
	V_RETURN(m_pd3dDevice->SetStreamSourceFreq(0, 1));
	return hr;
}
InstancedGeometry::~InstancedGeometry()
{
	SAFE_RELEASE(pGeoVB);
	SAFE_RELEASE(pGeoIB);
	SAFE_RELEASE(pInstanceVB);
	SAFE_RELEASE(m_pDecl);

	SAFE_DELETE(m_pVertexElement);
}