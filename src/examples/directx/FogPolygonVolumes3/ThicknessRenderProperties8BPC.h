/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderProperties8BPC.h

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


#ifndef H_THICKNESSRENDERPROPERTIES_8BPC_H
#define H_THICKNESSRENDERPROPERTIES_8BPC_H

#include "ThicknessRenderProperties.h"

#define DEFAULT_DEPTHRELATION				ThicknessRenderProperties8BPC::PVF_8_SURFACES_15_BIT_DEPTH


// For rendering with 8-bits per color channel targets
class ThicknessRenderProperties8BPC : public ThicknessRenderProperties
{
public:
	enum DepthValuesPerChannel
	{
					//  | # of surfaces that can be rendered before artifacts occur
					//  |
					//  |				| depth values from front to back clip planes
		PVF_256,	//  1 surfaces      24-bit
		PVF_128,	//  2 surfaces		21-bit
		PVF_64,		//  4 surfaces		18-bit
		PVF_32,		//  8 surfaces		15-bit = 32768
		PVF_16,		//  16 surfaces		12-bit = 4096
		PVF_8,		//  32 surfaces		9-bit  = 512
		PVF_4,		//  64 surfaces		6-bit  = 64
		PVF_2,		//  128 surfaces	3-bit  = 8
		PVF_UNSET
	};

	// Maximum number of surfaces that can overlap before artifacts
	//  occur.  Front faces are handled separately from back faces, 
	//  so if the max depth complexity is two, that is two front 
	//  surfaces and two back surfaces.
	// The resulting depth precision between near and far clip planes
	//  is listed as the _ BIT_DEPTH
	enum MaxFogDepthComplexity
	{
		PVF_1_SURFACES_24_BIT_DEPTH,
		PVF_2_SURFACES_21_BIT_DEPTH,
		PVF_4_SURFACES_18_BIT_DEPTH,
		PVF_8_SURFACES_15_BIT_DEPTH,
		PVF_16_SURFACES_12_BIT_DEPTH,
		PVF_32_SURFACES_9_BIT_DEPTH,
		PVF_64_SURFACES_6_BIT_DEPTH,
		PVF_128_SURFACES_3_BIT_DEPTH,
		PVF_MAXDEPTHUNSET
	};

	MaxFogDepthComplexity	m_MaxFogDepthComplexity;
	DepthValuesPerChannel	m_DepthValuesPerColorChannel;

	HRESULT SetParameters( float fThicknessToColorScale,
							float fNearClip,
							float fFarClip,
							MaxFogDepthComplexity depth_relation,
							bool bUsePS20,
							IDirect3DDevice9 * pDev );

	virtual void	SetThicknessToColorTexCoordScale( float fTexCrdScale );

	// m_fBitReduce determines what percentage of each color
	//  channel 8-bit value is used for depth bits as opposed
	//  to "carry" bits.
	// (256 * m_fBitReduce) values are used for depth, and 
	// 256*(1-m_fBitReduce) values are used for carry or overflow values.
	//
	// If m_fBitReduce = 0.25 then values 0 to 63 are used in
	//  the RGB depth ramps to encode depth, and values 64-255 are
	//  open for holding overflow when two or more values in the
	//  range [0,64] are added together.  In this case, only 4 values
	//  could be added and guaranteed to not saturate the color value.
	//  This saturation would produce an error as depths are accumulated.
	//
	// "number of depth values" is the number of depth values that
	//  an RGB color can hold.  It is the depth resolution for a 
	//  particular choice of m_fBitReduce.
	//  The rest of the RGB color values are used for "carry" space
	//  so that several values can be added before saturation and error
	//  occurs.
	// 
	// The "RGB-to-depth conversion coefficients" is the 3D vector
	//  we use in a dot-product operation with the RGB-encoded depth
	//  to convert the RGB-encoded depth back to a single depth value.	
	//  The very small size of some of these coefficients means we
	//  must use floating point calculations to decode the RGB-encoded
	//  depth information.
	// This is using only RGB.  If you also use the alpha channel, RGBA,
	//  you can get more depth precision, but this will not work for
	//  pixel shaders 1.1 - 1.3.

	// m_fBitReduce	| max RGB   | depth | number | num    | RGB-to-depth 
	//	value		| in single | bits  | of     | depths | 
	//				| depth		| per   | depth  | summed | conversion
	//				| value		| RGB	| values | before | coefficients
	//				|			| color	|        | error  | (R,G,B)
	// ------------------------------------------------------------------
	// 1.0			| 255		| 24    | 2^24   | 1      | (1, 1/256, 1/(256*256)) 
	// 0.5			| 127		| 21    | 2^21   | 2      | (1, 1/128, 1/(128*128)) 
	// 0.25			| 63		| 18    | 262144 | 4      | (1, 1/64, 1/(64*64))
	// 0.125		| 31		| 15    | 32768  | 8      | (1, 1/32, 1/1024)
	// 0.0625		| 15		| 12    | 4096   | 16     | (1, 1/16, 1/256)
	// 0.03125		| 7			| 9     | 512    | 32     | (1, 1/8, 1/64)

	float m_fBitReduce;

	// m_fGrnRamp and m_fBluRamp are used to determine the encoding
	// of depth as an RGB color.  They determine the texture coordinate
	// scale for the green and blue color ramps, and are used to control
	// how many green ramps occur per red color increment and how many 
	// blue color ramps occur per green color increment.
	// The values of these depend on the choice of m_fBitReduce

	float		m_fGrnRamp;
	float		m_fBluRamp;
	D3DXVECTOR4	m_vTexAddrWeights;
	bool		m_bUsePS20;

	// color ramps for encoding high precision depth across the bits of an A8R8G8B8 color
	IDirect3DTexture9 **	m_ppTexRedGreenRamp;
	IDirect3DTexture9 **	m_ppTexBlueRamp;
	HRESULT		CreateRampTextures( IDirect3DDevice9 * pD3DDev );
	HRESULT		FreeRampTextures();

	ThicknessRenderProperties8BPC();
	~ThicknessRenderProperties8BPC();
	void	SetAllNull();

protected:
	// used to track allocation & free
	IDirect3DTexture9 *		m_pTexRedGreenRamp;
	IDirect3DTexture9 *		m_pTexBlueRamp;

	void CreateGradientDithered( IDirect3DTexture9 ** ppTex,
								IDirect3DDevice9 * pD3DDev,
								int width, int height,
								byte lower, byte upper,
								D3DXVECTOR3 color_mask,
								const D3DCOLOR * pDither_upper );

	void CreateGradient2D( IDirect3DTexture9 ** ppTex2D,
							IDirect3DDevice9 * pD3DDev,
							int width, int height, byte lower, byte upper,
							D3DXVECTOR3 col_mask_u, D3DXVECTOR3 col_mask_v );

	friend class ThicknessRenderPS20_8bpc;
	friend class ThicknessRenderPS30_8bpc_MRT;
};

#endif

