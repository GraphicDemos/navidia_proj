/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  UtilityFunctions.h

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

#ifndef H_D3DCOMMON_UTILITYFUNCS_H
#define H_D3DCOMMON_UTILITYFUNCS_H

#include "shared\NV_StringFuncs.h"		// SDK\LIBS\inc\shared\NV_StringFuncs.h
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>

void	ListMatrix( const D3DXMATRIX & matrix );
void	ListVector( const D3DXVECTOR3 & vector );		// no linefeed after last element
void	ListVector( const TCHAR * prefix, const D3DXVECTOR3 & vector, const TCHAR * postfix );
void	ListVector( const D3DXVECTOR4 & vector );
void	ListVector( const TCHAR * prefix, const D3DXVECTOR4 & vector, const TCHAR * postfix );

void	TextOutMatrix( D3DXMATRIX * pMat );
tstring GetStringOfBits( SHORT n );

//-----------------------------------------------------------------------------------
// Returning [const TCHAR *] is ok since the functions return string literals
const TCHAR * GetStrD3DFORMAT(	D3DFORMAT format );
void		  GetStrD3DFVF(		DWORD fvf, TCHAR * out_pBuf, size_t sz );
void		  GetStrD3DLOCK(	DWORD flags, TCHAR * pBuf, size_t sz );
const TCHAR * GetStrD3DMULTISAMPLE_TYPE(	D3DMULTISAMPLE_TYPE multisampletype );
const TCHAR * GetStrD3DPOOL(	D3DPOOL pool );
const TCHAR * GetStrD3DRESOURCETYPE(	D3DRESOURCETYPE restype );
void		  GetStrUsage(	DWORD usage, TCHAR * pBuf, size_t sz );
const TCHAR * GetStrRenderState(	D3DRENDERSTATETYPE renderstate );
void		  GetStrRenderStateValue(	D3DRENDERSTATETYPE renderstate, DWORD value, TCHAR * out_pBuf, size_t buf_size );
const TCHAR * GetStrTextureStageState(	D3DTEXTURESTAGESTATETYPE type );
void		  GetStrTextureStageStateValue(		D3DTEXTURESTAGESTATETYPE type, DWORD value, TCHAR * out_pBuf, size_t buf_size );
const TCHAR * GetStrSamplerState(	D3DSAMPLERSTATETYPE type );
void		  GetStrSamplerStateValue(	D3DSAMPLERSTATETYPE type, DWORD value, TCHAR * out_pBuf, size_t buf_size );
const TCHAR * GetStrVERTEXELEMENT9Type(	 BYTE Type );
const TCHAR * GetStrVERTEXELEMENT9Method(	BYTE Method );
const TCHAR * GetStrVERTEXELEMENT9Usage(	BYTE Usage );
const TCHAR * GetStrD3DPRIMITIVETYPE(	D3DPRIMITIVETYPE primt );
const TCHAR * GetStrD3DQUERYTYPE(	D3DQUERYTYPE qt );

tstring tstrD3DCLEAR( DWORD flags );
tstring tstrD3DFORMAT( D3DFORMAT format );
tstring tstrD3DFVF( DWORD fvf );
tstring tstrD3DLOCK( DWORD flags );
tstring tstrD3DPOOL( D3DPOOL pool );
tstring tstrD3DUSAGE( DWORD usage );
tstring tstrRenderState( D3DRENDERSTATETYPE renderstate );
tstring tstrRenderStateValue( D3DRENDERSTATETYPE renderstate, DWORD value );
tstring tstrTextureStageStateValue( D3DTEXTURESTAGESTATETYPE type, DWORD value );
tstring tstrSamplerStateValue( D3DSAMPLERSTATETYPE type, DWORD value );
tstring tstrD3DMULTISAMPLE_TYPE( D3DMULTISAMPLE_TYPE mst );
tstring tstrD3DSURFACE_DESC( D3DSURFACE_DESC & desc );
tstring tstrD3DPRIMITIVETYPE( D3DPRIMITIVETYPE primt );
tstring tstrD3DQUERYTYPE( D3DQUERYTYPE qt );

tstring tstrBytes( BYTE * pBytes, size_t num_bytes );
tstring tstrShader( IDirect3DVertexShader9 * pShader );
tstring tstrShader( IDirect3DPixelShader9 * pShader );
tstring tstrShader( const DWORD * pdw );		// pdw must have valid shader opcode terminator!
// Breaks shader assembly code into individual lines
void	tstrShaderLines( const tstring & tstrShader, std::vector< tstring > * pvOutLines );

UINT	GetSizeOfFormatInBits( const D3DFORMAT & format );
UINT	GetSizeOfTypeInBytes( const D3DVERTEXELEMENT9 & elem );
UINT	GetSizeOfTypeInBytes( const D3DDECLTYPE & type );
UINT	GetDimensionOfType( const D3DDECLTYPE & type );
UINT	GetShaderSizeInBytes( const DWORD * pFunction );

size_t	GetEstTextureSizeInBytes( UINT Width, UINT Height, UINT Levels, D3DFORMAT Format );

HRESULT ConvertToFloat4( const void *pSrc, D3DDECLTYPE SrcFormat, D3DXVECTOR4 & out_Vec4 );
HRESULT ConvertFloat4To( const D3DXVECTOR4 & vec4, D3DDECLTYPE DestFormat, void * pDest );
HRESULT ConvertData( const void * pSrc, D3DDECLTYPE SrcFormat, void * pDest, D3DDECLTYPE DestFormat );

#endif					// H_D3DCOMMON_UTILITYFUNCS_H

