/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  WaterCoupler.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Class for coupling together two water animations.
One for a water texture which tiles across a large area.  The other for a
local detail water for a character to interact with.  This detail water blends
into the tiled water for a seamless transition.

-------------------------------------------------------------------------------|--------------------*/

#ifndef		H__WATERCOUPLER_GJ_H
#define		H__WATERCOUPLER_GJ_H

#include "dxstdafx.h"

#include <NV_D3DCommon/NV_D3DCommonTypes.h>
class PA_Water;
class MeshVBDot3;
class MeshVB;
class ITextureDisplay;
class Mesh;

struct WaterDesc
{
	int		tile_res_x;
	int		tile_res_y;
	int		detail_res_x;		// size of detail area in world space 
	int		detail_res_y;		//  determined from resolution & tile_width, tile_height
	float	tile_width;			// width & height in world space
	float	tile_height;
	int		num_tiles_x;
	int		num_tiles_y;
	// Assumes world space equals water object space 
	// No transform for water at various orientations yet.
	float	tile_base_x;		// base points of a tile, where tex coord is integer
	float	tile_base_y;	
};

class WaterCoupler
{
public:
	WaterDesc		m_WaterDesc;
	PA_Water *		m_pCA_WaterTiled;
	PA_Water *		m_pCA_WaterDetail;

	// These hold the water geometry
	MeshVBDot3	*	m_pTiledWaterGeo;
	MeshVBDot3	*	m_pDetailWaterGeo;

	virtual HRESULT ConfirmDevice( D3DCAPS9 * pCaps, DWORD dwBehavior, D3DFORMAT Format);
	virtual HRESULT Initialize( IDirect3DDevice9 * pDev, const WaterDesc & water_desc );
	virtual HRESULT Free();
	virtual HRESULT Tick();
	virtual bool    Keyboard(DWORD dwKey, UINT nFlags, bool bDown);

	IDirect3DTexture9 *	GetTiledNormalMap();
	IDirect3DTexture9 *	GetDetailNormalMap();

	// Add a displacement into the detail water texture where
	//  the 'character' or thing interacting with it is located
	void	AddInteractionDisplacement();
	// Render intermediates for debug purposes
	void	Dbg_RenderTiledToDetailedBlender();

	WaterCoupler();
	virtual ~WaterCoupler();

public:
	void	SetDetailObjCenter( float x, float y, float z );
	void	GetDetailObjCenter( float * x, float * y, float * z );
	void	GetDetailObjTarget( float * x, float * y, float * z );
	void	UpdateDetailWaterCenter();
	void	TranslateDetailObjCenter( float dx, float dy ); // translate target point
	void	SetDetailObjTarget( float x, float y );			// for setting target point directly

protected:
	IDirect3DDevice9 *	m_pD3DDev;
	D3DXVECTOR4		m_Const1;		// convenient constant values
	D3DXVECTOR4		m_Const2;
	MeshVB *		m_pGeoBlendObj_1;		// object where vertex alpha varies to blend from tiled to local unique water textures

	SM_SHADER_INDEX	m_VSHI_TexCoordGen;		// generates texture coords for detail object as it moves around
	SM_SHADER_INDEX m_VSHI_DbgShowBlender;	// For viewing the blending object

	D3DXVECTOR3		m_TiledWaterBasePoint;	// orients detail water obj to tiled water object
	D3DXVECTOR3		m_DetailObjCenter;		// center of the detail object geometry as it scrolls around and follows
											//  the interaction point
	D3DXVECTOR4		m_DetailTCOffset;		//  This maps texture of tiled object into the detail texture
	D3DXVECTOR3		m_DetailObjNewCenter;	// temporary storage until UpdateDetailWaterCenter() is called
	float			m_fDu;					// scrolling for when detail moves relative to tiled
	float			m_fDv;
	D3DXMATRIX		m_matCoverRenderTarget;
	bool			m_bControlTiled;		// send keyboard to tiled or detail water
	int				m_nControlWhat;
	float			m_fInsetDist;
	ITextureDisplay	*	m_pTextureDisplay;
	ShaderManager	*	m_pShaderManager;

	void	InitBlendGeo();
	void	FreeBlendGeo();
	void	CreateBlenderObject( Mesh * pMesh );
	void	InitWaterGeo();
	void	FreeWaterGeo();
	void	RenderTiledIntoDetail( IDirect3DTexture9 * pTiledTex );
};

#endif			// H__WATERCOUPLER_GJ_H
