/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  LoadPLYFile.cpp

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

#include <stdio.h>
#include "NV_D3DMeshDX9PCH.h"


LoadPLYFile::LoadPLYFile()
{
	SetAllNull();
}

LoadPLYFile::~LoadPLYFile()
{
	Free();
}

HRESULT LoadPLYFile::Free()
{
	HRESULT hr = S_OK;
	SAFE_DELETE( m_pMeshVB );
	SAFE_RELEASE( m_pD3DDev );
	SetAllNull();
	return( hr );
}


HRESULT LoadPLYFile::Initialize( IDirect3DDevice9 * pD3DDev,
									GetFilePath::GetFilePathFunction pGetFilePathFunction )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pD3DDev );
	m_pD3DDev = pD3DDev;
	m_pD3DDev->AddRef();

	m_GetFilePathFunction = pGetFilePathFunction;

	m_AABBMin	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_AABBMax	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_bAABBValid = false;
	D3DXMatrixIdentity( &m_matWorld );

	return( hr );
}

tstring LoadPLYFile::GetFilePath( const TCHAR * in_pFilePath, bool bVerbose )
{
	tstring filepath = TEXT("");
	if( in_pFilePath == NULL )
	{
		if( bVerbose )
			FMsg("LoadXFile::GetFilePath() input path is NULL!\n");
		return( TEXT("") );
	}

	if( m_GetFilePathFunction != NULL )
	{
		filepath = (*m_GetFilePathFunction)( in_pFilePath, bVerbose );
	}
	else
	{
		if( bVerbose == true )
			FMsg("LoadXFile::GetFilePath function is NULL, returning the input string!\n");
		filepath = in_pFilePath;
	}
	return( filepath );
}


//---------------------------------------------------------------------------
// Set the m_matWorld matrix so that it will scale the m_pMeshVB to the 
//   bounding box defined by aabb_center and aabb_size.  Each dimension of
//   aabb_size describes the length along each side of the bounding box into
//   which the mesh will fit.
//---------------------------------------------------------------------------
HRESULT LoadPLYFile::SetMatrixToXFormMesh( const D3DXVECTOR3 & aabb_center, 
										 const D3DXVECTOR3 & aabb_size,
										 const LoadPLYFileFlags & flags )
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
HRESULT LoadPLYFile::GetMatrixToXFormMesh( D3DXMATRIX * out_pMat,
											const D3DXVECTOR3 & aabb_center, 
											const D3DXVECTOR3 & aabb_size,
											const LoadPLYFileFlags & flags )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( out_pMat );
	MSG_BREAK_AND_RET_VAL_IF( m_bAABBValid == false, "Can't scale mesh unless the AABB data is valid!\n", E_FAIL );

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

	// Matrix to scale mesh to final size
	D3DXMatrixScaling( &sc, scale.x, scale.y, scale.z );
	// Matrix to translate the mesh center to the desired point, after scaling
	D3DXVECTOR3 translate = D3DXVECTOR3( -scale.x*mesh_center.x, -scale.y*mesh_center.y, -scale.z*mesh_center.z );
	translate += aabb_center;
	D3DXMatrixTranslation( &tf, translate.x, translate.y, translate.z );
	// First matrix to effect the vertex is on the left of the multiply
	D3DXMatrixMultiply( &mat, &sc, &tf );

	m_matWorld = mat;
	*out_pMat = mat;
	return( hr );
}


//--------------------------------------------------------------------------------
// Load a .ply file into the m_pMeshVB class.
//--------------------------------------------------------------------------------
HRESULT LoadPLYFile::LoadFile( const TCHAR * in_pFilePath, 
							   bool bVerbose,
							   MeshVB::VBUsage dynamic_or_static )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( in_pFilePath );
	FAIL_IF_NULL( m_pD3DDev );

	if( bVerbose )
		FMsg("LoadPLYFile::LoadFile( %s, %s )\n", in_pFilePath, bVerbose ? "TRUE" : "FALSE" );

	tstring file_path;
	file_path = GetFilePath( in_pFilePath );

	FILE * fp = NULL;
#ifdef UNICODE
	string str = NVStringConv::WStringToString( file_path );
	fp = fopen( str.c_str(), "rt" );
#else
	fp = fopen( file_path.c_str(), "rt" );
#endif
	if( fp == NULL )
	{
		if( bVerbose )
			FMsg("Couldn't open file [%s]\n", file_path.c_str() );
		return( E_FAIL );
	}

	bool readingheader = true;
	char buf[4096];
	UINT num_verts;
	UINT num_inds;

	while( readingheader )
	{
		fscanf( fp, "%s", buf );
		if( strcmp( buf, "element" ) == 0 )
		{
			fscanf( fp, "%s", buf );
			if( strcmp( buf, "vertex" ) == 0 )
			{
				fscanf( fp, "%u", & num_verts );
				if( bVerbose )
					FMsg("num verts: %u\n", num_verts );
			}
			if( strcmp( buf, "face" ) == 0 )
			{
				fscanf( fp, "%u", & num_inds );
				num_inds = num_inds * 3;
				if( bVerbose )
					FMsg("num faces: %u\n", num_inds / 3 );
			}
		}
		if( strcmp( buf, "end_header" ) == 0 )
		{
			readingheader = false;
			if( bVerbose )
				FMsg("Done reading header\n");
		}
	}
	//--------------
	Mesh	mesh;
	mesh.Allocate( num_verts, num_inds );
	mesh.m_PrimType = D3DPT_TRIANGLELIST;

	D3DXVECTOR3 pos;
	MeshVertex * pVert;
	UINT i;
	if( bVerbose )
		FMsg("Reading vertices: ");
	for( i=0; i < num_verts; i++ )
	{
		fscanf( fp, "%f", & ( pos.x ) );
		fscanf( fp, "%f", & ( pos.y ) );
		fscanf( fp, "%f", & ( pos.z ) );
		pVert = mesh.GetVertexPtr( i );
		if( pVert != NULL )
		{
			pVert->SetPosition( pos );
		}
		else
		{
			FMsg("error : pVert == NULL!\n" );
			assert( false );
		}

		if( bVerbose )
		{	// display 10 vertex positions as the mesh is loaded
			if( i % (num_verts/10) == 0 )
			{
				FMsg("%u  ", i );
			}
		}
	}
	if( bVerbose )
	{
		FMsg("\n");
		FMsg("Reading indices: ");
	}

	int numind;
	UINT uitmp;

	for( i=0; i < num_inds / 3; i++ )
	{
		fscanf( fp, "%d", & numind );
		
		if( numind != 3 )
		{
			FMsg("Bad num indices in a triangle!!\n");
			fscanf( fp, "%u", & uitmp );
			FMsg("%u ", uitmp );
			fscanf( fp, "%u", & uitmp );
			FMsg("%u ", uitmp );
			fscanf( fp, "%u", & uitmp );
			FMsg("%u ", uitmp );
			fscanf( fp, "%u", & uitmp );
			FMsg("%u ", uitmp );
			FMsg("\n");
			assert( false );
			return( E_FAIL );
		}

		fscanf( fp, "%u", & (mesh.m_pIndices[i*3]) );
		fscanf( fp, "%u", & (mesh.m_pIndices[i*3+1]) );
		fscanf( fp, "%u", & (mesh.m_pIndices[i*3+2]) );

		if( bVerbose )
		{
			if( i % (num_inds/10) == 0 )
			{
				FMsg("%u  ", i );
			}
		}

	}
	if( bVerbose )
		FMsg("\n");			// linefeed after last reported index read

	fclose( fp );
	fp = NULL;

	// Create vertex & index buffer object from the system memory Mesh
	SAFE_DELETE( m_pMeshVB );
	m_pMeshVB = new MeshVB;
	MSG_BREAK_AND_RET_VAL_IF( m_pMeshVB == NULL, "Couldn't create MeshVB!\n", E_FAIL );

	hr = m_pMeshVB->CreateFromMesh( &mesh, m_pD3DDev, dynamic_or_static );

	MeshProcessor mp;
	hr = mp.FindPositionMinMax( &mesh, 
								& m_AABBMin.x, & m_AABBMin.y, & m_AABBMin.z,
								& m_AABBMax.x, & m_AABBMax.y, & m_AABBMax.z );
	m_bAABBValid = true;

	if( bVerbose )
	{
		ListVector( TEXT("AABB min: "), m_AABBMin, TEXT("\n") );
		ListVector( TEXT("AABB max: "), m_AABBMax, TEXT("\n") );
		FMsg(TEXT("\n"));
	}

	return( hr );
}
