/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  PID_Controller.cpp

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

#include "PIDController.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

#define PID_UNSET_POSTION 9.9e9

PIDController::PIDController()
{
	m_fOverallGain			= 1.0f;
	m_fProportionalGain		= 0.8;
	m_fIntegratorGain		= 0.1;
	m_fDerivativeGain		= 1.0;
	m_fLastPosition			= PID_UNSET_POSTION;
	m_bVerbose				= false;
	m_fIState				= 0.0;
	m_fIMin					= -10000.0;
	m_fIMax					=  10000.0;
}

PIDController::~PIDController()
{
}

//----------------------------------------------------------------------------------
// Sets the min and max values for the integrated signal.  This is so that you can 
//  clamp the integration total so that the total times the gain is limited to your
//  min or max control signal.  In effect, this prevents the integrated total from
//  growing too large and slamming the controller up against one of the limits.
//----------------------------------------------------------------------------------
void PIDController::SetIntegratorMinMax( double fMin, double fMax )
{
	m_fIMax = fMax;
	m_fIMin = fMin;
}

//----------------------------------------------------------------------------------
// fError is the current position minus the target position
// fCurrentPosition is the current position of whatever the PID is trying to control
// return:  New control value based on current & previous errors & positions
//----------------------------------------------------------------------------------
double PIDController::Update( double fError, double fCurrentPosition )
{
	// zero derivative when we begin control
	if( m_fLastPosition == PID_UNSET_POSTION )
		m_fLastPosition = fCurrentPosition;

	double prop_cont;
	prop_cont = -m_fProportionalGain * fError;
	double deriv_cont;
	deriv_cont = m_fDerivativeGain * ( fCurrentPosition - m_fLastPosition );
	m_fLastPosition = fCurrentPosition;

	double integ_cont;
	m_fIState += fError;
	if( m_fIState > m_fIMax )
		m_fIState = m_fIMax;
	else if( m_fIState < m_fIMin )
		m_fIState = m_fIMin;
	integ_cont = -m_fIntegratorGain * m_fIState;

	double signal;
	signal = m_fOverallGain*( deriv_cont + integ_cont + prop_cont );

	if( m_bVerbose == true )
	{
		FMsg("PID err= %g  pos= %g    ret signal= %g    prop= %g  integ= %g  deriv= %g\n", fError, fCurrentPosition, signal, prop_cont, integ_cont, deriv_cont );
	}

	return( signal );
}
