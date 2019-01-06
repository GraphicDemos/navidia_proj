/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  TEST_PA_Water.h

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

#ifndef H_TEST_PAWATER_H
#define H_TEST_PAWATER_H

class PA_Water;
struct IDirect3DDevice9;

class TEST_PA_Water
{
public:
	PA_Water *			m_pWater;	
	IDirect3DDevice9 *	m_pD3DDev;
	ShaderManager *		m_pShaderManager;

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	HRESULT Tick( double fGlobalTimeInSeconds );

	HRESULT ConfirmDevice( D3DCAPS9 * pCaps, DWORD d, D3DFORMAT BackBufferFormat )
	{
		return( S_OK );
	};
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	TEST_PA_Water();
	~TEST_PA_Water();
	void SetAllNull();
};

#endif
