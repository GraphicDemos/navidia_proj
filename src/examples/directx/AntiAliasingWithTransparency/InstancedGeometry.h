#ifndef INSTANCEDGEOMETRY_H
#define INSTANCEDGEOMETRY_H
#include "nvafx.h"

#define SORT_FRONT_BACK 1
#define SORT_BACK_FRONT 2

class InstancedGeometry
{
private:
	IDirect3DDevice9* m_pd3dDevice;
	IDirect3DVertexBuffer9* pGeoVB;
	IDirect3DIndexBuffer9* pGeoIB;
	IDirect3DVertexBuffer9* pInstanceVB;

	IDirect3DVertexDeclaration9* m_pDecl;
	D3DVERTEXELEMENT9* m_pVertexElement;
	int iCount;

public:
	InstancedGeometry(IDirect3DDevice9* pd3dDevice, int count, D3DVERTEXELEMENT9* pDecl, int DeclCount, void* pGeoVBData, DWORD GeoVBSize, void* pGeoIBData, DWORD GeoIBSize, void* pInstanceVBData, DWORD InstanceVBSize);
	~InstancedGeometry();
	HRESULT Set();
	HRESULT ReSet();
};

#endif