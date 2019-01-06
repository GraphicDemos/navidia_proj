/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TrackRenderTargetTextures.cpp

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

#include "NV_D3DCommonDX9PCH.h"
#include <shared/UtilityFunctions.h>

// Return NULL if the surface is not in the texture/surface pairs
TexAndSurf * TexAndSurfTracker::Find( const IDirect3DTexture9 * pTex )
{
	size_t i;
	for( i=0; i < m_vTexAndSurfs.size(); i++ )
	{
		if( m_vTexAndSurfs.at(i).m_pTexture == pTex )
		{
			return( & (m_vTexAndSurfs.at(i)) );
		}
	}
	return( NULL );
}

// Return NULL if the surface is not in the texture/surface pairs
TexAndSurf * TexAndSurfTracker::Find( const IDirect3DBaseTexture9 * pBaseTex )
{
	size_t i;
	for( i=0; i < m_vTexAndSurfs.size(); i++ )
	{
		if( m_vTexAndSurfs.at(i).m_pTexture == pBaseTex )
		{
			return( & (m_vTexAndSurfs.at(i)) );
		}
	}
	return( NULL );
}


// Return NULL if the surface is not in the texture/surface pairs
TexAndSurf * TexAndSurfTracker::Find( const IDirect3DSurface9 * pSurf )
{
	size_t i;
	for( i=0; i < m_vTexAndSurfs.size(); i++ )
	{
		if( m_vTexAndSurfs.at(i).m_pSurface == pSurf )
		{
			return( & (m_vTexAndSurfs.at(i)) );
		}
	}
	return( NULL );
}

// Return NULL if the vector has no matching data
TexAndSurf * TexAndSurfTracker::Find( const TexAndSurf * pInPair )
{
	if( pInPair == NULL )
		return( NULL );
	size_t i;
	TexAndSurf * pArrayPair;
	for( i=0; i < m_vTexAndSurfs.size(); i++ )
	{
		pArrayPair = &(m_vTexAndSurfs.at(i));
		if( pArrayPair->m_pSurface == pInPair->m_pSurface &&
			pArrayPair->m_pTexture == pInPair->m_pTexture &&
			pArrayPair->m_uSurfaceLevel == pInPair->m_uSurfaceLevel )
		{
			return( pArrayPair );
		}
	}
	return( NULL );
}


void TexAndSurfTracker::AddIfNew( const TexAndSurf * pInPair )
{
	if( pInPair == NULL )
		return;
	TexAndSurf * pArrayPair;
	pArrayPair = Find( pInPair );
	if( pArrayPair == NULL )
	{
		// add the pair since we don't have it already
		m_vTexAndSurfs.push_back( *pInPair );

		// set the index and format strings
		size_t i;
		i = m_vTexAndSurfs.size();
		m_vTexAndSurfs.at(i-1).m_nIndex = (int)(i-1);
		pArrayPair = &(m_vTexAndSurfs.at(i-1));
		IDirect3DTexture9 * pTex;
		pTex = pInPair->m_pTexture;
		if( pTex != NULL )
		{
			D3DSURFACE_DESC desc;
			pTex->GetLevelDesc( 0, &desc );
			_tcscpy( m_vTexAndSurfs.at(i-1).m_FormatStr, GetStrD3DFORMAT( desc.Format ) );
		}
		// Set the texture index.  The texture index tracks which texture in the 
		// vector this belongs to.  There may be many pairs for a single render target
		// texture - each pair corresponds to a surface (mipmap) level - so search for the
		// texture in the separate vector that tracks them
		for( i=0; i < m_vTextures.size(); i++ )
		{
			if( pTex == m_vTextures.at(i) )
			{
				pArrayPair->m_nTextureIndex = (int) i;
				break;	// no more for loop
			}
		}
		// if we didn't find the texture, add it
		if( i == m_vTextures.size() )
		{
			m_vTextures.push_back( pArrayPair->m_pTexture );
			pArrayPair->m_nTextureIndex = (int)(m_vTextures.size()-1);
		}
	}
}
