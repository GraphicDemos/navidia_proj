/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  LoadPLYFile.h

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

#ifndef H_LOADPLYFILE_H
#define H_LOADPLYFILE_H

#include <string>
#include "shared\GetFilePath.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "NV_D3DMesh\MeshVB.h"

class LoadPLYFile
{
public:
	enum LoadPLYFileFlags
	{
		KEEP_ASPECT_RATIO,			// keep AABB aspect ratio when scaling
		NO_FLAGS
	};

public:
	IDirect3DDevice9 *		m_pD3DDev;
	MeshVB *				m_pMeshVB;

	D3DXVECTOR3				m_AABBMin;
	D3DXVECTOR3				m_AABBMax;
	bool					m_bAABBValid;
	D3DXMATRIX				m_matWorld;

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pD3DDev,
						GetFilePath::GetFilePathFunction pGetFilePathFunction = NULL );

	HRESULT LoadFile( const TCHAR * in_pFilePath,  bool bVerbose = false,
						MeshVB::VBUsage dynamic_or_static = MeshVB::STATIC );

	HRESULT SetMatrixToXFormMesh( const D3DXVECTOR3 & aabb_center, 
									 const D3DXVECTOR3 & aabb_size,
									 const LoadPLYFileFlags & flags );
	HRESULT	GetMatrixToXFormMesh( D3DXMATRIX * out_pMat,
									const D3DXVECTOR3 & aabb_center, 
									const D3DXVECTOR3 & aabb_size,
									const LoadPLYFileFlags & flags );

	LoadPLYFile();
	~LoadPLYFile();
	void SetAllNull()
	{
		m_pD3DDev				= NULL;
		m_pMeshVB				= NULL;
		m_GetFilePathFunction	= NULL;
	};

protected:
	GetFilePath::GetFilePathFunction	m_GetFilePathFunction;
	tstring		GetFilePath( const TCHAR * in_pFilePath, bool bVerbose = false );

};

#endif				// H_LOADPLYFILE_H
