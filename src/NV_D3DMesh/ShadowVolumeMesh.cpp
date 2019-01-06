/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  ShadowVolumeMesh.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the header file for comments.

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DMeshDX9PCH.h"
#include <algorithm>
#include <assert.h>
using namespace std;

typedef std::vector< int >	V_INT;
typedef std::vector< UINT >	V_UINT;

//---------------------------------------

ShadowVolumeMesh::ShadowVolumeMesh()
{
	::WingedEdgeMesh();

	m_wOriginalModelTriIndices	= 0;
	m_wLastTriIndex				= 0;
}


ShadowVolumeMesh::~ShadowVolumeMesh()
{
	Free();
}

////////////////////////////////////////////////////////////

HRESULT ShadowVolumeMesh::Free()
{
	return( S_OK );
}


void	ShadowVolumeMesh::GetTriCounts( UINT * pNumTotalTris, UINT * pNumWithoutShadowVolTris )
{
	if( pNumTotalTris != NULL )
	{
		*pNumTotalTris = m_wLastTriIndex;
	}
	if( pNumWithoutShadowVolTris != NULL )
	{
		*pNumWithoutShadowVolTris = m_wOriginalModelTriIndices;
	}
}

// Move each triangles out along a vector from the center of the object 
void ShadowVolumeMesh::ExplodeFaces( float distance )
{
	MeshProcessor mp;
	mp.ExplodeFaces( this, distance );
}

HRESULT		ShadowVolumeMesh::BuildShadowVolume( float weld_edge_threshold )
{
	assert( m_bIsValid );
	assert( m_PrimType == D3DPT_TRIANGLELIST );
	assert( sizeof(UINT) == 4 );

	assert( m_bOpenHull == false );

	HRESULT hr = S_OK;

	const float limit = 0.9f;

	if( weld_edge_threshold > limit )
	{
		FDebug("\n");
		FDebug("########################################################\n");
		FDebug("ShadowVolume BuildShadowVolume:\n");
		FDebug("weld_edge_threshold > %f\n", limit );
		FDebug("This will result in MANY additional triangles in areas where\n");
		FDebug("   they are not needed.  Try using a threshold closer to 0.6\n");
		FDebug("########################################################\n");
		FDebug("\n");
	}	


	FDebug("Old vert count:  %d\n", GetNumVertices() );


	hr = BuildWingedEdgeData();
	BREAK_IF( FAILED(hr) );

	MeshProcessor mp;
	mp.MakeFaceted( this );			// create new vertices.  no new triangles

	// Eliminate some vertices to join edges 
	//   where the face normals are similar.
	// Thresh is in the range [-1,1]
	// A value of 1.0 will weld only faces
	//  with the same normal.
	WeldFacetedFaces( weld_edge_threshold );

	// For each non-welded edge (each edge with different vertices
	//  at the same spatial position), create zero-area triangles
	//  to close the edge.
	
//	assert( IsWingedEdgeDataBuilt() );

	// rebuild the winged-edge data now that some faces
	// have been welded

	BuildWingedEdgeData();

	unsigned int		tri;
	unsigned int		n, i;

	UINT	last_edge_vertex;
	UINT	neighbor_edge;

	V_UINT	new_tris;
	new_tris.clear();

	V_UINT	vertices;

	V_UINT	reduced;
	V_UINT :: iterator r_end;

	// 1 array of WORDS for each vertex.  This is to form
	//  caps for areas where 3 or more vertices share the 
	//  same position.

	V_INT * stitch_pairs = new V_INT[ GetNumVertices() ];
	assert( stitch_pairs != NULL );


	V_UINT					neighbor_tris;
	std::vector< UINT * >	is_welded;		// corresponds to entries for neighbor_tris
	V_UINT					edge;

	WELD_VALUE				nweld;

	int nNumOpenHullFailures;
	nNumOpenHullFailures = 0;

	for( tri = 0; tri < GetNumIndices() / 3; tri++ )
	{
		neighbor_tris.clear();
		is_welded.clear();
		edge.clear();

		if( m_pTriNeighbors[tri].ind1 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind1 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded1 ));
			edge.push_back( 0 );
		}
		else
		{
			MSG_AND_RET_VAL_IF( FAILED(hr), "ind1 failed\n", hr );
		}

		if( m_pTriNeighbors[tri].ind2 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind2 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded2 ));
			edge.push_back( 1 );
		}
		else
		{
			MSG_AND_RET_VAL_IF( FAILED(hr), "ind2 failed\n", hr );
		}

		if( m_pTriNeighbors[tri].ind3 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind3 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded3 ));
			edge.push_back( 2 );
		}
		else
		{
			MSG_AND_RET_VAL_IF( FAILED(hr), "ind3 failed\n", hr );
		}


		if( neighbor_tris.size() != 3 )
		{
			m_bOpenHull = true;
			nNumOpenHullFailures++;
			if( nNumOpenHullFailures < 4 )
			{
				FDebug("**** Warning - object is open hull\n");
			}
		}

		for( n=0; n < neighbor_tris.size(); n++ )
		{

			if( *( is_welded[n] ) == NOT_WELDED )
			{
				// add vertices we need to stitch
				// if n = 2 then use vertices n, 0
				last_edge_vertex = ( edge[n] + 1 ) % 3;

				vertices.clear();
				reduced.clear();

				vertices.push_back( m_pIndices[ tri*3 + edge[n] ] );
				vertices.push_back( m_pIndices[ tri*3 + last_edge_vertex ] );

				// Which edge of the neighbor is the base trianlge, tri,
				//	 attached to?
				neighbor_edge = 4;

				if		( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind1 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded1;

					neighbor_edge = 0;
				}
				else if	( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind2 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded2;

					neighbor_edge = 1;
				}
				else if	( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind3 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded3;

					neighbor_edge = 2;
				}

				if( neighbor_edge < 0 || neighbor_edge > 2 )
				{
					FDebug("Error - can't find base tri location relative to neighbor!!\n");
					assert(false);
				}

				last_edge_vertex = ( neighbor_edge + 1 ) % 3;
				vertices.push_back( m_pIndices[ neighbor_tris[n]*3 + neighbor_edge ] );
				vertices.push_back( m_pIndices[ neighbor_tris[n]*3 + last_edge_vertex ] );

				// Some vertices pushed back above might refer to the 
				//  same vertex.  Need to eliminate duplicate entries, so
				//  make a vector, sort & eliminate duplicates

				for( i=0; i < vertices.size(); i++ )
				{
					reduced.push_back( vertices.at(i) );					
				}


				// Sort to prepare for unique() reduction
				sort( reduced.begin(), reduced.end() );

				// Eliminate duplicate vertex entries in order to count
				//  and see if we have 3 or 4 unique vertices along this edge
				// This determines whether to create 1 or 2 tris there.

				r_end = unique( reduced.begin(), reduced.end() );
				reduced.erase( r_end, reduced.end() );


				switch( reduced.size() )
				{
				case 3:
					// stitch new tri with appropriate winding order
					if( vertices.at(2) == vertices.at(1) )
					{
						new_tris.push_back( vertices.at(1) );
						new_tris.push_back( vertices.at(0) );
						new_tris.push_back( vertices.at(3) );

						// Add "stitch-pair" - This is a pointer 
						//  from one edge vertex to another, in 
						//  appropriate winding order for a triangle
						//  adjacent to the one we just added.

						stitch_pairs[ vertices.at(3)].push_back( vertices.at(0) );

					}
					else if( vertices.at(3) == vertices.at(0) )
					{
						new_tris.push_back( vertices.at(1) );
						new_tris.push_back( vertices.at(0) );
						new_tris.push_back( vertices.at(2) );

						stitch_pairs[ vertices.at(1)].push_back( vertices.at(2) );
					}
					else
					{
						FDebug("Error: bad degenerate vertex indices in stitching!\n");
						assert( false );
					}

					FDebug("Single tri stitched along an edge instead of a quad - One vert used twice\n");


					break;

				case 4:
					new_tris.push_back( vertices.at(1) );
					new_tris.push_back( vertices.at(0) );
					new_tris.push_back( vertices.at(2) );

					// Add "stitch-pair" - This is a pointer 
					//  from one edge vertex to another, in 
					//  appropriate winding order for a triangle
					//  adjacent to the one we just added.
					stitch_pairs[ vertices.at(1)].push_back( vertices.at(2) );

					new_tris.push_back( vertices.at(3) );
					new_tris.push_back( vertices.at(2) );
					new_tris.push_back( vertices.at(0) );

					stitch_pairs[ vertices.at(3)].push_back( vertices.at(0) );

					break;
				default:
//gj					FDebug("Bad # vertices found for quad creation!  %d\n", reduced.size() );
//gj					assert( false );
					break;

				}

				// Now mark the welded value as having been welded
				// Also mark the neighbor tris welded flag

				*( is_welded[n] ) = NOT_WELDED_STITCHED;

				switch( neighbor_edge )
				{
				case 0:
					m_pTriNeighbors[ neighbor_tris[n] ].welded1 = NOT_WELDED_STITCHED;
					break;
				case 1:
					m_pTriNeighbors[ neighbor_tris[n] ].welded2 = NOT_WELDED_STITCHED;
					break;
				case 2:
					m_pTriNeighbors[ neighbor_tris[n] ].welded3 = NOT_WELDED_STITCHED;
					break;

				default:
					FDebug("bad neighbor edge\n");
					assert( false );
				}

			}
		}
	}



	/////////////////////////////////////////////////////////
	// Knit caps where more than two vertices share the same
	//  position.
	//
	// These caps may not be needed - The object could be an
	//  open hull, but open only at POINTS (vertices), and
	//  never open along edges.
	// A point where > 2 vertices are located but with no 
	//  triangles between the points will never be extruded
	//  to open the hull.
	// If the object is "exploded" to move the vertices apart
	//  then these caps are needed.
	// Also, if the shadow volume vertices are moved inward
	//  along the normal in order to smooth the shadow intersection
	//  with the polygon model (to smooth the popping in and 
	//  out of shadow as the volume penetrates through), these
	//  caps are needed to correct for overlapping polys.  The
	//  cap will be flipped to the opposite facing & will correct
	//  for the overlapped regions.
	
	// Above, we built sets of "stitch pairs" for each edge where
	//  zero-area quads were created.  These pairs of vertices
	//  are the edges of those quads which are 1) not along the 
	//  original triangle edges, and 2) not the quad diagonal.
	// So, to make the caps, sort & search the stitch pairs.
	//  Wherever they form a closed loop, that's where we add
	//  a cap,  Winding order for the tris of this cap falls out
	//  of the order of verts in the stitch pair.

	// In most cases there will be only 1 entry for each vertex
	//  in the stitch pair.  If there are 2 entries, that means
	//  that TWO loops intersect.  One vertex points to two 
	//  appropriate verts that would be next in the loop.
	//@@@@  I don't handle that case in this code yet!  It just
	//  asserts false.
	
	
	V_UINT	ring;

	int		index, new_index;
	int		start_index;		// for sanity check
			
			// max verts in a ring to stitch with a cap
	int		MAX_IN_RING  = 20000;


	for( n=0; n < GetNumVertices(); n++ )
	{
		start_index = n;
		index = n;

		ring.clear();

		while( stitch_pairs[index].size() > 0 )
		{
			if( stitch_pairs[index].size() > 1 )
			{
				FDebug("\n");
				FDebug("Intersecting loops found!!  Yikes!  This case not coded up yet!!!\n");
				assert( false );
			}
			
			ring.push_back( index );

			new_index = stitch_pairs[index].at(0);	// get 1st vertex pointed to
													// This is next vertex in the loop

			// remove 1st element (1st vertex pointed to, for this
			//  base vertex)
			// In most cases this will cause the size to go to zero, so
			//  no further looping from there will occur

			stitch_pairs[index].erase( stitch_pairs[index].begin() );
			
			if( (int)ring.size() > MAX_IN_RING )
			{
				assert( false );
			}

			index = new_index;
		}


		if( ring.size() > 0 )
		{
			// FDebug("Ring found of size:  %d\n", ring.size() );

			if( ring.size() > 1 )
			{
				// If we've made a loop, then index will be start_index
				//  and this value WILL NOT be repeated at the end of the loop
				if( index != start_index )
				{
					FDebug("Can't stitch caps because not a closed loop!\n");
//					assert( false );
//					return( E_FAIL );
				}
				else if( ring.size() > 2 )
				{
					// make tris!
					
					for( i=1; i < ring.size() - 1; i++ )
					{
						new_tris.push_back( ring.at(0)   );
						new_tris.push_back( ring.at(i)   );
						new_tris.push_back( ring.at(i+1) );
					}
				}
			}

		}
	}
	

	/////////////////////////////////////////////////////////////
	// Re-allocate index data and add new_tris to the end of the
	//  regular geometry.


	FDebug("Old tri count:   %d\n", GetNumIndices() / 3  );


	m_wOriginalModelTriIndices = GetNumIndices();
	m_wLastTriIndex = GetNumIndices();


	if( new_tris.size() > 0 )
	{
		UINT  new_num_ind = (UINT) new_tris.size() + GetNumIndices();

		if( new_num_ind < 65536 )
		{
			m_wOriginalModelTriIndices = GetNumIndices();

			AllocateResizeIndices( new_num_ind );

			m_wLastTriIndex = GetNumIndices();

			assert( m_wLastTriIndex == ( m_wOriginalModelTriIndices + new_tris.size() ) );

			for( n = 0; n < new_tris.size(); n++ )
			{
				m_pIndices[ m_wOriginalModelTriIndices + n ] = new_tris.at(n);				
			}
		}
		else
		{
			FDebug("New object would have too many indices!!\n");
			FDebug("   Can't store them in our UINT data!!\n");
			assert( false );
			// Doesn't matter that temp data is not freed - that'll happen
			//  eventually
			return( E_FAIL );
		}
	}
	else
	{
		FDebug("No new tris to add!\n");
	}


	FDebug("New tri count:   %d\n", GetNumIndices() / 3  );
	FDebug("New vert count:  %d\n", GetNumVertices() );
	FDebug("\n");

	hr = FreeTemporaryData();
	BREAK_IF( FAILED(hr) );

	SAFE_ARRAY_DELETE( stitch_pairs );

	return( hr );
}


//@@@ BuildShadowVolume_CloseHoles reduce code duplication!
HRESULT		ShadowVolumeMesh::BuildShadowVolume_CloseHoles( float weld_edge_threshold )
{
	// This version will attempt to stitch caps across open holes in an object so
	//  that the stencil shadow volume vertex shader extrusion works properly and
	//  produces a watertight shadow volume.

	// The stitching is very simple.  It will stitch relatively flat holes very well,
	//   It will close more complex holes, but it is not smart about choosing triangles
	//   to make the cap, so for holes with complex topology the polys may intersect,
	//   fold over one another, or produce undesireable tesselations.
	// If this is a problem, please contact technical developer relations at nvidia,
	//   or Greg James, gjames@nvidia.com, and we'll be happy to help!
	//
	// This also stitches caps after inserting zero area tris for the shadow volume,
	//  so some capped objects will not extrude to the proper shadow volume shape.
	//  More robust code is needed to cap before adding zero area tris.


	assert( m_bIsValid );
	assert( m_PrimType == D3DPT_TRIANGLELIST );
	assert( sizeof(UINT) == 2 );

	assert( m_bOpenHull == false );

	HRESULT hr = S_OK;

	const float limit = 0.9f;

	if( weld_edge_threshold > limit )
	{
		FDebug("\n");
		FDebug("########################################################\n");
		FDebug("ShadowVolume BuildShadowVolume:\n");
		FDebug("weld_edge_threshold > %f\n", limit );
		FDebug("This will result in MANY additional triangles in areas where\n");
		FDebug("   they are not needed.  Try using a threshold closer to 0.6\n");
		FDebug("########################################################\n");
		FDebug("\n");
	}	


	hr = BuildWingedEdgeData();
	BREAK_IF( FAILED(hr) );

	MeshProcessor mp;
	mp.MakeFaceted( this );

	// Eliminate some vertices to join edges 
	//   where the face normals are similar.
	// Thresh is in the range [-1,1]
	// A value of 1.0 will weld only faces
	//  with the same normal.
	WeldFacetedFaces( weld_edge_threshold );


	// For each non-welded edge (each edge with different vertices
	//  at the same spatial position), create zero-area triangles
	//  to close the edge.
	

	// rebuild the winged-edge data now that some faces
	// have been welded

	BuildWingedEdgeData();

	unsigned int		tri;
	unsigned int		n, i;

	UINT	last_edge_vertex;
	UINT	neighbor_edge;

	V_UINT	new_tris;
	new_tris.clear();

	V_UINT	vertices;

	V_UINT	reduced;
	V_UINT :: iterator r_end;

	// 1 array of WORDS for each vertex.  This is to form
	//  caps for areas where 3 or more vertices share the 
	//  same position.

	V_INT * stitch_pairs = new V_INT[ GetNumVertices() ];
	assert( stitch_pairs != NULL );


	V_UINT					neighbor_tris;
	std::vector< UINT * >	is_welded;		// corresponds to entries for neighbor_tris
	V_UINT					edge;

	WELD_VALUE				nweld;

	UINT	nob_v1, nob_v2;


	for( tri = 0; tri < GetNumIndices() / 3; tri++ )
	{
		neighbor_tris.clear();
		is_welded.clear();
		edge.clear();


		// ind1   Is index or flag for the triangle along the first edge of
		//        triangle 'tri'.  This edge is from
		//        vertices:  0 to 1     of the triangle
		//
		// ind2   is bordering tri index or flag for edge between
		//        vertices   1 to 2
		//
		// ind3   is bordering tri index or flag for edge between
		//        vertices   2 to 0



		if( m_pTriNeighbors[tri].ind1 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind1 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded1 ));
			edge.push_back( 0 );
		}
		else
		{
			// Object has a hole in it.  
			// This edge has no bordering tri.
			// Edge has vertices A, B.  Winding for the tri is
			//  from A to B.
			// Add vertex A to vertex B's stitch pair array, so that
			//  in later processing this open hole can be detected
			//  and a cap created to close it.
			// The winding for the stitch pair is the winding appropriate
			//  for a triangle created from the pair (ie, is opposite the
			//  winding of the existing triangle with no bordering triangle

			nob_v1 = m_pIndices[ tri*3 + 1 ];
			nob_v2 = m_pIndices[ tri*3 + 0 ];

			stitch_pairs[ nob_v1  ].push_back( nob_v2 );
		}

		if( m_pTriNeighbors[tri].ind2 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind2 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded2 ));
			edge.push_back( 1 );
		}
		else
		{
			// See comment above for what's going on.
			// Hole on edge between verts 1, 2 of triangle 'tri'

			nob_v1 = m_pIndices[ tri*3 + 2 ];
			nob_v2 = m_pIndices[ tri*3 + 1 ];

			stitch_pairs[ nob_v1  ].push_back( nob_v2 );

		}


		if( m_pTriNeighbors[tri].ind3 != NO_NEIGHBOR )
		{
			neighbor_tris.push_back( m_pTriNeighbors[tri].ind3 );
			is_welded.push_back( &( m_pTriNeighbors[tri].welded3 ));
			edge.push_back( 2 );
		}
		else
		{
			// See comment above for what's going on.
			// Hole on edge between verts 2, 0 of triangle 'tri'

			nob_v1 = m_pIndices[ tri*3 + 0 ];
			nob_v2 = m_pIndices[ tri*3 + 2 ];

			stitch_pairs[ nob_v1  ].push_back( nob_v2 );

		}


		if( neighbor_tris.size() != 3 )
		{
			m_bOpenHull = true;
			FDebug("BuildShadowVolume_CloseHoles:  Hole found which will be closed!\n");
		}

		for( n=0; n < neighbor_tris.size(); n++ )
		{

			if( *( is_welded[n] ) == NOT_WELDED )
			{
				// add vertices we need to stitch
				// if n = 2 then use vertices n, 0
				last_edge_vertex = ( edge[n] + 1 ) % 3;

				vertices.clear();
				reduced.clear();

				vertices.push_back( m_pIndices[ tri*3 + edge[n] ] );
				vertices.push_back( m_pIndices[ tri*3 + last_edge_vertex ] );

				// Which edge of the neighbor is the base trianlge, tri,
				//	 attached to?
				neighbor_edge = 4;

				if		( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind1 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded1;

					neighbor_edge = 0;
				}
				else if	( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind2 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded2;

					neighbor_edge = 1;
				}
				else if	( tri == m_pTriNeighbors[ neighbor_tris[n] ].ind3 )
				{
					nweld = m_pTriNeighbors[ neighbor_tris[n] ].welded3;

					neighbor_edge = 2;
				}

				if( neighbor_edge < 0 || neighbor_edge > 2 )
				{
					FDebug("Error - can't find base tri location relative to neighbor!!\n");
					assert(false);
				}

				last_edge_vertex = ( neighbor_edge + 1 ) % 3;
				vertices.push_back( m_pIndices[ neighbor_tris[n]*3 + neighbor_edge ] );
				vertices.push_back( m_pIndices[ neighbor_tris[n]*3 + last_edge_vertex ] );

				// Some vertices pushed back above might refer to the 
				//  same vertex.  Need to eliminate duplicate entries, so
				//  make a vector, sort & eliminate duplicates

				for( i=0; i < vertices.size(); i++ )
				{
					reduced.push_back( vertices.at(i) );					
				}


				// Sort to prepare for unique() reduction
				sort( reduced.begin(), reduced.end() );

				// Eliminate duplicate vertex entries in order to count
				//  and see if we have 3 or 4 unique vertices along this edge
				// This determines whether to create 1 or 2 tris there.

				r_end = unique( reduced.begin(), reduced.end() );
				reduced.erase( r_end, reduced.end() );


				switch( reduced.size() )
				{
				case 3:
					// stitch new tri with appropriate winding order
					if( vertices.at(2) == vertices.at(1) )
					{
						new_tris.push_back( vertices.at(1) );
						new_tris.push_back( vertices.at(0) );
						new_tris.push_back( vertices.at(3) );

						// Add "stitch-pair" - This is a pointer 
						//  from one edge vertex to another, in 
						//  appropriate winding order for a triangle
						//  adjacent to the one we just added.

						stitch_pairs[ vertices.at(3)].push_back( vertices.at(0) );

					}
					else if( vertices.at(3) == vertices.at(0) )
					{
						new_tris.push_back( vertices.at(1) );
						new_tris.push_back( vertices.at(0) );
						new_tris.push_back( vertices.at(2) );

						stitch_pairs[ vertices.at(1)].push_back( vertices.at(2) );
					}
					else
					{
						FDebug("Error: bad degenerate vertex indices in stitching!\n");
						assert( false );
					}

					FDebug("Single tri stitched along an edge instead of a quad - One vert used twice\n");


					break;

				case 4:
					new_tris.push_back( vertices.at(1) );
					new_tris.push_back( vertices.at(0) );
					new_tris.push_back( vertices.at(2) );

					// Add "stitch-pair" - This is a pointer 
					//  from one edge vertex to another, in 
					//  appropriate winding order for a triangle
					//  adjacent to the one we just added.
					stitch_pairs[ vertices.at(1)].push_back( vertices.at(2) );

					new_tris.push_back( vertices.at(3) );
					new_tris.push_back( vertices.at(2) );
					new_tris.push_back( vertices.at(0) );

					stitch_pairs[ vertices.at(3)].push_back( vertices.at(0) );

					break;
				default:
//gj					FDebug("Bad # vertices found for quad creation!  %d\n", reduced.size() );
//gj					assert( false );
					break;

				}

				// Now mark the welded value as having been welded
				// Also mark the neighbor tris welded flag

				*( is_welded[n] ) = NOT_WELDED_STITCHED;

				switch( neighbor_edge )
				{
				case 0:
					m_pTriNeighbors[ neighbor_tris[n] ].welded1 = NOT_WELDED_STITCHED;
					break;
				case 1:
					m_pTriNeighbors[ neighbor_tris[n] ].welded2 = NOT_WELDED_STITCHED;
					break;
				case 2:
					m_pTriNeighbors[ neighbor_tris[n] ].welded3 = NOT_WELDED_STITCHED;
					break;

				default:
					FDebug("bad neighbor edge\n");
					assert( false );
				}

			}
		}
	}

	/////////////////////////////////////////////////////////
	// Knit caps where more than two vertices share the same
	//  position.
	//
	// These caps may not be needed - The object could be an
	//  open hull, but open only at POINTS (vertices), and
	//  never open along edges.
	// A point where > 2 vertices are located but with no 
	//  triangles between the points will never be extruded
	//  to open the hull.
	// If the object is "exploded" to move the vertices apart
	//  then these caps are needed.
	// Also, if the shadow volume vertices are moved inward
	//  along the normal in order to smooth the shadow intersection
	//  with the polygon model (to smooth the popping in and 
	//  out of shadow as the volume penetrates through), these
	//  caps are needed to correct for overlapping polys.  The
	//  cap will be flipped to the opposite facing & will correct
	//  for the overlapped regions.
	
	// Above, we built sets of "stitch pairs" for each edge where
	//  zero-area quads were created.  These pairs of vertices
	//  are the edges of those quads which are 1) not along the 
	//  original triangle edges, and 2) not the quad diagonal.
	// So, to make the caps, sort & search the stitch pairs.
	//  Wherever they form a closed loop, that's where we add
	//  a cap,  Winding order for the tris of this cap falls out
	//  of the order of verts in the stitch pair.

	// In most cases there will be only 1 entry for each vertex
	//  in the stitch pair.  If there are 2 entries, that means
	//  that TWO loops intersect.  One vertex points to two 
	//  appropriate verts that would be next in the loop.
	//@@@@  I don't handle that case in this code yet!  It just
	//  asserts false.
	
	
	V_UINT	ring;

	int		index, new_index;
	int		start_index;		// for sanity check
			
			// max verts in a ring to stitch with a cap
	int		MAX_IN_RING  = 20000;


	for( n=0; n < GetNumVertices(); n++ )
	{
		start_index = n;
		index = n;

		ring.clear();

		while( stitch_pairs[index].size() > 0 )
		{
			if( stitch_pairs[index].size() > 1 )
			{
				FDebug("\n");
				FDebug("Intersecting loops found!!  Yikes!  This case not coded up yet!!!\n");
				assert( false );
			}
			
			ring.push_back( index );

			new_index = stitch_pairs[index].at(0);	// get 1st vertex pointed to
													// This is next vertex in the loop

			// remove 1st element (1st vertex pointed to, for this
			//  base vertex)
			// In most cases this will cause the size to go to zero, so
			//  no further looping from there will occur

			stitch_pairs[index].erase( stitch_pairs[index].begin() );
			
			if( (int)ring.size() > MAX_IN_RING )
			{
				assert( false );
			}

			index = new_index;
		}


		if( ring.size() > 0 )
		{
			// FDebug("Ring found of size:  %d\n", ring.size() );

			if( ring.size() > 1 )
			{
				// If we've made a loop, then index will be start_index
				//  and this value WILL NOT be repeated at the end of the loop
				assert( index == start_index );
				
				if( ring.size() > 2 )
				{
					// make tris!
					
					for( i=1; i < ring.size() - 1; i++ )
					{
						new_tris.push_back( ring.at(0)   );
						new_tris.push_back( ring.at(i)   );
						new_tris.push_back( ring.at(i+1) );
					}
				}
			}
		}
	}
	

	/////////////////////////////////////////////////////////////
	// Re-allocate index data and add new_tris to the end of the
	//  regular geometry.


	FDebug("Old tri count:  %d\n", GetNumIndices() / 3  );

	m_wOriginalModelTriIndices = GetNumIndices();
	m_wLastTriIndex = GetNumIndices();


	if( new_tris.size() > 0 )
	{
		UINT  new_num_ind = (UINT) new_tris.size() + GetNumIndices();

		if( new_num_ind < 65536 )
		{
			m_wOriginalModelTriIndices = GetNumIndices();

			AllocateResizeIndices( new_num_ind );

			m_wLastTriIndex = GetNumIndices();

			assert( m_wLastTriIndex == ( m_wOriginalModelTriIndices + new_tris.size() ) );

			for( n = 0; n < new_tris.size(); n++ )
			{
				m_pIndices[ m_wOriginalModelTriIndices + n ] = new_tris.at(n);				
			}
		}
		else
		{
			FDebug("New object would have too many indices!!\n");
			FDebug("   Can't store them in our UINT data!!\n");
			assert( false );
			// Doesn't matter that temp data is not freed - that'll happen
			//  eventually
			return( E_FAIL );
		}
	}
	else
	{
		FDebug("No new tris to add!\n");
	}



	FDebug("New tri count:  %d\n\n", GetNumIndices() / 3  );

	hr = FreeTemporaryData();
	BREAK_IF( FAILED(hr) );

	SAFE_ARRAY_DELETE( stitch_pairs );

	return( hr );
}









