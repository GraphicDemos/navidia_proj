/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVBUtils.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Collection of utility functions for creating D3D vertex and index buffers from a Mesh

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_MESHVBUTILS_H
#define H_MESHVBUTILS_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include <vector>
using namespace std;

UINT	GetNumStreams( const D3DVERTEXELEMENT9 * pElems );
HRESULT SeparateStreams(	const D3DVERTEXELEMENT9 * pElems, 
							vector< vector<D3DVERTEXELEMENT9> > & out_vvElems,
							bool bAddTerminator = true,
							bool bVerbose = false );
UINT	GetNumElements( const D3DVERTEXELEMENT9 * pElems );
UINT	GetRequiredVBElementSize( const D3DVERTEXELEMENT9 * pElems );

// You must clean up the pointers returned by these functions by calling Release() when you're done
HRESULT CreateVBToMatchDeclaration( Mesh * in_pMesh, 
									IDirect3DVertexDeclaration9 * in_pDecl,
									DWORD usage,
									IDirect3DVertexBuffer9 ** ppVB );

HRESULT CreateVBsToMatchDeclaration(	Mesh * in_pMesh,
										IDirect3DVertexDeclaration9 * in_pDecl,
										DWORD usage,
										vector< IDirect3DVertexBuffer9 * > & VBs );

HRESULT CreateVBToMatchElements(	Mesh * in_pMesh,
									D3DVERTEXELEMENT9 * pElems,
									IDirect3DDevice9 * pDev,
									DWORD usage,
									IDirect3DVertexBuffer9 ** ppVB );

HRESULT CreateVBsToMatchElements(	Mesh * in_pMesh,
									D3DVERTEXELEMENT9 * pElemsToMatch,
									IDirect3DDevice9 * pDev,
									DWORD usage,
									vector< IDirect3DVertexBuffer9 * > & VBs );


#endif	H_MESHVBUTILS_H
