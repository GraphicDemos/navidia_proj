/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\TwoSidedPolys\
File:  TwoSidedPolysDemo.cpp

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

#pragma warning( disable : 4995 )		// old string functions marked as #pragma deprecated

#include "TwoSidedPolysDemo.h"
#include "MEDIA\Programs\D3D9_TwoSidedPolys\TwoSidedConstants.h"
#include <Mesh.h>
#include <ShaderManager.h>
#include <TextureFactory.h>
#include <MeshVB.h>
#include <MeshGeoCreator.h>
#include <MeshProcessor.h>

#define FILE_Leaf_fr_02_jpg			"MEDIA\\textures\\2d\\tile_dark_refl_2.jpg"
#define FILE_leaf_trans_01_jpg		"MEDIA\\textures\\2d\\tile_dark_trans_2.jpg"

#define FILE_TwoSided_vsh			"MEDIA\\programs\\D3D9_TwoSidedPolys\\TwoSided.vsh"
#define FILE_TwoSided_psh			"MEDIA\\programs\\D3D9_TwoSidedPolys\\TwoSided.psh"
#define FILE_TwoSidedNoLight_psh	"MEDIA\\programs\\D3D9_TwoSidedPolys\\TwoSidedNoLight.psh"

extern CModelViewerCamera	g_Camera;
// defined in this module
HRESULT	CreateTwistyMesh( Mesh * pMesh, float radius, float ring_width, int num_half_twists, int num_radial_sub, int num_width_sub );


TwoSidedPolysDemo::TwoSidedPolysDemo()
{
	SetAllNull();
}
TwoSidedPolysDemo::~TwoSidedPolysDemo()
{
	Free();
	SetAllNull();
}

void TwoSidedPolysDemo::SetAllNull()
{
	m_pD3DDev			= NULL;
	m_pTwistyMeshVB		= NULL;
	m_pLightMeshVB		= NULL;
	m_pShaderManager	= NULL;
	m_pTextureFactory	= NULL;

	m_ppTexFrontCurrent = NULL;
	m_ppTexTranslucentCurrent = NULL;
}

HRESULT TwoSidedPolysDemo::Free()
{
	HRESULT hr= S_OK;
	SAFE_DELETE( m_pTwistyMeshVB );
	SAFE_DELETE( m_pLightMeshVB );
	SAFE_DELETE( m_pShaderManager );
	SAFE_DELETE( m_pTextureFactory );
	SAFE_RELEASE( m_pD3DDev );
	SetAllNull();
	return( hr );
}

HRESULT TwoSidedPolysDemo::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	Free();
	RET_VAL_IF( pDev == NULL, E_FAIL );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_pShaderManager = new ShaderManager;
	RET_VAL_IF( m_pShaderManager == NULL, E_FAIL );
	hr = m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );
	RET_VAL_IF( FAILED(hr), hr );

	m_pTextureFactory = new TextureFactory;
	RET_VAL_IF( m_pTextureFactory == NULL, E_FAIL );
	hr = m_pTextureFactory->Initialize( GetFilePath::GetFilePath );
	RET_VAL_IF( FAILED(hr), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_TwoSided_vsh ), SM_SHADERTYPE_VERTEX, &m_VSHI_TwoSided );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load") TEXT( FILE_TwoSided_vsh ) TEXT("\n"), E_FAIL );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_TwoSided_psh ), SM_SHADERTYPE_PIXEL, &m_PSHI_TwoSided );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load") TEXT( FILE_TwoSided_psh ) TEXT("\n"), E_FAIL );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT( FILE_TwoSidedNoLight_psh ), SM_SHADERTYPE_PIXEL, &m_PSHI_TwoSidedNoLight );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load") TEXT( FILE_TwoSidedNoLight_psh ) TEXT("\n"), E_FAIL );

	m_ppTexFrontCurrent = m_pTextureFactory->CreateTextureFromFile( m_pD3DDev, TEXT( FILE_Leaf_fr_02_jpg ) );
	MSG_AND_RET_VAL_IF( m_ppTexFrontCurrent == NULL, TEXT("tex front current failed\n"), E_FAIL );

	m_ppTexTranslucentCurrent = m_pTextureFactory->CreateTextureFromFile( m_pD3DDev, TEXT( FILE_leaf_trans_01_jpg ) );
	MSG_AND_RET_VAL_IF( m_ppTexTranslucentCurrent == NULL, TEXT("tex transl current failed\n"), E_FAIL );

	// create twisty geometry where triangles can be viewwed from both sides
	Mesh mesh;
//	CreateTwistyMesh( &mesh, 1.3f, 0.6f, 5, 60, 4 );
//	CreateTwistyMesh( &mesh, 1.3f, 0.8f, 3, 60, 4 );
	CreateTwistyMesh( &mesh, 1.3f, 1.2f, 1, 60, 4 );
	m_pTwistyMeshVB = new MeshVB;
	MSG_AND_RET_VAL_IF( m_pTwistyMeshVB == NULL, TEXT("couldn't create twisty vb!\n"), E_FAIL );
	hr = m_pTwistyMeshVB->CreateFromMesh( &mesh, m_pD3DDev );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("couldn't create twisty mesh vb\n"), E_FAIL );

	// Create a stand-in object to show the light position
	MeshGeoCreator gc;
	Mesh msh;
	gc.InitSphereFromBox( &msh, 0.05f, D3DXVECTOR3( 1.0f, 0.0f, 0.0f ), 5,
							D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 5,
							D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), 5 );
	m_pLightMeshVB = new MeshVB;
	if( m_pLightMeshVB != NULL )
		m_pLightMeshVB->CreateFromMesh( &msh, m_pD3DDev );

	return( hr );
}

// Create a twisted ring like a washer that can twist into a mobius strip
HRESULT	CreateTwistyMesh( Mesh * pMesh, float radius, float ring_width, int num_half_twists, int num_radial_sub, int num_width_sub )
{
	RET_VAL_IF( pMesh == NULL, E_FAIL );
	HRESULT hr = S_OK;
	MeshGeoCreator gc;
	float half_w = ring_width / 2.0f;
	gc.InitTesselatedPlane( pMesh, D3DXVECTOR3( -half_w, 0.0f, 0.0f ), D3DXVECTOR2( 0.0f, 0.0f ),
								D3DXVECTOR3( half_w, 0.0f, 0.0f ), D3DXVECTOR2( 0.0f, 1.0f ),
								D3DXVECTOR3( -half_w, 1.0f, 0.0f ), D3DXVECTOR2( 2.0f, 0.0f ),
								num_width_sub, num_radial_sub );
	// treat x coord as param for radius
	// treat y coord as param for angle around the ring

	// twirl about the Y axis
	size_t i;
	MeshVertex * pV;
	D3DXVECTOR3 * pPos;
	D3DXVECTOR3 newpos;
	float twirl_angle;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pV = pMesh->GetVertexPtr( (UINT)i );
		pPos = pV->GetPositionP();
		twirl_angle = pPos->y * (float)NVMESH_PI * num_half_twists;

		newpos.x = (float)(pPos->x * cos(twirl_angle));
		newpos.z = (float)(pPos->x * sin(twirl_angle));
		newpos.y = pPos->y;
		pV->SetPosition( newpos );
	}
	// bend the twisted strip into a torus
	D3DXVECTOR3 basis1, basis2, basis3;
	basis3 = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	float sweep_angle;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		pPos = pMesh->GetVertexPtr( (UINT)i )->GetPositionP();
		sweep_angle = pPos->y * (float)NVMESH_PI * 2.0f;
		basis1 = D3DXVECTOR3( (float)cos(sweep_angle), (float)sin(sweep_angle), 0.0f );
		basis2 = D3DXVECTOR3( -(float)sin(sweep_angle), (float)cos(sweep_angle), 0.0f );
		newpos.x = basis1.x * radius + pPos->x * (basis1.x + basis2.x + basis3.x);
		newpos.y = basis1.y * radius + pPos->x * (basis1.y + basis2.y + basis3.y);
		newpos.z = pPos->z * (basis1.z + basis2.z + basis3.z);
		pMesh->GetVertexPtr( (UINT)i )->SetPosition( newpos );
	}
	// generate vertex normals
	MeshProcessor mp;
	mp.CalculateNormalsCCW( pMesh );

	return( hr );
}

LRESULT TwoSidedPolysDemo::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing )
{

	return( 0 );
}

HRESULT TwoSidedPolysDemo::Render( float fGlobalTimeInSeconds )
{
	RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	RET_VAL_IF( m_pShaderManager == NULL, E_FAIL );
	RET_VAL_IF( m_ppTexFrontCurrent == NULL, E_FAIL );
	RET_VAL_IF( m_ppTexTranslucentCurrent == NULL, E_FAIL );
	RET_VAL_IF( m_pTwistyMeshVB == NULL, E_FAIL );
	HRESULT hr = S_OK;

    // Clear the viewport
    m_pD3DDev->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000FF, 1.0f, 0L);

	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );

	D3DXVECTOR4 light_pos;
	float light_ang = fGlobalTimeInSeconds * 0.25f * (float)NVMESH_PI;
	float light_rad = 1.7f;
	light_pos = D3DXVECTOR4( (float)(light_rad*cos(light_ang)), 0.0f, (float)(light_rad*sin(light_ang)), 1.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_POS_OSPACE, (float*)&light_pos, 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_COLOR, (float*)D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ), 1 );
	// Get viewer position in world space
	D3DXVECTOR3 eye_objsp;		// eye position in object space
	D3DXMATRIX matWorld, matInvWorld;
	matWorld = *(g_Camera.GetWorldMatrix());
	D3DXMatrixInverse( &matInvWorld, NULL, &matWorld );
	D3DXVec3TransformCoord( &eye_objsp, g_Camera.GetEyePt(), &matInvWorld );
	m_pD3DDev->SetVertexShaderConstantF( CV_EYE_POS_OSPACE, (float*)D3DXVECTOR4( eye_objsp.x, eye_objsp.y, eye_objsp.z, 1.0f ), 1 );

	D3DXMATRIX matWVP, matWVPTrans;
	D3DXMatrixMultiply( &matWVP, g_Camera.GetWorldMatrix(), g_Camera.GetViewMatrix());
	D3DXMatrixMultiply( &matWVP, &matWVP, g_Camera.GetProjMatrix());
	D3DXMatrixTranspose( &matWVPTrans, &matWVP );
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*)&matWVPTrans, 4 );

	m_pD3DDev->SetVertexShaderConstantF( CV_ONE, (float*)D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ), 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_ZERO, (float*)D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), 1 );
	// CV_LIGHT_CONST .x = ambient for reflection texture
	//                .y = ambient for translucent light
	//                .z = attenuation for translucent light transmission
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_CONST, (float*)D3DXVECTOR4( 0.02f, 0.02f, 0.8f, 1.0f ), 1 );

	m_pShaderManager->SetShader( m_PSHI_TwoSided );
	m_pShaderManager->SetShader( m_VSHI_TwoSided );

	m_pD3DDev->SetTexture( 0, *m_ppTexFrontCurrent );
	m_pD3DDev->SetTexture( 1, *m_ppTexTranslucentCurrent );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );	// bilinear
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_POINT );	// bilinear

	m_pTwistyMeshVB->Draw();

	// Draw light object
	if( m_pLightMeshVB != NULL )
	{
		D3DXMATRIX matWorld;
		D3DXMatrixTranslation( &matWorld, light_pos.x, light_pos.y, light_pos.z );
		D3DXMatrixMultiply( &matWorld, &matWorld, g_Camera.GetWorldMatrix() );
		m_pD3DDev->SetTransform( D3DTS_WORLD, &matWorld );
		m_pD3DDev->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
		m_pD3DDev->SetTransform( D3DTS_PROJECTION, g_Camera.GetProjMatrix() );

		m_pD3DDev->SetPixelShader( NULL );
		m_pD3DDev->SetVertexShader( NULL );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TFACTOR );
		m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );
		m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
		m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, 0xFFFFFFFF );		// ARGB

		m_pLightMeshVB->Draw();
	}

	return( hr );
}

