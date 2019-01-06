/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  ShadowVolumeMeshVB.h

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

#ifndef H_SHADOWVOLUMEMESH_VB_H
#define H_SHADOWVOLUMEMESH_VB_H


#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "NV_D3DMesh\MeshVB.h"



class DECLSPEC_NV_D3D_MESH_API ShadowVolumeMeshVB : public MeshVB
{
private:
	UINT	m_wLastTriIndex;				// last tri of all geometry
											//   including zero-area stitches tris
	UINT	m_wOriginalModelTriIndices;		// last tri of original model data
											//   No zero-area tris
public:

	HRESULT DrawIndexRange( UINT start, UINT end );
	HRESULT DrawBaseModel();
	HRESULT DrawZeroAreaTris();
	HRESULT DrawAllTris();

	// You should call ShadowVolumeMesh::BuildShadowVolume() to create the 
	// proper data before calling this function.
	HRESULT	CreateFromShadowVolumeMesh( ShadowVolumeMesh * pObj, IDirect3DDevice9* pDev );


	ShadowVolumeMeshVB();
	~ShadowVolumeMeshVB();
};



#endif

