/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVBDot3.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
An object with vertex and index buffers.  Vertices are the MeshVertexDot3 type which has tangent
space basis vectors.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_MESHVBDOT3_H
#define H_MESHVBDOT3_H

#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "MeshVB.h"

class DECLSPEC_NV_D3D_MESH_API MeshVBDot3 : public MeshVB
{
public:
	virtual HRESULT CreateVertexDeclaration();
	virtual size_t	GetSizeOfVertexInBytes();
	virtual HRESULT UpdateVerticesFromMesh( Mesh * pObj );
public:
	MeshVBDot3();
	virtual ~MeshVBDot3();
};

#endif
