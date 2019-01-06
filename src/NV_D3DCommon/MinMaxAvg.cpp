/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  MinMaxAvg.cpp

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
#include "MinMaxAvg.h"

#define MINMAX_MAXDOUBLE 1.7e308

MinMaxAvg::MinMaxAvg()
{
	Reset();
}

void MinMaxAvg::Reset()
{
// 
	m_dMax = 0.0;
	m_dMin = 0.0;
	m_dTotal = 0.0;
	m_uNumInTotal = 0;
}

// Consider a new time.  Test against the min/max and add to the average.
void MinMaxAvg::AddValue( double val )
{
	if( m_uNumInTotal == 0 )
	{
		m_dMax = val;
		m_dMin = val;
	}
	else
	{
		if( val < m_dMin )
			m_dMin = val;
		if( val > m_dMax )
			m_dMax = val;
	}
	m_uNumInTotal++;
	m_dTotal += val;
}

double MinMaxAvg::GetAverage()
{
	double avg;
	if( m_uNumInTotal > 0 )
		avg = m_dTotal / ((double)m_uNumInTotal);
	else
		avg = 0.0;
	return( avg );
}

void MinMaxAvg::PrintToTxtFile( FILE * fp )
{
	if( fp == NULL )
		return;
	fprintf( fp, "num= %u    avg= %g    min= %g    max= %g", m_uNumInTotal, GetAverage(), m_dMin, m_dMax );
}

tstring MinMaxAvg::ToTString()
{
	tstring tstr;
	tstr = tstrPrintf( TEXT("min= %g  max= %g  avg= %g  num=%u"), m_dMin, m_dMax, GetAverage(), m_uNumInTotal );
	return( tstr );
}

