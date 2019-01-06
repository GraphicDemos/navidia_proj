/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  LoadXFile.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the .h for comments.

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DCommonDX9PCH.h"
#include "shared\GetFilePath.h"

#include <string>
using namespace std;


LoadXFile::LoadXFile()
{
	SetAllNull();
}

LoadXFile::~LoadXFile()
{
	Free();
}


HRESULT LoadXFile::Free()
{
	FreeLoadedData();
	FREE_GUARANTEED_ALLOC( m_ppTextureFactory, m_pTextureFactory );
	SAFE_RELEASE( m_pD3DDev );
	SetAllNull();
	return( S_OK );
}

HRESULT LoadXFile::Initialize( IDirect3DDevice9 * pDev, 
								GetFilePath::GetFilePathFunction pGetFilePathFunction,
								TextureFactory ** ppTextureFactory )
{
	HRESULT hr = S_OK;
	hr = Free();
	FAIL_IF_NULL( pDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_GetFilePathFunction = pGetFilePathFunction;

	// if ppTextureFactory is NULL, create one for ourselves
	GUARANTEE_ALLOCATED( ppTextureFactory, m_ppTextureFactory, m_pTextureFactory, TextureFactory, Initialize( m_GetFilePathFunction ));

	return( hr );
}

bool LoadXFile::ToggleWireframe()
{
	m_bWireframe = !m_bWireframe;
	return( m_bWireframe );
}

tstring LoadXFile::GetFilePath( const TCHAR * in_pFilePath )
{
	tstring filepath = TEXT("");
	if( in_pFilePath == NULL )
	{
		FMsg("LoadXFile::GetFilePath() input path is NULL!\n");
		return( TEXT("") );
	}

	bool bVerbose = false;
	if( m_GetFilePathFunction != NULL )
	{
		filepath = (*m_GetFilePathFunction)( in_pFilePath, bVerbose );
	}
	else
	{
		FMsg("LoadXFile::GetFilePath function is NULL, returning the input string!\n");
		filepath = in_pFilePath;
	}
	return( filepath );
}

TextureFactory ** LoadXFile::GetTextureFactoryPP()
{
	return( m_ppTextureFactory );
}

TextureFactory * LoadXFile::GetTextureFactory()
{
	if( m_ppTextureFactory == NULL )
		return( NULL );
	else
		return( *m_ppTextureFactory );
}


HRESULT LoadXFile::FreeLoadedData()
{
	HRESULT hr = S_OK;
	// For now, this keeps all textures alive in the TextureFactory
	SAFE_DELETE_ARRAY( m_pMeshMaterials );
	SAFE_RELEASE( m_pMesh );
	return( hr );
}

HRESULT LoadXFile::ListMeshInfo()
{
	HRESULT hr = S_OK;
	UINT i;

	FMsg("LoadXFile::ListMeshInfo() for mesh at 0x%x\n", m_pMesh );
	MSG_AND_RET_VAL_IF( m_pMesh==NULL, "Mesh pointer is NULL!\n", hr );

	DWORD dwNumFaces;
	DWORD dwNumVertices;
	dwNumFaces = m_pMesh->GetNumFaces();
	dwNumVertices = m_pMesh->GetNumVertices();

	FMsg("num faces = %u     num vertices = %u\n", dwNumFaces, dwNumVertices );
	FMsg("num DrawPrimitive(..) calls = %u\n", m_dwNumMaterials );

	// get number of attributes
	DWORD				dwAttributeTableSize;
	hr = m_pMesh->GetAttributeTable( NULL,  &dwAttributeTableSize );
	FMsg("attribute table size = %u\n", dwAttributeTableSize );

	if( dwAttributeTableSize > 0 )
	{
		D3DXATTRIBUTERANGE * pAttributeTable = new D3DXATTRIBUTERANGE[ dwAttributeTableSize ];
		FAIL_IF_NULL( pAttributeTable );
		hr = m_pMesh->GetAttributeTable( pAttributeTable,  &dwAttributeTableSize );
		if( SUCCEEDED(hr) )
		{
			for( i=0; i < dwAttributeTableSize; i++ )
			{
				FMsg("AttributeTable[i].AttribId    = %u\n", pAttributeTable[i].AttribId );
				FMsg("AttributeTable[i].FaceCount   = %u\n", pAttributeTable[i].FaceCount );
				FMsg("AttributeTable[i].FaceStart   = %u\n", pAttributeTable[i].FaceStart );
				FMsg("AttributeTable[i].VertexCount = %u\n", pAttributeTable[i].VertexCount );
				FMsg("AttributeTable[i].VertexStart = %u\n", pAttributeTable[i].VertexStart );
			}
		}
		else
		{
			FMsg("Couldn't get attribute table!\n");
		}
		SAFE_DELETE_ARRAY( pAttributeTable );
	}

	DWORD dwNumBytesPerVertex;
	dwNumBytesPerVertex = m_pMesh->GetNumBytesPerVertex();
	FMsg("num bytes per vertex = %u\n", dwNumBytesPerVertex );

	// Get mesh size and extents
	D3DXVECTOR3 aabb_min, aabb_max;
	GetMeshAABB( GetMesh(), &aabb_min, &aabb_max, true );

	FMsg("Mesh AABB corners: ( %f, %f, %f ) to ( %f, %f, %f )\n", aabb_min.x, aabb_min.y, aabb_min.z,
					aabb_max.x, aabb_max.y, aabb_max.z );
	FMsg("\n");

	return( hr );
}

//-------------------------------------------------------------------------------
// Get the mesh's axis aligned bounding box.
//-------------------------------------------------------------------------------
HRESULT LoadXFile::GetMeshAABB( LPD3DXMESH in_pMesh, D3DXVECTOR3 * out_pMinPoint, D3DXVECTOR3 * out_pMaxPoint, bool bVerbose )
{
	// HRESULT WINAPI D3DXComputeBoundingBox( const D3DXVECTOR3 *pFirstPosition, DWORD NumVertices, DWORD dwStride, D3DXVECTOR3 *pMin, D3DXVECTOR3 *pMax );
	if( m_bAABBValid )
	{
		if( out_pMinPoint != NULL )
			*out_pMinPoint = m_AABBMin;
		if( out_pMaxPoint != NULL )
			*out_pMaxPoint = m_AABBMax;	
		return( S_OK );
	}

	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pMesh );
	D3DXVECTOR3 min, max;

	LPDIRECT3DVERTEXBUFFER9	pVB;
	m_pMesh->GetVertexBuffer( &pVB );
	MSG_AND_RET_VAL_IF( pVB==NULL, "Couldn't get vertex buffer!\n", E_FAIL );

	DWORD dwNumBytesPerVertex;
	dwNumBytesPerVertex = m_pMesh->GetNumBytesPerVertex();

	D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
	m_pMesh->GetDeclaration( Declaration );

	// #define D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}
	UINT i = 0;
	UINT pos_ind;
	bool bIsFloat3, bDone = false;
	DWORD pos_offset;
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	while( ( bDone == false ) && ( i < 1000 ) )
	{
		// FMsg("decl %u\n", i );		
		if( Declaration[i].Usage == D3DDECLUSAGE_POSITION )
		{
			pos_ind = i;
			pos_offset = Declaration[i].Offset;
			if( Declaration[i].Type == D3DDECLTYPE_FLOAT3 )
			{
				bIsFloat3 = true;
			}
			bDone = true;
		}

		if( Declaration[i].Method == end.Method &&
			Declaration[i].Offset == end.Offset &&
			Declaration[i].Stream == end.Stream &&
			Declaration[i].Usage == end.Usage )
		{
			bDone = true;
		}
	}

	if( bVerbose )
		FMsg("Found position at offset %u bytes   %s\n", pos_offset, bIsFloat3 ? "float3" : "not float3" );

	if( !bIsFloat3 )
	{
		FMsg("Can't read position that is not float3!\n");
		return( E_FAIL );
	}

	BYTE * pData;
	hr = m_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**) & pData );
	MSG_AND_RET_VAL_IF( pData == NULL || FAILED(hr), "Couldn't lock vertex buffer!\n", E_FAIL );

	D3DVERTEXBUFFER_DESC vbdesc;
	pVB->GetDesc( &vbdesc );

	UINT SizeInBytes = vbdesc.Size;
	if( bVerbose )
		FMsg("vb size in bytes = %u\n", SizeInBytes );

	D3DXVECTOR3 * pFloat3;
	pFloat3 = (D3DXVECTOR3*) (pData + pos_offset);
	min = *pFloat3;
	max = *pFloat3;
	for( i = pos_offset; i < SizeInBytes; i += dwNumBytesPerVertex )
	{
		pFloat3 = (D3DXVECTOR3*) ( pData + i );
	
		if( pFloat3->x < min.x )
			min.x = pFloat3->x;
		if( pFloat3->x > max.x )
			max.x = pFloat3->x;
		if( pFloat3->y < min.y )
			min.y = pFloat3->y;
		if( pFloat3->y > max.y )
			max.y = pFloat3->y;
		if( pFloat3->z < min.z )
			min.z = pFloat3->z;
		if( pFloat3->z > max.z )
			max.z = pFloat3->z;
	}

	m_bAABBValid	= true;
	m_AABBMin		= min;
	m_AABBMax		= max;
	if( out_pMinPoint != NULL )
		*out_pMinPoint = min;
	if( out_pMaxPoint != NULL )
		*out_pMaxPoint = max;

	SAFE_RELEASE( pVB );
	m_pMesh->UnlockVertexBuffer();
	return( hr );
}


HRESULT LoadXFile::SetMatrixToXFormMesh( const D3DXVECTOR3 & aabb_center, const D3DXVECTOR3 & aabb_size,
										 const LoadXFileFlags & flags )
{
	HRESULT hr = S_OK;
	hr = GetMatrixToXFormMesh( &m_matWorld, aabb_center, aabb_size, flags );
	return( hr );
}


//----------------------------------------------------------------------------
// Compute and return a matrix that will transform the mesh into the bounding
//  box defined by aabb_center and aabb_size.  Each field of aabb_size describes
//  the length along each side of the bounding box into which the mesh will fit.
// Also sets m_matWorld to the calculated matrix.
// If flags = KEEP_ASPECT_RATIO then the scale is taken from the aabb_size.x
//  parameter, and the y and z scales are computed from the existing aabb size
//  so that the mesh is scaled uniformly.
//----------------------------------------------------------------------------
HRESULT LoadXFile::GetMatrixToXFormMesh( D3DXMATRIX * out_pMat, const D3DXVECTOR3 & aabb_center, const D3DXVECTOR3 & aabb_size,
										 const LoadXFileFlags & flags )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( out_pMat );
	LPD3DXMESH pMesh = GetMesh();
	FAIL_IF_NULL( pMesh );

	// fills m_AABBMin and m_AABBMax
	hr = GetMeshAABB( pMesh, NULL, NULL );
	D3DXVECTOR3 aabb_diag = m_AABBMax - m_AABBMin;
	D3DXVECTOR3 scale;

	if( aabb_diag.x == 0.0f )
	{
		FMsg("x dimension is 0.0f, scaling x by 1.0f\n");
		aabb_diag.x = aabb_size.x;
	}
	if( aabb_diag.y == 0.0f )
	{
		FMsg("y dimension is 0.0f, scaling y by 1.0f\n");
		aabb_diag.y = aabb_size.y;
	}
	if( aabb_diag.z == 0.0f )
	{
		FMsg("z dimension is 0.0f, scaling z by 1.0f\n");
		aabb_diag.z = aabb_size.z;
	}

	if( flags == KEEP_ASPECT_RATIO )
	{
		scale.x = aabb_size.x / aabb_diag.x;
		scale.y = scale.z = scale.x;
	}
	else
	{
		scale.x = aabb_size.x / aabb_diag.x;
		scale.y = aabb_size.y / aabb_diag.y;
		scale.z = aabb_size.z / aabb_diag.z;
	}

	D3DXVECTOR3 mesh_center = ( m_AABBMax + m_AABBMin ) / 2.0f;

	D3DXMATRIX t0, tf, sc, mat;
	// matrix to translate mesh to 0,0,0
	D3DXMatrixTranslation( &t0, mesh_center.x, mesh_center.y, mesh_center.z );
	D3DXMatrixScaling( &sc, scale.x, scale.y, scale.z );
	D3DXMatrixTranslation( &tf, aabb_center.x, aabb_center.y, aabb_center.z );

	D3DXMatrixMultiply( &mat, &t0, &sc );
	D3DXMatrixMultiply( &mat, &mat, &tf );

	m_matWorld = mat;
	*out_pMat = mat;

	return( hr );
}

D3DXMATRIX * LoadXFile::GetMatrixP()
{
	return( &m_matWorld );	
}

HRESULT LoadXFile::LoadTexture( LPCTSTR pFilename, IDirect3DTexture9 *** out_pppTexture, bool bVerbose )
{
	HRESULT hr = S_OK;
	TextureFactory * pTF = GetTextureFactory();
	FAIL_IF_NULL( pTF );
	FAIL_IF_NULL( pFilename );
	bool bPassedVerbose;
//	bPassedVerbose = bVerbose;
	bPassedVerbose = false;
	IDirect3DTexture9	** ppTex = NULL;

	if( bVerbose )
		FMsg(TEXT("LoadTexture trying : %s\n"), pFilename );

	if( pFilename != NULL &&
		lstrlen(pFilename) > 0 )
	{
		ppTex = pTF->CreateTextureFromFile( m_pD3DDev, pFilename, bPassedVerbose );

		if( bVerbose && ( ppTex != NULL ))
		{
			FMsg(TEXT("LoadXFile::LoadTexture( \"%s\" ) SUCCEEDED\n"), pFilename );
		}
		if( ppTex == NULL )
		{
			// Try finding the file from the filename alone by eliminating any path info in the name.
			if( bVerbose )
				FMsg(TEXT("LoadXFile::LoadTexture( \"%s\" ) failed\n"), pFilename );

			tstring name;
			name = GetFilenameFromFullPath( pFilename );
			
			if( bVerbose )
				FMsg(TEXT("  Trying : %s\n"), name.c_str() );

			ppTex = pTF->CreateTextureFromFile( m_pD3DDev, name.c_str(), bPassedVerbose );

			if( (ppTex == NULL) && 
				(m_strDefaultTexFilename.length()  > 0) )
			{
				if( bVerbose )
				{
					FMsg("  Texture loading failed.  Loading default texture instead\n");
				}
				ppTex = pTF->CreateTextureFromFile( m_pD3DDev, m_strDefaultTexFilename.c_str(), bPassedVerbose );
			}
			else
			{
				if( bVerbose )
					FMsg(TEXT("  LoadXFile::LoadTexture( \"%s\" ) SUCCEEDED\n"), name.c_str() );
			}
		}
	}
	else
	{
		hr = E_FAIL;
	}
	*out_pppTexture = ppTex;
	return( hr );
}


HRESULT LoadXFile::LoadMaterials( const LPD3DXBUFFER pD3DXMtrlBuffer, DWORD dwNumMaterials, bool bVerbose )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pD3DXMtrlBuffer );
	if( dwNumMaterials == 0 )
		return(hr);

	// Extract the material properties and texture names from the material bufer
    D3DXMATERIAL * d3dxMaterials;
	d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	FAIL_IF_NULL( d3dxMaterials );

	SAFE_DELETE_ARRAY( m_pMeshMaterials );
	m_pMeshMaterials = new D3DMATERIAL9[ dwNumMaterials ];
	FAIL_IF_NULL( m_pMeshMaterials );
	m_dwNumMaterials = dwNumMaterials;

	SAFE_DELETE_ARRAY( m_Textures );
	m_Textures = new IDirect3DTexture9**[ dwNumMaterials ];
	FAIL_IF_NULL( m_Textures );

	// extract material properties and texture names from the pD3DXMtrlBuffer
	DWORD i;
	for( i=0; i < m_dwNumMaterials; i++ )
	{
		m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		// Set the ambient color.  D3DX does not do this.
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;
		m_Textures[i] = NULL;
#ifdef UNICODE
		//FMsg("LoadXFile::LoadMaterials : %s\n", d3dxMaterials[i].pTextureFilename );
		wstring wstr = NVStringConv::lpcstrToWString( d3dxMaterials[i].pTextureFilename );
		//FMsg(TEXT("                 wstring : %s\n"), wstr.c_str() );
		LoadTexture( wstr.c_str(), &(m_Textures[i]), bVerbose );
#else
		LoadTexture( d3dxMaterials[i].pTextureFilename, &(m_Textures[i]), bVerbose );
#endif
	}
	return( hr );
}



HRESULT LoadXFile::LoadFile( const TCHAR * in_pFilePath, bool bVerbose )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pFilePath );
	if( bVerbose )
		FMsg("LoadXFile::LoadFile( \"%s\" ) called\n", in_pFilePath );

	tstring file_path;
	file_path = GetFilePath( in_pFilePath );

	bool bSamePathAsInput;
	// bSamePathAsInput = true if strings are equal
	bSamePathAsInput = ( _tcscmp( file_path.c_str(), in_pFilePath ) == 0 );

	if( bVerbose )
	{
		if( bSamePathAsInput )
			FMsg("  GetFilePath() result is same as input path\n" );
		else
			FMsg("  GetFilePath() found \"%s\" for input file name\n", file_path.c_str() );
	}

	// Attempt to load the .x file
    LPD3DXBUFFER	pD3DXMtrlBuffer = NULL;
	DWORD			dwNumMaterials;

	SAFE_RELEASE( m_pMesh );
	hr = D3DXLoadMeshFromX( file_path.c_str(), 
							D3DXMESH_MANAGED,	// both sysmem and GPU hw mem buffers
							m_pD3DDev, 
							NULL,				// no adjacency
							&pD3DXMtrlBuffer,
							NULL,				// no effects
							&dwNumMaterials,
							&m_pMesh );
	if( FAILED(hr) )
	{
		FMsg("LoadXFile::LoadFile( \"%s\" )  D3DXLoadMeshFromX(..) failed!\n", file_path.c_str() );
		return( hr );
	}

	m_dwNumMaterials = dwNumMaterials;
	if( bVerbose )
		FMsg("num materials = %u\n", m_dwNumMaterials );

	hr = LoadMaterials( pD3DXMtrlBuffer, dwNumMaterials, bVerbose );
	if( FAILED(hr) )
	{
		FMsg("LoadXFile::LoadFile(..) Couldn't LoadMaterials(..)\n");
	}

	SAFE_RELEASE( pD3DXMtrlBuffer );
	if( bVerbose )
		FMsg("LoadXFile::LoadFile(..) returning %s\n", FAILED(hr) ? "FAILED" : "SUCCEEDED" );
	return( hr );
}


//------------------------------------------------------------------------------
// num_sections is the number to draw starting with start_section
//------------------------------------------------------------------------------
HRESULT LoadXFile::RenderSections( DWORD start_section, DWORD num_sections, bool bSetTexture, bool bSetMaterial )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pMesh );
	DWORD i, end_limit;
	if( start_section >= m_dwNumMaterials )
		return( S_OK );
	end_limit = start_section + num_sections;
	if( end_limit > m_dwNumMaterials )
		end_limit = m_dwNumMaterials;


	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,	m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	for( i = start_section; i < end_limit; i++ )
	{
		if( bSetMaterial )
		{
	        // Set the material and texture for this subset
			m_pD3DDev->SetMaterial( &m_pMeshMaterials[i] );
		}
		if( bSetTexture )
		{
			if( m_Textures[i] != NULL )
				m_pD3DDev->SetTexture( 0, *(m_Textures[i]) );
		}

        // Draw the mesh subset
		m_pMesh->DrawSubset( i );
	}
	return( hr );
}


HRESULT	LoadXFile::Render( bool bSetTexture, bool bSetMaterial )
{
	HRESULT hr = S_OK;
	hr = RenderSections( 0, m_dwNumMaterials, bSetTexture, bSetMaterial );
	return( hr );
}

