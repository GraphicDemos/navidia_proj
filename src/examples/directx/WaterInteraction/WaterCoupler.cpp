/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  WaterCoupler.cpp

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


#include <assert.h>
#include "WaterCoupler.h"
#include "PA_Water.h"
#include <NV_D3DCommon\NV_D3DCommon.h>
#include <Mesh.h>
#include <MeshVB.h>
#include <ShaderManager.h>
#include <MeshVBDot3.h>
#include <MeshSectionStitcher.h>
#include <MeshGeoCreator.h>

#include "Media\Programs\D3D9_WaterInteraction\WaterInteractionConstants.h"

#define FILE_Displacement1		"MEDIA\\textures\\2D\\Displacement1.bmp"
#define FILE_WC_TexCoordGen		"MEDIA\\programs\\D3D9_WaterInteraction\\WC_TexCoordGen.vsh"

LowPassFilter2D	gLPF;

#ifndef ASSERT_RET_IF_FAILED
#define ASSERT_RET_IF_FAILED( hr )	\
	if( FAILED(hr) )				\
	{								\
		assert( false );			\
		return( hr );				\
	}						
#endif

WaterCoupler::WaterCoupler()
{
	m_pCA_WaterTiled	= NULL;
	m_pCA_WaterDetail	= NULL;
	m_pGeoBlendObj_1	= NULL;
	m_pTiledWaterGeo = NULL;
	m_pDetailWaterGeo = NULL;
	m_VSHI_TexCoordGen		= 0;
	m_VSHI_DbgShowBlender	= 0;
	m_pD3DDev = NULL;
	m_pTextureDisplay	= NULL;
	m_pShaderManager	= NULL;
}

WaterCoupler::~WaterCoupler()
{
	Free();
}

HRESULT WaterCoupler::Free()
{	
	m_VSHI_TexCoordGen		= 0;
	m_VSHI_DbgShowBlender	= 0;
	SAFE_DELETE( m_pCA_WaterTiled );
	SAFE_DELETE( m_pCA_WaterDetail );
	SAFE_DELETE( m_pTextureDisplay );
	SAFE_DELETE( m_pShaderManager );
	FreeBlendGeo();
	FreeWaterGeo();
	if( m_pD3DDev != NULL )
	{
		SAFE_RELEASE( m_pD3DDev );
	}
	return( S_OK );
}

HRESULT WaterCoupler::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
	HRESULT hr = S_OK;
	PA_Water water;
	hr = water.ConfirmDevice( pCaps, dwBehavior, Format );
	if( FAILED(hr) )
	{
		FDebug("Water class will not work on this device!\n");
		return( hr );
	}
	return( hr );
}

//---------------------------------------------------------------------
// Creates geometry used to blend between the unique local simulated water texture
//  and the tiled simulated water texture.  This allows features from the tiled
//  water to blend smoothly into the unique local texture.  Local objects in the 
//  world, like characters, can affect the interior of the unique water simulation,
//  but these disturbances will not spread to the tiled water simulation.
// This object has four rings of varying opacity.  It is fully opaque at the outer
//  edge, and very transparent toward the center.  The center is large and slightly
//  opaque in order to couple just a little bit of the tiled water to the interior
//  of the local water.  This is done because the average level of the water tends
//  to oscillate up and down a bit, so the tiled and detail average values could
//  oscillate out of phase with one another.  The weak interior coupling keeps them
//  in phase so there are no strange artifacts or slopes when coupling the two 
//  simulations together.
//---------------------------------------------------------------------
void	WaterCoupler::CreateBlenderObject( Mesh * pMesh )
{
	RET_IF( pMesh == NULL );
	vector < D3DXVECTOR3 >	pos;
	vector < D3DXVECTOR2 >	tex;
	vector < DWORD >		col;
	float inset_dist = m_fInsetDist;
	int		n_vert_per_section;
	int		n_sections;
	DWORD col_in, col_in2, col_in3, col_out;
	// Texture coords generated automaticaly in the vshader using input position info
	float pos_min_x =  0.0f;
	float pos_min_y =  0.0f;
	float pos_max_x =  1.0f;
	float pos_max_y =  1.0f;
	inset_dist *= 1.0f;
	// ins2 slightly biases most of the inside of the detail texture toward the value
	//  from the tiled texture.
	float ins2, ins3;

	col_in3		= 0x00020202;
	col_in		= 0x00A0A0A0;
	col_in2		= 0x000A0A0A;
	col_out		= 0x00FFFFFF;
	ins2	= 0.15f;
	ins3	= 0.35f;

	//  First cross-section of an extruded object.  Four vertices forming one corner of a square.
	pos.push_back( D3DXVECTOR3( pos_min_x,				pos_min_y,				0.0f ) );
	col.push_back( col_out );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_min_x + inset_dist, pos_min_y + inset_dist, 0.0f ) );
	col.push_back( col_in );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_min_x + ins2,		pos_min_y + ins2,		0.0f ) );
	col.push_back( col_in2 );
	pos.push_back( D3DXVECTOR3( pos_min_x + ins3,		pos_min_y + ins3,		0.0f ) );
	col.push_back( col_in3 );
	// Second cross section.  Four more vertices of the next corner of the square object.
	pos.push_back( D3DXVECTOR3( pos_max_x,				pos_min_y,		0.0f ) );
	col.push_back( col_out );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_max_x - inset_dist,	pos_min_y + inset_dist,	0.0f ) );
	col.push_back( col_in );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_max_x - ins2,		pos_min_y + ins2,		0.0f ) );
	col.push_back( col_in2 );
	pos.push_back( D3DXVECTOR3( pos_max_x - ins3,		pos_min_y + ins3,		0.0f ) );
	col.push_back( col_in3 );	// ARGB
	// Third cross section for the next corner around the square object.
	pos.push_back( D3DXVECTOR3( pos_max_x,				pos_max_y,				0.0f ) );
	col.push_back( col_out );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_max_x - inset_dist,	pos_max_y - inset_dist,	0.0f ) );
	col.push_back( col_in );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_max_x - ins2,		pos_max_y - ins2,		0.0f ) );
	col.push_back( col_in2 );
	pos.push_back( D3DXVECTOR3( pos_max_x - ins3,		pos_max_y - ins3,		0.0f ) );
	col.push_back( col_in3 );	// ARGB
	// Last cross section.  This one lies between the 3rd and 1st cross sections, and 
	// is the other corner of the square object.
	pos.push_back( D3DXVECTOR3( pos_min_x,				pos_max_y,				0.0f ) );
	col.push_back( col_out );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_min_x + inset_dist,	pos_max_y - inset_dist,	0.0f ) );
	col.push_back( col_in );	// ARGB
	pos.push_back( D3DXVECTOR3( pos_min_x + ins2,		pos_max_y - ins2,		0.0f ) );
	col.push_back( col_in2 );
	pos.push_back( D3DXVECTOR3( pos_min_x + ins3,		pos_max_y - ins3,		0.0f ) );
	col.push_back( col_in3 );	// ARGB

	n_vert_per_section	= 4;
	n_sections			= 4;
	bool	bStitchSec = false;
	bool	bStitchEnd = true;

	// Create the object, subdivide with 0,0 as the subdivision isn't needed.
	MeshSectionStitcher mss;
	mss.InitExtrusion( pMesh, &pos[0], NULL, &col[0], NULL,
						n_vert_per_section, n_sections,
						bStitchSec, bStitchEnd );
}

void	WaterCoupler::InitBlendGeo()
{
	// Create a geometry used to blend the tiled water into the border
	//  of the unique 'detail' water.  The size & properties of this
	//  object can vary dramaticaly.  What matters most is that the
	//  object have opaque alpha values on its outermost edge, and
	//  be rendered with matrices that map the outer edges exactly to
	//  the edges of the unique detail water texture.
	// This geometry should be almost entirely transparent in the center
	//  so as to allow the unique detail water texture to oscillate
	//  with unique values in the center where the character or other
	//  object is causing displacements.

	FreeBlendGeo();
	Mesh mesh;
	CreateBlenderObject( &mesh );
	// Create a vertex buffer object from the cpu data
	m_pGeoBlendObj_1 = new MeshVB;
	BREAK_AND_RET_IF( m_pGeoBlendObj_1 == NULL );
	m_pGeoBlendObj_1->CreateFromMesh( &mesh, m_pD3DDev );
}

void WaterCoupler::FreeBlendGeo()
{
	SAFE_DELETE( m_pGeoBlendObj_1 );
}

void WaterCoupler::InitWaterGeo()
{
	FreeWaterGeo();
	Mesh mesh;
	MeshGeoCreator gc;

	// Make water object based on values in m_WaterDesc
	// This is a simple tesselated quad onto which the water
	//  textures are applied in rendering.  
	// ***  Note that when the camera is close to the water and viewing the
	//  edge of the detail water, the tesselation level of this object and
	//  the object on which the detail water is applied should be high and
	//  match one another.  This is to prevent differences in the interpolated
	//  eye-to-vertex vectors which become large when a vertex of the detail
	//  water lies within a large triangle of the tiled water.
	// *** As an alternative, instead of moving smoothly & continuously in relation
	//  to the tiled water, the detail water object could be made to move in 
	//  quantized steps that are the size of one triangle of the tiled water.
	//  This way the detail water vertices always lie directly on top of the 
	//  tiled water vertices, so there are no differences in interpolated 
	//  eye-to-vertex vectors.
	D3DXVECTOR3 base, ccw, cw;
	// Centered in position about 0,0
	float xmin, xmax, ymin, ymax;
	// For both dot3 and EMBM reflection
	xmin = - ( m_WaterDesc.tile_width * m_WaterDesc.num_tiles_x ) / 2.0f;
	xmax = - xmin;
	ymin = - ( m_WaterDesc.tile_height * m_WaterDesc.num_tiles_y ) / 2.0f;
	ymax = - ymin;
	// Tiled water has integer coords at (0,0), so this will be
	//  our base point for later calculations matching the detailed water
	//  object to the base tiled water plane.
	// If the tiled water plane changes location, then this value
	//  should also change to follow the point where tex coords are 
	//  integer (not fractional)
	m_TiledWaterBasePoint = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	// Points that determine the edges of the tesselated plane 
	base = D3DXVECTOR3( xmin, ymin, 0.0f );
	ccw  = D3DXVECTOR3( xmin, ymax, 0.0f );
	cw   = D3DXVECTOR3( xmax, ymin, 0.0f );
	// UV coords for the plane
	D3DXVECTOR2 uv_base = D3DXVECTOR2( 0.0f, 0.0f );
	D3DXVECTOR2 uv_ccw  = D3DXVECTOR2( 0.0f, m_WaterDesc.num_tiles_y );
	D3DXVECTOR2 uv_cw   = D3DXVECTOR2( m_WaterDesc.num_tiles_x, 0.0f );

	int ndiv_x = m_WaterDesc.num_tiles_x * 3 - 1;
	int ndiv_y = m_WaterDesc.num_tiles_y * 3 - 1;
	gc.InitTesselatedPlane( &mesh, base, uv_base, ccw, uv_ccw, cw, uv_cw, ndiv_x, ndiv_y );

	// make the vertex buffer object for it
	m_pTiledWaterGeo = new MeshVBDot3;
	assert( m_pTiledWaterGeo != NULL );

	m_pTiledWaterGeo->CreateFromMesh( &mesh, m_pD3DDev );

	// Make a second simple tesselated quad for rendering the
	//  detail water
	// Center this obj about 0 also, making it the size of a 
	//  single tile of the tiled water
	xmin = - m_WaterDesc.tile_width / 2.0f;
	xmax = - xmin;
	ymin = - m_WaterDesc.tile_height / 2.0f;
	ymax = - ymin;
	// Points that determine the edges of the tesselated plane 
	base = D3DXVECTOR3( xmin, ymin, 0.0f );
	ccw  = D3DXVECTOR3( xmin, ymax, 0.0f );
	cw   = D3DXVECTOR3( xmax, ymin, 0.0f );

	uv_base = D3DXVECTOR2( 0.0f, 0.0f );
	uv_ccw  = D3DXVECTOR2( 0.0f, 1.0f );
	uv_cw   = D3DXVECTOR2( 1.0f, 0.0f );

	ndiv_x = m_WaterDesc.num_tiles_x - 1 + 3;
	ndiv_y = m_WaterDesc.num_tiles_y - 1 + 3;

	gc.InitTesselatedPlane( &mesh, base, uv_base, ccw, uv_ccw, cw, uv_cw, ndiv_x, ndiv_y );

	// make the VB object for it
	m_pDetailWaterGeo = new MeshVBDot3;
	BREAK_AND_RET_IF( m_pDetailWaterGeo == NULL );
	m_pDetailWaterGeo->CreateFromMesh( &mesh, m_pD3DDev );
}

void WaterCoupler::FreeWaterGeo()
{
	SAFE_DELETE( m_pTiledWaterGeo );
	SAFE_DELETE( m_pDetailWaterGeo );
}

HRESULT WaterCoupler::Initialize( IDirect3DDevice9 * pDev, const WaterDesc & water_desc )
{
	RET_VAL_IF( pDev == NULL, E_FAIL );
	HRESULT hr = S_OK;
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_bControlTiled = false;
	m_fInsetDist = 0.05f;
	m_nControlWhat = 0;
	m_pTextureDisplay = new TextureDisplay2;
	FAIL_IF_NULL( m_pTextureDisplay );
	m_pTextureDisplay->Initialize( m_pD3DDev );

	m_pShaderManager = new ShaderManager;
	FAIL_IF_NULL( m_pShaderManager );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );
	
	m_pCA_WaterTiled  = new PA_Water;
	FAIL_IF_NULL( m_pCA_WaterTiled != NULL );
	m_pCA_WaterDetail = new PA_Water;
	FAIL_IF_NULL( m_pCA_WaterDetail != NULL );

	hr = m_pCA_WaterTiled->Initialize( pDev, water_desc.tile_res_x, water_desc.tile_res_y,
										TEXT( FILE_Displacement1 ),
										PA_Water::DM_DOT3X2_MAP,
										& m_pShaderManager,
										& m_pTextureDisplay			);
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	hr = m_pCA_WaterDetail->Initialize( pDev, water_desc.detail_res_x, water_desc.detail_res_y,
										TEXT( FILE_Displacement1 ),
										PA_Water::DM_DOT3X2_MAP,
										& m_pShaderManager,
										& m_pTextureDisplay			);
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	m_pCA_WaterTiled->m_fDropletFreq	= 0.9f;
	m_pCA_WaterDetail->m_fDropletFreq	= 0.0f;
	m_pCA_WaterDetail->m_bWrap			= true;

	m_WaterDesc = water_desc;

	// must set m_WaterDesc before calling these
	InitBlendGeo();			// geometry for maintaining the coupling
							// Used to blend textures into one another
	InitWaterGeo();			// Scene geometry on which to render the textures 

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT(FILE_WC_TexCoordGen), SM_SHADERTYPE_VERTEX, &m_VSHI_TexCoordGen );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	m_TiledWaterBasePoint	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_DetailObjCenter		= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	SetDetailObjCenter( 0.0f, 0.0f, 0.0f );
	m_DetailTCOffset		= D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );
	// Set convenient constants to make vshaders work
	m_Const1	= D3DXVECTOR4( 0.0f, 0.5f,  1.0f, 2.0f );
	m_Const2	= D3DXVECTOR4( 0.0f, 0.5f, -1.0f, 1.0f );
	
	// Make matrix to render 0,0 to 1,1 object to exactly cover a render target
	D3DXMATRIX	matView, matProj, matViewProj, matWorld;
	D3DXVECTOR3 const vEyePt    = D3DXVECTOR3( 0.5f,  0.5f, -5.0f );
	D3DXVECTOR3 const vLookatPt = D3DXVECTOR3( 0.5f,  0.5f, 0.0f );
	D3DXVECTOR3 const vUp       = D3DXVECTOR3( 0.0f,  1.0f, 0.0f );

	// Set World, View, Projection, and combination matrices.
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUp);
	D3DXMatrixOrthoLH(  &matProj, 0.5f, 0.5f, 0.2f, 20.0f);
	D3DXMatrixOrthoLH(  &matProj, 1.0f, 1.0f, 0.2f, 20.0f);
    D3DXMatrixMultiply( &matViewProj, &matView, &matProj);

    // draw a single quad to texture:
    // the quad covers the whole "screen" exactly
	D3DXMatrixScaling(		&matWorld, 1.0f, 1.0f, 1.0f);
	D3DXMatrixMultiply(		&m_matCoverRenderTarget, &matWorld, &matViewProj);
    D3DXMatrixTranspose(	&m_matCoverRenderTarget, &m_matCoverRenderTarget);
	return( hr );
}

HRESULT WaterCoupler::Tick()
{
	RET_VAL_IF( m_pCA_WaterTiled == NULL, E_FAIL );
	RET_VAL_IF( m_pCA_WaterDetail == NULL, E_FAIL );
	HRESULT hr = S_OK;
	// Update vars used to position the detail relative to the tiled
	UpdateDetailWaterCenter();
	// Do one time step for each animating water texture
	m_pCA_WaterTiled->Tick();

	IDirect3DTexture9 * pPrevTiled;
	pPrevTiled = m_pCA_WaterTiled->GetPrevStateTexture();

	RenderTiledIntoDetail( pPrevTiled );
	m_pCA_WaterDetail->SetScrollAmount( m_fDu, m_fDv );
	m_pCA_WaterDetail->Tick();
	return( hr );
}

IDirect3DTexture9 *		WaterCoupler::GetDetailNormalMap()
{
	assert( m_pCA_WaterDetail != NULL );
	IDirect3DTexture9 * pTex;
	pTex = m_pCA_WaterDetail->GetOutputTexture();
	assert( pTex != NULL );
	return( pTex );
}

IDirect3DTexture9 *		WaterCoupler::GetTiledNormalMap()
{
	assert( m_pCA_WaterTiled != NULL );
	IDirect3DTexture9 * pTex;
	pTex = m_pCA_WaterTiled->GetOutputTexture();
	assert( pTex != NULL );
	return( pTex );
}

bool WaterCoupler::Keyboard( DWORD dwKey, UINT nFlags, bool bDown )
{
	bool res, res2;
	res = res2 = false;
	if( m_pCA_WaterTiled != NULL && m_pCA_WaterDetail != NULL )
	{
		if( bDown )
		{
			switch( dwKey )
			{
			case '0':
				m_nControlWhat++;
				if( m_nControlWhat > 2 )
					m_nControlWhat = 0;
				switch( m_nControlWhat )
				{
				case 0:
					FDebug("Controlling both textures\n");
					break;
				case 1:
					FDebug("Controlling Tiled texture\n");
					break;
				case 2:
					FDebug("Controlling Detail texture\n");
					break;
				}
				return(true);
				break;
			
			case '6':
				m_fInsetDist -= 0.003f;
				FDebug("m_fInsetDist = %f\n", m_fInsetDist );
				InitBlendGeo();
				return(true);
				break;

			case '7':
				m_fInsetDist += 0.003f;
				FDebug("m_fInsetDist = %f\n", m_fInsetDist );
				InitBlendGeo();
				return(true);
				break;
			}
		}
		switch( m_nControlWhat )
		{
		case 0:
			//FDebug("Controlling both textures\n");
			res = m_pCA_WaterTiled->Keyboard( dwKey, nFlags, bDown );
			res2 = m_pCA_WaterDetail->Keyboard( dwKey, nFlags, bDown );
			break;
		case 1:
			//FDebug("Controlling Tiled texture\n");
			res = m_pCA_WaterTiled->Keyboard( dwKey, nFlags, bDown );
			break;
		case 2:
			//FDebug("Controlling Detail texture\n);
			res2 = m_pCA_WaterDetail->Keyboard( dwKey, nFlags, bDown );
			break;
		}
		// if either true, returned true
		res = res || res2;
	}
	return( res );
}

void	WaterCoupler::AddInteractionDisplacement()
{
	if( m_pCA_WaterDetail != NULL )
	{
		float x,y;
		// compute coordinates of droplet relative to 
		//  detail object center.
		x = gLPF.x_targ - m_DetailObjCenter.x;
		y = gLPF.y_targ - m_DetailObjCenter.y;
		x = x / m_WaterDesc.tile_width;
		y = y / m_WaterDesc.tile_height;
		// 0.5, 0.5 is the center of the PA_Water object as far as adding
		//  displacements is concerned.  Add our x,y offset from the center
		//  to the (0.5, 0.5) coordinate for the center
		x = 0.5f + x;
		y = 0.5f + y;
		const float ulim = 1.0f;
		const float llim = 0.0f;
		if( x < ulim && y < ulim && x > llim && y > llim )
		{
			m_pCA_WaterDetail->AddDroplet( x, y, 0.1f );
		}
	}
}

void	WaterCoupler::UpdateDetailWaterCenter()
{
	// Update variables used to position the detail water
	// x,y,z is world space position
	// Note that the position used for rendering may not change immediately
	// To avoid extra render-to-texture steps, the position may only change
	//   when each of the simulation textures are updated, and this could
	//   be at a lower rate than the framerate.
	gLPF.Tick();
	m_DetailObjNewCenter.x = gLPF.x;
	m_DetailObjNewCenter.y = gLPF.y;
	m_fDu = ( m_DetailObjNewCenter.x - m_DetailObjCenter.x ) / m_WaterDesc.tile_width;
	m_fDv = ( m_DetailObjNewCenter.y - m_DetailObjCenter.y ) / m_WaterDesc.tile_height;
	m_DetailObjCenter.x = m_DetailObjNewCenter.x;
	m_DetailObjCenter.y = m_DetailObjNewCenter.y;

	float tw, th;
	tw = m_WaterDesc.tile_width;
	th = m_WaterDesc.tile_height;

	// Need to find the texture coord offset to map tiled texture 
	//  into detail texture.
	D3DXVECTOR3  offset;
	offset = m_DetailObjCenter - D3DXVECTOR3( tw / 2.0f, th / 2.0f, 0.0f );
	offset = offset - m_TiledWaterBasePoint;

	// Scale to the tile size:so the offset is no longer for
	//  world space position but for texture coordinates
	offset.x = offset.x / m_WaterDesc.tile_width;
	offset.y = offset.y / m_WaterDesc.tile_height;

	// D3D samples from upper left of each texel, so we need to add a half
	//  texel offset to hit the texel center.  
	// OpenGL samples from texel centers, so this offset would not be
	//  necessary under OpenGL.
	float htw, hth;					// half texel width and height
	htw = 0.5f / m_WaterDesc.detail_res_x;
	hth = 0.5f / m_WaterDesc.detail_res_y;
	m_DetailTCOffset.x = -offset.x + 0.0f - htw;
	m_DetailTCOffset.y = -offset.y + 0.0f - hth;
	m_DetailTCOffset.z = -offset.z + 0.0f;
	m_DetailTCOffset.w = 0.0f;
}

void	WaterCoupler::TranslateDetailObjCenter( float dx, float dy )
{
	float nx, ny;
	nx = gLPF.x_targ + dx;
	ny = gLPF.y_targ + dy;
	gLPF.SetTarget( nx, ny );
}

void	WaterCoupler::SetDetailObjTarget( float x, float y )
{
	gLPF.SetTarget( x, y );
}

void	WaterCoupler::SetDetailObjCenter( float x, float y, float z )
{
	// x,y,z is world space position
	// Note that the position used for rendering may not change immediately
	// To avoid extra render-to-texture steps, the position may only change
	//   when each of the simulation textures are updated, and this could
	//   be at a lower rate than the framerate.
	// UpdateDetailWaterCenter() changes the variables which position
	//   the water
	m_DetailObjNewCenter.x = x;
	m_DetailObjNewCenter.y = y;
	m_DetailObjNewCenter.z = z;
}

void	WaterCoupler::GetDetailObjTarget( float * x, float * y, float * z )
{
	*x = gLPF.x_targ;
	*y = gLPF.y_targ;
	*z = 0.0f;
}

void	WaterCoupler::GetDetailObjCenter( float * x, float * y, float * z )
{
	// x,y,z is world space position
	*x = m_DetailObjCenter.x;
	*y = m_DetailObjCenter.y;
	*z = m_DetailObjCenter.z;
}

void	WaterCoupler::Dbg_RenderTiledToDetailedBlender()
{
	RET_IF( m_pD3DDev == NULL );
	RET_IF( m_pShaderManager == NULL );
	HRESULT hr;
	m_pD3DDev->SetVertexShaderConstantF( CV_TEXCOORD_BASE, (float*)&m_DetailTCOffset, 1 );
	D3DXVECTOR4 tmp( m_WaterDesc.tile_width, m_WaterDesc.tile_height, 1.0f, 1.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_TILE_SIZE, (float*)&tmp, 1 );
		
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1, (float*) &m_Const1,	1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_2, (float*) &m_Const2,	1 );
	hr = m_pShaderManager->SetShader( m_VSHI_TexCoordGen );
	BREAK_AND_RET_IF( FAILED(hr) );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_MODULATE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE 	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
	m_pGeoBlendObj_1->Draw();
}

void	WaterCoupler::RenderTiledIntoDetail( IDirect3DTexture9 * pTiledTex )
{
	RET_IF( pTiledTex == NULL );
	RET_IF( m_pD3DDev == NULL );
	RET_IF( m_pShaderManager == NULL );
	HRESULT hr;
	D3DXVECTOR4 texoff = m_DetailTCOffset;
	texoff.x += m_fDu;
	texoff.y += m_fDv;
	m_pD3DDev->SetVertexShaderConstantF( CV_TEXCOORD_BASE, (float*)&texoff, 1 );
	D3DXVECTOR4 tmp( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_TILE_SIZE, (float*)&tmp,		1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_1, (float*)&m_Const1,	1 );	
	m_pD3DDev->SetVertexShaderConstantF( CV_CONSTS_2, (float*)&m_Const2,	1 );

	hr = m_pShaderManager->SetShader( m_VSHI_TexCoordGen );
	BREAK_IF( FAILED(hr) );

	// Set matrix to render object with pos from 0,0 to 1,1 into
	//  a texture render target to exactly cover the target
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, &m_matCoverRenderTarget(0, 0), 4 );
	// Set render state to render height and velocity from tiled
	//	texture into height and velocity of detail texture
	m_pD3DDev->SetPixelShader( NULL );
	// Point sample
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pD3DDev->SetTexture( 0, pTiledTex );
	m_pD3DDev->SetTexture( 1, NULL );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );

	// Vertex shader replicates vertex blue to all colors & alpha, so
	//  alpha blending can be used to blend the tiled texture into the
	//  detailed.  
	// HOWEVER - The water state textures use alpha as data, so we must
	//  be sure that whatever this operation writes into alpha is 
	//  repaired by the CA_Water shader.  The water shader does this in
	//  the first step of each time step (the smoothing step) by replicating
	//  the blue into alpha.  The water shader requires that the blue 
	//  and alpha channels hold the same value, which represents the
	//  height of the water surface.
	// This alpha blend is safe because the PA_Water is smart enough to compensate.

	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,		D3DBLEND_SRCALPHA		);
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA	);
	// Select the RGB color of the tiled texture
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE 		);
	// Select the diffuse color of the model geometry.  The blend geometry 
	//  vertex color can thus be used to control the slope of the blend between
	//  tiled and detail water.
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_SELECTARG1	);
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_DIFFUSE		);
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
	// Set render target to the detailed texture
	m_pCA_WaterDetail->RenderTarget_SetToCurrentStateTexture();
	// Render blender object into the detailed texture 
	// This renders the tiled texture into the border of the
	//  localized detail texture
	m_pGeoBlendObj_1->Draw();
	// Restore original render target
	m_pCA_WaterDetail->RenderTarget_Restore();
}
