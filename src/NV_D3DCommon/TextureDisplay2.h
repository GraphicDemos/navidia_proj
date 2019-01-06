/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TextureDisplay2.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
A utility class for displaying textures on 2D screen-aligned quads.
Unlike TextureDisplay, TextureDisplay2 does not use a vertex shader.  Instead, it uses a collection
of quad objects.
It also uses the new NV_D3DMesh library for the geometry.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_TEXTUREDISPLAY2_GJ_H
#define H_TEXTUREDISPLAY2_GJ_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "NV_D3DCommon_decl_.h"

// Generic TextureDisplay interface
class DECLSPEC_NV_D3D_COMMON_API ITextureDisplay
{
public:
	virtual HRESULT Initialize( IDirect3DDevice9 * pDev )	=0;		// abstract, pure virtual base class
	virtual HRESULT Free()									=0;

	virtual HRESULT AddTexture( TD_TEXID * out_ID, IDirect3DTexture9 ** in_ppTex, const FRECT & in_fRect ) =0;
	virtual HRESULT RemoveTexture( const TD_TEXID & in_ID )	=0;

	virtual HRESULT SetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** in_ppTex )			=0;
	virtual HRESULT GetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** out_ppTex )		=0;
	virtual IDirect3DTexture9 * GetTextureP( const TD_TEXID & in_TD )							=0;
	virtual HRESULT SetTextureRect( const TD_TEXID & in_ID, const FRECT & in_fRect )			=0;
	virtual HRESULT GetTextureRect( const TD_TEXID & in_ID, FRECT * pfRect )					=0;
	virtual HRESULT SetTextureCoords( const TD_TEXID & in_ID, const FRECT & in_fRect )			=0;

	virtual HRESULT SetStateForRendering( const TD_TEXID & in_ID, bool bSetPixelState = true,
										bool bSetVertexState = true )							=0;
	virtual HRESULT Render( const TD_TEXID & in_ID, bool bSetPixelState = true, 
										bool bSetVertexState = true )							=0;

	ITextureDisplay()				{};		
	virtual ~ITextureDisplay()		{};		// very imporant that these be (); and not undefined or =0;
};


class DECLSPEC_NV_D3D_COMMON_API TextureDisplay2 : public ITextureDisplay
{
public:
	IDirect3DDevice9 * m_pD3DDev;

	virtual HRESULT Initialize( IDirect3DDevice9 * pDev );
	virtual HRESULT Free();
	
	virtual HRESULT AddTexture( TD_TEXID * out_ID, IDirect3DTexture9 ** in_ppTex, const FRECT & in_fRect );
	virtual HRESULT RemoveTexture( const TD_TEXID & in_ID );		// does nothing

	virtual HRESULT SetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** in_ppTex );
	virtual HRESULT GetTexture( const TD_TEXID & in_ID, IDirect3DTexture9 ** out_ppTex );
	virtual IDirect3DTexture9 * GetTextureP( const TD_TEXID & in_TD );
	virtual HRESULT SetTextureRect( const TD_TEXID & in_ID, const FRECT & in_fRect );
	virtual HRESULT GetTextureRect( const TD_TEXID & in_ID, FRECT * pfRect );
	virtual HRESULT SetTextureCoords( const TD_TEXID & in_ID, const FRECT & in_fRect );

	virtual HRESULT SetStateForRendering( const TD_TEXID & in_ID, bool bSetPixelState = true, bool bSetVertexState = true );
	virtual HRESULT Render( const TD_TEXID & in_ID, bool bSetPixelState = true, bool bSetVertexState = true );

	TextureDisplay2();
	~TextureDisplay2();

protected:
	class Displayable
	{
	public:
		IDirect3DTexture9	**	m_ppTexture;
		UINT					m_uStartIndex;
		UINT					m_uPrimCount;		// 0 if the Displayable is not used
		Displayable()
		{
			m_ppTexture = NULL;
			m_uStartIndex = 0;
			m_uPrimCount = 0;
		};
	};

	Displayable *	m_pDisplayables;
	UINT			m_uNumDisplayables;
	Mesh *			m_pAllQuads;		// one mesh, lots of quads as separate triangles
	MeshVB *		m_pAllQuadsVB;		// one vertex buffer mesh, lots of quads

	HRESULT	ResizeDisplayables( UINT num );
	HRESULT FreeDisplayables();

	HRESULT SetRenderState( const TD_TEXID & in_ID );

	void	MapWindowsCoordsToHCLIP( const FRECT & in_fRect, FRECT * pOutRect );
	void	MapHCLIPCoordsToWindowsCoords( const FRECT & in_fRect, FRECT * pOutRect );

	virtual void SetAllNull()
	{
		m_pD3DDev		= NULL;
		m_pAllQuads		= NULL;
		m_pAllQuadsVB	= NULL;
		m_pDisplayables = NULL;
		m_uNumDisplayables = 0;
	}
};

#endif
