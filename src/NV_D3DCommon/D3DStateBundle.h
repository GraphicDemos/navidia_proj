/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DStateBundle.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A D3DStateBundle can contain any D3D Device state settings.  It stores pointers to some state data,
such as matrices and shader constants, so you must guarantee that the data pointed to remains valid
for as long as you use the bundle.

-------------------------------------------------------------------------------|--------------------*/


#ifndef H_D3DSTATEBUNDLE_H
#define H_D3DSTATEBUNDLE_H

#include <d3d9.h>
#include <d3dx9.h>

#include <vector>
using namespace std;

#include "D3DDeviceStates.h"

class D3DStateBundle
{
public:
	vector< D3DRenderState >		m_vRenderStates;
	vector< D3DTexture >			m_vTextures;
	vector< D3DTextureStageState >	m_vTextureStates;
	vector< D3DSamplerState >		m_vSamplerStates;
	vector< D3DTransform >			m_vTransforms;
	vector< D3DRenderTarget >		m_vRenderTargets;
	vector< D3DDepthStencilSurface> m_vDepthStencilSurfaces;
	vector< D3DStreamSource >		m_vStreamSources;

	vector< D3DShaderConstant * >		m_vShaderConstants;

	void	Clear();
	HRESULT Apply( IDirect3DDevice9 * pDev, bool bVerbose = false );
	void	ListToDebugConsole();

	void	SetRenderState( D3DRENDERSTATETYPE State, DWORD dwValue );
	void	SetTexture( DWORD Sampler, IDirect3DTexture9 * pTexture );
	void	SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value );
	void	SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value );
	void	SetTransform( D3DTRANSFORMSTATETYPE state, D3DMATRIX * pMat );
	void	SetRenderTarget( DWORD dwIndex, IDirect3DSurface9 * pSurf );
	void	SetDepthStencilSurface( IDirect3DSurface9 * pSurf );
	void	SetStreamSource( UINT num, IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride );
	void	SetVertexShaderConstantF( UINT start_register, float * pData, UINT count );
	void	SetVertexShaderConstnatB( UINT start_register, BOOL * pData, UINT count );
	void	SetVertexShaderConstantI( UINT start_register, int * pData, UINT count );
	void	SetPixelShaderConstantF( UINT start_register, float * pData, UINT count );
	void	SetPixelShaderConstantB( UINT start_register, BOOL * pData, UINT count );
	void	SetPixelShaderConstantI( UINT start_register, int * pData, UINT count );

	D3DStateBundle();
	~D3DStateBundle();
};

class D3DGeometryBundle
{
public:
	D3DVertexDeclaration **			m_ppVertexDeclaration;
	vector< D3DStreamSource >		m_vStreamSources;
	D3DIndices **					m_ppIndices;

	void	Clear();
	HRESULT	Apply( IDirect3DDevice9 * pDev, bool bVerbose );

	D3DGeometryBundle();
	~D3DGeometryBundle();
	void SetAllNull()
	{
		m_ppIndices				= NULL;
		m_ppVertexDeclaration	= NULL;
	}
};


#endif					// H_D3DSTATEBUNDLE_H
