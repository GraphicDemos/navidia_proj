/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  NormalsCalculator.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A helper class to calculate vertex normals efficiently using ring-1 neighbor info with winding.

This class uses a specialized method to calculate normals for mesh geometry when vertices are
moved by some transform not described by a matrix.  The calculation depends on the mesh vertex
positions.  The MeshProcessor class has code to calculate smooth vertex normals by summing face
normals.  The method used here in NormalsCalculator is equivalent to that method, except that here
the triangle perpendicular vectors are not normalized before being added together.  This means 
that each triangle's face normal is weighted by the triangle area.  Triangles will lower area 
will contribute less to the smooth face normal at each vertex.  The lack of normalization allows
the math for calculating and summing perpendicular vectors to be simplified using algebra.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NORMALSCALCULATOR_GJ_H
#define H_NORMALSCALCULATOR_GJ_H

#include <windows.h>

#include "NV_D3DMesh\NV_D3DMeshTypes.h"


class NormalsCalculator
{
public:
	HRESULT	CalculateNormals( Mesh * pMesh, GeoIndexRing1Neighbors *pNeighborInfo );
	HRESULT CalculateNormals( Mesh * pMesh, GeoIndexRing1Neighbors * pNeighborInfo, D3DXVECTOR3 * pPos );

	// Compares Mesh1's vertex normals to Mesh2's by computing the dotproduct between each normal.
	//  Mesh1->vert[n] is compared to Mesh2->vert[n], and the min, max, and average of all the dotproducts
	//  for n=0 to the number of vertices is output.
	void	CompareNormals( Mesh * pMesh1, Mesh * pMesh2,
							UINT & out_nvert_compared,
							float & out_max_dp3, float & out_min_dp3, float & out_avg_dp3,
							bool bVerbose );

	NormalsCalculator();
	~NormalsCalculator();
};


#endif


