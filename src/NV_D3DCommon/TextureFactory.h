/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TextureFactory.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Class to encapsulate texture creation, loading, and clean up.
Mostly, this saves the work of having to track texture pointers and free them
on exit.

The class is designed to handle large numbers of textures, so freeing a texture
is fast and doesn't require a search through all managed pointers.  The texture is
freed, the pointer is set to NULL and left for later garbage collection --
meaning the pointer itself is not deleted and other refs to it are not deleted.


5/29/2003 - Removed D3D device pointer.  It's not needed

-------------------------------------------------------------------------------|--------------------*/

#ifndef NV_H_TEXTUREFACTORY_H
#define NV_H_TEXTUREFACTORY_H

#pragma warning(disable: 4786)
#include <vector>
#include <string>
using namespace std;
#include "shared\GetFilePath.h"

#include "NV_D3DCommon_decl_.h"
class DECLSPEC_NV_D3D_COMMON_API TextureFilenamePair
{
public:
	IDirect3DTexture9 **	m_ppTexture;
	tstring					m_tsFilename;
};


class DECLSPEC_NV_D3D_COMMON_API TextureFactory
{
public:
	bool	m_bAlwaysReportErrors;
	bool	m_bTrackFilenamesAndDontDuplicate;		// store filenames in TextureFilenamePairs
	bool	m_bUsePoolManaged;

	// Main class interface functions
	virtual HRESULT Initialize( GetFilePath::GetFilePathFunction file_path_callback = NULL );
	virtual HRESULT Free();

	// There is no need to release the pointer or delete the pointer.
	// It it tracked by this class and released on Free()
	IDirect3DSurface9 ** GetTextureSurface(  IDirect3DTexture9 * pInTexture,
												UINT level );

	IDirect3DTexture9 ** CreateTexture( IDirect3DDevice9 * pD3DDev,
											UINT Width,
											UINT Height,
											UINT Levels,
											DWORD Usage,
											D3DFORMAT Format,
											D3DPOOL Pool		);

	IDirect3DTexture9 ** CreateTextureFromFile( IDirect3DDevice9 * pD3DDev,
													LPCTSTR pSrcFile,
													bool bVerbose = false );

	// just like D3DXCreateTextureFromFile() //@@@ DEPRECATED
	HRESULT	CreateTextureFromFile(	IDirect3DDevice9 * pD3DDev,
									LPCTSTR pSrcFile,
									IDirect3DTexture9 ** ppTexture );

	HRESULT ReloadTexturesFromDisk( bool bVerbose = false );

public:
	TextureFactory();
	virtual ~TextureFactory();

	// Uses m_pGetFilePathCallback to try to find the file
	tstring			GetFilePath( const TCHAR * filename, bool bVerbose = false );

	// Releases any texture, whether this class created it or not
	// This does not delete the texture pointer, but sets it to NULL for later garbage collection.
	HRESULT FreeTexture( IDirect3DTexture9 ** ppTexToFree );
	HRESULT FreeSurface( IDirect3DSurface9 ** ppSurfaceToFree );
	// Delete NULL pointers and reduce size of arrays tracking the pointers
	HRESULT GarbageCollect();

	// Functions so you can create your own textures and have them cleaned
	//  up by this class.  There is no need to call these if you create the
	//  textures using this class's functions.
		// Add a texture to be released
		// The texture pointer will not be deleted
	HRESULT AddTexturePtrToRelease( IDirect3DTexture9 ** ppTexToRelease );
		// The texture pointer will be released, then deleted
	HRESULT AddTexturePtrToDelete( IDirect3DTexture9 ** ppTexToDelete );
	HRESULT AddSurfacePtrToRelease( IDirect3DSurface9 ** ppSurfPtrToRelease );
	HRESULT AddSurfacePtrToDelete( IDirect3DSurface9 ** ppSurfPtrToDelete );

	void	AddTextureFilenamePair( IDirect3DTexture9 ** ppTex, LPCTSTR pFilename );
	TextureFilenamePair * IsTextureLoaded( IDirect3DDevice9 * pD3DDev, LPCTSTR pSrcFile );

protected:
	UINT	m_uTextureMemoryInBytes;
	UINT	m_uOtherMemoryInBytes;

	// pointers to texture pointers to free on exit
	vector< IDirect3DTexture9 ** >  m_vppFVTexToFree;			// ptrs are released, then deleted
	vector< IDirect3DTexture9 ** >  m_vppTFTexToRelease;		// ptrs are just released, not deleted
	vector< TextureFilenamePair >     m_vTextureFilenamePairs;	// 

	vector< IDirect3DSurface9 ** >  m_vppTFSurfacePtrToDelete;	// ptrs are released, then deleted
	vector< IDirect3DSurface9 ** >  m_vppTFSurfacePtrToRelease;	// ptrs are released

	// Function that finds the full path of a given filename, 
	// so apps can pass in their own file finding routines
	GetFilePath::GetFilePathFunction	m_pGetFilePathFunction;

	virtual void	SetAllNull();
	void LowerTexAllocationAmount( IDirect3DTexture9 * pTex );
	void AddTexAllocationAmount( IDirect3DTexture9 * pTex, int sign = 1 );
	void AddAllocationAmount( UINT uSizeInBytes, int sign = 1 );
};


#endif			// NV_H_TEXTUREFACTORY_H
