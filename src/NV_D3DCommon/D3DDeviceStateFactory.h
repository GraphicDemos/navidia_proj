/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceStateFactory.h

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

#ifndef H_D3DDEVICESTATEFACTORY_H
#define H_D3DDEVICESTATEFACTORY_H

#include "NV_D3DCommon\D3DDeviceStates.h"
#include <vector>
using namespace std;

class D3DDeviceStateFactory
{
public:

	D3DStreamSource **			CreateD3DStreamSource( UINT num, IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride );
	D3DVertexDeclaration **		CreateD3DVertexDeclaration( IDirect3DVertexDeclaration9 * pDecl );
	D3DIndices **				CreateD3DIndices( IDirect3DIndexBuffer9 * pIndexData );

	HRESULT	Free();
	D3DDeviceStateFactory();
	~D3DDeviceStateFactory();

protected:
	vector< D3DDeviceState ** >		m_vppStatesAlloc;		// for tracking what to free
};

#endif							// H_D3DDEVICESTATEFACTORY_H
