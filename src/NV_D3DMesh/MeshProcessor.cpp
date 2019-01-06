/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshProcessor.cpp

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

#define NVD3DMESH_NOLIB
#include "NV_D3DMeshDX9PCH.h"


HRESULT MeshProcessor::ListVertexPositions( Mesh * pMesh, UINT first, UINT last )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;
	last = min( last, pMesh->GetNumVertices() );
	UINT i;
	for( i=first; i < last; i++ )
	{
		FMsg(TEXT("vertex %5u : (%g,  %g,  %g)\n"), i,
				pMesh->m_pVertices[i].pos.x,  
				pMesh->m_pVertices[i].pos.y,  
				pMesh->m_pVertices[i].pos.z );
	}
	return( hr );
}

HRESULT MeshProcessor::ListTriangleIndices( Mesh * pMesh, UINT first, UINT last )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;
	last = min( last, pMesh->GetNumIndices() );
	UINT i;
	for( i=first; i < last; i++ )
	{
		FMsg(TEXT("%5u "), pMesh->m_pIndices[i] );
		if( i%10 == 9 )
			FMsg(TEXT("\n"));
	}
	FMsg(TEXT("\n"));
	return( hr );
}


#ifndef CASE_SETSTRING
#define CASE_SETSTRING( b, f ) case f : strcpy( b, #f ); break;
#endif

HRESULT MeshProcessor::ListMeshStats( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	char buf[48];
	switch( pMesh->m_PrimType )
	{
		CASE_SETSTRING( buf, D3DPT_POINTLIST )
		CASE_SETSTRING( buf, D3DPT_LINELIST )
		CASE_SETSTRING( buf, D3DPT_LINESTRIP )
		CASE_SETSTRING( buf, D3DPT_TRIANGLELIST )
		CASE_SETSTRING( buf, D3DPT_TRIANGLESTRIP )
		CASE_SETSTRING( buf, D3DPT_TRIANGLEFAN )
		default:
			strcpy( buf, "<UNKNOWN>" );
			break;
	}
	buf[30] = '\0';

	FMsg(TEXT("Mesh has %u vertices, %u indices, and %u triangles.  Prim type = %s\n"), 
		pMesh->GetNumVertices(),
		pMesh->GetNumIndices(),
		pMesh->GetNumTriangles(),
		buf );
	return( S_OK );
}


// Any number of the pointers can be NULL.
// Internally, all values are calculated and tracked by the function.
// They are not returned if the pointers are NULL.
HRESULT	MeshProcessor::FindPositionMinMax( const Mesh * pInMesh,
											float * pMinx, float * pMiny, float * pMinz,
											float * pMaxx, float * pMaxy, float * pMaxz )
{
	FAIL_IF_NULL( pInMesh );
	float minx, miny, minz, maxx, maxy, maxz;
	UINT nvert; 
	nvert = pInMesh->GetNumVertices();

	if( nvert > 0 )
	{
		minx = maxx = pInMesh->m_pVertices[0].pos.x;
		miny = maxy = pInMesh->m_pVertices[0].pos.y;
		minz = maxz = pInMesh->m_pVertices[0].pos.z;
	}
	else
	{
		return( E_FAIL );
	}

	UINT i;
	float x,y,z;
	for( i=0; i < nvert; i++ )
	{
		x = pInMesh->m_pVertices[i].pos.x;
		y = pInMesh->m_pVertices[i].pos.y;
		z = pInMesh->m_pVertices[i].pos.z;
		if( x < minx )
			minx = x;
		if( x > maxx )
			maxx = x;
		if( y < miny )
			miny = y;
		if( y > maxy )
			maxy = y;
		if( z < minz )
			minz = z;
		if( z > maxz )
			maxz = z;
	}

	if( pMinx != NULL )
		*pMinx = minx;
	if( pMiny != NULL )
		*pMiny = miny;
	if( pMinz != NULL )
		*pMinz = minz;
	if( pMaxx != NULL )
		*pMaxx = maxx;
	if( pMaxy != NULL )
		*pMaxy = maxy;
	if( pMaxz != NULL )
		*pMaxz = maxz;
	return( S_OK );
}

HRESULT MeshProcessor::FindPositionMinMax( const Mesh * pInMesh, D3DXVECTOR3 * pOutAABBMin, D3DXVECTOR3 * pOutAABBMax )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pInMesh );
	RET_VAL_IF( pOutAABBMin == NULL && pOutAABBMax == NULL, E_FAIL );
	if( pOutAABBMin == NULL )
	{
		hr = FindPositionMinMax( pInMesh, NULL, NULL, NULL,
										  & pOutAABBMax->x, & pOutAABBMax->y, & pOutAABBMax->z );
	}
	else if( pOutAABBMax == NULL )
	{
		hr = FindPositionMinMax( pInMesh, & pOutAABBMin->x, & pOutAABBMin->y, & pOutAABBMin->z,
										  NULL, NULL, NULL );
	}
	else
	{
		hr = FindPositionMinMax( pInMesh, & pOutAABBMin->x, & pOutAABBMin->y, & pOutAABBMin->z,
										  & pOutAABBMax->x, & pOutAABBMax->y, & pOutAABBMax->z );
	}
	return( hr );	
}


HRESULT MeshProcessor::CalculateAABBCenter( const Mesh * pInMesh, D3DXVECTOR3 * pOutputCenter )
{
	FAIL_IF_NULL( pInMesh );
	FAIL_IF_NULL( pOutputCenter );
	HRESULT hr = S_OK;

	D3DXVECTOR3 min, max, center;
	hr = FindPositionMinMax( pInMesh, &min.x, &min.y, &min.z, &max.x, &max.y, &max.z );

	*pOutputCenter = ( min + max ) / 2.0f;

	return( hr );
}


HRESULT MeshProcessor::ApplyFunctionToPositions( Mesh * pOutMesh, Mesh * pInputMesh, PF_3DIN_3DOUT pFunc,
												 UINT uStartVert, UINT uNumVerts )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOutMesh );
	FAIL_IF_NULL( pInputMesh );
	FAIL_IF_NULL( pFunc );

	if( pOutMesh->GetNumVertices() < pInputMesh->GetNumVertices() )
		pOutMesh->AllocateResizeVertices( pInputMesh->GetNumVertices() );

	UINT uStopBefore;
	uStartVert = min( uStartVert, pInputMesh->GetNumVertices() );
	uStopBefore = uStartVert + uNumVerts;
	uStopBefore = min( uStopBefore, pInputMesh->GetNumVertices() );

	UINT i;
	float x, y, z, ix, iy, iz;
	for( i=uStartVert; i < uStopBefore; i++ )
	{
		ix = pInputMesh->m_pVertices[i].pos.x;
		iy = pInputMesh->m_pVertices[i].pos.y;
		iz = pInputMesh->m_pVertices[i].pos.z;
		hr = (*pFunc)( ix, iy, iz, &x, &y, &z );
		assert( SUCCEEDED( hr ) );
		pOutMesh->m_pVertices[i].pos.x = x;
		pOutMesh->m_pVertices[i].pos.y = y;
		pOutMesh->m_pVertices[i].pos.z = z;
	}

	return( hr );
}

//------------------------------------------------------------------
// Apply the function pointed to by pFunc to each vertex of pMesh
// The function can modify each vertex however it likes
//------------------------------------------------------------------
HRESULT MeshProcessor::ApplyFunction( Mesh * pMesh, PF_MESHVERTEX pFunc, UINT uStartVert, UINT uNumVerts )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pFunc );

	UINT uStopBefore;
	uStartVert = min( uStartVert, pMesh->GetNumVertices() );
	uStopBefore = uStartVert + uNumVerts;
	uStopBefore = min( uStopBefore, pMesh->GetNumVertices() );

	UINT i;
	for( i=uStartVert; i < uStopBefore; i++ )
	{
		hr = (*pFunc)( pMesh->GetVertexPtr( i ) );
		assert( SUCCEEDED( hr ) );
	}
	return( hr );
}

//------------------------------------------------------------------
// ProjectVertsToSphere(..)
// Move the vertices of the object so they lie on the surface of a 
//  sphere of the given radius centered at the origin.  Vertices
//  are moved along their vector to the origin (0,0,0).
// 
//------------------------------------------------------------------
HRESULT MeshProcessor::ProjectVertsToSphere( Mesh * pMesh, float radius )
{
	FAIL_IF_NULL( pMesh );

	UINT i;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		D3DXVec3Normalize( &(pMesh->m_pVertices[i].pos), &(pMesh->m_pVertices[i].pos) );
		pMesh->m_pVertices[i].nrm = pMesh->m_pVertices[i].pos;
		pMesh->m_pVertices[i].pos = pMesh->m_pVertices[i].pos * radius;
	}
	return( S_OK );
}


//------------------------------------------------------------------
// Explosion_origin is calculated to be the center of the object's axis-aligned bounding box (AABB)
// These functions do not re-calculate face normals.
//------------------------------------------------------------------
HRESULT MeshProcessor::ExplodeFaces( Mesh * pMesh, float distance )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	D3DXVECTOR3	center;
	hr = CalculateAABBCenter( pMesh, & center );
	hr = ExplodeFaces( pMesh, distance, center );
	return( hr );	
}


HRESULT	MeshProcessor::ExplodeFaces( Mesh * pMesh, float distance, D3DXVECTOR3 explosion_origin )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );

	D3DXVECTOR3 tri_center;
	DWORD i1, i2, i3;
	D3DXVECTOR3 vec_from_center;

	// store displacements in a temporary array
	D3DXVECTOR3	* pDisplacements = new D3DXVECTOR3[ pMesh->GetNumVertices() ];
	FAIL_IF_NULL( pDisplacements );

	memset( pDisplacements, 0, sizeof( D3DXVECTOR3 ) * pMesh->GetNumVertices() );

	UINT i;
	for( i=0; i < pMesh->GetNumTriangles(); i++ )
	{
		pMesh->CalculateTriangleCenter( i, & tri_center );

		vec_from_center = tri_center - explosion_origin;
		D3DXVec3Normalize( &vec_from_center, &vec_from_center );

		// distance is applied after finding direction
		pMesh->GetTriangleIndices( i, &i1, &i2, &i3 );

		UINT v1, v2, v3;
		v1 = pMesh->m_pIndices[i*3];
		v2 = pMesh->m_pIndices[i*3+1];
		v3 = pMesh->m_pIndices[i*3+2];

		pDisplacements[ i1 ] += vec_from_center;
		pDisplacements[ i2 ] += vec_from_center;
		pDisplacements[ i3 ] += vec_from_center;		
	}

	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		D3DXVec3Normalize( &pDisplacements[i], &pDisplacements[i] );
		pDisplacements[i] *= distance;

		pMesh->m_pVertices[i].pos += pDisplacements[i];
	}

	SAFE_ARRAY_DELETE( pDisplacements );
	return( hr );
}	


//------------------------------------------------------------------
// Calculate and assign vertex normals to the mesh by summing face
//  normals.  The vertices are assumed to be specified in counter-clockwise
//  winding order.  The face normal for triangle (a,b,c) is computed from
//  the vertices a, b, and c as:
//    nrm =  (b-a) CROSS (c-a)
//------------------------------------------------------------------
HRESULT	MeshProcessor::CalculateNormalsCCW( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;
	MSG_AND_RET_VAL_IF( pMesh->m_PrimType != D3DPT_TRIANGLELIST, "CalculateNormalsCCW() can only process D3DPT_TRIANGLELIST\n", E_FAIL );

	UINT i;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pMesh->m_pVertices[i].nrm = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	}

	D3DXVECTOR3 d2, d3, nrm;
	UINT v1, v2, v3;
	for( i=0; i < pMesh->GetNumIndices()-2; /* i is inc below */ )
	{
		v1 = pMesh->m_pIndices[i++];
		v2 = pMesh->m_pIndices[i++];
		v3 = pMesh->m_pIndices[i++];
		d2 = pMesh->m_pVertices[v2].pos - pMesh->m_pVertices[v1].pos;
		d3 = pMesh->m_pVertices[v3].pos - pMesh->m_pVertices[v1].pos;
		D3DXVec3Cross( &nrm, &d2, &d3 );
		D3DXVec3Normalize( &nrm, &nrm );
		// Add the face normal to each of the vertice's running total of face normals
		// The sum of all face normals will be normalized after this loop
		pMesh->m_pVertices[v1].nrm += nrm;
		pMesh->m_pVertices[v2].nrm += nrm;
		pMesh->m_pVertices[v3].nrm += nrm;
	}

	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		D3DXVec3Normalize( &pMesh->m_pVertices[i].nrm, &pMesh->m_pVertices[i].nrm );
	}

	return( hr );
}

HRESULT MeshProcessor::CalculateNormalsCW( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;
	hr = CalculateNormalsCCW( pMesh );
	hr = FlipNormals( pMesh );	
	return( hr );
}

// flip all vertex normals
HRESULT MeshProcessor::FlipNormals( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	pMesh->FlipNormals();
	return( S_OK );
}

// flip the winding order for all triangles
HRESULT MeshProcessor::FlipWinding( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	pMesh->FlipWinding();
	return( S_OK );
}


// set each vertex's color to represent the vertex normal
HRESULT MeshProcessor::SetColorFromVertexNormal( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;	
	
	UINT ind;
	D3DXVECTOR3 nrm;
	float hlf = 0.5f;
	int r, g, b;
	for( ind = 0; ind < pMesh->GetNumVertices(); ind++ )
	{
		nrm.x = pMesh->m_pVertices[ind].nrm.x * hlf + hlf;
		nrm.y = pMesh->m_pVertices[ind].nrm.y * hlf + hlf;
		nrm.z = pMesh->m_pVertices[ind].nrm.z * hlf + hlf;
		r = (int) (nrm.x * 255.0f);
		g = (int) (nrm.y * 255.0f);
		b = (int) (nrm.z * 255.0f);
		pMesh->m_pVertices[ind].diffuse = D3DCOLOR_ARGB( 0, r, g, b );
	}
	return( hr );
}

HRESULT MeshProcessor::MakeFaceted( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;

	// Copy the input object since we'll be changing it, and need the old
	//  data as the changes occur
	Mesh	clone;
	MeshGeoCreator gc;
	hr = gc.InitClone( &clone, pMesh );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't create copy of object!\n", hr );

	// allocate as many vertices as there are vertex indices in the original mesh
	hr = pMesh->AllocateVertices( clone.GetNumIndices() );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't allocate new vertices\n", hr );

	// Fill in unique vertices for each triangle from the original mesh 
	UINT i;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pMesh->m_pVertices[i] = clone.m_pVertices[ clone.m_pIndices[i] ];
	}

	// set the indices to reference each new vertex.  
	for( i=0; i < pMesh->GetNumIndices(); i++ )
	{
		pMesh->m_pIndices[i] = i;
	}
	return( hr );
}

//----------------------------------------------------------------------------------------
// input_pos_translation  is applied to pInputMesh's vertex positions before using them to
//   generate the noise values.  If you animate this parameter, the noise will translate
//   through the mesh object.
// This function does not re-compute vertex normals after displacing the vertex positions.
//----------------------------------------------------------------------------------------
HRESULT	MeshProcessor::AddPositionNoise1D( Mesh * pOutMesh, Mesh * pInputMesh, 
											D3DXVECTOR3 noise_dir,
											D3DXVECTOR3 input_pos_translation,
											GridNoiseComponent * pNoiseComponents,
											UINT uNumNoiseComponents )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOutMesh );
	FAIL_IF_NULL( pInputMesh );
	FAIL_IF_NULL( pNoiseComponents );

	// If in and out meshes don't match, make them match
	if( pOutMesh->GetNumVertices() < pInputMesh->GetNumVertices() )
	{
		MeshGeoCreator gc;
		gc.InitClone( pOutMesh, pInputMesh );
	}

	NoiseGrid3D noise_grid;	
	float fNoiseVal;
	D3DXVECTOR3 pos, noise_vec;

	UINT i;
	vector< GridNoiseComponent > vNoiseComponents;
	for( i=0; i < uNumNoiseComponents; i++ )
	{
		vNoiseComponents.push_back( pNoiseComponents[i] );
	}

	for( i=0; i < pInputMesh->GetNumVertices(); i++ )
	{
		pos = pInputMesh->GetVertexPosition(i) + input_pos_translation;
		noise_grid.NoiseScalarSigned( &fNoiseVal, pos.x, pos.y, pos.z, & vNoiseComponents );

		noise_vec = noise_dir * fNoiseVal;
		pOutMesh->m_pVertices[i].pos = pInputMesh->GetVertexPosition(i) + noise_vec;
	}
	return( hr );
}


// This function does not re-compute vertex normals after displacing the vertex positions.
HRESULT	MeshProcessor::AddPositionNoise1D( Mesh * pOutMesh, Mesh * pInputMesh, 
											D3DXVECTOR3 noise_dir,
											GridNoiseComponent * pNoiseComponents,
											UINT uNumNoiseComponents )
{
	HRESULT hr = S_OK;
	hr = AddPositionNoise1D( pOutMesh, pInputMesh, noise_dir,
								D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
								pNoiseComponents,
								uNumNoiseComponents );
	return( hr );
}


HRESULT MeshProcessor::AddPositionNoise3D( Mesh * pOutMesh, Mesh * pInputMesh,
											D3DXVECTOR3 input_pos_translation,
											GridNoiseComponent * pNoiseComponents,
											UINT uNumNoiseComponents )
{
	HRESULT hr = S_OK;



	return( hr );
}	


HRESULT MeshProcessor::RemoveUnusedVertices( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;

	MeshBeingProcessed mbp;
	mbp.InitFromMesh( pMesh );

	UINT nvert;
	nvert = pMesh->GetNumVertices();
	MSG_AND_RET_VAL_IF( nvert==0, "Mesh has zero vertices!\n", E_FAIL );

	bool * pVertsUsed = new bool[ nvert ];
	FAIL_IF_NULL( pVertsUsed );

	UINT i;
	for( i=0; i < nvert; i++ )
	{
		pVertsUsed[i] = false;
	}

	UINT vi;
	for( i=0; i < pMesh->GetNumIndices(); i++ )
	{
		vi = pMesh->m_pIndices[i];
		vi = min( vi, nvert-1 );
		pVertsUsed[ vi ] = true;
	}

	UINT src, dest;
	dest = 0;
	// scan forward through the vertices once
	for( src = 0; src < nvert; src++ )
	{
		// If a vertex is used, see if there are any unused vertices
		// behind it at dest.  If there are, swap the used and unused vertices
		// If the vertex is not used, dest is not incremented, so dest always
		//  tracks the lowest position unused vertex.
		if( pVertsUsed[src] == true )
		{
			if( src != dest )
			{
				mbp.SwapVertices( src, dest );
				swap( pVertsUsed[src], pVertsUsed[dest] );
			}
			dest++;
		}
	}

	mbp.GetProcessedMesh( pMesh );
	pMesh->AllocateResizeVertices( dest );		// reduce the size of the vertex array 
	SAFE_ARRAY_DELETE( pVertsUsed );
	return( hr );
}

// Compare all vertices in pMesh.  If two or more vertices lie within the thresholds 
//  supplied, remove all but one vertex and remap the triangles to use the one remaining
//  vertex.
// This function does not yet use any binning strategy to find similar vertices.  It
//  does a brute force (N^2) comparison of every vertex against every other.
HRESULT MeshProcessor::RemoveDegenerateVerts( Mesh * pMesh, float fPositionThresh,
								float fNormalDP3Thresh, float fTexcoordThresh,
								bool bLeaveUnusedVertsInPlace )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	MeshBeingProcessed mbp;
	mbp.InitFromMesh( pMesh );
	UINT i, j;
	MeshVertex * pv1;
	bool bWithinTollerances;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pv1 = pMesh->GetVertexPtr(i);

		for( j=i+1; j < pMesh->GetNumVertices(); j++ )
		{
			bWithinTollerances = pv1->VertexWithinTolerances( pMesh->GetVertexPtr(j), fPositionThresh,
																fNormalDP3Thresh, fTexcoordThresh );
			if( bWithinTollerances )
			{
				// eliminate the j vertex, setting all tris that used to use the j vertex to 
				//  use the i vertex instead.
				mbp.MergeVertices( i, j );
			}
		}
	}
	mbp.GetProcessedMesh( pMesh );
	if( !bLeaveUnusedVertsInPlace )
	{
		// Do extra processing to remove the vertices that are no longer used from the 
		//  m_pVertices array
		RemoveUnusedVertices( pMesh );
	}
	return( hr );
}

// Delete the vertices and any triangles that use the vertices.
// This function is a slow implementation.  A better implementation could be written if you have 
//   several vertices to eliminate.
HRESULT MeshProcessor::RemoveVertices( Mesh * pMesh, UINT * pVerticesToRemove, UINT uNumVerticesInArray )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pVerticesToRemove );

	UINT n, i, repli;
	for( i=0; i < pMesh->GetNumIndices(); i++ )
	{
		for( n=0; n < uNumVerticesInArray; n++ )
		{
			if( pMesh->m_pIndices[i] == pVerticesToRemove[n] )
			{
				// Replace the index with another index from the same triangle.
				// This makes the triangle go degenerate and the vertex go unused.
				// They can be scanned for and eliminated later.
				if( i%3 > 0 )
					repli = i-1;
				else
					repli = i+1;
				pMesh->m_pIndices[i] = pMesh->m_pIndices[ repli ];
			}
		}
	}

	hr = RemoveUnusedVertices( pMesh );
	hr = RemoveDegenerateTriangles( pMesh );
	return( hr );
}

float MeshProcessor::CalculateTriangleArea( Mesh * pMesh, UINT vert_index_1, UINT vert_index_2, UINT vert_index_3 )
{
	RET_VAL_IF( pMesh == NULL, 0.0f );
	D3DXVECTOR3 v2, v3;
	v2 = pMesh->GetVertexPosition( vert_index_2 ) - pMesh->GetVertexPosition( vert_index_1 );
	v3 = pMesh->GetVertexPosition( vert_index_3 ) - pMesh->GetVertexPosition( vert_index_1 );
	D3DXVec3Cross( &v3, &v2, &v3 );
	float area;
	area = (float) sqrt( v3.x*v3.x + v3.y*v3.y + v3.z*v3.z );
	return( area );
}


HRESULT MeshProcessor::RemoveZeroAreaTriangles( Mesh * pMesh, float area_threshold )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );

	DWORD i, i1, i2, i3;
	vector< UINT >	vNewTriIndices;
	vNewTriIndices.reserve( pMesh->GetNumIndices() );
	float tri_area;
	for( i=0; i < pMesh->GetNumTriangles(); i++ )
	{
		pMesh->GetTriangleIndices( i, &i1, &i2, &i3 );

		// check for degenerate triangle
		if( i1 == i2 || i2 == i3 || i3 == i1 )
			continue;		// next loop iteration, do not add the triangle to the new list

		tri_area = CalculateTriangleArea( pMesh, i1, i2, i3 );
		if( tri_area <= area_threshold )
			continue;		// do not add the triangle

		// add the triangle to the new index list
		vNewTriIndices.push_back( i1 );
		vNewTriIndices.push_back( i2 );
		vNewTriIndices.push_back( i3 );
	}
	// replaces pMesh's triangles with the new list
	hr = pMesh->AllocateResizeIndices( (UINT) vNewTriIndices.size() );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't resize tri indices!\n", hr );
	for( i=0; i < pMesh->GetNumIndices(); i++ )
	{
		pMesh->m_pIndices[i] = vNewTriIndices.at(i);
	}

	return( hr );
}


HRESULT MeshProcessor::RemoveDegenerateTriangles( Mesh * pMesh )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );

	DWORD i, i1, i2, i3;
	vector< UINT >	vNewTriIndices;
	vNewTriIndices.reserve( pMesh->GetNumIndices() );
	for( i=0; i < pMesh->GetNumTriangles(); i++ )
	{
		pMesh->GetTriangleIndices( i, &i1, &i2, &i3 );

		// check for degenerate triangle
		if( i1 == i2 || i2 == i3 || i3 == i1 )
			continue;		// next loop iteration, do not add the triangle to the new list

		// add the triangle to the new index list
		vNewTriIndices.push_back( i1 );
		vNewTriIndices.push_back( i2 );
		vNewTriIndices.push_back( i3 );
	}
	// replaces pMesh's triangles with the new list
	hr = pMesh->AllocateResizeIndices( (UINT) vNewTriIndices.size() );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't resize tri indices!\n", hr );
	for( i=0; i < pMesh->GetNumIndices(); i++ )
	{
		pMesh->m_pIndices[i] = vNewTriIndices.at(i);
	}
	return( hr );
}



