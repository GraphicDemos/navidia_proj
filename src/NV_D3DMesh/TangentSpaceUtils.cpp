/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  TangentSpaceUtils.cpp

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


#include "MeshVertexDot3.h"
#include "TangentSpaceUtils.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

#define SMALL_FLOAT 1e-12
#define WORD_MAX	65535

// Creates tangent space basis vectors for the input pVertices based on the triangles in pIndices
// Position and texture coords are read from pVertices, and results are written to the tangent space
//  basis vectors of pVertices.
// This function does check for vertex position repetitions - ie vertices with the 
//  same position but different texture coordinates or normals.  The function
//  FixTangentBasisWithPositionRepetitions(..) will find the similar positions and
//  try to smooth the tangent space basis at those vertices.
HRESULT CreateBasisVectors( MeshVertexDot3 * pVertices, size_t uNumVertices, DWORD * pIndices, size_t uNumIndices )
{
	BREAK_AND_RET_VAL_IF( pVertices == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( pIndices == NULL, E_FAIL );
	HRESULT hr = S_OK;
	DWORD i;
	// Clear the basis vectors
	for( i=0; i < uNumVertices; i++)
	{
		pVertices[i].S = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		pVertices[i].T = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		// no need to clear SxT since that is derived from S and T
	}

	// Walk through the triangle list and calculate gradiants for each triangle.
	// Sum the results into the S and T components.
	DWORD TriIndex[3];
	D3DXVECTOR3 du, dv;
	D3DXVECTOR3 edge01;
	D3DXVECTOR3 edge02;
	D3DXVECTOR3 cp;
    for( i = 0; i < uNumIndices; i += 3 )
    {       		
		TriIndex[0] = pIndices[i];
		TriIndex[1] = pIndices[i+1];
		TriIndex[2] = pIndices[i+2];
		if( TriIndex[0] >= uNumVertices ) return( E_FAIL );
		if( TriIndex[1] >= uNumVertices ) return( E_FAIL );
		if( TriIndex[2] >= uNumVertices ) return( E_FAIL );
		MeshVertexDot3& v0 = pVertices[TriIndex[0]];
		MeshVertexDot3& v1 = pVertices[TriIndex[1]];
		MeshVertexDot3& v2 = pVertices[TriIndex[2]];
		// x component for S and T
		edge01 = D3DXVECTOR3( v1.Position.x - v0.Position.x, v1.Texture.x - v0.Texture.x, v1.Texture.y - v0.Texture.y );
		edge02 = D3DXVECTOR3( v2.Position.x - v0.Position.x, v2.Texture.x - v0.Texture.x, v2.Texture.y - v0.Texture.y );
		D3DXVec3Cross(&cp, &edge01, &edge02);
		if ( fabs(cp.x) > SMALL_FLOAT )
		{
			v0.S.x += -cp.y / cp.x;
			v0.T.x += -cp.z / cp.x;
			v1.S.x += -cp.y / cp.x;
			v1.T.x += -cp.z / cp.x;
			v2.S.x += -cp.y / cp.x;
			v2.T.x += -cp.z / cp.x;
		}

		// y component for S and T
		edge01 = D3DXVECTOR3( v1.Position.y - v0.Position.y, v1.Texture.x - v0.Texture.x, v1.Texture.y - v0.Texture.y );
		edge02 = D3DXVECTOR3( v2.Position.y - v0.Position.y, v2.Texture.x - v0.Texture.x, v2.Texture.y - v0.Texture.y );
		D3DXVec3Cross(&cp, &edge01, &edge02);
		if ( fabs(cp.x) > SMALL_FLOAT )
		{
			v0.S.y += -cp.y / cp.x;
			v0.T.y += -cp.z / cp.x;
			v1.S.y += -cp.y / cp.x;
			v1.T.y += -cp.z / cp.x;			
			v2.S.y += -cp.y / cp.x;
			v2.T.y += -cp.z / cp.x;
		}

		// z component for S and T
		edge01 = D3DXVECTOR3( v1.Position.z - v0.Position.z, v1.Texture.x - v0.Texture.x, v1.Texture.y - v0.Texture.y );
		edge02 = D3DXVECTOR3( v2.Position.z - v0.Position.z, v2.Texture.x - v0.Texture.x, v2.Texture.y - v0.Texture.y );
		D3DXVec3Cross(&cp, &edge01, &edge02);
		if ( fabs(cp.x) > SMALL_FLOAT )
		{
			v0.S.z += -cp.y / cp.x;
			v0.T.z += -cp.z / cp.x;
			v1.S.z += -cp.y / cp.x;
			v1.T.z += -cp.z / cp.x;
			v2.S.z += -cp.y / cp.x;
			v2.T.z += -cp.z / cp.x;
		}
    }
	// Before normalizing the S, T vectors we have to 
	//  find degenerate vertices to average the basis
	//  for points which share position but not UV
	//  coordinate (ie lathed objects with a seam where
	//  vertices are duplicated).
	// If we were to search for these degenerate points after 
	//  normalizing the S & T, the averaging of these normalized
	//  (pre-averaged) vectors would not be correct over the
	//  seam, and a discontinuity might appear.		
//@	FindAndFixDegenerateVertexBasis( pVertices, dwNumVertices, pIndices, dwNumIndices, false );
	hr = FixTangentBasisWithPositionRepetitions( pVertices, uNumVertices, pIndices, uNumIndices, false );

    // Calculate the SxT vector
  	for(i = 0; i < uNumVertices; i++)
  	{		
  		// Normalize the S, T vectors
  		D3DXVec3Normalize( &pVertices[i].S, &pVertices[i].S );
  		D3DXVec3Normalize( &pVertices[i].T, &pVertices[i].T );
  		// Get the cross of the S and T vectors
  		D3DXVec3Cross( &pVertices[i].SxT, &pVertices[i].S, &pVertices[i].T );
  		// Need a normalized normal
  		D3DXVec3Normalize( &pVertices[i].Normal, &pVertices[i].Normal );
 		// Get the direction of the SxT vector
  		if (D3DXVec3Dot( &pVertices[i].SxT, &pVertices[i].Normal) < 0.0f )
  		{
  			pVertices[i].SxT = -pVertices[i].SxT;
  		}
  	}
	return S_OK;
}

//----------------------------------------------------------------------------
// FindAndFixDegenerateVertexBasis(..)
//  Takes a vertex buffer with vertices of type Dot3Vertex.  Indices must be
//  for an INDEXED TRIANGLE LIST.
// This finds vertices within epsilon in position and averages their tangent
//  bases to make a smooth tangent space across the model.  This is useful for
//  lathed objects or authored models which duplicate vertices along material
//  boundaries.
// Tangent Basis must have already been computed for this to work! =)
//
// If bUnifyNormals is true, then the vertex normals are averaged too
//----------------------------------------------------------------------------
HRESULT FixTangentBasisWithPositionRepetitions(	MeshVertexDot3 *	pVertices,
					size_t uNumVertices, DWORD * pIndices, size_t uNumIndices,
					bool bUnifyNormals )
{
	BREAK_AND_RET_VAL_IF( pVertices == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( pIndices == NULL, E_FAIL );
	float epsilon = 1.0e-5f;
	float x,y,z,dist;
	size_t i,j;

	// Sloppy, but allocate a pointer and char for each vertex
	// As overlapping vertices are found, increment their duplicate_count
	//   and allocate an array of MAX_OVERLAP vertex indices to reference
	//   which vertices overlap.
	#define MAX_OVERLAP 50

	BYTE * duplicate_count = new BYTE[ uNumVertices ];
		// duplicate_index is array of pointers to bins.  Each bin is
		// allocated when a match is found.
	size_t ** duplicate_index = (size_t**) calloc( uNumVertices, sizeof(size_t*) );

	memset( duplicate_count, 0, uNumVertices * sizeof( BYTE  ) );
	memset( duplicate_index, 0, uNumVertices * sizeof( size_t* ) );

	// Need to search the mesh for vertices with the same spatial coordinate
	// These are vertices duplicated for lathed/wrapped objects to make a
	//   2nd set of texture coordinates at the point in order to avoid bad
	//   texture wrapping
	// In short:  For each vertex, find any other vertices that share it's 
	//   position.  "Average" the tangent space basis calculated above for
	//   these duplicate vertices.  This is not rigorous, but works well 
	//   to fix up authored models.  ** Models should not have T juntions! **

	// Check each vert with every other.  There's no reason to check
	//   j with i after doing i with j, so start j from i ( so we test
	//   1 with 2 but never 2 with 1 again).
	// This is a model pre-processing step and only done once.  For large
	//   models, compute this off-line if you have to and store the resultant
	//   data.
	// The whole thing could be made much more efficient (linked list, etc)

	for( i=0; i < uNumVertices; i++ )
	{
		for(j=i+1; j < uNumVertices; j++ )
		{
			x = pVertices[i].Position.x - pVertices[j].Position.x;
			y = pVertices[i].Position.y - pVertices[j].Position.y;
			z = pVertices[i].Position.z - pVertices[j].Position.z;
			dist = x*x + y*y + z*z;
			
			if( dist < epsilon )
			{
				// if i matches j and k, just record into i.  j will be 
				//  half full as it will only match k, but this is
				//  taken care of when i is processed.
				if( duplicate_count[i] == 0 )
				{
					// allocate bin
					duplicate_index[i] = new size_t[MAX_OVERLAP];
				}
				if( duplicate_count[i] < MAX_OVERLAP )
				{
					duplicate_index[i][duplicate_count[i]] = j;
					duplicate_count[i] ++;
				}
				else
				{
					FDebug("Ran out of bin storage!!\n");
					assert( false );
				}
			}
		}
		/*
		if( duplicate_count[i] > 0 )
		{
			FDebug("Vertex %d\t matches: ", i );
			for(j=0; j < duplicate_count[i]; j++ )
			{
				FDebug("%d\t", duplicate_index[i][j] );
			}
			FDebug("\n");
		}
		*/
	}

	// Now average the tangent spaces & write the new result to
	//  each duplicated vertex
	D3DXVECTOR3	S_temp, T_temp, SxT_temp, N_temp;

	for( i = 0; i < uNumVertices; i++ )
	{
		// do < 10 check to not average the basis at poles of sphere or
		//  other ridiculous geometry with too many degenerate vertices
		if( duplicate_count[i] > 0 && duplicate_count[i] < 10 )
		{
			//	FDebug("Averaging vert prop at %d for %d vertices\n", i, duplicate_count[i]);
			// If there are more than 5 vertices sharing this point then
			//  the point is probably some kind of lathe axis node.  No need to
			//  process it here
			// Set accumulator to value for vertex in question

			S_temp		= pVertices[i].S;
			T_temp		= pVertices[i].T;
			SxT_temp	= pVertices[i].SxT;
			N_temp		= pVertices[i].Normal;

			// add in basis vectors for all other vertices which
			//  have the same positon (found above)
			for( j=0; j < duplicate_count[i]; j++ )
			{
				S_temp		= S_temp   + pVertices[duplicate_index[i][j]].S;
				T_temp		= T_temp   + pVertices[duplicate_index[i][j]].T;
				SxT_temp	= SxT_temp + pVertices[duplicate_index[i][j]].SxT;
				N_temp		= N_temp   + pVertices[duplicate_index[i][j]].Normal;
			}

			// Normalize the basis vectors
			// Note that SxT might not be perpendicular to S and T
			//  anymore.  Not absolutely necessary to re-do the 
			//  cross product.
			D3DXVec3Normalize( & S_temp, & S_temp );
			D3DXVec3Normalize( & T_temp, & T_temp );
			D3DXVec3Normalize( & SxT_temp, & SxT_temp );
			D3DXVec3Normalize( & N_temp, & N_temp );

			// Write the average basis to the first vertex for which
			//   the duplicates were found

			pVertices[i].S = S_temp;
			pVertices[i].T = T_temp;
			pVertices[i].SxT = SxT_temp;
			if( bUnifyNormals )
				pVertices[i].Normal = N_temp;

			// Write to all later vertices with the same position
			for(j=0; j < duplicate_count[i]; j++ )
			{
				// Set the vertices in the same position to
				//  the average basis.
				pVertices[duplicate_index[i][j]].S = S_temp;
				pVertices[duplicate_index[i][j]].T = T_temp;
				pVertices[duplicate_index[i][j]].SxT = SxT_temp;

				if( bUnifyNormals )
					pVertices[duplicate_index[i][j]].Normal = N_temp;

				// Kill the duplicate index lists of all vertices of
				//  higher index which overlap this one.  This is so
				//  higher index vertices do not average a smaller 
				//  subset of bases.
				// Arrays are de-allocated later
				duplicate_count[ duplicate_index[i][j] ] = 0;
			}
		}
		if( duplicate_index[i] != NULL )
		{
			delete [] duplicate_index[i];
			duplicate_index[i] = NULL;
			duplicate_count[i] = 0;
		}
	}
	delete [] duplicate_count;
	free( duplicate_index );
	return( S_OK );
}
