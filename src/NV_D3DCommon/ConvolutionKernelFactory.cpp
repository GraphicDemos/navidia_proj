/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  ConvolutionKernelFactory.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:


******************************************************************************/

#include "NV_D3DCommonDX9PCH.h"


ConvolutionKernelFactory::~ConvolutionKernelFactory()
{
	UINT i;
	ConvolutionKernel ** ppK;
	for( i=0; i < m_vppKernels.size(); i++ )
	{
		ppK = m_vppKernels.at(i);
		if( ppK == NULL )
			continue;
		SAFE_DELETE( *ppK );
		SAFE_DELETE( ppK );
	}
	m_vppKernels.clear();
}



ConvolutionKernel1D ** ConvolutionKernelFactory::Create1DFromGaussians( Gaussian * pDesc,
																		int num_in_pDesc,
																		int num_elements_to_create )
{
	// pDesc		pointer to array of gaussian descriptions
	// num_desc		number of descriptions pointed to by pDesc
	// num_elements	size of the kernel in elements.  The elements are
	//				centered about zero.
	//
	// The Gaussian properties are adjusted to the kernel size.  In other words,
	//  the coordinates at which the Gaussian amplitudes are calculated are 
	//  normalized to the kernel size, so if you change the kernel size, the 
	//  Gaussian will be scaled to always look the same across the kernel.


	RET_NULL_IF_NULL( pDesc );

	ConvolutionKernel1D ** ppK;
	ppK = Create1DKernel();
	if( ppK == NULL )
	{		
		FMsg("Couldn't create 1D kernel ptr!\n");
		assert( false );
		return( NULL );
	}
	if( *ppK == NULL )
	{
		FMsg("Couldn't create 1D kernel!\n");
		assert( false );
		return( NULL );
	}

	// create elements & calculate element coords
	if( num_elements_to_create < 0 )
		num_elements_to_create = 0;

	ConvolutionKernelElement1D * pElem = new ConvolutionKernelElement1D[ num_elements_to_create ];
	RET_NULL_IF_NULL( pElem );

	int n;
	float center_offset;
	center_offset = ( num_elements_to_create - 1.0f ) / 2.0f;

	float coord_scale;
	coord_scale = center_offset;
	if( coord_scale == 0.0f )
		coord_scale = 1.0f;

	float coord;
	UINT i;

	for( n=0; n < num_elements_to_create; n++ )
	{
		coord = ( n - center_offset );		// coord at which sample is taken
		pElem[n].m_fDx = coord;

		coord = coord / coord_scale;		// coord at which gaussian is calculated

		// calculate element weight
		pElem[n].m_fCoef = 0.0f;
		for( i = 0; i < (UINT)num_in_pDesc; i++ )
		{
			pElem[n].m_fCoef += pDesc[i].Evaluate( coord );
		}

		(*ppK)->AddElement( pElem[n] );
	}

	SAFE_DELETE_ARRAY( pElem );

	return( ppK );
}


ConvolutionKernel1D ** ConvolutionKernelFactory::Create1DKernel()
{
	ConvolutionKernel1D ** ppK = new (ConvolutionKernel1D*);
	RET_NULL_IF_NULL( ppK );
	*ppK = new ConvolutionKernel1D;
	if( *ppK == NULL )
	{
		SAFE_DELETE( ppK );
		return( NULL );
	}
	m_vppKernels.push_back( (ConvolutionKernel**) ppK );
	return( ppK );
}

