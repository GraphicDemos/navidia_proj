/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  LowPassFilter2D.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A low-pass filter.  This has a 2D target point and a 2D current location.  The
current location approaches the target point slowly over time.

Also has hard constraint to keep x,y within a certain distance from the target

-------------------------------------------------------------------------------|--------------------*/

#ifndef GJ_H_LOWPASSFILTER2D_H
#define GJ_H_LOWPASSFILTER2D_H

#include "NV_D3DCommon_decl_.h"
class DECLSPEC_NV_D3D_COMMON_API LowPassFilter2D
{
public:
	float x;						// current position
	float y;	
	float x_targ;
	float y_targ;
	float last_x;
	float last_y;
	float m_FractionMovePerTick;	// new x = current + (targ-current)*m_FractionMovePerTick

	float	m_fAxisConstraintLimit;
	bool	m_bUseIndependentAxisConstraint;
	bool	m_bUseRadiusConstraint;

	void	SetTarget( float nx, float ny );
	void	ConstrainPosition();
	void	Tick();
	LowPassFilter2D();
	~LowPassFilter2D();
};

#endif


