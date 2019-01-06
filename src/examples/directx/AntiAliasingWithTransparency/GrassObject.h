#ifndef GRASSOBJECT_H
#define GRASSOBJECT_H
#include "nvafx.h"
#include <string>
#include <stack>

#include "InstancedGeometry.h"
#include "EffectFactory.h"

class GrassObject : public EffectFactory
{
private:
	InstancedGeometry* m_pIG;

public:
	GrassObject(IDirect3DDevice9* pd3dDevice, std::stack<D3DXMATRIX>* worldPosStack, LPCWSTR texture, LPCWSTR normalmap);
	~GrassObject();

	HRESULT Render(LPCSTR tech, float alphaScale, double fTime = 0.0f);
};

#endif