/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DMesh\
File:  NV_D3DMesh.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
NV_D3DMesh Library generic header.  
This library replaces the old SimpleObject, SimpleVBObject, and related classes that were in NV_D3DCommon

This is the main header for the NV_D3DMesh library for DX9.  Include this in your own code that
uses the NV_D3DMesh library.  It will automaticaly add the NV_D3DMeshDX9.lib to your linker
inputs using #pragma comment( lib, "NV_D3DMeshDX9.lib" )

You can use NV_D3DMeshTypes.h in your headers so that only the types are defined.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NVD3DMESH_LIB_H
#define H_NVD3DMESH_LIB_H

// Include headers for all library modules
#include "NV_D3DMesh\GeoIndexRing1Neighbors.h"
#include "NV_D3DMesh\LoadPLYFile.h"
#include "NV_D3DMesh\Mesh.h"
#include "NV_D3DMesh\MeshBeingProcessed.h"
#include "NV_D3DMesh\MeshVB.h"
#include "NV_D3DMesh\MeshVertex.h"
#include "NV_D3DMesh\MeshVBDot3.h"
#include "NV_D3DMesh\MeshVertexDot3.h"
#include "NV_D3DMesh\MeshVBUtils.h"
#include "NV_D3DMesh\MeshVertex.h"
#include "NV_D3DMesh\MeshGeoCreator.h"
#include "NV_D3DMesh\MeshProcessor.h"
#include "NV_D3DMesh\MeshSectionStitcher.h"
#include "NV_D3DMesh\NormalsCalculator.h"
#include "NV_D3DMesh\QuadVB.h"
#include "NV_D3DMesh\QuadVBVertex.h"
#include "NV_D3Dmesh\ShadowVolumemesh.h"
#include "NV_D3DMesh\ShadowVolumeMeshVB.h"
#include "NV_D3DMesh\TangentSpaceUtils.h"
#include "NV_D3DMesh\WingedEdgeMesh.h"

#include "NV_D3DMesh\NV_D3DMeshLibSelector.h"

#endif		// H_NVD3DMESH_H
