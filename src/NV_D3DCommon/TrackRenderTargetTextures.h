/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TrackRenderTargetTextures.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
//@@ TrackRenderTargetTextures need to restructure data

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_TRACK_RTTS_H
#define H_TRACK_RTTS_H

#include "NV_D3DCommonDX9PCH.h"
#include <vector>
using namespace std;


struct TexAndSurf
{
	IDirect3DTexture9 * m_pTexture;
	IDirect3DSurface9 * m_pSurface;
	UINT				m_uSurfaceLevel;

	// set automaticaly by the tracker
	int		m_nIndex;
	int		m_nTextureIndex;
	TCHAR	m_FormatStr[256];
};

class TexAndSurfTracker
{
public:
	vector< TexAndSurf >			m_vTexAndSurfs;
	vector< IDirect3DTexture9 * >	m_vTextures;			// one entry for each unique render target texture

	TexAndSurf * Find( const IDirect3DTexture9 * pTex );
	TexAndSurf * Find( const IDirect3DBaseTexture9 * pBaseTex );
	TexAndSurf * Find( const IDirect3DSurface9 * pSurf );
	TexAndSurf * Find( const TexAndSurf * pInPair );
	void	AddIfNew( const TexAndSurf * pInPair );
};

#endif
