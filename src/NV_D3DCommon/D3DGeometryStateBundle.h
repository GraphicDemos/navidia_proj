/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DGeometryStateBundle.h

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


#ifndef H_D3DGEOMETRYSTATEBUNDLE_H
#define H_D3DGEOMETRYSTATEBUNDLE_H

#include "NV_D3DCommon\D3DDeviceStates.h"
#include <vector>
using namespace std;

class D3DGeometryStateBundle
{
public:
	enum D3DGSBState
	{
		D3DGSB_DISABLED,
		D3DGSB_INDEXED,
		D3DGSB_NONINDEXED,
		D3DGSB_FORCEDWORD = 0xFFFFFFFF
	};
public:
	vector< D3DDeviceState ** >		m_vppStates;	// stream sources, vertex declarations, index sources

	D3DGSBState				m_State;		// off, indexed prim, non-indexed prim
	D3DPRIMITIVETYPE		m_PrimType;
	UINT					m_uStart;
	UINT					m_uPrimitiveCount;
	UINT					m_uNumVertices;

	// These do nothing if m_bEnabled is false
	// apply the bundle & call Draw..Primitive()
	HRESULT		Render( IDirect3DDevice9 * pDev, bool bVerbose = false );
	HRESULT		Render( IDirect3DDevice9 * pDev, UINT start_index, UINT primitive_count, bool bVerbose = false );
	HRESULT		ApplyWithoutRendering( IDirect3DDevice9 * pDev );	// apply m_vStates

	D3DGeometryStateBundle();
	~D3DGeometryStateBundle();
	D3DGeometryStateBundle( D3DGeometryStateBundle & bdl )
	{
		size_t sz;
		m_vppStates.clear();
		for( sz = 0; sz < m_vppStates.size(); sz++ )
		{
			m_vppStates.push_back( bdl.m_vppStates.at(sz) );
		}
		m_State = bdl.m_State;
		m_PrimType			= bdl.m_PrimType;
		m_uStart			= bdl.m_uStart;;
		m_uPrimitiveCount	= bdl.m_uPrimitiveCount;
		m_uNumVertices		= bdl.m_uNumVertices;
	}
};

#endif								// H_D3DGEOMETRYSTATEBUNDLE_H

