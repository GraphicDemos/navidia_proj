/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TimeInfo.h

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

#ifndef H_TIMEINFO_H
#define H_TIMEINFO_H


class TimeInfo
{
public:
	double m_dMaxTime;
	double m_dMinTime;
	double m_dAvgTime;
	UINT   m_uNumInAvg;

	// Consider a new time.  Test against the min/max and add to the average.
	void AddTime( double time )
	{
		double tot;
		tot = m_dAvgTime * m_uNumInAvg;
		tot = tot + time;
		m_uNumInAvg++;
		m_dAvgTime = tot / m_uNumInAvg;
		if( time > m_dMaxTime )
			m_dMaxTime = time;
		if( m_dMinTime == -1.0 )
			m_dMinTime = time;
		else if( time < m_dMinTime )
			m_dMinTime = time;
	}
	void PrintToTxtFile( FILE * fp )
	{
		if( fp == NULL )
			return;
		fprintf( fp, "num %u       avg %g      min %g       max %g", m_uNumInAvg, m_dAvgTime, m_dMinTime, m_dMaxTime );
	}

	TimeInfo()
	{
		m_dMaxTime = 0.0;
		m_dMinTime = -1.0;
		m_dAvgTime = 0.0;
		m_uNumInAvg = 0;
	}
};

#endif

