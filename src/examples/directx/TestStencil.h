/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\StencilShadow\
File:  TestStencil.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Performs a simple increment and decrement of the stencil buffer and displays the stencil values
as RGB colors.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_TESTSTENCIL_H
#define H_TESTSTENCIL_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include <vector>

class TestStencil
{
public:
	IDirect3DDevice9 *	m_pD3DDev;
	ITextureDisplay *	m_pTextureDisplay;

	TD_TEXID			m_Fullscreen;
	vector< TD_TEXID >	m_vStencilWriteRects;

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	HRESULT Render();

	HRESULT	RevealStencilValues( DWORD dwARGBColorPerLayer, bool bClearColor );

	TestStencil();
	~TestStencil();
	void SetAllNull()
	{
		m_pD3DDev				= NULL;
		m_pTextureDisplay		= NULL;
	};
};

#endif		// H_TESTSTENCIL_H
