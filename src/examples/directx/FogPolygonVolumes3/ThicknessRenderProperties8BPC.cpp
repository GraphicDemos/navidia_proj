/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderProperties8BPC.cpp

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
#include "ThicknessRenderProperties8BPC.h"
#include "shared\NV_StringFuncs.h"

void ThicknessRenderProperties8BPC::SetAllNull()
{
	m_pTexRedGreenRamp			= NULL;
	m_pTexBlueRamp				= NULL;
	m_ppTexRedGreenRamp			= NULL;
	m_ppTexBlueRamp				= NULL;
}

ThicknessRenderProperties8BPC::ThicknessRenderProperties8BPC()
{
	SetAllNull();
}

ThicknessRenderProperties8BPC::~ThicknessRenderProperties8BPC()
{
	FreeRampTextures();
	SetAllNull();
}

HRESULT ThicknessRenderProperties8BPC::FreeRampTextures()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pTexRedGreenRamp );
	SAFE_RELEASE( m_pTexBlueRamp );
	return( hr );
}



// Expensive, since it creates new device textures
HRESULT ThicknessRenderProperties8BPC::SetParameters( float fThicknessToColorScale,
					float fNearClip, float fFarClip, MaxFogDepthComplexity depth_relation, bool bUsePS20,
					IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );

	ThicknessRenderProperties::SetParameters( fThicknessToColorScale, fNearClip, fFarClip );

	m_bUsePS20 = bUsePS20;
	
	m_MaxFogDepthComplexity = depth_relation;
	float base_scale = 10.0f;
	float diff = 2.0f;

	switch( m_MaxFogDepthComplexity )
	{
	case PVF_1_SURFACES_24_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_256;
		m_fTexCrdPrecisionFactor = base_scale / (diff*diff*diff*diff);
		break;
	case PVF_2_SURFACES_21_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_128;
		m_fTexCrdPrecisionFactor = base_scale / (diff*diff*diff);
		break;
	case PVF_4_SURFACES_18_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_64;
		m_fTexCrdPrecisionFactor = base_scale / (diff*diff);
		break;
	case PVF_8_SURFACES_15_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_32;
		m_fTexCrdPrecisionFactor = base_scale / diff;
		break;
	case PVF_16_SURFACES_12_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_16;
		m_fTexCrdPrecisionFactor = base_scale;
		break;
	case PVF_32_SURFACES_9_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_8;
		m_fTexCrdPrecisionFactor = base_scale * diff;
		break;
	case PVF_64_SURFACES_6_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_4;
		m_fTexCrdPrecisionFactor = base_scale * diff * diff;
		break;
	case PVF_128_SURFACES_3_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_2;
		m_fTexCrdPrecisionFactor = base_scale * diff * diff * diff;
		break;

	default :
		FMsg("Unrecognized MaxFogDepthComplexity\n");
		assert( false );
		return( E_FAIL );
		break;
	}

	float little_bands;
	switch( m_DepthValuesPerColorChannel )
	{
	case PVF_256 :
		m_fBitReduce = 1.0f;
		little_bands = 256.0f;
		break;
	case PVF_128 :
		m_fBitReduce = 0.5f;
		little_bands = 128.0f;
		break;
	case PVF_64 :
		m_fBitReduce = 0.25f;
		little_bands = 64.0f;
		break;
	case PVF_32 :
		m_fBitReduce = 0.125f;
		little_bands = 32.0f;
		break;
	case PVF_16 :
		m_fBitReduce = 0.0625f;
		little_bands = 16.0f;
		break;
	case PVF_8 :
		m_fBitReduce = 0.03125f;
		little_bands = 8.0f;
		break;
	case PVF_4 :
		m_fBitReduce = 0.015625;
		little_bands = 4.0f;
		break;
	case PVF_2 :
		m_fBitReduce = 0.0078125;
		little_bands = 2.0f;
		break;

	default:
		FDebug("unknown # depth values per channel -- using 16 as default\n");
		m_fBitReduce = 0.0625f;
		little_bands = 16.0f;
		assert( false );
	}

	m_fGrnRamp = m_fBitReduce * 256.0f;		// how many times green cycles for 1 red cycle
	m_fBluRamp = m_fGrnRamp * m_fGrnRamp;
	m_fGrnRamp = little_bands;
	m_fBluRamp = little_bands * little_bands;

	SetThicknessToColorTexCoordScale( fThicknessToColorScale );

	CreateRampTextures( pDev );

	return( hr );
}

void ThicknessRenderProperties8BPC::SetThicknessToColorTexCoordScale( float fTexCrdScale )
{
	m_fTexCrdScale = fTexCrdScale;

	// Set weights for converting RGB encoded depth to scalar
	//  depth value (texture coordinate) used to access a simple
	//  grayscale ramp texture

	// alpha is weighted as negative green, so dithering lowest
	//  low bit values can wrap down to the next lower green value
	m_vTexAddrWeights = D3DXVECTOR4(	1.0f, 
										1.0f / m_fGrnRamp,
										1.0f / m_fBluRamp,
										-1.0f / m_fGrnRamp );
	m_vTexAddrWeights.x *= m_fTexCrdScale * m_fTexCrdPrecisionFactor;
	m_vTexAddrWeights.y *= m_fTexCrdScale * m_fTexCrdPrecisionFactor;
	m_vTexAddrWeights.z *= m_fTexCrdScale * m_fTexCrdPrecisionFactor;
	m_vTexAddrWeights.w *= m_fTexCrdScale * m_fTexCrdPrecisionFactor;

	// If not using PS20, thickness calc will be 2x the result as
	//  with PS20, so divide the scale in half.
	// This is because D3D forces us to use _bx2 modifier as opposed
	//  to the _bias modifier.
	if( !m_bUsePS20 )
	{
		m_vTexAddrWeights.x *= 0.5f;
		m_vTexAddrWeights.y *= 0.5f;
		m_vTexAddrWeights.z *= 0.5f;
		m_vTexAddrWeights.w *= 0.5f;
	}
}


HRESULT	ThicknessRenderProperties8BPC::CreateRampTextures( IDirect3DDevice9 * pD3DDev )
{
	HRESULT hr = S_OK;

	int size_divide = (int)( 1.0f / m_fBitReduce );

	// Create texture gradients used to encode depth as an RGB color.
	// Blue is a single gradient in a single texture.
	// Red and green gradients are held in one 2D texture with 
	//   each gradient going in one axis (green in X, red in Y )
	//
	// This class will generate it's own gradient textures, and
	//  the blue texture, which encodes the least significant bits
	//  of depth, are dithered to eliminate aliasing artifacts
	// There is a vertex shader 'dither control' variable to control
	//  the texture coordinates fetching from the dithered texture.


	// 2D gradient in red and green
	// No dithering
	CreateGradient2D( & m_pTexRedGreenRamp,
						pD3DDev,
						256 / size_divide, 256 / size_divide,
						0, (byte)( 255.0f * m_fBitReduce ),
						D3DXVECTOR3( 1.0f, 0.0f, 0.0f ),	// color mask u axis
						D3DXVECTOR3( 0.0f, 1.0f, 0.0f )  );	// color mask v axis RGB


	D3DCOLOR upper_color;
	upper_color = D3DCOLOR_ARGB( 0, 0, 1, 0 );	 //  1/255 of green

	// extra width for dithered values
	int width = 8 * 256 / size_divide;

	if( width > 1024 )
		width = 1024;

	// 64 in y for dithered values.

	CreateGradientDithered( & m_pTexBlueRamp,
								pD3DDev,
								width, 64,
								0, (byte)( 255.0f * m_fBitReduce ),
								D3DXVECTOR3( 0.0f, 0.0f, 1.0f ),	// blue
								& upper_color );

	m_ppTexRedGreenRamp = &m_pTexRedGreenRamp;
	m_ppTexBlueRamp		= &m_pTexBlueRamp;


	return( hr );
}



// Creates two gradients in a 2D texture.
// col_mask_u is applied to 1st gradient in U axis
// col_mask_v is applied to 2nd gradient in V axis
//
// each color mask should have values in the range [0,1]
// Color mask is from [0,1] and gets multiplied into
//  the color components
void ThicknessRenderProperties8BPC::CreateGradient2D( IDirect3DTexture9 ** ppTex2D,
												    IDirect3DDevice9 * pD3DDev,
													int width, int height, byte lower, byte upper,
													D3DXVECTOR3 col_mask_u, D3DXVECTOR3 col_mask_v )
{
	BREAK_AND_RET_IF( pD3DDev == NULL );
	BREAK_AND_RET_IF( ppTex2D == NULL );
	HRESULT hr;
	SAFE_RELEASE( *ppTex2D );

	hr = pD3DDev->CreateTexture( width, height,
									1,				// mip levels
									0,				// usage
									D3DFMT_A8R8G8B8,
									D3DPOOL_MANAGED,
									ppTex2D,
									NULL );
	if( FAILED(hr))
	{
		FDebug("Can't create texture!\n");
		assert( false );
		return;
	}
	BREAK_AND_RET_IF( *ppTex2D == NULL );

	// Now lock and fill the texture with the gradient
	IDirect3DTexture9 * pTex = *ppTex2D;
	D3DLOCKED_RECT lr;
	hr = pTex->LockRect( 0, &lr,
						NULL,		// region to lock - NULL locks whole thing
						0 );		// No special lock flags
									// Can't use LOCK_DISCARD unless texture is DYNAMIC
	BREAK_IF( FAILED(hr) );

	byte * pixel;
	byte * pBase = (BYTE*) lr.pBits;
	int bytes_per_pixel = 4;
	assert( bytes_per_pixel * width == lr.Pitch );

	byte colu;
	byte colv;
	int i,j;
	D3DXVECTOR3 sum;

	for( j=0; j < height; j++ )
	{
		for( i=0; i < width; i++ )
		{
			pixel = pBase + j * lr.Pitch + i * bytes_per_pixel;

			// calculate gradient values
			colu = (byte)( lower + (upper-lower) * (float)i / ((float)width - 1.0f));
			colv = (byte)( lower + (upper-lower) * (float)j / ((float)height - 1.0f));
			sum = colu * col_mask_u + colv * col_mask_v;

			// alpha should be 255 because it is used later in alpha test
			*((D3DCOLOR*)pixel) = D3DCOLOR_ARGB( 0, (BYTE)( sum.x ), (BYTE)( sum.y ), (BYTE)( sum.z ) );
		}	
	}

	hr = pTex->UnlockRect( 0 );
	BREAK_IF( FAILED(hr) );
	FDebug("Created 2D gradient texture!\n");

	// Option to save the texture to a file
#if 0
	string filename;
	filename = "C:\\RG_0_15.bmp";
	FMsg("Texture filename [%s]\n", filename.c_str() );
	
	// D3DXIFF_BMP can't be parsed by Photoshop
	//  but if you re-save it in another app it will be ok for Photoshop
	hr = D3DXSaveTextureToFile( filename.c_str(),
								D3DXIFF_BMP,
								*ppTex2D,
								NULL	);		// palette 
	BREAK_IF( FAILED(hr) );
#endif

}



void ThicknessRenderProperties8BPC::CreateGradientDithered( IDirect3DTexture9 ** ppTex,
														    IDirect3DDevice9 * pD3DDev,
															int width, int height,
															byte lower, byte upper,
															D3DXVECTOR3 color_mask,
															const D3DCOLOR * pDither_upper )
{	
	BREAK_AND_RET_IF( pD3DDev == NULL );

	// width, height are resolution of texture
	// lower, upper are values for the limits of the gradient
	// color_mask is applied to byte values to generate the color
	// pDither_upper can point to a color value.  If not NULL, then the 
	//    upper texels (rightmost) are dithered to the color pointed to.

	// Set true to save the resulting texture to a file for inspection
	bool bSaveTexture = false;


	// Texture dither should continue at the top and bottom to the 
	//  next highest and lowest values, respectively, but this would
	//  require storing and blending a negative green value at the
	//  lowest dither points.  That's impossible, but we can store a
	//  positive value in the alpha channel and treat it as a negative
	//  green value in pixel shaders 2.0.
	// We can't treat the alpha channel as negative green value with
	//  pixel shaders 1.3 in the depth comparison shader because we
	//  run out of instructions.
	// Dither the lower bits only if using ps_2_0.

	bool bDitherLowerToBlueAndAlpha = false;
	if( m_bUsePS20 )
	{
		bDitherLowerToBlueAndAlpha = true;
	}


	HRESULT hr = S_OK;

	// Creates a gradient in X axis of the texture
	// Color mask is from [0,1] and gets multiplied into
	//  the color components

	BREAK_AND_RET_IF( ppTex == NULL );
	BREAK_AND_RET_IF( pD3DDev == NULL);	
	SAFE_RELEASE( *ppTex );

	// Generating mip maps for this low bits texture does not
	// improve performance, but increasing or decreasing the
	// texture coordinate scale does affect performance.  As the
	// texture coordinate dither scale increases, performance 
	// decreases.  This is due to the degree of coherence of
	// samples taken from the texture and the texture cache
	// utilization.

	hr = pD3DDev->CreateTexture( width, height,
									1,				// mip levels, 0 = all
									0,				// usage
									D3DFMT_A8R8G8B8,
									D3DPOOL_MANAGED,
									ppTex,
									NULL );
	if( FAILED(hr))
	{
		FDebug("CreateGradientDithered() Can't create texture!\n");
		assert( false );
		return;
	}
	BREAK_AND_RET_IF( *ppTex == NULL );

	// Now lock and fill the texture with the gradient
	IDirect3DTexture9 * pTex = *ppTex;
	D3DLOCKED_RECT lr;
	hr = pTex->LockRect( 0, &lr,
						NULL,		// region to lock - NULL locks whole thing
						0 );		// No special lock flags
									// Can't use LOCK_DISCARD unless texture is DYNAMIC
	BREAK_IF( FAILED(hr) );

	byte * pixel;
	byte * pBase = (BYTE*) lr.pBits;

	int bytes_per_pixel = 4;

	assert( bytes_per_pixel * width == lr.Pitch );

	int val;
	int dithered_val;
	DWORD color;
	int noiseinc;
	float fcol;
	float frand;
	int i,j;

	for( j=0; j < height; j++ )
	{
		for( i=0; i < width; i++ )
		{
			pixel = pBase + j * lr.Pitch + i * bytes_per_pixel;

			// calculate gradient values
			val = (int)( lower + (upper+1-lower) * ((float)i / ((float)width)));
			fcol = lower + (upper+1-lower) * ((float)i / ((float)width));

			fcol = (float) fmod( fcol, 1.0f );
			frand = ((float)rand()) / ((float)RAND_MAX);
			if( fcol < 0.5f )
			{
				fcol = 0.5f - fcol;
				if( frand < fcol )
					noiseinc = -1;
				else
					noiseinc = 0;
			}
			else
			{
				fcol = fcol - 0.5f;			// 0 at middle of color band
											// 0.5 at right edge of color band
				if( frand > fcol )
					noiseinc = 0;
				else
					noiseinc = 1;
			}

			dithered_val = val + noiseinc;
			if( dithered_val < 0 )
			{
				if( bDitherLowerToBlueAndAlpha )
				{
					// Dither bottom into blue and alpha
					// Alpha will become negative green increment

					color = D3DCOLOR_ARGB( 1, 
											(BYTE)( upper * color_mask.x ),
											(BYTE)( upper * color_mask.y ),
											(BYTE)( upper * color_mask.z ) );
				}
				else
				{
					color = 0x00;
				}
			}
			else if( dithered_val > upper )
			{
				if( pDither_upper != NULL )
				{
					color = *pDither_upper;
				}
				else
				{
					// color = maximum value
					dithered_val = dithered_val - 1;
					color = D3DCOLOR_ARGB( 0, 
											(BYTE)( dithered_val * color_mask.x),
											(BYTE)( dithered_val * color_mask.y),
											(BYTE)( dithered_val * color_mask.z) );
				}
			}
			else
			{
				// standard dithered value
				color = D3DCOLOR_ARGB( 0, 
										(BYTE)( dithered_val * color_mask.x),
										(BYTE)( dithered_val * color_mask.y),
										(BYTE)( dithered_val * color_mask.z) );
			}

			// If at the top line of the texture, do not dither, because
			//  dither control of (0,0,0,0) accesses the top line
			// This is only so the demo can illustrate the difference between
			//  dither and no dither.  If you always dither, you can remove this
			//  j==0 case.
			if( j == 0 )
			{
				color = D3DCOLOR_ARGB( 0,
										(BYTE)( val * color_mask.x),
										(BYTE)( val * color_mask.y),
										(BYTE)( val * color_mask.z) );
			}


			*((D3DCOLOR*)pixel) = color;
		}	
	}

	hr = pTex->UnlockRect( 0 );
	BREAK_IF( FAILED(hr) );

	// mip mapping the blue bits texture doesn't improve perf
//	pTex->SetAutoGenFilterType( D3DTEXF_POINT );
//	pTex->GenerateMipSubLevels();

	FDebug("Created dithered gradient texture.  Resolution: %d, %d\n", width, height );

	if( bSaveTexture == true )
	{
		TCHAR filename[200];
		_stprintf( filename, TEXT("C:\\PVFDither_%d_%d%s.bmp"), width, height, pDither_upper ? TEXT("_WrapDither") : TEXT("") );
		FMsg("Texture filename [%s]\n", filename );
		
		// D3DXIFF_TGA is not yet supported
		// D3DXIFF_BMP can't be parsed by Photoshop
		//   but if you open in another app and re-save then it is ok.
		// D3DXIFF_PPM is not supported
		// PNG is not supported
		hr = D3DXSaveTextureToFile( filename,
									D3DXIFF_BMP,
									*ppTex,
									NULL	);		// palette 
		BREAK_IF( FAILED(hr) );
	}
}



