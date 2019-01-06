/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  GeoIndexRing1Neighbors.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See .h for comments

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DMeshDX9PCH.h"
#include <algorithm>
using namespace std;

// Define to do more checking on function inputs (pointers and ranges)
// #define ERROR_CHECK1

void ListNeighborInfo( vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvNW );
void AddNeighbors( VIND_TYPE vert, VIND_TYPE n1, VIND_TYPE n2,
					VIND_TYPE * pNeighbors, UINT num_per_vert,
					UINT * pNumNeighbors, UINT num_vertices,
					vector< VIND_TYPE > * pvExtrasVertIndex,
					vector< VIND_TYPE > * pvExtrasNeighbor );
void AddNeighbors( VIND_TYPE vert, VIND_TYPE n1, VIND_TYPE n2,
					GeoIndexRing1Neighbors::NeighborWithWinding * pNeighborsW, UINT num_per_vert,
					UINT * pNumNeighbors, UINT num_vertices,
					vector< VIND_TYPE > * pvExtrasVertIndex,
					vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvExtrasNeighborW );
bool NWEqual( GeoIndexRing1Neighbors::NeighborWithWinding a, GeoIndexRing1Neighbors::NeighborWithWinding b );
bool NWLessThan( GeoIndexRing1Neighbors::NeighborWithWinding a, GeoIndexRing1Neighbors::NeighborWithWinding b );
void ReduceInfo( vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvNW );

//-------------------------------------------------------------

GeoIndexRing1Neighbors::GeoIndexRing1Neighbors()
{
	SetAllNull();

	m_bReportTempMemoryUsed = false;
	m_bReportDataMemory		= false;
}


GeoIndexRing1Neighbors::~GeoIndexRing1Neighbors()
{
	Free();
}

HRESULT GeoIndexRing1Neighbors::Free()
{
	HRESULT hr = S_OK;
	SAFE_ARRAY_DELETE( m_pRing1Neighbors );
	SAFE_ARRAY_DELETE( m_pNeighborsWithWinding );
	SAFE_ARRAY_DELETE( m_pRing1NeighborsStart );
	SAFE_ARRAY_DELETE( m_pRing1NeighborsNumber );

	SetAllNull();
	return( hr );
}

//---------------------------------------------------------------------------------

// Scan the array pIndices looking for the largest value.
// This is the largest vertex index, and so is the number of vertices
UINT	GeoIndexRing1Neighbors::ComputeNumVertices( VIND_TYPE * pIndices, UINT num_indices )
{
	UINT num_vert = 0;
	if( pIndices == NULL )
	{
		return( 0 );
	}

	UINT i;
	VIND_TYPE v;
	for( i=0; i < num_indices; i++ )
	{
		v = pIndices[i] + 1;	// vertex num is the vertex index + 1
		if( v > num_vert )
			num_vert = v;
	}

	return( num_vert );
}


void AddNeighbors( VIND_TYPE vert, VIND_TYPE n1, VIND_TYPE n2,
					VIND_TYPE * pNeighbors, UINT num_per_vert,
					UINT * pNumNeighbors, UINT num_vertices,
					vector< VIND_TYPE > * pvExtrasVertIndex,
					vector< VIND_TYPE > * pvExtrasNeighbor )
{
#ifdef ERROR_CHECK1
	BREAK_IF( pNeighbors == NULL );
	BREAK_IF( pNumNeighbors == NULL );
	BREAK_IF( pwExtrasVertIndex == NULL );
	BREAK_IF( pwExtrasNeighbor == NULL );
	BREAK_IF( n1 >= num_vertices );
	BREAK_IF( n2 >= num_vertices );
	BREAK_IF( vert >= num_vertices );
#endif

	UINT idx;
	if( pNumNeighbors[vert] < num_per_vert-1 )		// -1 since we have to add two neighbor verts
	{
		// if storage available, add the vert neighbors to the big fixed sized array
		idx = vert * num_per_vert + pNumNeighbors[vert];

		if( idx >= num_per_vert * num_vertices - 1 )
		{	
			assert( false );
			return;
		}

		pNeighbors[ idx+0 ] = n1;
		pNeighbors[ idx+1 ] = n2;
		pNumNeighbors[vert] = pNumNeighbors[vert] + 2;
	}
	else
	{	
		// add the vert and neighbors to the extras arrays
		pvExtrasVertIndex->push_back( vert );
		pvExtrasNeighbor->push_back( n1 );

		pvExtrasVertIndex->push_back( vert );
		pvExtrasNeighbor->push_back( n2 );
	}
}

void AddNeighbors( VIND_TYPE vert, VIND_TYPE n1, VIND_TYPE n2,
					GeoIndexRing1Neighbors::NeighborWithWinding * pNeighborsW, UINT num_per_vert,
					UINT * pNumNeighbors, UINT num_vertices,
					vector< VIND_TYPE > * pvExtrasVertIndex,
					vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvExtrasNeighborW )
{
#ifdef ERROR_CHECK1
	if( pNeighborsW == NULL ||
		pNumNeighbors == NULL ||
		pvExtrasVertIndex == NULL ||
		pvExtrasNeighborW == NULL )
	{
		assert( false );
		return;
	}
	if( n1 >= num_vertices )
		assert( false );
	if( n2 >= num_vertices )
		assert( false );
	if( vert >= num_vertices )
		assert( false );
#endif

	UINT idx;
	if( pNumNeighbors[vert] < num_per_vert - 1 )  // -1 since we add two to m_pNeighborsW: one for forward, one for backward
	{
		// if storage available, add the vert neighbors to the big fixed sized array
		idx = vert * num_per_vert + pNumNeighbors[vert];

		if( idx >= num_per_vert * num_vertices - 1)
		{	
			assert( false );
			return;
		}

		pNeighborsW[ idx+0 ].m_uRing1Neighbor			= n1;
		pNeighborsW[ idx+0 ].m_uForwardWindingNeighbor	= n2;
		pNeighborsW[ idx+0 ].m_uBackwardWindingNeighbor	= (UINT)GeoIndexRing1Neighbors::NoData;

		pNeighborsW[ idx+1 ].m_uRing1Neighbor			= n2;
		pNeighborsW[ idx+1 ].m_uForwardWindingNeighbor	= (UINT)GeoIndexRing1Neighbors::NoData;
		pNeighborsW[ idx+1 ].m_uBackwardWindingNeighbor	= n1;

		pNumNeighbors[vert] = pNumNeighbors[vert] + 2;
	}
	else
	{	
		GeoIndexRing1Neighbors::NeighborWithWinding nw;
		nw.m_uRing1Neighbor				= n1;
		nw.m_uForwardWindingNeighbor	= n2;
		nw.m_uBackwardWindingNeighbor	= (UINT)GeoIndexRing1Neighbors::NoData;

		// add the vert and neighbors to the extras arrays
		pvExtrasVertIndex->push_back( vert );		// which vertex these neighbors belong to
		pvExtrasNeighborW->push_back( nw );

		nw.m_uRing1Neighbor				= n2;
		nw.m_uForwardWindingNeighbor	= (UINT)GeoIndexRing1Neighbors::NoData;
		nw.m_uBackwardWindingNeighbor	= n1;
		pvExtrasVertIndex->push_back( vert );		// which vertex these neighbors belong to
		pvExtrasNeighborW->push_back( nw );
	}
}



HRESULT GeoIndexRing1Neighbors::ComputeRing1NeighborInfo( VIND_TYPE * pIndices, UINT num_indices, UINT num_vertices )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pIndices );
	if( num_indices == 0 ||
		num_vertices == 0 )
	{
		assert( false );
		return( E_FAIL );
	}

	// Create a temporary array to hold data as it is gathered
	// Begin by assuming that each vertex will have 10 ring-1 neighbors, and that we store each neighbor 
	//  twice before compacting the array.  If we run out of space for a given vertex, the extra neighbors
	//  will be kept in another dynamic array

	UINT num_per_vert;		// assumed number of neighbors per vertex
	num_per_vert = 10 * 2;	// 10 neighbors, each repeated once

	// allocate temp array to store neighbors
	VIND_TYPE *	pNeighbors;
	UINT		uNumNeighborArray;
	uNumNeighborArray = num_per_vert * num_vertices;
	pNeighbors = new VIND_TYPE[ uNumNeighborArray ];
	FAIL_IF_NULL( pNeighbors );

	// allocate array to track how many neighbors have been found for each vertex
	UINT *	pNumNeighbors;
	pNumNeighbors = new UINT[ num_vertices ];
	FAIL_IF_NULL( pNumNeighbors );


	// extra storage in case a vertex has more than num_per_vert neighbors
	// pairs of vertex index with a neighbor for that vertex.
	// I could make a structure of { UINT vertIndex; UINT vertNeighbor } but why bother... 
	vector< VIND_TYPE >	vExtrasVertIndex;
	vector< VIND_TYPE >  vExtrasNeighbor;
	//--------------------------------------------

	// Set number of neighbors array to zero
	memset( pNumNeighbors, 0, sizeof( UINT ) * num_vertices );

	UINT i;
	VIND_TYPE a,b,c;		// triangle vertex indices
	for( i=0; i < num_indices; i += 3 )
	{
		a = pIndices[i];
		b = pIndices[i+1];
		c = pIndices[i+2];

		AddNeighbors( a, b, c, pNeighbors, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighbor );
		AddNeighbors( b, c, a, pNeighbors, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighbor );
		AddNeighbors( c, a, b, pNeighbors, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighbor );
	}

/*
	// report what was found
	FMsg("\n");
	int n;
	for( i=0; i < num_vertices; i++ )
	{
		FMsg("%5.5u : ", i );
		for( n=0; n < pNumNeighbors[i]; n++ )
		{
			FMsg("%u, ", pNeighbors[ i * num_per_vert + n] );
		}
		FMsg("\n");
	}
	FMsg("\n");

	FMsg("Extras:\n");
	for( i=0; i < vExtrasVertIndex.size(); i++ )
	{
		FMsg(" vert %8.8u  : neighbor %u\n", vExtrasVertIndex.at(i), vExtrasNeighbor.at(i) ); 
	}
	FMsg("\n");
// */

	//-----------------------------------------------
	// Create the permanent data structure

	// destroy data with no winding info, as it will be out of date after this
	SAFE_ARRAY_DELETE( m_pNeighborsWithWinding );
	m_uNeighborsWithWindingSize = 0;

	// allocate start & num neighbors data arrays
	SAFE_ARRAY_DELETE( m_pRing1NeighborsStart );
	SAFE_ARRAY_DELETE( m_pRing1NeighborsNumber );
	m_uRing1NeighborsStartSize = num_vertices;
	m_pRing1NeighborsStart = new UINT[ m_uRing1NeighborsStartSize ];
	FAIL_IF_NULL( m_pRing1NeighborsStart );
	m_pRing1NeighborsNumber = new UINT[ m_uRing1NeighborsStartSize ];
	FAIL_IF_NULL( m_pRing1NeighborsNumber );

	UINT xtra, n;
	vector< VIND_TYPE > vVertNeighbors;		// temp storage for sorting & incorporating the Extras arrays
	vector< VIND_TYPE >	vFinal;				// all vertices' neighbor info
	vFinal.reserve( num_vertices * 10 );	// arbitrary guess based on max of 10 neighbors per vert

	for( i=0; i < num_vertices; i++ )
	{
		vVertNeighbors.clear();
		vVertNeighbors.reserve( num_per_vert + 5 );		// arbitrary guess

		// Copy neighbor info in the big array to the small array to sort
		// This could be eliminated if there is no Extras data
		for( n=0; n < pNumNeighbors[i]; n++ )
		{
			vVertNeighbors.push_back( pNeighbors[ i * num_per_vert + n] );
		}

		// incorporate vertices from the extras array if there are any
		if( pNumNeighbors[i] >= num_per_vert - 1 )
		{
			// look for data in the extras arrays, copying it to the temp array
			for( xtra=0; xtra < vExtrasVertIndex.size(); xtra++ )
			{
				if( vExtrasVertIndex.at(xtra) == i )
				{
					// add the neighbors to the small temp array
					vVertNeighbors.push_back( vExtrasNeighbor.at(xtra) );
				}
			}
		}

		// sort the array of neighbors
		sort( vVertNeighbors.begin(), vVertNeighbors.end() );
		// eliminate duplicate entries
		vector<VIND_TYPE>::iterator ui, unique_end;
		unique_end = unique( vVertNeighbors.begin(), vVertNeighbors.end() );

		// Copy the ring-1 neighbor info for this vertex to a vector
		// The whole vector will be copied to the final stored data at the end of this func.
		// The copy could be eliminated if the final data were a vector<UINT> instead of a plain pointer

		m_pRing1NeighborsStart[i] = (UINT) vFinal.size();
		for( ui = vVertNeighbors.begin(); ui < unique_end; ui++ )
		{
			vFinal.push_back( *ui );				
		}
		m_pRing1NeighborsNumber[i] = (UINT) vFinal.size() - m_pRing1NeighborsStart[i];
	}

	// allocate the permanent storage and copy data to it
	SAFE_ARRAY_DELETE( m_pRing1Neighbors );
	m_uRing1NeighborsSize = (UINT) vFinal.size();
	m_pRing1Neighbors = new VIND_TYPE[ m_uRing1NeighborsSize ];
	FAIL_IF_NULL( m_pRing1Neighbors );

	for( i=0; i < m_uRing1NeighborsSize; i++ )
	{
		m_pRing1Neighbors[i] = vFinal[i];
	}

	//--------------------------------------------

	// Option to report roughly the amount of storage used
	if( m_bReportTempMemoryUsed )
	{
		UINT nBytes;
		nBytes = 0;
		nBytes += sizeof(UINT) * num_per_vert * num_vertices;		// pNeighbors
		nBytes += sizeof(UINT) * num_vertices;						// pNumNeighbors
		nBytes += (UINT) vExtrasVertIndex.size() * sizeof(UINT) * 2;		// extras vectors
		nBytes += (UINT) vFinal.size() * sizeof(UINT);						// final acumulation vector

		FMsg("ComputeRing1NeighborInfo used about %u bytes of temp storage\n", nBytes );
		nBytes = (UINT) vExtrasVertIndex.size() * sizeof(VIND_TYPE) * 2;
		FMsg("       %u bytes of that were in the dynamic extra storage vectors\n", nBytes );
		FMsg("       %u vert-neighbor pairs in the dynamic vectors\n", vExtrasVertIndex.size() );
	}
	if( m_bReportDataMemory == true )
	{
		ListDataMemoryUsed();
	}

	SAFE_ARRAY_DELETE( pNeighbors );
	SAFE_ARRAY_DELETE( pNumNeighbors );

	return( hr );
}


	// No argument for num_vertices requires the number to be calculated from the index array
	// This will take more time.
HRESULT GeoIndexRing1Neighbors::ComputeRing1NeighborInfo( VIND_TYPE * pIndices, UINT num_indices )
{
	HRESULT hr = S_OK;
	UINT num_vert;
	num_vert = ComputeNumVertices( pIndices, num_indices );
	hr = ComputeRing1NeighborInfo( pIndices, num_indices, num_vert );
	return( hr );
}


bool NWEqual( GeoIndexRing1Neighbors::NeighborWithWinding a, GeoIndexRing1Neighbors::NeighborWithWinding b )
{
	return( (a.m_uRing1Neighbor			== b.m_uRing1Neighbor) &&
			(a.m_uForwardWindingNeighbor == b.m_uForwardWindingNeighbor) &&
			(a.m_uBackwardWindingNeighbor == b.m_uBackwardWindingNeighbor) );
}

bool NWLessThan( GeoIndexRing1Neighbors::NeighborWithWinding a, GeoIndexRing1Neighbors::NeighborWithWinding b )
{
	if( a.m_uRing1Neighbor < b.m_uRing1Neighbor )
		return( true );
	else if( a.m_uRing1Neighbor > b.m_uRing1Neighbor )
		return( false );

	if( a.m_uForwardWindingNeighbor < b.m_uForwardWindingNeighbor )
		return( true );
	else if( a.m_uForwardWindingNeighbor > b.m_uForwardWindingNeighbor )
		return( false );

	if( a.m_uBackwardWindingNeighbor < b.m_uBackwardWindingNeighbor )
		return( true );
	else if( a.m_uBackwardWindingNeighbor > b.m_uBackwardWindingNeighbor )
		return( false );
	return( false );
}


void ReduceInfo( vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvNW )
{
	// The reduced set is computed in the vector pointed to by pvNW 
	// so the return data is the pvNW vector itself

	if( pvNW == NULL )
		return;

	// sort the array of neighbors
	sort( pvNW->begin(), pvNW->end(), NWLessThan );
	// eliminate duplicate entries
	vector< GeoIndexRing1Neighbors::NeighborWithWinding >::iterator unique_end;
	unique_end = unique( pvNW->begin(), pvNW->end(), NWEqual );

	// eliminate the elements on the end of the array
	pvNW->erase( unique_end, pvNW->end() );

	// reduce the entries
	UINT i,n;
	vector< GeoIndexRing1Neighbors::NeighborWithWinding >::iterator p;	
	bool fill_forward, fill_backward;

	for( i=0; i < pvNW->size(); i++ )
	{
		p = pvNW->begin();
		p = p + i;

		fill_forward = (*p).m_uForwardWindingNeighbor == GeoIndexRing1Neighbors::NoData;
		fill_backward = (*p).m_uBackwardWindingNeighbor == GeoIndexRing1Neighbors::NoData;
		if( fill_forward && fill_backward )
		{
			FMsg("You should never have an empty forward and backward link at the same time!\n");
			assert( false );
			return;
		}

		// only scan forward if we have room at this element
		if( fill_forward || fill_backward )
		{
			for( n=i+1; n < pvNW->size(); n++ )
			{
				if( pvNW->at(n).m_uRing1Neighbor == (*p).m_uRing1Neighbor )
				{
					// don't consider this n element if both of it's fields are full
					if( pvNW->at(n).m_uForwardWindingNeighbor != GeoIndexRing1Neighbors::NoData && 
						pvNW->at(n).m_uBackwardWindingNeighbor != GeoIndexRing1Neighbors::NoData )
					{
						continue;	// go to next iteration of for(n..) loop
					}

					// if we need to fill a forward link, and forward data is avail...
					if( fill_forward && (pvNW->at(n).m_uForwardWindingNeighbor != GeoIndexRing1Neighbors::NoData) )
					{
						// copy data back to the element with the empty field at index i
						(*p).m_uForwardWindingNeighbor = pvNW->at(n).m_uForwardWindingNeighbor;
						pvNW->at(n).m_uForwardWindingNeighbor = (UINT)GeoIndexRing1Neighbors::NoData;

						if( pvNW->at(n).m_uBackwardWindingNeighbor != GeoIndexRing1Neighbors::NoData )
						{
							FMsg("neighbor entry that was copied from has other data!\n");
							assert( false );
						}
						// remove the element we copied from
						pvNW->erase( pvNW->begin() + n );
						// break out of the for(n..) loop
						break;
					}

					// if we need to fill a backward link, and backward data is avail...
					if( fill_backward && (pvNW->at(n).m_uBackwardWindingNeighbor != GeoIndexRing1Neighbors::NoData) )
					{
						// copy data back to the element with the empty field at index i
						(*p).m_uBackwardWindingNeighbor = pvNW->at(n).m_uBackwardWindingNeighbor;
						pvNW->at(n).m_uBackwardWindingNeighbor = (UINT)GeoIndexRing1Neighbors::NoData;

						if( pvNW->at(n).m_uForwardWindingNeighbor != GeoIndexRing1Neighbors::NoData )
						{
							FMsg("neighbor entry that was copied from has other data in forward field!\n");
							assert( false );
						}
						// remove the element we copied from
						pvNW->erase( pvNW->begin() + n );
						// break out of the for(n..) loop
						break;
					}
				}
				else
				{
					// stop the for(n..) loop
					// data is sorted by m_uRing1Neighbor, so once they are different, there will not be another match
					break;
				}
			}	// for(n..)
		}
	}
}


HRESULT GeoIndexRing1Neighbors::ComputeRing1NeighborInfoWithWinding( VIND_TYPE * pIndices, 
													UINT num_indices, UINT num_vertices )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pIndices );
	if( num_indices == 0 ||
		num_vertices == 0 )
	{
		assert( false );
		return( E_FAIL );
	}

	// Create a temporary array to hold data as it is gathered
	// Begin by assuming that each vertex will have 10 ring-1 neighbors, and that we store each neighbor 
	//  twice before compacting the array.  If we run out of space for a given vertex, the extra neighbors
	//  will be kept in a single dynamic array

	UINT num_per_vert;		// assumed number of neighbors per vertex
	num_per_vert = 10 * 2;	// 10 neighbors, each repeated once

		// allocate temp array to store neighbors with winding info
	NeighborWithWinding *	pNeighborsW;
	UINT					uNumNeighborArray;
	uNumNeighborArray = num_per_vert * num_vertices;
	pNeighborsW = new NeighborWithWinding[ uNumNeighborArray ];
	FAIL_IF_NULL( pNeighborsW );

		// allocate array to track how many neighbors have been found for each vertex
	UINT *	pNumNeighbors;
	pNumNeighbors = new UINT[ num_vertices ];
	FAIL_IF_NULL( pNumNeighbors );

	// extra storage in case a vertex has more than num_per_vert neighbors
	// pairs of vertex index with a neighbor for that vertex.
	// I could make a structure of { UINT vertIndex; UINT vertNeighbor } but why bother... 
	vector< VIND_TYPE >				vExtrasVertIndex;
	vector< NeighborWithWinding >	vExtrasNeighborW;
	//--------------------------------------------

	// Set number of neighbors array to zero
	memset( pNumNeighbors, 0, sizeof( UINT ) * num_vertices );

	UINT i;
	VIND_TYPE a,b,c;		// triangle vertex indices
	for( i=0; i < num_indices; i += 3 )
	{
		a = pIndices[i];
		b = pIndices[i+1];
		c = pIndices[i+2];

		AddNeighbors( a, b, c, pNeighborsW, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighborW );
		AddNeighbors( b, c, a, pNeighborsW, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighborW );
		AddNeighbors( c, a, b, pNeighborsW, num_per_vert, pNumNeighbors, num_vertices, & vExtrasVertIndex, & vExtrasNeighborW );
	}

	//-----------------------------------------------
	// Create the permanent data structure

	// destroy data with no winding info, as it will be out of date after this
	SAFE_ARRAY_DELETE( m_pRing1Neighbors );
	m_uRing1NeighborsSize = 0;

	// allocate start & num neighbors data arrays
	// these hold the position to start reading each vert's neighbors, and how many neighbors there are
	SAFE_ARRAY_DELETE( m_pRing1NeighborsStart );
	SAFE_ARRAY_DELETE( m_pRing1NeighborsNumber );
	m_uRing1NeighborsStartSize = num_vertices;
	m_pRing1NeighborsStart = new UINT[ m_uRing1NeighborsStartSize ];
	FAIL_IF_NULL( m_pRing1NeighborsStart );
	m_pRing1NeighborsNumber = new UINT[ m_uRing1NeighborsStartSize ];
	FAIL_IF_NULL( m_pRing1NeighborsNumber );

	UINT xtra, n;
	vector< NeighborWithWinding >	vVertNeighborsW;	// temp storage for sorting & incorporating the Extras arrays
	vector< NeighborWithWinding >	vFinalW;			// all vertices' neighbor info
	vFinalW.reserve( num_vertices * 10 );	// arbitrary guess based on max of 10 neighbors per vert

	for( i=0; i < num_vertices; i++ )
	{
		vVertNeighborsW.clear();
		vVertNeighborsW.reserve( num_per_vert + 5 );		// arbitrary guess

		// Copy neighbor info in the big array to the small array to sort
		// This could be eliminated if there is no Extras data
		for( n=0; n < pNumNeighbors[i]; n++ )
		{
			vVertNeighborsW.push_back( pNeighborsW[ i * num_per_vert + n] );

/*			// diagnostic option to print out data
			FMsg(" v : %u   n : %u %u %u\n", i, pNeighborsW[ i * num_per_vert + n].m_uRing1Neighbor,
				pNeighborsW[ i * num_per_vert + n].m_uForwardWindingNeighbor,
				pNeighborsW[ i * num_per_vert + n].m_uBackwardWindingNeighbor );
			UINT sz = vVertNeighborsW.size() - 1;
			FMsg(" v : %u   n : %u %u %u\n\n", i, vVertNeighborsW.at(sz).m_uRing1Neighbor,
				vVertNeighborsW.at(sz).m_uForwardWindingNeighbor,
				vVertNeighborsW.at(sz).m_uBackwardWindingNeighbor );
// */
		}

		// incorporate vertices from the extras array if there are any
		if( pNumNeighbors[i] >= num_per_vert - 1 )
		{
			// look for data in the extras arrays, copying it to the temp array
			for( xtra=0; xtra < vExtrasVertIndex.size(); xtra++ )
			{
				if( vExtrasVertIndex.at(xtra) == i )
				{
					// add the neighbors to the small temp array
					vVertNeighborsW.push_back( vExtrasNeighborW.at(xtra) );
				}
			}
		}

		// Reduce the vector of neighbor info and winding by filling in the empty forward and
		//  backward data.  Also eliminate duplicate entries, if any (there probably shouldn't be, unless
		//  a triangle is duplicated in the original index data)
		// For example:
		// (1,0,NA),(1,NA,2),(2,3,NA),(2,NA,4),(2,NA,5)
		//  will be reduced to:
		// (1,0,2),(2,3,4),(2,NA,5)
/*
		FMsg("------------------ Neighbors before reduction\n");
		FMsg("Vertex %u\n", i );
		ListNeighborInfo( & vVertNeighborsW );
		FMsg("\n");
// */
		ReduceInfo( & vVertNeighborsW );
/*
		FMsg("---- Neighbors after reduction\n");
		ListNeighborInfo( & vVertNeighborsW );
		FMsg("\n");
//*/

		// Copy the ring-1 neighbor info for this vertex to a vector that holds info for all vertices
		// The whole vector will be copied to the final stored data at the end of this func.
		// The copy could be eliminated if the final data were a vector<UINT> instead of a plain pointer

		m_pRing1NeighborsStart[i] = (UINT) vFinalW.size();
		for( n=0; n < vVertNeighborsW.size(); n++ )
		{
			vFinalW.push_back( vVertNeighborsW[n] );
		}
		m_pRing1NeighborsNumber[i] = (UINT) vFinalW.size() - m_pRing1NeighborsStart[i];

	}

	// allocate the permanent storage and copy data to it
	SAFE_ARRAY_DELETE( m_pNeighborsWithWinding );
	m_uNeighborsWithWindingSize = (UINT) vFinalW.size();
	m_pNeighborsWithWinding = new NeighborWithWinding[ m_uNeighborsWithWindingSize ];
	FAIL_IF_NULL( m_pNeighborsWithWinding );

	for( i=0; i < m_uNeighborsWithWindingSize; i++ )
	{
		m_pNeighborsWithWinding[i] = vFinalW[i];
	}

	//--------------------------------------------
	// Option to report roughly the amount of temporary storage used
	if( m_bReportTempMemoryUsed )
	{
		UINT nBytes;
		nBytes = 0;
		nBytes += sizeof(NeighborWithWinding) * num_per_vert * num_vertices;	// pNeighborsW
		nBytes += sizeof(UINT) * num_vertices;									// pNumNeighbors
		nBytes += (UINT) vExtrasVertIndex.size() * sizeof(UINT);
		nBytes += (UINT) vExtrasNeighborW.size() * sizeof(NeighborWithWinding);
		nBytes += (UINT) vFinalW.size() * sizeof(NeighborWithWinding);

		FMsg("ComputeRing1NeighborInfoWithWinding used about %u bytes of temp storage\n", nBytes );
		nBytes = (UINT) vExtrasVertIndex.size() * (sizeof(UINT) + sizeof(NeighborWithWinding));
		FMsg("       %u bytes of that were in the dynamic extra storage vectors\n", nBytes );
		FMsg("       %u vert-neighbor pairs in the dynamic vectors\n", vExtrasVertIndex.size() );
	}
	if( m_bReportDataMemory == true )
	{
		ListDataMemoryUsed();
	}

	SAFE_ARRAY_DELETE( pNeighborsW );
	SAFE_ARRAY_DELETE( pNumNeighbors );

	return( hr );
}

HRESULT GeoIndexRing1Neighbors::ComputeRing1NeighborInfoWithWinding( VIND_TYPE * pIndices, UINT num_indices )
{
	HRESULT hr = S_OK;
	UINT num_vert;
	num_vert = ComputeNumVertices( pIndices, num_indices );
	hr = ComputeRing1NeighborInfoWithWinding( pIndices, num_indices, num_vert );
	return( hr );
}



void GeoIndexRing1Neighbors::ListRing1NeighborInfo( UINT start_vert, UINT end_vert )
{
	// reports into with winding or info without winding
	if( m_pRing1Neighbors == NULL &&
		m_pNeighborsWithWinding == NULL )
	{
		FMsg("Both neighbor info arrays are NULL!\n");
		return;
	}
	if( start_vert >= m_uRing1NeighborsStartSize )
	{
		FMsg("Start vert out of bounds.  Not reporting any info\n");
		return;
	}
	if( end_vert >= m_uRing1NeighborsStartSize )
	{
		FMsg("End vert out of bounds.  Correcting...\n");
		if( m_uRing1NeighborsStartSize > 0 )
			end_vert = m_uRing1NeighborsStartSize - 1;
		else
			end_vert = 0;
	}
	if( end_vert < start_vert )
	{
		UINT tmp = end_vert;
		end_vert = start_vert;
		start_vert = tmp;
	}

	UINT v;	
	UINT n_start, n_end;		// neighbor start / end positions
	UINT n;

	if( m_pRing1Neighbors != NULL )
	{
		FMsg("Neighbor info : no winding\n");

		for( v = start_vert; v < end_vert; v++ )
		{
			n_start = m_pRing1NeighborsStart[v];
			n_end = n_start + m_pRing1NeighborsNumber[v];

			// report neighbor info
			FMsg(" vert %8u : ", v );
			for( n = n_start; n < n_end; n++ )
			{
				FMsg("%u, ", m_pRing1Neighbors[n] );
			}
			FMsg("\n");
		}
	}
	FMsg("\n");

	if( m_pNeighborsWithWinding != NULL )
	{
		FMsg("Neighbor info : with winding\n");

		for( v = start_vert; v < end_vert; v++ )
		{
			n_start = m_pRing1NeighborsStart[v];
			n_end = n_start + m_pRing1NeighborsNumber[v];

			// report neighbor info with winding
			FMsg(" vert %8u : ", v );
			for( n = n_start; n < n_end; n++ )
			{
				FMsg("[%u : ", m_pNeighborsWithWinding[n].m_uRing1Neighbor );
				if( m_pNeighborsWithWinding[n].m_uForwardWindingNeighbor == NoData )
					FMsg("NA, ");
				else
					FMsg("%u, ", m_pNeighborsWithWinding[n].m_uForwardWindingNeighbor );

				if( m_pNeighborsWithWinding[n].m_uBackwardWindingNeighbor == NoData )
					FMsg("NA] ");
				else
					FMsg("%u] ", m_pNeighborsWithWinding[n].m_uBackwardWindingNeighbor );
			}
			FMsg("\n");
		}
	}
}


void ListNeighborInfo( vector< GeoIndexRing1Neighbors::NeighborWithWinding > * pvNW )
{
	if( pvNW == NULL )
		return;
	UINT i;
	for( i=0; i < pvNW->size(); i++ )
	{
		FMsg(" %u : %u %u\n", pvNW->at(i).m_uRing1Neighbor, 
								pvNW->at(i).m_uForwardWindingNeighbor, 
								pvNW->at(i).m_uBackwardWindingNeighbor );
	}
}


void GeoIndexRing1Neighbors::ListDataMemoryUsed()
{
	UINT uBytes = 0;
	uBytes += m_uRing1NeighborsSize * sizeof(UINT);
	uBytes += m_uNeighborsWithWindingSize * sizeof(NeighborWithWinding);	// m_pNeighborsWithWinding
	uBytes += m_uRing1NeighborsStartSize * sizeof(UINT) * 2;				// m_pRing1NeighborsStart, m_pRing1NeighborsNumber

	FMsg("GeoIndexRing1Neighbors data memory in use: %.3f kb\n", ((float)uBytes) / 1024.0f );
	FMsg("    Neighbors              :  %u bytes\n", m_uRing1NeighborsSize * sizeof(UINT) );
	FMsg("    Neighbors with winding :  %u bytes\n", m_uNeighborsWithWindingSize * sizeof(NeighborWithWinding) );
	FMsg("    Indexing values        :  %u bytes\n", m_uRing1NeighborsStartSize * sizeof(UINT) * 2 );
	FMsg("    avg mem per vertex     :  %.1f bytes\n", ((float)uBytes) / m_uRing1NeighborsStartSize );
}

bool GeoIndexRing1Neighbors::IsNeighborsWithWindingInfoValid()
{
	if( m_pNeighborsWithWinding == NULL )
		return( false );
	if( m_uNeighborsWithWindingSize == 0 )
		return( false );
	if( m_pRing1NeighborsStart == NULL )
		return( false );
	if( m_pRing1NeighborsNumber == NULL )
		return( false );
	if( m_uRing1NeighborsStartSize == 0 )
		return( false );
	return( true );	
}

bool GeoIndexRing1Neighbors::IsNeighborsInfoValid()
{	
	if( m_pRing1Neighbors == NULL )
		return( false );
	if( m_uRing1NeighborsSize == 0 )
		return( false );
	if( m_pRing1NeighborsStart == NULL )
		return( false );
	if( m_pRing1NeighborsNumber == NULL )
		return( false );
	if( m_uRing1NeighborsStartSize == 0 )
		return( false );
	return( true );	
}

UINT	GeoIndexRing1Neighbors::GetNumVertices()
{
	return( m_uRing1NeighborsStartSize );
}

UINT	GeoIndexRing1Neighbors::GetNumNeighbors( VIND_TYPE vertex )
{
	if( vertex >= m_uRing1NeighborsStartSize )
		return( 0 );
	return( m_pRing1NeighborsNumber[ vertex ] );
}


/*---------------
in_vertex			vertex whos neighbor info to fetch
in_n_neighbor		number of neighbor to fetch.  use range [ 0, GetNumNeighbors(in_vertex)-1 ]
out_neighbor		neighbor vertex index
out_fwd_neighbor	vertex index of the forward winding neighbor of out_neighbor
out_bck_neighbor	vertex index of the backward winding neighbor of out_neighbor
out_has_fwd			1 if out_fwd_neighbor is valid, 0 if not
out_has_bck			1 if out_bck_neighbor is valid, 0 if not

There can be neighbors with several or no forward or backward winding neighbors.
*** No checks are made to see if inputs are in a valid range
---------------*/
void GeoIndexRing1Neighbors::GetNeighbor( VIND_TYPE in_vertex, VIND_TYPE in_n_neighbor,
		VIND_TYPE & out_neighbor, VIND_TYPE & out_fwd_neighbor, VIND_TYPE & out_bck_neighbor, 
		int & out_has_fwd, int & out_has_bck )
{
#ifdef ERROR_CHECK1
	BREAK_IF( m_pNeighborsWithWinding == NULL );
	BREAK_IF( in_vertex >= m_uRing1NeighborsStartSize );
	BREAK_IF( in_n_neighbor >= m_pRing1NeighborsNumber[ in_vertex ]);
	BREAK_IF( true != 1 );
	BREAK_IF( false != 0 );
#endif

	UINT ind;
	ind = m_pRing1NeighborsStart[in_vertex] + in_n_neighbor;

	out_neighbor		= m_pNeighborsWithWinding[ ind ].m_uRing1Neighbor;
	out_fwd_neighbor	= m_pNeighborsWithWinding[ ind ].m_uForwardWindingNeighbor;
	out_bck_neighbor	= m_pNeighborsWithWinding[ ind ].m_uBackwardWindingNeighbor;

/*
	// The math below is equivalent to:
	if( out_fwd_neighbor == NoData )
	{
		out_has_fwd = 0;
		out_fwd_neighbor = 0;
	}
	else
	{
		out_has_fwd = 1;
	}
	if( out_bck_neighbor == NoData )
	{
		out_has_bck = 0;
		out_bck_neighbor = 0;
	}
	else
	{
		out_has_bck = 1;
	}
*/

	// relies on bool true = 1, false = 0
	bool b;
	b = ! (out_fwd_neighbor == NoData);		// 0 if == NoData, 1 if not
	out_fwd_neighbor	= out_fwd_neighbor * b;
	out_has_fwd			= b;
	b = ! (out_bck_neighbor == NoData);
	out_bck_neighbor	= out_bck_neighbor * b;
	out_has_bck			= b;

}






