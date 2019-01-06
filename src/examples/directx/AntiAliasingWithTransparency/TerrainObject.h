#ifndef TERRAINOBJECT_H
#define TERRAINOBJECT_H
#include "EffectFactory.h"
#include "InstancedGeometry.h"

class TerrainObject : public EffectFactory
{
private:
	ID3DXMesh* m_pMesh;
	D3DCULL m_cFront;
	D3DCULL m_cBack;
public:
	TerrainObject(IDirect3DDevice9* pd3dDevice);
	~TerrainObject();

	HRESULT RenderGeometry();
	HRESULT RenderFront();
	HRESULT RenderBack();
	HRESULT Render();
	HRESULT ReturnCoverageSize(int* x, int* y);
	HRESULT SetFront(D3DCULL front);
};

#endif