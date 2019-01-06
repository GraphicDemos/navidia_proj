/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderProperties.h

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


#ifndef H_THICKNESSRENDERPROPERTIES_H
#define H_THICKNESSRENDERPROPERTIES_H


#define DEFAULT_THICKNESS_TO_COLOR_SCALE	424.0f
#define DEFAULT_NEARCLIP					0.1f
#define DEFAULT_FARCLIP						50.0f
#define DEFAULT_USEPS20						true



// Base class for rendering based on depth or thickness
class ThicknessRenderProperties
{
public:
	// Blend ops for rendering fog color to the screen
	DWORD	m_dwVolumeColorToScreenSrcBlend;
	DWORD	m_dwVolumeColorToScreenDestBlend;
	DWORD	m_dwVolumeColorToScreenBlendOp;

	// Texture that maps fog object thickness to a color
	IDirect3DTexture9 **	m_ppTexFogThicknessToColor;

	// Scale value which affects dot-product texture address operation
	//  conversion of RGB-encoded depth value to a scalar texture coordinate
	//  This can increate or decrease the range of texture coordinates used
	//  to access m_ppTexFogThicknessToColor to convert fog volume thickness
	//  into a fog color.
	float	m_fTexCrdScale;					// value the user can set
	float	m_fTexCrdPrecisionFactor;		// multiplied by user param so look is the
											// same for various choices of depth precision

	float m_fNearClip;			// near and far clip plane distances
	float m_fFarClip;

	// m_fScale is multiplied by the screen space vertex depth before
	//  the depth is converted to RGB color texture coordinates.  The
	//  near and far clip plane distances affect the value of m_fScale
	//  to map the full RGB depth color range to the full screen depth
	//  range.
	// You can modify m_fScale to increase or decrease the number of
	//  RGB ramps with depth, but increasing m_fScale will cause the
	//  RGB color range to repeat as distance increases.  This will
	//  cause errors if an object spans the depth at which the RGB
	//  color range wraps back around from its maximum to its minimum
	//  value.

	float	m_fScale;

	HRESULT SetParameters( float fThicknessToColorScale, float fNearClip, float fFarClip );
	HRESULT SetClipPlanes( float fNearClip, float fFarClip );

	virtual void	SetThicknessToColorTexCoordScale( float fTexCrdScale );
	virtual float	GetThicknessToColorTexCoordScale()						{ return( m_fTexCrdScale ); }; 

	ThicknessRenderProperties();


protected:
	// Constants used to normalize the w depth to the range [0,1] from the
	//   near clip plane to the far clip plane.
	D3DXVECTOR4 m_NormalizeWDepth;

};




// For rendering with floating point render targets
class ThicknessRenderPropertiesFP : ThicknessRenderProperties
{
public:


};


#endif

