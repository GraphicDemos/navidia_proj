/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  NoiseGrid3D.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
NoiseGrid3D - Generates noise over a volume of space given an input position and a set of noise
parameters governing frequency and amplitude.

This class does not create a large volume data set of noise.  Instead it generates repeatable
pseudo-random values using the input coordinate and noise parameters to create a hash value for
that point in space.  The hash value generates a noise value, and neighboring noise values in
a grid pattern are also computed.  The output noise value is interpolated between the noise
values generated at each grid point, so the frequency of the grid determines the smoothness
of the noise.

The basic noise generation based on a spatial hash function comes from:
[Ward 1991]:
G. Ward. "A recursive implementation of the Perlin noise function"
	Graphics Gems I, 396-401, 1991 AP Professional, 
	
** Only a single 3D interpolation filter has been coded.  Other simpler filters have not been.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NV_NoiseGrid3D_H_GJ
#define H_NV_NoiseGrid3D_H_GJ

#include <windows.h>
#pragma warning( disable : 4786 )
#include <vector>

using namespace std;


struct GridNoiseComponent
{
	float freq_x, freq_y, freq_z;
	float amplitude;
	DWORD seed;

	enum FilterType
	{
		GNFT_POINT,
		GNFT_TRILINEAR,		// 2x2x2 nearest points
		GNFT_TRICUBIC,		//@@ not implemented.  Use > 2x2x2 samples for cubic interpolation
	};

	FilterType	filter;

	GridNoiseComponent()
	{			
		GridNoiseComponent::GridNoiseComponent( 1.0f, 1.0f, 1, GridNoiseComponent::GNFT_TRILINEAR );
	};
	GridNoiseComponent( float freq, float amp, DWORD inSeed, FilterType inFilter )
	{
		freq_x = freq_y = freq_z = freq;
		amplitude = amp;
		seed = inSeed;
		filter = inFilter;		
	};
};


class NoiseGrid3D
{
public:
	struct intvec3
	{
		int x;
		int y;
		int z;

		intvec3()								{ x=y=z=0; };
		intvec3( float ix, float iy, float iz ) { x=(int)ix;y=(int)iy;z=(int)iz; };
	};

public:

	// generates scalar noise value for spatial point x,y,z with
	// the freqs and amplitudes in the vector of GridNoiseComponent
	// Noise values pOut are positive from [0,mag] where mag is determined
	//  by the magnitudes of the GridNoiseComponents
	float NoiseScalar( float * pOut,
						float x, float y, float z,
						const vector< GridNoiseComponent > * pvComponents );

	// Same as NoiseScalar but generates signed floating point values that range
	//  in magnitude from [-mag/2, mag/2] from each GridNoiseComponent.
	float NoiseScalarSigned( float *pOut, float x, float y, float z, 
							 const vector< GridNoiseComponent > * pvComponents,
							 bool bSigned = true );


	// generates 2D noise vector for spatial point x,y,z
	void  NoiseVec2D( float * pOutX, float * pOutY,
						float x, float y, float z,
						const vector< GridNoiseComponent > * pvComponents );

	void NoiseVec3D( float & OutX, float & OutY, float & OutZ,
					 float in_xpos, float in_ypos, float in_zpos,
					 const vector< GridNoiseComponent > & vComponents );


	NoiseGrid3D() { };

	inline	float ufRand( int coord );

	inline float rand3a( int x, int y, int z )	{ return( ufRand( x*67  + y*59  + z*71  ));};
	inline float rand3b( int x, int y, int z )	{ return( ufRand( x*73  + y*79  + z*83  ));};
	inline float rand3c( int x, int y, int z )	{ return( ufRand( x*89  + y*97  + z*101 ));};
	inline float rand3d( int x, int y, int z )	{ return( ufRand( x*103 + y*107 + z*109 ));};

};


inline 
float NoiseGrid3D::ufRand( int coord )
{
	// return pseudo-random float from [0,1] based on
	//  coordinate coord.
	// Hash function & 3rd order polynomial from [Ward 1991]:
	// G. Ward. "A recursive implementation of the Perlin noise function"
	//		Graphics Gems I, 396-401, 1991 AP Professional, 

	// assumes two's-complement integer values
	// ^ = bitwise XOR
	coord = ( coord << 13 ) ^ coord;
	coord = ( coord*(coord*coord*15731 + 789221) + 1376312589) & 65535;
	return( coord / 65535.0f );
}


#endif					// H_NV_NoiseGrid3D_H_GJ
