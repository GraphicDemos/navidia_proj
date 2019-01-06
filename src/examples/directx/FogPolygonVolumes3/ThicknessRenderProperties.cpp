/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderProperties.cpp

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

#include "dxstdafx.h"

#include <NV_D3DCommon\NV_D3DCommonDX9.h>
#include "ThicknessRenderProperties.h"


ThicknessRenderProperties::ThicknessRenderProperties()
{
	m_dwVolumeColorToScreenSrcBlend		=	D3DBLEND_ONE;
	m_dwVolumeColorToScreenDestBlend	=	D3DBLEND_INVSRCCOLOR;
	m_dwVolumeColorToScreenBlendOp		=	D3DBLENDOP_ADD;

	SetParameters( DEFAULT_THICKNESS_TO_COLOR_SCALE, DEFAULT_NEARCLIP, DEFAULT_FARCLIP );

	m_ppTexFogThicknessToColor				= NULL;
	m_fTexCrdPrecisionFactor				= 1.0f;
}

HRESULT ThicknessRenderProperties::SetClipPlanes( float fNearClip, float fFarClip )
{
	if( fFarClip > fNearClip )
	{
		m_fFarClip = fFarClip;
		m_fNearClip = fNearClip;
	}
	else
	{
		m_fFarClip = fNearClip;
		m_fNearClip = fFarClip;		
	}

	// ( w - m_fNearClip ) / ( m_fFarClip - m_fNearClip ) = 
	// w * 1 / ( m_fFarClip - m_fNearClip ) - m_fNearclip / ( m_fFarClip - m_fNearClip )
	m_NormalizeWDepth = D3DXVECTOR4(	1.0f / ( m_fFarClip - m_fNearClip ),
										- m_fNearClip / ( m_fFarClip - m_fNearClip ),
										0.0f,
										1.0f );
	return( S_OK );
}


HRESULT ThicknessRenderProperties::SetParameters( float fThicknessToColorScale,
													float fNearClip,
													float fFarClip )
{
	HRESULT hr = S_OK;
	SetClipPlanes( fNearClip, fFarClip );
	SetThicknessToColorTexCoordScale( fThicknessToColorScale );
	m_fScale = 1.0f;
	m_fTexCrdPrecisionFactor	= 1.0f;
	return( hr );
}

// Virtual function so it can be overridden if a particular implementation needs to
//  do more processing when the parameter changes.
void ThicknessRenderProperties::SetThicknessToColorTexCoordScale( float fTexCrdScale )
{
	m_fTexCrdScale = fTexCrdScale;
}

