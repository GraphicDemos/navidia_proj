/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceStates.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
These structures accept pointers to data.  Unlike the D3D runtime, when you set a state, these
structures do NOT copy the data pointed to.  They keep only the data pointer, so you must make sure
that your data does not go out of scope for as long as these structures hold your pointers.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_D3DDEVICESTATES_H
#define H_D3DDEVICESTATES_H

#include <d3d9.h>
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include <string>
using namespace std;

// Defines the following:
/*
class D3DDeviceState
class D3DRenderState			: public D3DDeviceState
class D3DTexture				: public D3DDeviceState
class D3DTextureStageState		: public D3DDeviceState
class D3DSamplerState			: public D3DDeviceState
class D3DTransform				: public D3DDeviceState
class D3DRenderTarget			: public D3DDeviceState
class D3DDepthStencilSurface	: public D3DDeviceState
class D3DStreamSource			: public D3DDeviceState
class D3DVertexDeclaration		: public D3DDeviceState
class D3DIndices				: public D3DDeviceState

class D3DShaderConstant : public D3DDeviceState
class D3DVertexShaderConstantF : public D3DShaderConstant
class D3DVertexShaderConstantB : public D3DShaderConstant
class D3DVertexShaderConstantI : public D3DShaderConstant
class D3DPixelShaderConstantF : public D3DShaderConstant
class D3DPixelShaderConstantB : public D3DShaderConstant
class D3DPixelShaderConstantI : public D3DShaderConstant
*/


class D3DDeviceState
{
public:
	virtual HRESULT ApplySafe( IDirect3DDevice9 * pDev )			=0;
	virtual HRESULT Apply( IDirect3DDevice9 * pDev )				=0;
	virtual void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DRenderState : public D3DDeviceState
{
public:
	D3DRENDERSTATETYPE	m_State;
	DWORD				m_dwValue;

	D3DRenderState();
	D3DRenderState( D3DRENDERSTATETYPE State, DWORD dwValue );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DTexture : public D3DDeviceState
{
public:
	DWORD				m_dwSampler;
	IDirect3DTexture9 *	m_pTexture;

	D3DTexture();
	D3DTexture( DWORD Sampler, IDirect3DTexture9 * pTexture );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DTextureStageState : public D3DDeviceState
{
public:
	DWORD						m_dwStage;
    D3DTEXTURESTAGESTATETYPE	m_Type;
    DWORD						m_dwValue;

	D3DTextureStageState();
	D3DTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DSamplerState : public D3DDeviceState
{
public:
	DWORD					m_dwSampler;
	D3DSAMPLERSTATETYPE		m_Type;
	DWORD					m_dwValue;

	D3DSamplerState();
	D3DSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DTransform : public D3DDeviceState
{
public:
	D3DTRANSFORMSTATETYPE	m_State;
	D3DMATRIX *				m_pMatrix;

	D3DTransform();
	D3DTransform( D3DTRANSFORMSTATETYPE state, D3DMATRIX * pMat );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DRenderTarget : public D3DDeviceState
{
public:
	DWORD				m_dwIndex;
	IDirect3DSurface9 *	m_pColorSurface;

	D3DRenderTarget();
	D3DRenderTarget( DWORD dwIndex, IDirect3DSurface9 * pSurf );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DDepthStencilSurface : public D3DDeviceState
{
public:
	IDirect3DSurface9 *	m_pDepthSurface;

	D3DDepthStencilSurface();
	D3DDepthStencilSurface( IDirect3DSurface9 * pSurf );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DStreamSource : public D3DDeviceState
{
public:
	UINT						m_uStreamNumber;
	IDirect3DVertexBuffer9 *	m_pStreamData;
	UINT						m_uOffsetInBytes;
	UINT						m_uStride;

	D3DStreamSource();
	D3DStreamSource( UINT num, IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DVertexDeclaration : public D3DDeviceState
{
public:
	IDirect3DVertexDeclaration9 * m_pDecl;

	D3DVertexDeclaration();
	D3DVertexDeclaration( IDirect3DVertexDeclaration9 * pDecl );
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DIndices : public D3DDeviceState
{
public:
	IDirect3DIndexBuffer9 * m_pIndexData;

	D3DIndices();
	D3DIndices( IDirect3DIndexBuffer9 * pIndexData);
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

//---------------------------------------------------------------------------
class D3DShaderConstant : public D3DDeviceState
{
public:
	UINT	m_uStartRegister;
	void *	m_pConstantData;
    UINT	m_uCount;
	D3DShaderConstant();
	D3DShaderConstant( UINT start_register, void * pData, UINT count )
	{
		m_uStartRegister = start_register;
		m_pConstantData = pData;
		m_uCount = count;
	};
};

class D3DVertexShaderConstantF : public D3DShaderConstant
{
public:
	D3DVertexShaderConstantF();
	D3DVertexShaderConstantF( UINT start_register, float * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DVertexShaderConstantB : public D3DShaderConstant
{
public:
	D3DVertexShaderConstantB();
	D3DVertexShaderConstantB( UINT start_register, BOOL * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DVertexShaderConstantI : public D3DShaderConstant
{
public:
	D3DVertexShaderConstantI();
	D3DVertexShaderConstantI( UINT start_register, int * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DPixelShaderConstantF : public D3DShaderConstant
{
public:
	D3DPixelShaderConstantF();
	D3DPixelShaderConstantF( UINT start_register, float * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DPixelShaderConstantB : public D3DShaderConstant
{
public:
	D3DPixelShaderConstantB();
	D3DPixelShaderConstantB( UINT start_register, BOOL * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

class D3DPixelShaderConstantI : public D3DShaderConstant
{
public:
	D3DPixelShaderConstantI();
	D3DPixelShaderConstantI( UINT start_register, int * pData, UINT count )
		: D3DShaderConstant( start_register, pData, count )
	{
	};
	HRESULT ApplySafe( IDirect3DDevice9 * pDev );
	HRESULT Apply( IDirect3DDevice9 * pDev );
	void	GetReportStr( char * pBuf, size_t buf_size );
};

#endif
