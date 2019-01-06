/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  QuadVBVertex.cpp

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


#include "NV_D3DMeshDX9PCH.h"

// You must Release the declaration created returned by this function before your app exits.
IDirect3DVertexDeclaration9 * QuadVBVertex::CreateDeclaration( IDirect3DDevice9 * pDev )
{
	RET_VAL_IF( pDev == NULL, NULL );
	D3DVERTEXELEMENT9 pElem[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9 * pDecl;
	HRESULT hr;
	hr = pDev->CreateVertexDeclaration( pElem, &pDecl );
	RET_VAL_IF( FAILED(hr), NULL );
	return( pDecl );
}

DWORD	QuadVBVertex::GetFVF()
{
	DWORD fvf;
	fvf = D3DFVF_XYZ;
	return( fvf );
}
