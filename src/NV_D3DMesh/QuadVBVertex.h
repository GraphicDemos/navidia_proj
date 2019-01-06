

#ifndef H_QUADVBVERTEX_H
#define H_QUADVBVERTEX_H

#include "NV_D3DMeshDX9PCH.h"

class QuadVBVertex
{
public:
	D3DXVECTOR3	pos;
	IDirect3DVertexDeclaration9 *	CreateDeclaration( IDirect3DDevice9 * pDev );
	DWORD							GetFVF();
};


#endif
