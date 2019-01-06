/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceStates.cpp

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

#include "NV_D3DCommonDX9PCH.h"
#include <shared/UtilityFunctions.h>

#include <stdio.h>
#include <stdlib.h>

#define TMPSTRSZ	512

void D3DDeviceState::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	strncpy( pBuf, "D3DDeviceState base class\n", buf_size );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DRenderState::D3DRenderState()
{
}
D3DRenderState::D3DRenderState( D3DRENDERSTATETYPE State, DWORD dwValue )
{
	m_State = State;
	m_dwValue = dwValue;
}
HRESULT D3DRenderState::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev )) ;
}
HRESULT D3DRenderState::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetRenderState( m_State, m_dwValue ) );
}
void	D3DRenderState::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	const TCHAR * pState = GetStrRenderState( m_State );
	TCHAR p2[ TMPSTRSZ ];
	GetStrRenderStateValue( m_State, m_dwValue, p2, TMPSTRSZ );
	_snprintf( pBuf, buf_size, "SetRenderState( %s, %s );\n", pState, p2 );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DTexture::D3DTexture()
{
}
D3DTexture::D3DTexture( DWORD Sampler, IDirect3DTexture9 * pTexture )
{
	m_dwSampler = Sampler;
	m_pTexture = pTexture;
}
HRESULT D3DTexture::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DTexture::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetTexture( m_dwSampler, m_pTexture ) );
}
void	D3DTexture::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetTexture( %u, 0x%p );\n", m_dwSampler, m_pTexture );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DTextureStageState::D3DTextureStageState()
{
}
D3DTextureStageState::D3DTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value )
{
	m_dwStage = stage;
	m_Type = type;
	m_dwValue = value;
}
HRESULT D3DTextureStageState::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DTextureStageState::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetTextureStageState( m_dwStage, m_Type, m_dwValue ) );
}
void	D3DTextureStageState::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	const TCHAR * pState = GetStrTextureStageState( m_Type );
	TCHAR p2[ TMPSTRSZ ];
	GetStrTextureStageStateValue( m_Type, m_dwValue, p2, TMPSTRSZ );
//	_snprintf( pBuf, buf_size, "SetTextureStageState( %u, %u, %u );\n", m_dwStage, m_Type, m_dwValue );
	_snprintf( pBuf, buf_size, "SetTextureStageState( %u, %s, %s );\n", m_dwStage, pState, p2 );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DSamplerState::D3DSamplerState()
{
}
D3DSamplerState::D3DSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
{
	m_dwSampler = sampler;
	m_Type = type;
	m_dwValue = value;
}
HRESULT D3DSamplerState::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DSamplerState::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetSamplerState( m_dwSampler, m_Type, m_dwValue ));
}
void	D3DSamplerState::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	const TCHAR * pState = GetStrSamplerState( m_Type );
	TCHAR p2[ TMPSTRSZ ];
	GetStrSamplerStateValue( m_Type, m_dwValue, p2, TMPSTRSZ );
//	_snprintf( pBuf, buf_size, "SetSamplerState( %u, %u, %u );\n", m_dwSampler, m_Type, m_dwValue );
	_snprintf( pBuf, buf_size, "SetSamplerState( %u, %s, %s );\n", m_dwSampler, pState, p2 );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DTransform::D3DTransform()
{
}
D3DTransform::D3DTransform( D3DTRANSFORMSTATETYPE state, D3DMATRIX * pMat )
{
	m_State = state;
	m_pMatrix = pMat;
}
HRESULT D3DTransform::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DTransform::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetTransform( m_State, m_pMatrix ) );
}
void	D3DTransform::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetTransform( %u, 0x%p );\n", m_State, m_pMatrix );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
D3DRenderTarget::D3DRenderTarget()
{
}
D3DRenderTarget::D3DRenderTarget( DWORD dwIndex, IDirect3DSurface9 * pSurf )
{
	m_dwIndex = dwIndex;
	m_pColorSurface = pSurf;
}
HRESULT D3DRenderTarget::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DRenderTarget::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetRenderTarget( m_dwIndex, m_pColorSurface ) );
}
void D3DRenderTarget::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetRenderTarget( %u, 0x%p );\n", m_dwIndex, m_pColorSurface );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
D3DDepthStencilSurface::D3DDepthStencilSurface()
{
}
D3DDepthStencilSurface::D3DDepthStencilSurface( IDirect3DSurface9 * pSurf )
{
	m_pDepthSurface = pSurf;
}
HRESULT D3DDepthStencilSurface::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DDepthStencilSurface::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetDepthStencilSurface( m_pDepthSurface ) );
}
void	D3DDepthStencilSurface::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetDepthStencilSurface( 0x%p );\n", m_pDepthSurface );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
D3DStreamSource::D3DStreamSource()
{
}
D3DStreamSource::D3DStreamSource( UINT num, IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride )
{
	m_uStreamNumber = num;
	m_pStreamData = pVB;
	m_uOffsetInBytes = offset;
	m_uStride = stride;
}
HRESULT D3DStreamSource::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DStreamSource::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetStreamSource( m_uStreamNumber, m_pStreamData, m_uOffsetInBytes, m_uStride ) );
}
void	D3DStreamSource::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetStreamSource( %u, 0x%p, %u, %u );\n", m_uStreamNumber, m_pStreamData, m_uOffsetInBytes, m_uStride );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
D3DVertexDeclaration::D3DVertexDeclaration()
{
}
D3DVertexDeclaration::D3DVertexDeclaration( IDirect3DVertexDeclaration9 * pDecl )
{
	m_pDecl = pDecl;
}
HRESULT D3DVertexDeclaration::ApplySafe( IDirect3DDevice9 * pDev )
{	
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DVertexDeclaration::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetVertexDeclaration( m_pDecl ) );
}
void	D3DVertexDeclaration::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetVertexDeclaration( 0x%p );\n", m_pDecl );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
D3DIndices::D3DIndices()
{
}
D3DIndices::D3DIndices( IDirect3DIndexBuffer9 * pIndexData )
{
	m_pIndexData = pIndexData;
}
HRESULT D3DIndices::ApplySafe( IDirect3DDevice9 * pDev )
{	
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DIndices::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetIndices( m_pIndexData ) );
}
void	D3DIndices::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetIndices( 0x%p );\n", m_pIndexData );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
HRESULT D3DVertexShaderConstantF::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DVertexShaderConstantF::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetVertexShaderConstantF( m_uStartRegister, (float*)m_pConstantData, m_uCount ));
}
void	D3DVertexShaderConstantF::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetVertexShaderConstantF( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
HRESULT D3DVertexShaderConstantB::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DVertexShaderConstantB::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetVertexShaderConstantB( m_uStartRegister, (BOOL*)m_pConstantData, m_uCount ));
}
void	D3DVertexShaderConstantB::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetVertexShaderConstantB( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
HRESULT D3DVertexShaderConstantI::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DVertexShaderConstantI::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetVertexShaderConstantB( m_uStartRegister, (int*)m_pConstantData, m_uCount ));
}
void	D3DVertexShaderConstantI::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetVertexShaderConstantI( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}

//---------------------------------------------------------------
HRESULT D3DPixelShaderConstantF::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DPixelShaderConstantF::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetPixelShaderConstantF( m_uStartRegister, (float*)m_pConstantData, m_uCount ));
}
void	D3DPixelShaderConstantF::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetPixelShaderConstantF( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
HRESULT D3DPixelShaderConstantB::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DPixelShaderConstantB::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetPixelShaderConstantB( m_uStartRegister, (BOOL*)m_pConstantData, m_uCount ));
}
void	D3DPixelShaderConstantB::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetPixelShaderConstantB( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}
//---------------------------------------------------------------
HRESULT D3DPixelShaderConstantI::ApplySafe( IDirect3DDevice9 * pDev )
{
	FAIL_IF_NULL( pDev );
	return( Apply( pDev ));
}
HRESULT D3DPixelShaderConstantI::Apply( IDirect3DDevice9 * pDev )
{
	return( pDev->SetPixelShaderConstantB( m_uStartRegister, (int*)m_pConstantData, m_uCount ));
}
void	D3DPixelShaderConstantI::GetReportStr( char * pBuf, size_t buf_size )
{
	RET_IF( pBuf == NULL );
	RET_IF( buf_size == 0 );
	_snprintf( pBuf, buf_size, "SetPixelShaderConstantI( %u, 0x%p, %u );\n", m_uStartRegister, m_pConstantData, m_uCount );
	pBuf[ buf_size - 1 ] = 0;
}