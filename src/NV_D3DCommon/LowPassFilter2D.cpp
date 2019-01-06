/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  LowPassFilter2D.cpp

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
#include "LowPassFilter2D.h"

LowPassFilter2D::LowPassFilter2D()
{
	x_targ = 0.0f;
	y_targ = 0.0f;
	x = 0.0f;
	y = 0.0f;
	m_FractionMovePerTick = 0.015f;
	m_fAxisConstraintLimit = 0.2f * 0.375f;		// a convenient number for the water simulation demo
	m_bUseIndependentAxisConstraint = true;
	m_bUseRadiusConstraint = false;
}
LowPassFilter2D::~LowPassFilter2D()
{
}

void LowPassFilter2D::SetTarget( float nx, float ny )
{
	x_targ = nx;
	y_targ = ny;
	ConstrainPosition();
}

void LowPassFilter2D::ConstrainPosition()
{
	// check and limit the pos if out of bounds
	float dx, dy;
	dx = x_targ - x;
	dy = y_targ - y;

	if( m_bUseRadiusConstraint )
	{
		// radius constraint
		float d = dx*dx + dy*dy;
		if( d > m_fAxisConstraintLimit * m_fAxisConstraintLimit )
		{
			d = sqrt( d );
			// normalize it
			dx	= dx / d;
			dy  = dy / d;
			// multiply by limit to make it as long as limit
			dx *= m_fAxisConstraintLimit;
			dy *= m_fAxisConstraintLimit;
			x = x_targ - dx;
			y = y_targ - dy;
		}
	}

	if( m_bUseIndependentAxisConstraint )
	{
		// independent axis constraint
		if( dx > m_fAxisConstraintLimit )
		{
			x = x_targ - m_fAxisConstraintLimit;
		}
		else if( dx < - m_fAxisConstraintLimit )
		{
			x = x_targ + m_fAxisConstraintLimit;
		}
		if( dy > m_fAxisConstraintLimit )
		{
			y = y_targ - m_fAxisConstraintLimit;
		}
		else if( dy < -m_fAxisConstraintLimit )
		{
			y = y_targ + m_fAxisConstraintLimit;
		}
	}
}

void LowPassFilter2D::Tick()
{
	last_x = x;
	last_y = y;
	x = x + ( x_targ - x ) * m_FractionMovePerTick;
	y = y + ( y_targ - y ) * m_FractionMovePerTick;
	ConstrainPosition();
}
