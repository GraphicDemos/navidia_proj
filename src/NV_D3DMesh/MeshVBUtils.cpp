/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVBUtils.cpp

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


// for string decode functions

// Return the number of streams specified in the pElems array
// pElems must have a D3DDECL_END() to mark the end of the array
UINT GetNumStreams( const D3DVERTEXELEMENT9 * pElems )
{
	UINT count = 0;
	UINT iter = 0;
	RET_VAL_IF( pElems == NULL, 0 );
	vector< WORD > vStreams;
	bool bFound;
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	size_t i;

	while( (!(	(pElems[iter].Stream == end.Stream) &&		// not the end
				(pElems[iter].Offset == end.Offset) && 
				(pElems[iter].Type == end.Type )	)
			 )
			&& 
			( iter < 256 ) )		// not carrying on forever
	{

		bFound = false;
		for( i=0; i < vStreams.size(); i++ )
		{
			if( vStreams.at(i) == pElems[iter].Stream )
			{
				bFound = true;
				break;
			}
		}
		if( !bFound )
			vStreams.push_back( pElems[iter].Stream );
		iter++;
	}

	count = (UINT) vStreams.size();
	return( count );
}

UINT GetNumElements( const D3DVERTEXELEMENT9 * pElems )
{
	RET_VAL_IF( pElems == NULL, 0 );
	UINT uNumElems = 0;
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	while( (!(	(pElems[uNumElems].Stream == end.Stream) &&		// not the end
				(pElems[uNumElems].Offset == end.Offset) && 
				(pElems[uNumElems].Type == end.Type )	)
			 )
			&& 
			( uNumElems < 256 ) )		// not carrying on forever
	{
		uNumElems++;
	}
	return( uNumElems );
}


// Separate pElems into arrays of D3DVERTEXELEMENT9s corresponding to
//  each stream specified in pElems.  For example, if
//  pElems = { { 0,..,POSITION,..}, { 0,..,NORMAL,..}, { 1,..,COLOR,..} }
//  then the array of arrays of elements will be:
// out_vvElems[0][0] = { 0,..,POSITION,..}
// out_vvElems[0][1] = { 0,..,NORMAL,..}
// out_vvElems[1][0] = { 1,..,COLOR,..}
//
// If bAddTerminator is true, then D3DDECL_END() is added to the end of each array
//  and you will get:
// out_vvElems[0][0] = { 0,..,POSITION,..}
// out_vvElems[0][1] = { 0,..,NORMAL,..}
// out_vvElems[0][2] = { D3DDECL_END() }
// out_vvElems[1][0] = { 1,..,COLOR,..}
// out_vvElems[1][1] = { D3DDECL_END() }
//
HRESULT SeparateStreams( const D3DVERTEXELEMENT9 * pElems,
							vector< vector<D3DVERTEXELEMENT9> > & out_vvElems,
							bool bAddTerminator,
							bool bVerbose )
{
	HRESULT hr = S_OK;
	UINT count = 0;
	UINT iter = 0;
	RET_VAL_IF( pElems == NULL, 0 );
	vector< WORD > vStreams;
	bool bFound;
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	size_t i;
	out_vvElems.clear();

	while( (!(	(pElems[iter].Stream == end.Stream) &&		// not the end
				(pElems[iter].Offset == end.Offset) && 
				(pElems[iter].Type == end.Type )	)
			 )
			&& 
			( iter < 256 ) )		// not carrying on forever
	{
		bFound = false;
		for( i=0; i < vStreams.size(); i++ )
		{
			if( vStreams.at(i) == pElems[iter].Stream )
			{
				out_vvElems.at(i).push_back( pElems[iter] );
				bFound = true;
				break;
			}
		}
		if( !bFound )
		{
			vStreams.push_back( pElems[iter].Stream );
			vector< D3DVERTEXELEMENT9 > vec;
			vec.push_back( pElems[iter] );
			out_vvElems.push_back( vec );
		}
		iter++;
	}
	if( bAddTerminator )
	{
		for( i=0; i < out_vvElems.size(); i++ )
		{
			out_vvElems.at(i).push_back( end );
		}
	}
	if( bVerbose )
	{
		// output a report
		size_t n;
		for( i=0; i < out_vvElems.size(); i++ )
		{
			for( n=0; n < out_vvElems.at(i).size(); n++ )
			{
				D3DVERTEXELEMENT9 & elem = out_vvElems.at(i).at(n);
				const TCHAR * szType	= GetStrVERTEXELEMENT9Type( elem.Type );
				const TCHAR * szMethod	= GetStrVERTEXELEMENT9Method( elem.Method );
				const TCHAR * szUsage	= GetStrVERTEXELEMENT9Usage( elem.Usage );
				FMsg(TEXT("v[%u][%u] = { %u, %u, %s, %s, %s, %u }\n"), i, n, elem.Stream, elem.Offset, szType, szMethod, szUsage, elem.UsageIndex );
			}
		}
	}
	return( hr );
}

UINT GetRequiredVBElementSize( const D3DVERTEXELEMENT9 * pElems )
{
	RET_VAL_IF( pElems == NULL, 0 );
	UINT uSizeInBytes;
	UINT uNumStreams;
	uNumStreams = GetNumStreams( pElems );
	MSG_AND_RET_VAL_IF( uNumStreams != 1, "GetRequiredVertexSize() pElems has != 1 stream!\n", 0 );
	UINT uNumElems;
	uNumElems = GetNumElements( pElems );
	UINT uMaxOffset = 0;
	UINT uSizeOfThingAtMaxOffset = 0;
	UINT i;
	// find the max offset
	for( i=0; i < uNumElems; i++ )
	{
		// Does not check for overlapping data
		if( pElems[i].Offset > uMaxOffset )
		{
			uMaxOffset = pElems[i].Offset;
			uSizeOfThingAtMaxOffset = GetSizeOfTypeInBytes( pElems[i] );
			if( uSizeOfThingAtMaxOffset == 0 )
			{
				FMsg(TEXT("GetRequiredVBElementSize() GetSizeOfTypeInBytes() failed - thing should have non-zero size!\n"));
				assert( false );
				return( 0 );
			}
		}
		else if( pElems[i].Offset == uMaxOffset )
		{
			uMaxOffset = pElems[i].Offset;
			uSizeOfThingAtMaxOffset = max( uSizeOfThingAtMaxOffset, GetSizeOfTypeInBytes( pElems[i] ));
		}
	}
	uSizeInBytes = uMaxOffset + uSizeOfThingAtMaxOffset;
	// check against D3DX value
	UINT d3dxsize;
	d3dxsize = D3DXGetDeclVertexSize( pElems, pElems[0].Stream );
	if( d3dxsize != uSizeInBytes )
	{
		FMsg(TEXT("GetRequiredVBElementSize value doesn't match D3DX's size : %u   d3dx: %u\n"), uSizeInBytes, d3dxsize );
		// text report of the format:
		vector< vector< D3DVERTEXELEMENT9 > > vvElems;
		FMsg("  Vertex element structure:\n");
		SeparateStreams( pElems, vvElems, true, true );
		assert( false );
		return( d3dxsize );
	}
	return( uSizeInBytes );
}


HRESULT CreateVBToMatchDeclaration( Mesh * in_pMesh, 
									IDirect3DVertexDeclaration9 * in_pDecl,
									DWORD usage,
									IDirect3DVertexBuffer9 ** ppVB )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pDecl );
	D3DVERTEXELEMENT9 pElems[256];
	UINT numElems;
	hr = in_pDecl->GetDeclaration( pElems, &numElems );
	RET_VAL_IF( FAILED(hr), hr );
	IDirect3DDevice9 * pDev;
	hr = in_pDecl->GetDevice( &pDev );
	RET_VAL_IF( FAILED(hr), hr );
	RET_VAL_IF( pDev==NULL, E_FAIL );

	hr = CreateVBToMatchElements( in_pMesh, pElems, pDev, usage, ppVB );
	MSG_IF( FAILED(hr), "CreateVBToMatchElements(..) FAILED!\n");
	return( hr );
}

HRESULT CreateVBsToMatchDeclaration(	Mesh * in_pMesh,
										IDirect3DVertexDeclaration9 * in_pDecl,
										DWORD usage,
										vector< IDirect3DVertexBuffer9 * > & VBs )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pDecl );
	D3DVERTEXELEMENT9 pElems[256];
	UINT numElems;
	hr = in_pDecl->GetDeclaration( pElems, &numElems );
	RET_VAL_IF( FAILED(hr), hr );
	IDirect3DDevice9 * pDev;
	hr = in_pDecl->GetDevice( &pDev );
	RET_VAL_IF( FAILED(hr), hr );
	RET_VAL_IF( pDev==NULL, E_FAIL );

	hr = CreateVBsToMatchElements( in_pMesh, pElems, pDev, usage, VBs );
	MSG_IF( FAILED(hr), "CreateVBsToMatchDeclaration(..) FAILED!\n");
	return( hr );
}

HRESULT CreateVBToMatchElements(	Mesh * in_pMesh,
									D3DVERTEXELEMENT9 * pElems,
									IDirect3DDevice9 * pDev,
									DWORD usage,
									IDirect3DVertexBuffer9 ** ppVB )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pMesh );
	FAIL_IF_NULL( pElems );
	FAIL_IF_NULL( pDev );
	FAIL_IF_NULL( ppVB );
	UINT uNumStreams;
	uNumStreams = GetNumStreams( pElems );
	MSG_AND_RET_VAL_IF( uNumStreams != 1, "CreateVBToMatchElements() pElems has != 1 stream!\n", E_FAIL );

	UINT uVBElemSizeInBytes;
	uVBElemSizeInBytes = GetRequiredVBElementSize( pElems );

	// Create the vertex buffer
	hr = pDev->CreateVertexBuffer( in_pMesh->GetNumVertices()*uVBElemSizeInBytes, usage, NULL, D3DPOOL_DEFAULT, ppVB, NULL );
	MSG_AND_RET_VAL_IF( FAILED(hr), "CreateVBToMatchElements() couldn't create VB!\n", hr );

	// lock and fill the VB
	BYTE * pByte;
	DWORD lockflags = 0;
	if( usage & D3DUSAGE_DYNAMIC )
		lockflags = D3DLOCK_DISCARD;
	hr = (*ppVB)->Lock( 0, 0, (void**)&pByte, lockflags );
	MSG_AND_RET_VAL_IF( FAILED(hr), "CreateVBToMatchElements() Couldn't lock VB\n", hr );

	UINT i, nvert;
	BYTE * pDest, *pSrc;
	UINT uDDest, uDSrc;
	uDSrc = sizeof( MeshVertex );
	uDDest = uVBElemSizeInBytes;
	UINT uSzToCopy;
	UINT uNumElems = GetNumElements( pElems );

	for( i=0; i < uNumElems; i++ )
	{
		pDest = pByte + pElems[i].Offset;
		uSzToCopy = GetSizeOfTypeInBytes( pElems[i] );

		bool bCopy = false;
		switch( pElems[i].Usage )
		{
		case D3DDECLUSAGE_POSITION :
			pSrc = (BYTE*)(in_pMesh->m_pVertices[0].GetPositionP());
			bCopy = true;
			break;
		case D3DDECLUSAGE_NORMAL :
			pSrc = (BYTE*)(in_pMesh->m_pVertices[0].GetNormalP());
			bCopy = true;
			break;
		case D3DDECLUSAGE_TEXCOORD :
			pSrc = (BYTE*)(in_pMesh->m_pVertices[0].GetTexcoord0P());
			bCopy = true;
			break;
		case D3DDECLUSAGE_COLOR :
			pSrc = (BYTE*)(in_pMesh->m_pVertices[0].GetColorP());
			bCopy = true;
			break;
		}

		if( bCopy == true )
		{
			FAIL_IF_NULL( pSrc );			
			for( nvert=0; nvert < in_pMesh->GetNumVertices(); nvert++ )
			{
				memcpy( (void*)pDest, (void*)pSrc, uSzToCopy );
				pDest += uDDest;
				pSrc += uDSrc;
			}
		}
	}
	
	(*ppVB)->Unlock();
	return( hr );
}


// pElemsToMatch must end with D3DDECL_END()
// out_VBs are created in order of streams encountered in pElemsToMatch
// out_VBs are filled with data read from in_pMesh
HRESULT CreateVBsToMatchElements(	Mesh * in_pMesh,
									D3DVERTEXELEMENT9 * pElemsToMatch,
									IDirect3DDevice9 * pDev,
									DWORD usage,
									vector< IDirect3DVertexBuffer9 * >	& out_VBs )
{
	HRESULT hr = S_OK;
	// Separate pElemsToMatch into arrays of elements for each individual stream
	// Default is to have each array terminated with a D3DDECL_END()
	vector< vector< D3DVERTEXELEMENT9 > >		vvElems;
	hr = SeparateStreams( pElemsToMatch, vvElems );
	MSG_AND_RET_VAL_IF( FAILED(hr), "SeparateStreams(..) failed!\n", hr );

	// Generate a vertex buffer for each stream source specified in the arrays of elements
	out_VBs.clear();
	IDirect3DVertexBuffer9 * pVB;
	size_t s;
	for( s=0; s < vvElems.size(); s++ )
	{
		hr = CreateVBToMatchElements( in_pMesh, &vvElems.at(s).at(0), pDev, usage, &pVB );
		MSG_AND_RET_VAL_IF( FAILED(hr), "CreateVBsToMatchElements CreateVBToMatchElements() failed for a group\n", hr );
		out_VBs.push_back( pVB );
	}
	return( hr );
}
