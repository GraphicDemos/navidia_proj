/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  ConvolutionKernelFactory.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
The class structure is poor because of problems of inheritance and in-line 
member function definitions.


-------------------------------------------------------------------------------|--------------------*/

#ifndef GJ_CONVOLUTIONKERNELFACTORY_H_
#define GJ_CONVOLUTIONKERNELFACTORY_H_

#include <windows.h>
#include <vector>
#include <math.h>

#include "shared\NV_Common.h"
using namespace std;

class ConvolutionKernelElement1D;
class ConvolutionKernelElement2D;
class ConvolutionKernelElement3D;
typedef	ConvolutionKernelElement1D		KernelElement;
typedef ConvolutionKernelElement1D *	KernelElementP;
typedef ConvolutionKernelElement1D **	KernelElementPP;


// Describes width, amplitude, and offset from 0.0 of a Gaussian
//@@ only 1D for now, wouldn't a templated class for various dimensions be good? yes.
class Gaussian
{	
public:
	float	m_fRadiusScale;		// multiplies the coordinate 
	float	m_fOffsetCoord;		// offset from 0.0
	float	m_fAmplitude;
	float	m_fOffsetOutput;	// offset the output value by this amount

	Gaussian()		{	SetToZero();	};
	~Gaussian()		{	SetToZero();	};
	void	SetToZero()
	{
		m_fRadiusScale = m_fOffsetCoord = m_fAmplitude = m_fOffsetOutput = 0.0f;
	};

			// 1D evaluation, y = 0.0f
	float	Evaluate( float coordinate )
	{
		coordinate = ( coordinate - m_fOffsetCoord ) * m_fRadiusScale;
		coordinate = (float) ( m_fAmplitude / exp ( coordinate * coordinate ));
		coordinate += m_fOffsetOutput;
		return( coordinate );
	}
};


class ConvolutionKernelElement1D
{
public:
	float	m_fDx;		// offset from kernel center at which to take the sample
	float	m_fCoef;	// coefficient

	ConvolutionKernelElement1D()
	{
		m_fDx = m_fCoef = 0.0f;
	};
	float	GetWeight()
	{
		return( m_fCoef );
	}
};

class ConvolutionKernelElement2D : public ConvolutionKernelElement1D
{
public:
	float	m_fDy;		// offset from kernel center at which to take the sample

	ConvolutionKernelElement2D()
		: ConvolutionKernelElement1D()
	{
		m_fDy = 0.0f;
	};
};

class ConvolutionKernelElement3D : public ConvolutionKernelElement2D
{
public:
	float	m_fDz;

	ConvolutionKernelElement3D()
		: ConvolutionKernelElement2D()
	{
		m_fDz = 0.0f;
	};
};


class ConvolutionKernel
{
public:
	enum KernelType
	{
		KERNEL_1D,
		KERNEL_2D,
		KERNEL_3D,
		KERNEL_UNSET
	};

	vector< KernelElement* >	m_vpKernelElements;

	ConvolutionKernel()
	{
		m_eKernelType = KERNEL_UNSET;
	}
	virtual ~ConvolutionKernel()
	{
		RemoveElements();
		m_eKernelType = KERNEL_UNSET;
	}


	KernelType	GetKernelType()		{ return( m_eKernelType );	};

	void	RemoveElements()
	{		
		UINT i;
		for( i=0; i < m_vpKernelElements.size(); i++ )
		{
			SAFE_DELETE( m_vpKernelElements.at(i) );
		}
		m_vpKernelElements.clear();
	};

	UINT	GetNumElements()
	{
		return( (UINT) m_vpKernelElements.size() );
	}
protected:
	KernelType					m_eKernelType;

	//@@@ SLOPPY - added as pointers but deleted by remove -- no alloc func
	void	AddElement( KernelElement * pElement )
	{
		m_vpKernelElements.push_back( pElement );
	}
};

class ConvolutionKernel1D : public ConvolutionKernel
{
public:
	ConvolutionKernel1D()
	{	
		m_eKernelType = KERNEL_1D;
	};

	void	AddElement( const ConvolutionKernelElement1D & element )
	{
		ConvolutionKernelElement1D * p1 = new ConvolutionKernelElement1D;
		*p1 = element;
		this->ConvolutionKernel::AddElement( p1 );
	}

	float	GetWeight( int element_index )
	{
		float weight = 0.0f;
		UINT ksz;
		ksz = (UINT) m_vpKernelElements.size();
		if( element_index >= 0 && (UINT)element_index < ksz )
		{
			ConvolutionKernelElement1D * pE;
			pE = m_vpKernelElements.at( element_index );
			if( pE != NULL )
			{
				weight = pE->GetWeight();
			}
		}
		return( weight );
	};

};

class ConvolutionKernel2D : public ConvolutionKernel
{
public:
	ConvolutionKernel2D()	{ m_eKernelType = KERNEL_2D; };

	void	AddElement( const ConvolutionKernelElement2D & element )
	{
		ConvolutionKernelElement2D * p1 = new ConvolutionKernelElement2D;
		*p1 = element;
		this->ConvolutionKernel::AddElement( p1 );
	}
};

class ConvolutionKernel3D : public ConvolutionKernel
{
public:
	ConvolutionKernel3D()	{ m_eKernelType = KERNEL_3D; };

	void	AddElement( const ConvolutionKernelElement3D & element )
	{
		ConvolutionKernelElement3D * p1 = new ConvolutionKernelElement3D;
		*p1 = element;
		this->ConvolutionKernel::AddElement( p1 );
	}
};

class ConvolutionKernelFactory
{
public: 
	ConvolutionKernelFactory()	{};
	~ConvolutionKernelFactory();
	ConvolutionKernel1D ** Create1DFromGaussians( Gaussian * pDesc, int num_in_pDesc,
													int num_elements_to_create );
	ConvolutionKernel1D ** Create1DKernel();
protected:
	vector< ConvolutionKernel** >	m_vppKernels;
};

#endif
