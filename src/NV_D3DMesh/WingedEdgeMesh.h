/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  WingedEdgeMesh.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Code for generating a winged-edge type structure for geometry.  Edges are not 
tracked, rather, each triangle has up to three neighbors and params to describe
the connections to neighboring faces.

This is a hastily implemented class to get some geometry working for stencil shadow volume
rendering.

Creation of the winged-edge data will fail if an edge is shared between more than
two triangles.  

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_WINGEDEDGEMESH_H
#define H_WINGEDEDGEMESH_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "NV_D3DMesh\Mesh.h"

#include <vector>

typedef UINT		WELD_VALUE;

#define NOT_WELDED			0
#define IS_WELDED			1
#define NOT_WELDED_STITCHED	2
#define NO_NEIGHBOR			65535			//@@ revise


//--------------------------------------------

class WingedEdgeMesh : public Mesh
{
public:
	struct CollapsedSetEntry
	{
		// Data member for 1 vertex
		// Used to create alternate mapping into smaller set of vertices
		//  when finding winged-edge connections

		UINT	IndexToOriginalVerts;	// Index into m_pVertices for the data
										//  so two verts originaly degenerate in position
										//  can point to the same vertex in m_pVertices

		UINT	CollapsedSetIndex;		// index value (array position) of the vertex
										//  in the collapsed set of vertices.
										// This is used to build a list of which 
										//  triangles each vertex is a part of
	};

	struct NeighborTriIndices
	{
		UINT		ind1;		// Index to first neighbor triangle
								//  This neighbor tri should be on the side
								//  formed by the first (v0) and second (v1) vertices
								//  of the root triangle to which this struct
								//  belongs
								// Max value is m_wNumInd / 3 // 3 indices per tri
		WELD_VALUE	welded1;	// Is the edge between v0, v1 and ind1 triangle
								//  degenerate?  Ie, Are v0, v1 shared by the neighbor
								//  triangle?
		UINT		ind2;		// Neighbor adjacent to the v1, v2 points
		WELD_VALUE	welded2;

		UINT		ind3;		// Neighbor adjacent to v2, v0 points
		WELD_VALUE	welded3;

		D3DXVECTOR3	face_normal;
	};


private:

	float	m_fPositionSqrdThreshold;		// for determining which vertices
											//  are degenerate in position

	bool	m_bWingedEdgeDataBuilt;

	CollapsedSetEntry	* m_pCollapsedVertexSet;	// each entry maps 1 m_pVertices vertex
													// to a location in the collapsed set
	UINT	m_wNumInCollapsedSet;

	UINT	* m_wTriVertexIndicesToCollapsedSet;

	NeighborTriIndices * m_pTriNeighbors;		// one NeighborTriIndices struct for
												// each tri in m_pIndices

	Mesh	* m_pWingedEdgeMarkersObject;	// geometry which marks the winged-edge
											// connections.


	HRESULT AllocateWingedEdgeData( UINT numverts, UINT numtris );

	void	BuildNonDegeneratePositions();
	void	BuildNonEdgeDegenerateTris();

	void	CompareTris( const std::vector< UINT > & set1,
					     const std::vector< UINT > & set2, 
						 std::vector< UINT > & result );


	void	Warning_OpenHull();			// warning message about having an open hull
	void	Warning_SoloEdge();			// An edge has < 2 tris shared

	void	SetToConstructionValues();

	void	RemoveUnusedVerts();		// eliminates unused verts after exploding tris
										//  and processing


public:

	bool			m_bMarkWeldedVertices;	// use only for diagnostic/debug

	bool			m_bOpenHull;
	bool			m_bSoloEdge;


	HRESULT			BuildWingedEdgeData();


							// Generate additional geometry in the
							//  m_pWingedEdgeMarkersObject to mark the 
							//  winged-edge topology connections.
							// Currently a crude visualization.
	Mesh **			GenerateWingedEdgeMarkers();

							// weld faces (collapse degenerate vertices
							//  and edges) if the dot product of the
							//  face normals is greater than threshold.
	void			WeldFacetedFaces( float threshold );


	virtual void	ReverseWinding();  

	
	virtual HRESULT Free();
	HRESULT			FreeTemporaryData();

	bool			IsWingedEdgeDataBuilt();

	WingedEdgeMesh();
	virtual ~WingedEdgeMesh();

	friend class ShadowVolumeMesh;
};




#endif		// H_WINGEDEDGEMESH_H