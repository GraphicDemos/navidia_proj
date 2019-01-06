/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\TwoSidedPolys\
File:  TwoSidedPolysDemo.h

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

#ifndef H_TWOSIDEDPOLYS_H
#define H_TWOSIDEDPOLYS_H

#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/SDKmisc.h>

#include <NV_D3DCommon/NV_D3DCommonTypes.h>

class ShaderManager;
class TextureFactory;
class MeshVB;

class TwoSidedPolysDemo
{
public :
	IDirect3DDevice9 *		m_pD3DDev;
	MeshVB *				m_pTwistyMeshVB;
	MeshVB *				m_pLightMeshVB;

	ShaderManager *			m_pShaderManager;
	SM_SHADER_INDEX			m_VSHI_TwoSided;
	SM_SHADER_INDEX			m_PSHI_TwoSided;
	SM_SHADER_INDEX			m_PSHI_TwoSidedNoLight;

	TextureFactory *		m_pTextureFactory;
	IDirect3DTexture9 **	m_ppTexFrontCurrent;
	IDirect3DTexture9 **	m_ppTexTranslucentCurrent;


	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing );
	HRESULT Render( float fGlobalTimeInSeconds );

	TwoSidedPolysDemo();
	~TwoSidedPolysDemo();
	void SetAllNull();
};

#endif

