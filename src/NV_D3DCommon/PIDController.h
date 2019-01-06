/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  PID_Controller.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A proportional-integral-derivative (PID) controller.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_PIDCONTROLLER_H
#define H_PIDCONTROLLER_H

class PIDController
{
public:
	double	m_fOverallGain;
	double	m_fProportionalGain;
	double	m_fIntegratorGain;
	double	m_fDerivativeGain;

	bool	m_bVerbose;			// true to enable output of control state to console

	double	Update( double fError, double fCurrentPosition );
	void	SetIntegratorMinMax( double fMin, double fMax );
	PIDController();
	~PIDController();

protected:
	double	m_fIState;			// integrated value
	double	m_fIMax;			// integrated state max value
	double	m_fIMin;			// integrated state min value
	double	m_fLastPosition;	// for computing derivative term
};

#endif
