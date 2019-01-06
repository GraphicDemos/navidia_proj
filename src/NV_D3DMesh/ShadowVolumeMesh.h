/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  SDK\LIBS\src\NV_D3DMesh\
File:  ShadowVolumeMesh.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A class for building shadow volume geometry compatible with a vertex shader
program for extruding the volume.

This class builds a closed hull with potential zero-area triangles so that
the shadow volume extrusion will work properly.

The input object must be a closed hull with no T-junctions.

Creating the object calls WeldFacetedFaces() which will re-calculate vertex
normals if any faces were welded into the same "smoothing group."  To avoid this
recalculation, pass a threshold greater than 1.0f and no faces will be welded,
however a great deal of zero-area triangles will be created.

For hardware rendering, create a ShadowVolumeMeshVB object from this object.


Revision history:
1/6/2004 - Adapted from ShadowVolumeObject class.

-------------------------------------------------------------------------------|--------------------*/



#ifndef H_NV_SHADOWVOLUMEMESH_H
#define H_NV_SHADOWVOLUMEMESH_H

#include "NV_D3DMesh\WingedEdgeMesh.h"


class DECLSPEC_NV_D3D_MESH_API ShadowVolumeMesh : public WingedEdgeMesh
{
private:
	// When drawing model use tris     0 to m_nOriginalModelTriIndices
	// When drawing shadow volume, use 0 to m_nLastTriIndex

	// Original model tris are from indices 0
	//  to m_nOriginalModelIndices
	UINT	m_wOriginalModelTriIndices;

	// zero-area tris for extruding shadow volume
	//   are from indices m_nOriginalModelTriIndices to
	//   m_nLastTriIndex;
	UINT	m_wLastTriIndex;

public:

	// Will weld & not create zero-area triangles between
	//   faces whos normals are similar.  If the dot product
	//   of adjacent face normals is greater than weld_edge_threshold
	//   then no zero-area tris will be created there.  Instead,
	//   the tris will be guaranteed to use the same vertices so 
	//   that no breaking of the closed hull occurs when the 
	//   shadow volume extruding vertex shader is used.

	HRESULT		BuildShadowVolume( float weld_edge_threshold );

	HRESULT		BuildShadowVolume_CloseHoles( float weld_edge_threshold );


	virtual HRESULT Free();
	
	virtual void ExplodeFaces( float distance );	// move faces out along vector from
													//  center of object

	void	GetTriCounts( UINT * pNumTotalTris, UINT * pNumWithoutShadowVolTris );


	ShadowVolumeMesh();
	virtual ~ShadowVolumeMesh();
};




#endif		// H_NV_SHADOWVOLUMEMESH_H