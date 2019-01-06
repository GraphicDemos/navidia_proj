/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  NoiseGrid3D.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the header for comments.

-------------------------------------------------------------------------------|--------------------*/

#include "NoiseGrid3D.h"
#include <assert.h>
#include <math.h>

float NoiseGrid3D::NoiseScalar( float * pOut,
								float x, float y, float z,
								const vector< GridNoiseComponent > * pvComponents )
{
	float val;
	val = NoiseScalarSigned( pOut, x, y, z, pvComponents, false );
	return( val );
}


// bSigned = true for noise from [-mag/2,mag/2] instead of [0,mag]
float NoiseGrid3D::NoiseScalarSigned( float *pOut, float x, float y, float z, 
										const vector< GridNoiseComponent > * pvComponents,
										bool bSigned )
{
	// see class decl for comments
	if( pOut == NULL )
		return( 0.0f );
	if( pvComponents == NULL )
	{
		*pOut = 0.0f;
		return( 0.0f );
	}
	if( pvComponents->size() < 1 )
	{
		*pOut = 0.0f;
		return( 0.0f );
	}

	float noise_val = 0.0f;
	DWORD dwx, dwy, dwz;
	float grid_x, grid_y, grid_z;	// quantized x,y,z for repeatability
	const int trilinear_offsets[8][3] = {	{0,0,0}, {1,0,0}, {0,1,0}, {1,1,0},
											{0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}	};
	float	grid_vals[8];
	unsigned int nc;
	float px, py, pz;

	for( nc = 0; nc < pvComponents->size(); nc ++ )
	{
		const GridNoiseComponent * pNC = &( pvComponents->at(nc) );
		px = x * pNC->freq_x;
		py = y * pNC->freq_y;
		pz = z * pNC->freq_z;
		// clamp to closest lesser grid point.
		grid_x = (float) floor( px );
		grid_y = (float) floor( py );
		grid_z = (float) floor( pz );
		dwx = (DWORD) grid_x;
		dwy = (DWORD) grid_y;
		dwz = (DWORD) grid_z;
		px = px - grid_x;	// for interpolation
		py = py - grid_y;
		pz = pz - grid_z;

		//@ NoiseGrid3D::NoiseScalarSigned always using trilinear regardless of noise component type
		int oc;
		DWORD sx, sy, sz;		
		dwx += pNC->seed * 3;
		dwy += pNC->seed * 7;
		dwz += pNC->seed * 11;

		// compute noise values for neighboring grid points
		for( oc=0; oc < 8; oc++ )
		{
			sx = dwx + trilinear_offsets[oc][0];
			sy = dwy + trilinear_offsets[oc][1];
			sz = dwz + trilinear_offsets[oc][2];
			
			// grid point rand values 
			// ie. random values at each grid point depending on the spatial index
			grid_vals[oc] = rand3a( sx, sy, sz );
		}

		// interpolate between the grid values
		// hard-coded dependency on coords in vTLOff
		float xa00, xa10, xa01, xa11;
		float ya0, ya1;
		float avg;

		// a * (1-px) + b * px = a + px * (b-a);
		xa00 = grid_vals[0] + px * ( grid_vals[1]-grid_vals[0] );
		xa10 = grid_vals[2] + px * ( grid_vals[3]-grid_vals[2] );
		xa01 = grid_vals[4] + px * ( grid_vals[5]-grid_vals[4] );
		xa11 = grid_vals[6] + px * ( grid_vals[7]-grid_vals[6] );

		ya0 = xa00 + py * ( xa10 - xa00 );
		ya1 = xa01 + py * ( xa11 - xa01 );
		avg = ya0 + pz * ( ya1 - ya0 );

		// add the trilinear sample to the total noise_value
		if( bSigned )
			noise_val += avg * ( pNC->amplitude - pNC->amplitude / 2.0f );
		else
			noise_val += avg * pNC->amplitude;
	}

	*pOut = noise_val;
	return( *pOut );
}



void NoiseGrid3D::NoiseVec2D( float * pOutX, float * pOutY,
								float x, float y, float z,
								const vector< GridNoiseComponent > * pvComponents )							  
{
	// see class decl for comments
	assert( false );
}



void NoiseGrid3D::NoiseVec3D( float & OutX, float & OutY, float & OutZ,
								float in_xpos, float in_ypos, float in_zpos,
								const vector< GridNoiseComponent > & vComponents )
{

	float noise_val = 0.0f;
	DWORD dwx, dwy, dwz;

	float grid_x, grid_y, grid_z;	// quantized x,y,z for repeatability

	const int trilinear_offsets[8][3] = {	{0,0,0}, {1,0,0}, {0,1,0}, {1,1,0},
											{0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}	};
	float	grid_vals[8][3];

	unsigned int nc;
	float px, py, pz;

	OutX = OutY = OutZ = 0.0f;
	float interpolated[3];		// x,y,z

	for( nc = 0; nc < vComponents.size(); nc ++ )
	{
		const GridNoiseComponent * pNC = &( vComponents.at(nc) );

		//@ no freedom to change origin of grid for different components
		px = in_xpos * pNC->freq_x;
		py = in_ypos * pNC->freq_y;
		pz = in_zpos * pNC->freq_z;
		// clamp to closest lesser grid point.
		grid_x = (float) floor( px );
		grid_y = (float) floor( py );
		grid_z = (float) floor( pz );
		px = px - grid_x;		// [0,1] parameters for interpolation
		py = py - grid_y;
		pz = pz - grid_z;
		dwx = (DWORD) grid_x + pNC->seed;	// integer grid coordinate
		dwy = (DWORD) grid_y + pNC->seed;
		dwz = (DWORD) grid_z + pNC->seed;

		//@ always using trilinear
		int oc;
		DWORD sx, sy, sz;		
		for( oc=0; oc < 8; oc++ )
		{
			sx = dwx + trilinear_offsets[oc][0];
			sy = dwy + trilinear_offsets[oc][1];
			sz = dwz + trilinear_offsets[oc][2];
			
			// grid point rand values 
			// ie. random values at each grid point depending on the spatial index
			grid_vals[oc][0] = rand3a( sx, sy, sz );
			grid_vals[oc][1] = rand3b( sx, sy, sz );
			grid_vals[oc][2] = rand3c( sx, sy, sz );
		}

		// interpolate between the grid values
		// hard-coded dependency on coords in vTLOff
		float xa00, xa10, xa01, xa11;
		float ya0, ya1;

		for( int j=0; j < 3; j++ )
		{
			// a * (1-px) + b * px = a + px * (b-a);
			xa00 = grid_vals[0][j] + px * ( grid_vals[1][j]-grid_vals[0][j] );
			xa10 = grid_vals[2][j] + px * ( grid_vals[3][j]-grid_vals[2][j] );
			xa01 = grid_vals[4][j] + px * ( grid_vals[5][j]-grid_vals[4][j] );
			xa11 = grid_vals[6][j] + px * ( grid_vals[7][j]-grid_vals[6][j] );
			ya0 = xa00 + py * ( xa10 - xa00 );
			ya1 = xa01 + py * ( xa11 - xa01 );
			interpolated[j] = ya0 + pz * ( ya1 - ya0 );
		}
		// add the trilinear samples to the total noise value
		OutX += interpolated[0] * pNC->amplitude;
		OutY += interpolated[1] * pNC->amplitude;
		OutZ += interpolated[2] * pNC->amplitude;
	}
}
