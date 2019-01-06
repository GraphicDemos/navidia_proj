/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  GeoIndexRing1Neighbors.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A utility class that accepts an array of triangle vertex indices and assembles a list of 
each vertex's ring-1 neighbors.

One of the data structures is specialized for a task of computing vertex normals efficiently, 
where not only do the ring-1 neighbors need to be known, but the winding order and the
next and previous ring-1 neighbors in the winding order need to be known.

The index array is for a collection of indexed triangles, with three indices for each triangle
Indexed tri strips or tri fans are not supported as inputs.


-------------------------------------------------------------------------------|--------------------*/


#ifndef H_GEOINDEXRING1NEIGHBORS_GJ_H
#define H_GEOINDEXRING1NEIGHBORS_GJ_H

#include <windows.h>
typedef DWORD	VIND_TYPE;

class GeoIndexRing1Neighbors
{
public:
	enum IVals
	{
		NoData = 0xFFFFFFFF
	};
	struct NeighborWithWinding
	{
		VIND_TYPE	m_uRing1Neighbor;
		VIND_TYPE	m_uForwardWindingNeighbor;
		VIND_TYPE	m_uBackwardWindingNeighbor;
	};
public:

	// Arrays used when winding info is not needed
	VIND_TYPE *	m_pRing1Neighbors;							// big array of all vertices' ring-1 neighbors
	UINT	m_uRing1NeighborsSize;							// size of m_pRing1Neighbors array

	// Arrays used when winding info is needed
	NeighborWithWinding *	m_pNeighborsWithWinding;
	UINT					m_uNeighborsWithWindingSize;	// size of m_pNeighborsWithWinding array

	UINT *  m_pRing1NeighborsStart;		// starting location in m_pRing1Neighbors for each vertex's neighbors
	UINT *  m_pRing1NeighborsNumber;	// number of ring-1 neighbors for each vertex.  
										// This is how many to read from m_pRing1Neighbors starting from m_pRing1NeighborsStart
	UINT	m_uRing1NeighborsStartSize;	// size of m_pRing1NeighborsStart array.  This is also the number of vertices

	// example:
	// triangle vertex indices are:  0, 1, 2,  0, 2, 3  (2 triangles, 4 vertices)
	// For this data:
	// m_uRing1NeighborsStartSize = 4
	// m_pRing1Neighbors = { 1,2,3,  2,0,  3,0,1,  0,2 }  (10 values)
	// m_pRing1NeighborsSize = 10
	//		The number of m_pRing1Neighbors is 10
	// m_pRing1NeighborsStart = { 0, 3, 5, 8 }
	//		vertex 0's neighbors start at position 0 of m_pRing1Neighbors
	//		vertex 1's neighbors start at position 3 of m_pRing1Neighbors, etc.
	// m_pRing1NeighborsNumber = { 3, 2, 3, 2 }
	//		vertex 0 has 3 ring-1 neighbors
	//		vertex 1 has 2 ring-1 neighbors, etc.

	// Scan the array of indices to find the maximum vertex index
	UINT	ComputeNumVertices( VIND_TYPE * pIndices, UINT num_indices );


	// These compute the ring-1 neighbors and store one value per ring-1 neighbor.  The value 
	//  stored is the vertex index as read out of the pIndices array.
	HRESULT ComputeRing1NeighborInfo( VIND_TYPE * pIndices, UINT num_indices, UINT num_vertices );
	// Same function as above, but with no input for num_vertices.  This will take more time, because
	// it must compute num_vertices by scanning the index array.
	HRESULT ComputeRing1NeighborInfo( VIND_TYPE * pIndices, UINT num_indices );

	// These functions generate the ring-1 neighbor data that ComputeRing1NeighborInfo(..) does, and
	// they also store each neighbor's forward winding and backward winding neighbors.  These neighbors
	// are the ring-1 neighbors next to the given ring-1 neighbor stored in
	// m_pNeighborsWithWinding->m_uRing1Neighbor
	// The data is somewhat reduntant, but it is in a form that allows for fast calculation of smooth
	// vertex normals based on the ring-1 neighbor info.
	// In the case where a ring-1 neighbor has more than one neighbor in a given winding direction, like
	// two neighbors in the forward winding direction, another entry is created in m_pNeighborsWithWinding
	// that ring-1 neighbor.
	HRESULT ComputeRing1NeighborInfoWithWinding( VIND_TYPE * pIndices, UINT num_indices, UINT num_vertices );
	HRESULT ComputeRing1NeighborInfoWithWinding( VIND_TYPE * pIndices, UINT num_indices );

	//---- functions to access the data computed by this class -----------
	UINT	GetNumVertices();
	UINT	GetNumNeighbors( VIND_TYPE vertex );
	void	GetNeighbor( VIND_TYPE in_vertex, VIND_TYPE in_n_neighbor,
						VIND_TYPE & out_neighbor, VIND_TYPE & out_fwd_neighbor, VIND_TYPE & out_bck_neighbor, 
						int & out_has_fwd, int & out_has_bck );
	//--------------------------------------------------------------------

	bool	m_bReportTempMemoryUsed;		// true to report temporary memory used in the computations
	bool	m_bReportDataMemory;			// true to report amount of storage for the neighbor info after finishing a calc

	void	ListDataMemoryUsed();
	void	ListRing1NeighborInfo( UINT start_vert, UINT end_vert );
	bool	IsNeighborsWithWindingInfoValid();
	bool	IsNeighborsInfoValid();

	GeoIndexRing1Neighbors();
	~GeoIndexRing1Neighbors();
	void	SetAllNull()
	{
		m_pRing1Neighbors		= NULL;
		m_uRing1NeighborsSize	= 0;

		m_pNeighborsWithWinding	= NULL;
		m_uNeighborsWithWindingSize = 0;

		m_pRing1NeighborsStart	= NULL;
		m_pRing1NeighborsNumber	= NULL;
		m_uRing1NeighborsStartSize	= 0;
	}
	HRESULT Free();
};
	
#endif


