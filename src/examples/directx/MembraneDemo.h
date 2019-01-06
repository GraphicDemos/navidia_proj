/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\Membrane\
File:  MembraneDemo.h

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

#ifndef H_MEMBRANE_DEMO_H
#define H_MEMBRANE_DEMO_H

#include "dxstdafx.h"

#include <LoadXFile.h>
#include <TextureFactory.h>
#include <ShaderManager.h>
#include <TextureDisplay2.h>

#include <vector>
using namespace std;


class MembraneDemo
{
public:
	IDirect3DDevice9 *	m_pD3DDev;
	ShaderManager *		m_pShaderManager;
	LoadXFile *			m_pLoadXFile;

	vector< TextureFilenamePair >	m_vGradientTextures;
	IDirect3DTexture9 **	m_ppTexCurrent;
	tstring *				m_ptstrCurrentTextureFilename;
	size_t					m_uTexCurrent;

	SM_SHADER_INDEX		m_VSHI_MembraneShader;

	ITextureDisplay *	m_pTextureDisplay;			// used only to display a 2D rendering of the color ramp texture 
	TD_TEXID			m_TID_Checkerboard;			// id's returned by ITextureDisplay
	TD_TEXID			m_TID_RampTexture;

	MembraneDemo();
	~MembraneDemo();
	void	SetAllNull();
	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	HRESULT ReloadTextures();
	HRESULT UseNextColorRamp();
	HRESULT Render();
};

#endif
