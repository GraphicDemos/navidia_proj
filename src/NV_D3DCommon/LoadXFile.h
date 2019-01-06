/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  LoadXFile.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Eventually, this class may become more general and use various features of .x files, but for now it is very limited.

Microsoft has a simple .x file loader in the Direct3D Summer 2003 Update folder:
DX90SDK\Samples\C++\Direct3D\Tutorials\Tut06_Meshes

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NVLOADXFILE_H
#define H_NVLOADXFILE_H

#include <string>
#include "shared\GetFilePath.h"
#include "NV_D3DCommon\NV_D3DCommonTypes.h"

#include "NV_D3DCommon_decl_.h"
#define DEFAULT_TEXFILENAME		"white_8x8.jpg"


class DECLSPEC_NV_D3D_COMMON_API LoadXFile
{
public:
	enum LoadXFileFlags
	{
		KEEP_ASPECT_RATIO,			// keep AABB aspect ratio when scaling
		NO_FLAGS
	};

public:
	IDirect3DDevice9	*	m_pD3DDev;
	bool					m_bWireframe;

	LPD3DXMESH				m_pMesh;
	D3DMATERIAL9 *			m_pMeshMaterials;		// array of materials
	DWORD					m_dwNumMaterials;		// size of m_pMeshMaterials array
	IDirect3DTexture9 ***	m_Textures;				// array of texture **
													// size is also m_dwNumMaterials
	D3DXVECTOR3				m_AABBMin;
	D3DXVECTOR3				m_AABBMax;
	bool					m_bAABBValid;
	D3DXMATRIX				m_matWorld;

	tstring					m_strDefaultTexFilename;	// texture to load when no texture can be found

	// main interface functions
	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev, 
						GetFilePath::GetFilePathFunction pGetFilePathFunction = NULL,
						TextureFactory ** ppTextureFactory = NULL );

	HRESULT FreeLoadedData();
	HRESULT LoadFile( const TCHAR * in_pFilePath,  bool bVerbose = false );

	LPD3DXMESH	GetMesh()	{ return( m_pMesh ); };
	HRESULT	GetMatrixToXFormMesh( D3DXMATRIX * out_pMat, const D3DXVECTOR3 & aabb_center, 
									const D3DXVECTOR3 & aabb_size, const LoadXFileFlags & flags = NO_FLAGS );
	HRESULT SetMatrixToXFormMesh( const D3DXVECTOR3 & aabb_center, const D3DXVECTOR3 & aabb_size, 
									const LoadXFileFlags & flags = NO_FLAGS );
	D3DXMATRIX * GetMatrixP();

	HRESULT RenderSections( DWORD start_section, DWORD end_section,  bool bSetTexture = true, bool bSetMaterial = true );
	HRESULT	Render( bool bSetTexture = true, bool bSetMaterial = true);

	bool	ToggleWireframe();			// returns the value of wireframe after toggling

	// secondary interface functions
	TextureFactory ** GetTextureFactoryPP();
	HRESULT		LoadMaterials( const LPD3DXBUFFER pD3DXMtrlBuffer, DWORD dwNumMaterials, bool bVerbose = false );
	HRESULT		ListMeshInfo();
	HRESULT		LoadTexture( LPCTSTR pFilename, IDirect3DTexture9 *** out_pppTexture, bool bVerbose = false );

	HRESULT		GetMeshAABB( LPD3DXMESH in_pMesh, D3DXVECTOR3 * out_pMinPoint, D3DXVECTOR3 * out_pMaxPoint, bool bVerbose = false );

	LoadXFile();
	~LoadXFile();

protected:	
	GetFilePath::GetFilePathFunction	m_GetFilePathFunction;
	tstring		GetFilePath( const TCHAR * in_pFilePath );

	TextureFactory *		m_pTextureFactory;		// NULL unless TextureFactory is allocated by this class
	TextureFactory **		m_ppTextureFactory;		// points to m_pTextureFactory or another pointer
	TextureFactory *		GetTextureFactory();

	virtual void SetAllNull()
	{
		m_pD3DDev			= NULL;
		m_pMesh				= NULL;
		m_pMeshMaterials	= NULL;
		m_dwNumMaterials	= 0;
		m_Textures			= NULL;
		m_ppTextureFactory	= NULL;
		m_pTextureFactory	= NULL;

		m_bWireframe			= false;
		m_GetFilePathFunction	= NULL;

		m_AABBMin			= D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_AABBMax			= D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bAABBValid		= false;

		m_strDefaultTexFilename = TEXT(DEFAULT_TEXFILENAME);
		D3DXMatrixIdentity( &m_matWorld );
	}
};

#endif				// H_NVLOADXFILE_H
