/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TextureFactory.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the header for various coments about the functions


-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DCommonDX9PCH.h"

TextureFactory::TextureFactory()
{
	m_bAlwaysReportErrors				= true;
	m_bTrackFilenamesAndDontDuplicate	= true;
	m_bUsePoolManaged					= true;
	m_uTextureMemoryInBytes				= 0;
	m_uOtherMemoryInBytes				= 0;
	SetAllNull();
}

TextureFactory::~TextureFactory()
{
	Free();
}

void TextureFactory::SetAllNull()
{
	m_pGetFilePathFunction	= NULL;
}

HRESULT TextureFactory::Free()
{
	HRESULT hr = S_OK;
	UINT i;
	// Release surfaces
	IDirect3DSurface9 ** ppSurf;
	for( i=0; i < m_vppTFSurfacePtrToRelease.size(); i++ )
	{
		ppSurf = m_vppTFSurfacePtrToRelease.at(i);

		if( ppSurf != NULL )
		{
			SAFE_RELEASE( *ppSurf );
			m_vppTFSurfacePtrToRelease.at(i) = NULL;
		}
	}
	// delete pointers
	for( i=0; i < m_vppTFSurfacePtrToDelete.size(); i++ )
	{
		ppSurf = m_vppTFSurfacePtrToDelete.at(i);

		if( ppSurf != NULL )
		{
			SAFE_RELEASE( *ppSurf );
			SAFE_DELETE( ppSurf );
			m_vppTFSurfacePtrToDelete.at(i) = NULL;
		}
	}

	IDirect3DTexture9 ** ppTex;
	// Release textures we might have been given
	// Don't delete pointers
	for( i=0; i < m_vppTFTexToRelease.size(); i++ )
	{
		ppTex = m_vppTFTexToRelease.at(i);

		if( ppTex != NULL )
		{
			// release the device's texture
			SAFE_RELEASE( *ppTex );
			m_vppTFTexToRelease.at(i) = NULL;
		}
	}

	// release textures and delete pointers
	for( i=0; i < m_vppFVTexToFree.size(); i++ )
	{
		ppTex = m_vppFVTexToFree.at(i);

		if( ppTex != NULL )
		{
			// release the device's texture
			SAFE_RELEASE( *ppTex );
			// delete the pointer
			SAFE_DELETE( ppTex );
			m_vppFVTexToFree.at(i) = NULL;
		}
	}
	m_vppFVTexToFree.clear();
	m_uTextureMemoryInBytes				= 0;
	m_uOtherMemoryInBytes				= 0;
	return( hr );
}

HRESULT TextureFactory::Initialize( GetFilePath::GetFilePathFunction file_path_callback )
{
	HRESULT hr = S_OK;
	hr = Free();	
	m_pGetFilePathFunction = file_path_callback;
	return( hr );
}

tstring TextureFactory::GetFilePath( const TCHAR * filename, bool bVerbose )
{	
	// Uses m_pGetFilePathCallback to try to find the file
	if( m_pGetFilePathFunction != NULL )
	{
		return( (*m_pGetFilePathFunction)( filename, bVerbose ) );
	}
	return( filename );
}

void	TextureFactory::LowerTexAllocationAmount( IDirect3DTexture9 * pTex )
{
	AddTexAllocationAmount( pTex, -1 );
}

void	TextureFactory::AddTexAllocationAmount( IDirect3DTexture9 * pTex, int sign )
{
/*//@@@ AddTexAllocationAmount not implemented
	RET_IF( pTex == NULL );
	int size;
	D3DSURFACE_DESC desc;
	int bytes_per_pixel;
	bytes_per_pixel = GetSizeOfFormatInBytes( desc.Format );
	DWORD dwNumLev;
	dwNumLev = pTex->GetLevelCount();
	DWORD i;
	for( i=0; i < dwNumLev; i++ )
	{
		pTex->GetLevelDesc( i, &desc );
	}

	m_uTextureMemoryInBytes += size * sign;
*/
}

void	TextureFactory::AddAllocationAmount( UINT uSizeInBytes, int sign )
{
	m_uOtherMemoryInBytes += (int)uSizeInBytes * sign;
}

HRESULT TextureFactory::FreeTexture( IDirect3DTexture9 ** ppTexToFree )
{
	HRESULT hr = S_OK;

	// Free the texture and pointer, but do not reduce the size of the 
	//  array of texture pointers to free.  This saves having to traverse the array
	//@ Could track the number of freed textures and if it gets high enough or 
	// if the array gets large enough, you could search it and reduce it's size.
	//@ Does not even check if we allocated the texture.  This saves the time
	//  that a search would require.

	if( ppTexToFree != NULL )
	{
		// release the device texture
		SAFE_RELEASE( *ppTexToFree );

		// Do not delete the pointer.  If we did, the array of texture handles
		//  would point to memory which is no longer valid.  To fix this, we'd
		//  have to search the array and remove the bad handle.  
		// Instead, SAFE_RELEASE sets the pointer value to NULL.  We can garbage
		//  collect in the array later by removing handles to NULL pointers.
		//
		// There is no need to search and eliminate entries from the texture + filename
		//  pairs, since the querry for if a texture is loaded will return NULL if
		//  the texture pointer is NULL.
	}
	return( hr );
}

HRESULT TextureFactory::FreeSurface( IDirect3DSurface9 ** ppSurfaceToFree )
{
	// See comments above in FreeTexture
	HRESULT hr = S_OK;
	if( ppSurfaceToFree != NULL )
	{
		SAFE_RELEASE( *ppSurfaceToFree );
	}
	return( hr );
}

HRESULT TextureFactory::GarbageCollect()
{
	// Delete NULL pointers and reduce size of arrays tracking
	//  the pointers
	FDebug("TextureFactory::GarbageCollect() not implemented!\n");
	return( E_FAIL );
}

HRESULT TextureFactory::AddTexturePtrToRelease( IDirect3DTexture9 ** ppTexToRelease )
{
	// Add a texture to be released
	// The texture pointer will not be deleted
	HRESULT hr = S_OK;
	m_vppTFTexToRelease.push_back( ppTexToRelease );
	return( hr );
}
// Add a texture and texture pointer to cleanup.  On cleanup, the texture will be released
// and the pointer will be deleted.
HRESULT TextureFactory::AddTexturePtrToDelete( IDirect3DTexture9 ** ppTexToDelete )
{
	// The texture pointer will be released, then deleted

	HRESULT hr = S_OK;
	m_vppFVTexToFree.push_back( ppTexToDelete );
	return( hr );
}

// Add a surface to release on cleanup.
HRESULT TextureFactory::AddSurfacePtrToRelease( IDirect3DSurface9 ** ppSurfPtrToRelease )
{
	HRESULT hr = S_OK;
	m_vppTFSurfacePtrToRelease.push_back( ppSurfPtrToRelease );
	return( hr );
}

// Add a pointer.  On cleanup, the surface will be released and the pointer will be deleted.
HRESULT TextureFactory::AddSurfacePtrToDelete( IDirect3DSurface9 ** ppSurfPtrToDelete )
{
	HRESULT hr = S_OK;
	m_vppTFSurfacePtrToDelete.push_back( ppSurfPtrToDelete );
	return( hr );
}

// The calling app does not need to release the surface or delete the pointer.
IDirect3DSurface9 ** TextureFactory::GetTextureSurface(	IDirect3DTexture9 * pInTexture,
															UINT level )
{
	HRESULT hr = S_OK;
	if( pInTexture == NULL )
	{
		FMsg("TextureFactory::GetTextureSurface() input pInTexture = NULL!\n");
		return( NULL );
	}
	IDirect3DSurface9 ** ppSurf = new (IDirect3DSurface9*);
	if( ppSurf == NULL )
	{
		FMsg("TextureFactory::GetTextureSurface() Couldn't create surface pointer!\n");
		return( NULL );
	}
	hr = pInTexture->GetSurfaceLevel( level, ppSurf );
	if( FAILED(hr) )
	{
		FMsg("TextureFactory::GetTextureSurface() GetSurfaceLevel() failed!\n");
		return( NULL );
	}
	hr = AddSurfacePtrToDelete( ppSurf );
	return( ppSurf );
}


IDirect3DTexture9 ** TextureFactory::CreateTexture( IDirect3DDevice9 * pDev,
														UINT Width,
														UINT Height,
														UINT Levels,
														DWORD Usage,
														D3DFORMAT Format,
														D3DPOOL Pool		)
{
	RET_NULL_IF_NULL( pDev );
	IDirect3DTexture9 ** ppTex;
	ppTex = new IDirect3DTexture9*;
	RET_NULL_IF_NULL( ppTex );
	*ppTex = NULL;
	HRESULT hr;
	hr = pDev->CreateTexture( Width, Height, Levels, Usage, Format, Pool, ppTex, NULL );

	if( FAILED(hr) )
	{
		FMsg("TextureFactory::CreateTexture(..) failed!\n");
		assert( false );
		SAFE_DELETE( ppTex );
		ppTex = NULL;
	}
	else
	{
		AddTexturePtrToDelete( ppTex );
	}

	return( ppTex );
}

//---------------------------------------------------------------------------
// Returns NULL if texture has not been loaded for the device, otherwise it
// returns the address of the texture handle and filename pair structure.
// For now, this does not use any fancy data structures to speed the handling
//  and searching a large set of textures.
//---------------------------------------------------------------------------
TextureFilenamePair * TextureFactory::IsTextureLoaded( IDirect3DDevice9 * pDev, LPCTSTR pSrcFile )
{
	RET_VAL_IF( pDev == NULL, NULL );
	RET_VAL_IF( pSrcFile == NULL, NULL );
	TextureFilenamePair * pPair = NULL;

	size_t i;
	int cmp;
	for( i=0; i < m_vTextureFilenamePairs.size(); i++ )
	{
		cmp = _tcscmp( pSrcFile, m_vTextureFilenamePairs.at(i).m_tsFilename.c_str() );
		if( cmp == 0 )		// if strings identical
		{
			IDirect3DTexture9 ** ppTex;
			ppTex = m_vTextureFilenamePairs.at(i).m_ppTexture;
			if( ppTex != NULL )
			{
				IDirect3DTexture9 * pTex;
				pTex = *ppTex;
				if( pTex != NULL )
				{
					IDirect3DDevice9 * pTexDev;
					pTex->GetDevice( &pTexDev );
					if( pTexDev == pDev )
					{
						SAFE_RELEASE( pTexDev );
						return( & (m_vTextureFilenamePairs.at(i)) );
					}
					SAFE_RELEASE( pTexDev );
				}
			}
		}
	}
	return( pPair );
}

//---------------------------------------------------------------------------
// Add texture handle and associated filename.
// For now, this does not use any fancy data structures to speed the handling
//  and searching a large set of textures.
//---------------------------------------------------------------------------
void	TextureFactory::AddTextureFilenamePair( IDirect3DTexture9 ** ppTex, LPCTSTR pFilename )
{
	TextureFilenamePair pair;
	pair.m_ppTexture = ppTex;
	pair.m_tsFilename = pFilename;
	m_vTextureFilenamePairs.push_back( pair );
}

//---------------------------------------------------------------------------
// Returns NULL if texture could not be created.
// Textures created in POOL_DEFAULT
//---------------------------------------------------------------------------
IDirect3DTexture9 ** TextureFactory::CreateTextureFromFile( IDirect3DDevice9 * pDev,
															  LPCTSTR pSrcFile,
															  bool bVerbose )
{
	HRESULT hr = S_OK;
	RET_NULL_IF_NULL( pDev );
	RET_NULL_IF_NULL( pSrcFile );
	RET_VAL_IF( _tcslen(pSrcFile) == 0, NULL );
	IDirect3DTexture9 * pTex;
	
	// Do not load the texture again if it is already loaded.
	if( m_bTrackFilenamesAndDontDuplicate )
	{
		TextureFilenamePair * pPair;
		pPair = IsTextureLoaded( pDev, pSrcFile );
		if( pPair != NULL )
		{
			if( bVerbose )
				FMsg(TEXT("TextureFactory::CreateTextureFromFile texture is already loaded, not reloading : %s\n"), pSrcFile );
			return( pPair->m_ppTexture );
		}
	}

	tstring filepath;
	filepath = GetFilePath( pSrcFile, bVerbose ).c_str();
	if( m_bUsePoolManaged )
		hr = D3DXCreateTextureFromFile( pDev, filepath.c_str(), &pTex );
	else
		hr = D3DXCreateTextureFromFileEx( pDev, filepath.c_str(), 
				D3DX_DEFAULT, D3DX_DEFAULT,	D3DX_DEFAULT,		// width, height, mip 
				0, D3DFMT_UNKNOWN,
				D3DPOOL_DEFAULT,
				D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,		// filter, mipfilter, colorkey, src info, palette
				&pTex );

	if( FAILED(hr) && ( bVerbose || m_bAlwaysReportErrors) )
	{
		FMsg(TEXT("TextureFactory:: D3DXCreateTextureFromFile() failed.  File : %s\n"), filepath.c_str() );
	}

	IDirect3DTexture9 ** ppTex = NULL;
	if( SUCCEEDED(hr) )
	{
		ppTex = new IDirect3DTexture9*;
		RET_NULL_IF_NULL( ppTex );
		*ppTex = pTex;
		AddTexturePtrToDelete( ppTex );
		if( m_bTrackFilenamesAndDontDuplicate )
		{
			// do not track the full file path filepath.c_str() );
			AddTextureFilenamePair( ppTex, pSrcFile );
		}
	}
	return( ppTex );
}

// Textures created in POOL_DEFAULT
HRESULT	TextureFactory::CreateTextureFromFile(	IDirect3DDevice9 * pDev,
												LPCTSTR pSrcFile,
												IDirect3DTexture9 ** ppTexture )
{
	FAIL_IF_NULL( ppTexture );
	FAIL_IF_NULL( pDev );
	FAIL_IF_NULL( pSrcFile );
	HRESULT hr = S_OK;

	FMsg("TextureFactory::CreateTextureFromFile( <3> ) is deprecated, use the func with only 2 args!\n");

	tstring filepath;
	filepath = GetFilePath( pSrcFile ).c_str();

	if( m_bUsePoolManaged )
		hr = D3DXCreateTextureFromFile( pDev, filepath.c_str(), ppTexture );
	else
		hr = D3DXCreateTextureFromFileEx( pDev, 
			filepath.c_str(), 
			D3DX_DEFAULT, D3DX_DEFAULT,	D3DX_DEFAULT,		// width, height, mip 
			0, D3DFMT_UNKNOWN,
			D3DPOOL_DEFAULT,
			D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,		// filter, mipfilter, colorkey, src info, palette
			ppTexture );

	if( FAILED(hr) && m_bAlwaysReportErrors )
	{
		FMsg(TEXT("TextureFactory D3DXCreateTextureFromFile() failed.  File : %s\n"), filepath.c_str() );
	}
	AddTexturePtrToRelease( ppTexture );
	return( hr );
}

// Apps using this class the right way (keeping pointers to the pointer, and not
// copying the pointer itself), will have the new texture with no problems.
HRESULT TextureFactory::ReloadTexturesFromDisk( bool bVerbose )
{
	HRESULT hr = S_OK;
	hr = E_FAIL;
	FMsg(TEXT("%s not implemented\n"), __FUNCTION__ );

	IDirect3DTexture9 ** ppTex;
	size_t i;
	for( i=0; i < m_vTextureFilenamePairs.size(); i++ )
	{
		ppTex = m_vTextureFilenamePairs.at(i).m_ppTexture;
		if( ppTex == NULL )
			continue;			// next iter of for() loop
		if( *ppTex == NULL )
			continue;
		
		IDirect3DDevice9 * pDev;
		(*ppTex)->GetDevice( &pDev );
		if( pDev == NULL )
			continue;

		(*ppTex)->Release();

		tstring filepath;
		filepath = GetFilePath( m_vTextureFilenamePairs.at(i).m_tsFilename.c_str() );
		// The pointer *ppTex will be updated to point to the newly loaded texture.
		// Apps using this class the right way (keeping pointers to the pointer, and not
		// copying the pointer itself), will have the new texture with no problems.

		if( m_bUsePoolManaged )
			hr = D3DXCreateTextureFromFile( pDev, filepath.c_str(), ppTex );
		else
			hr = D3DXCreateTextureFromFileEx( pDev, 
				filepath.c_str(), 
				D3DX_DEFAULT, D3DX_DEFAULT,	D3DX_DEFAULT,		// width, height, mip 
				0, D3DFMT_UNKNOWN,
				D3DPOOL_DEFAULT,
				D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,		// filter, mipfilter, colorkey, src info, palette
				ppTex );

		SAFE_RELEASE( pDev );
		if( bVerbose )
		{
			tstring tstr;
			tstrPrintf( TEXT("%s failed to load: %s\n"), TEXT(__FUNCTION__), filepath.c_str() );
			MSG_AND_RET_VAL_IF( FAILED(hr), tstr.c_str(), E_FAIL );
			FMsg( TEXT("%s reloaded file: %s\n"), TEXT(__FUNCTION__), filepath.c_str() );
		}
	}
	return( hr );
}


