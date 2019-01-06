/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DMesh\
File:  NV_D3DMeshTypes.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Define types for the NV_D3DMesh library

-------------------------------------------------------------------------------|--------------------*/


#ifndef H_NVD3DMESHTYPES_H
#define H_NVD3DMESHTYPES_H

#include <d3d9.h>
#include <d3dx9.h>

class Mesh;
class MeshVertex;
class MeshVB;
class MeshVBDot3;
class MeshVertex;
class MeshVertexDecl;
class MeshVertexDot3;
class MeshVertexDot3Decl;
class MeshGeoCreator;
class MeshProcessor;
class MeshBeingProcessed;
class WingedEdgeMesh;
class ShadowVolumeMesh;
class ShadowVolumeMeshVB;
class QuadVB;
class QuadVBVertex;

class CalculateNormals;
class GeoIndexRing1Neighbors;

class LoadPLYFile;



#ifndef NVMESH_PI
	#define NVMESH_PI      3.1415926535897932384626433832795028841971693993751
#endif

typedef DWORD	VIND_TYPE;		// type used for vertex indices

#endif
