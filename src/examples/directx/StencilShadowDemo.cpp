/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\StencilShadow\
File:  StencilShadowDemo.cpp

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


#include "StencilShadowDemo.h"
#include "MEDIA\programs\Constants_StencilShadow.h"
#include "NV_D3DCommon\NV_D3DCommonDX9.h"
#include "NV_D3DMesh\NV_D3DMeshDX9.h"
#include "shared\NV_Error.h"
#include "shared\NV_Common.h"
#include "TestStencil.h"
#include "ShadowSceneMeshes.h"

using namespace std;

//-------------------------------------------------------

extern TestStencil *		g_pTestStencil;
extern CFirstPersonCamera	g_Camera;

#define VSH_DirectionalLight	TEXT("MEDIA\\programs\\DirectionalLight.vsh")
#define VSH_ExtrudeVolume		TEXT("MEDIA\\programs\\ExtrudeVolume.vsh")

#define TEX_BLANK			TEXT("blank.tga")
#define TEX_BASETEX01		TEXT("basetex_01.jpg")

#define DTOR (D3DX_PI / 180.0f)
#define STENCIL_REVEAL_COLOR	0x00200000

//-------------------------------------------------------
CDXUTTimer		g_Timer;

struct QuadVertex
{
	float x, y, z;
	DWORD color;
};

//-------------------------------------------------------

StencilShadowDemo::StencilShadowDemo()
{
	SetAllNull();
}

StencilShadowDemo::~StencilShadowDemo()
{
	Free();
	FreeSysMemMeshes();
	SetAllNull();
}


HRESULT	StencilShadowDemo::FreeSysMemMeshes()
{
	SAFE_DELETE( m_pMeshes );
	return( S_OK );
}

HRESULT StencilShadowDemo::CreateSysMemMeshes()
{
	HRESULT hr = S_OK;
	FreeSysMemMeshes();
	m_pMeshes = new ShadowSceneMeshes;
	FAIL_IF_NULL( m_pMeshes );
	hr = m_pMeshes->CreateShadowVolumeMeshes();
	return( hr );
}


HRESULT StencilShadowDemo::Free()
{
	SAFE_DELETE( m_pLightMesh );
	SAFE_DELETE( m_pQuad );
	SAFE_DELETE( m_pShaderManager );
	SAFE_RELEASE( m_pDecalTexture );
	SAFE_RELEASE( m_pLightObjTexture );
	if( m_pMeshes != NULL )
	{
		m_pMeshes->FreeDeviceVBs();
	}
	SAFE_RELEASE( m_pD3DDev );
	return( S_OK );
}

bool StencilShadowDemo::IsTwoSidedStencilSupported( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	if( pDev == NULL )
		return( false );
	D3DCAPS9 caps;
    hr = pDev->GetDeviceCaps( &caps );
	RET_VAL_IF( FAILED(hr), false );

	bool result = false;
	if( (caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) == D3DSTENCILCAPS_TWOSIDED )
		result = true;
	FMsg("Two-sided stencil is %s\n", result ? "supported" : "not supported" );
	return( result );
}

HRESULT StencilShadowDemo::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_bRenderCastingObject	= true;
	m_bRotateLight			= true;
	m_bWireframe			= false;
	m_bNoCull				= false;	// true to disable backface culling
	m_bRevealStencilAsColor	= false;

	m_fHtFogFac			= 0.55f;
	m_fExplodeDist		= 0.0f;

	m_bHWSupportsTwoSidedStencil = IsTwoSidedStencilSupported( m_pD3DDev );
	int nShadeMode = 0;
	if( m_bHWSupportsTwoSidedStencil == true )
	{
		m_eShadeModes[nShadeMode++] = STENCIL_SHADOW_TWOSIDED;
	}
	m_eShadeModes[nShadeMode++] = STENCIL_SHADOW_TWO_PASS;
	m_eShadeModes[nShadeMode++] = ALPHA_SHADOW_VOLUME;
	m_eShadeModes[nShadeMode++] = DONT_DRAW_SHADOW;
	m_nNumShadeModes = nShadeMode;
	m_nCurrentShadeMode = 0;

	g_Timer.Start();

	m_pQuad = new QuadVB;
	FAIL_IF_NULL( m_pQuad );
	m_pQuad->Initialize( m_pD3DDev, -1.0f, 1.0f, 1.0f, -1.0f, 0.01f );

	m_pLightMesh = new MeshVB;
	FAIL_IF_NULL( m_pLightMesh );

	// Make light stand-in object:
	Mesh * pLight = new Mesh;
	FAIL_IF_NULL( pLight );
	MeshGeoCreator gc;
	gc.InitSphere( pLight, 0.12f, 8, 8 );
	m_pLightMesh->CreateFromMesh( pLight, m_pD3DDev );
	SAFE_DELETE( pLight );

	// Create device-dependent vertex & index buffer geometry
	FAIL_IF_NULL( m_pMeshes );
	m_pMeshes->CreateDeviceVBs( m_pD3DDev );

	// multiplicative factor to darken shadow areas
	// higher value = lighter shadow
	m_dwShadowAttenValue = 0xB0B0B0B0;

	// Load vertex shader for two-sided lighting
	vector<DWORD> Declaration;

	m_pShaderManager = new ShaderManager;
	FAIL_IF_NULL( m_pShaderManager );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );

	hr = m_pShaderManager->LoadAndAssembleShader( VSH_DirectionalLight, SM_SHADERTYPE_VERTEX, & m_VSHI_DirectionalLight );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") VSH_DirectionalLight, hr );

	hr = m_pShaderManager->LoadAndAssembleShader( VSH_ExtrudeVolume, SM_SHADERTYPE_VERTEX, & m_VSHI_ExtrudeVolume );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") VSH_ExtrudeVolume, hr );

	hr = D3DXCreateTextureFromFile( m_pD3DDev, GetFilePath::GetMediaFilePath(TEX_BLANK).c_str(), &m_pLightObjTexture);
	if (FAILED(hr))
	{
		FDebug("Could not create m_pLightObjTexture texture!");
		return E_FAIL;
	}

	hr = D3DXCreateTextureFromFile( m_pD3DDev, GetFilePath::GetMediaFilePath(TEX_BASETEX01).c_str(), &m_pDecalTexture);
	if (FAILED(hr))
	{
		FDebug("Could not create m_pDecalTexture texture!");
		return E_FAIL;
	}

	// Camera stuff
	SetDefaultView();
	return S_OK;
}

void StencilShadowDemo::SetDefaultView()
{
    D3DXVECTOR3 vFromPt;
    D3DXVECTOR3 vLookatPt;
	vFromPt   = D3DXVECTOR3( 2.781765f, 2.529960f, 6.807855f );
	vLookatPt = D3DXVECTOR3( 2.265073f, 2.389370f, 5.963304f );
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
}

void StencilShadowDemo::KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown )
{
	UINT charcode;

	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_SPACE:
			m_nCurrentShadeMode++;
			m_nCurrentShadeMode = m_nCurrentShadeMode % m_nNumShadeModes;
			switch( m_eShadeModes[m_nCurrentShadeMode] )
			{
			case STENCIL_SHADOW_TWO_PASS:
				FMsg("Shade mode = STENCIL_SHADOW_TWO_PASS\n");
				break;
			case STENCIL_SHADOW_TWOSIDED:
				FMsg("Shade mode = STENCIL_SHADOW_TWOSIDED\n");
				break;
			case ALPHA_SHADOW_VOLUME:
				FMsg("Shade mode = ALPHA_SHADOW_VOLUME\n");
				break;
			case DONT_DRAW_SHADOW:
				FMsg("Shade mode = DONT_DRAW_SHADOW\n");
				break;
			}
			break;

		case 'R':
			m_bRevealStencilAsColor = !m_bRevealStencilAsColor;
			FMsg("m_bRevealStencilAsColor = %s\n", m_bRevealStencilAsColor ? "true" : "false" );
			break;

		case 'C':
			ListVector( TEXT("eye    point: "), (D3DXVECTOR3)*g_Camera.GetEyePt(), TEXT("\n") );
			ListVector( TEXT("lookat point: "), (D3DXVECTOR3)*g_Camera.GetLookAtPt(), TEXT("\n") );
			break;

		case 'L':
			m_bRotateLight = !m_bRotateLight;
			FMsg("m_bRotateLight = %d\n", m_bRotateLight );
			break;

		case 'O':
			m_bRenderCastingObject = !m_bRenderCastingObject;
			break;

		case VK_HOME :
		case VK_NUMPAD7 :
			SetDefaultView();
			break;

		default:
			charcode = MapVirtualKey( nChar, 2 );
			switch( charcode )
			{
			case '[':
			case '{':
				break;
			case ']':
			case '}':
				break;
			}
			break;
		}
	}

/*
//@@ OLD key options 
	str += "L     - Start/Stop light rotation\n";
	str += "SPC   - Render shadow or display volume alpha blended\n";
*/

}

HRESULT StencilShadowDemo::RenderSceneNonCasters( TGroup<SceneNonCaster> * pGrp, D3DXMATRIX * pMatCameraWVP )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pGrp );
	FAIL_IF_NULL( pMatCameraWVP );

	D3DXMATRIX matWVP, matWVPTrans, matObjWorldInv;

	size_t i;
	SceneNonCaster * pNC;
	D3DXVECTOR4 v4LightObjSpc;

	for( i=0; i < pGrp->GetNumElements(); i++ )
	{
		pNC = pGrp->GetElement( i );
		if( pNC == NULL )
			continue;
		if( pNC->m_pMeshVB == NULL )
			continue;

		D3DXMatrixMultiply( &matWVP, &(pNC->m_matWorld), pMatCameraWVP );
		D3DXMatrixTranspose( &matWVPTrans, &matWVP );	
		m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, &matWVPTrans(0,0), 4);
		// Transform light position from world space to object space
		D3DXMatrixInverse( &matObjWorldInv, NULL, &(pNC->m_matWorld) );	
		D3DXVec3Transform( &v4LightObjSpc, &m_LightPos, &matObjWorldInv );
		// Set vshader constants
		m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_POS_OSPACE, (float*)&v4LightObjSpc.x, 1 );
		// Render
		pNC->m_pMeshVB->Draw();
	}
	return( hr );
}

HRESULT StencilShadowDemo::RenderSceneCasters( TGroup<SceneCaster> * pGrp, D3DXMATRIX * pMatCameraWVP )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pGrp );
	FAIL_IF_NULL( pMatCameraWVP );

	D3DXMATRIX matWVP, matWVPTrans, matObjWorldInv;

	size_t i;
	SceneCaster * pSceneCaster;
	D3DXVECTOR4 v4LightObjSpc;

	for( i=0; i < pGrp->GetNumElements(); i++ )
	{
		pSceneCaster = pGrp->GetElement( i );
		if( pSceneCaster == NULL )
			continue;
		if( pSceneCaster->m_pSVMeshVB == NULL )
			continue;

		D3DXMatrixMultiply( &matWVP, &(pSceneCaster->m_matWorld), pMatCameraWVP );
		D3DXMatrixTranspose( &matWVPTrans, &matWVP );	
		m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, &matWVPTrans(0,0), 4);

		// Transform light position from world space to object space
		D3DXMatrixInverse( &matObjWorldInv, NULL, &(pSceneCaster->m_matWorld) );	
		D3DXVec3Transform( &v4LightObjSpc, &m_LightPos, &matObjWorldInv );

		// Set vshader constants
		m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_POS_OSPACE, (float*)&v4LightObjSpc.x, 1 );

		float inset = pSceneCaster->fShadowInset;
		m_pD3DDev->SetVertexShaderConstantF( CV_FATNESS_SCALE,
					D3DXVECTOR4( inset, inset, inset, 1.0f ), 1);

		float dist = pSceneCaster->fExDist;
		m_pD3DDev->SetVertexShaderConstantF( CV_SHDVOL_DIST,
					D3DXVECTOR4( dist, dist, dist, 0.0f ), 1 );
	
		// Render
		pSceneCaster->m_pSVMeshVB->Draw();
	}
	return( hr );
}

void StencilShadowDemo::AnimateScene()
{
	if( m_bRotateLight )
	{
		double fTimeInSeconds = g_Timer.GetElapsedTime();
		// Increase rotation
		m_fLightAngle += (float)( 1.5 * fTimeInSeconds );
		m_fLightAngle = fmod( m_fLightAngle, 3.14159f * 2.0f );
	}
	else
	{
		g_Timer.Start();
	}

	// Set light position in world space
	float lightr = 2.5f;
	m_LightPos.x  = lightr * (float)sin( m_fLightAngle );
	m_LightPos.y  = lightr * (float)cos( m_fLightAngle );
	m_LightPos.z  = 5.0f;
	//-------------------------------------------------

}


HRESULT StencilShadowDemo::Render()
{
	HRESULT hr = S_OK;
	D3DXMATRIX	matInvWorld;
	D3DXMATRIX	matTemp;
	D3DXVECTOR4  eyeObjSpc, lightObjSpc;	   // eye and light positions in object space

	// Clear to grey
	m_pD3DDev->Clear( 0, NULL, 
						D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
						0x00AAAAAA,			// ARGB
						1.0f,
						0x00 );	

	// Set vshader color switch to use regular diffuse color
	D3DXVECTOR4  regular( 1.0f, 0.0f, 0.0f, 0.0f ); // color switch mask
	m_pD3DDev->SetVertexShaderConstantF( CV_COLORSWITCH	, (float*)&regular, 1 );
	m_pD3DDev->SetVertexShaderConstantF( CV_ONE,		D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_ZERO,		D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f), 1);

	float ambient = 0.4f;
	ambient = 1.0f;
	ambient = 0.3f;
	ambient = 0.20f;
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_CONST,	 D3DXVECTOR4( ambient, 0.1f, 0.7f, 0.0f), 1);
	m_pD3DDev->SetVertexShaderConstantF( CV_FACTORS,	 D3DXVECTOR4( 0.4f, 0.25f, 0.125f, 0.33333f ), 1 );

	// Set light color in vertex shader constant
	D3DXVECTOR4 lcol( 1.0f, 1.0f, 1.0f, 0.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_COLOR, (float*)&lcol, 1);


	m_pD3DDev->SetPixelShaderConstantF( CP_HEIGHT_FOG_COLOR, D3DXVECTOR4(0.90f, 0.90f, 1.0f, 0.0f), 1);

	if( m_bWireframe )
	{
		m_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	else
	{
		m_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	m_pD3DDev->SetRenderState( D3DRS_LIGHTING,			TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,				D3DCMP_LESS );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
	m_pD3DDev->SetRenderState( D3DRS_NORMALIZENORMALS,	TRUE );
	m_pD3DDev->SetRenderState( D3DRS_LOCALVIEWER,		TRUE );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
	m_pD3DDev->SetRenderState( D3DRS_SPECULARENABLE,	FALSE );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,			false );
	m_pD3DDev->SetRenderState( D3DRS_FOGVERTEXMODE,		D3DFOG_NONE );  
	m_pD3DDev->SetRenderState( D3DRS_FOGTABLEMODE,		D3DFOG_NONE );
	m_pD3DDev->SetRenderState( D3DRS_FOGCOLOR,			0x00aaaaaa  );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false  );


	AnimateScene();

	//-------------------------------------------------------------
	
	// Set vertex shader for rendering objects normally
	m_pShaderManager->SetShader( m_VSHI_DirectionalLight );
	m_pD3DDev->SetPixelShader( NULL );

	// Correct for the lack fo ability to specify CFirstPersonCamera's UP vector
	D3DXMATRIX matWorld;
	D3DXMATRIX matCameraWVP;
	D3DXMatrixRotationX( &matWorld, -3.14159f/2.0f );
	D3DXMatrixMultiply( &matCameraWVP, &matWorld, g_Camera.GetViewMatrix() );
	D3DXMatrixMultiply( &matCameraWVP, &matCameraWVP, g_Camera.GetProjMatrix() );



	// Transform light and cam pos to object space and set vshader constants
	D3DXVec3Transform( &lightObjSpc, &m_LightPos, D3DXMatrixIdentity(&matTemp) );

	// Set vshader constants
	m_pD3DDev->SetVertexShaderConstantF( CV_LIGHT_POS_OSPACE, (float*)&lightObjSpc.x, 1 );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	// For vertex shader fog, must set D3DFOG_NONE
	// New way for Vertex Shader
	m_fFogStart = 5.0f;
	m_fFogEnd   = 17.0f;
	m_fFogRange = m_fFogEnd - m_fFogStart;

	D3DXVECTOR4  vFog( m_fFogStart, m_fFogEnd, m_fFogRange, 0.5f );
	m_pD3DDev->SetVertexShaderConstantF( CV_FOGPARAMS, (float*)&vFog, 1 );

	D3DXVECTOR4  vHtFog( m_fHtFogFac, 0.1f, 0.1f, 0.5f );
	m_pD3DDev->SetVertexShaderConstantF( CV_HEIGHT_FOG_PARAMS, (float*)&vHtFog, 1 );

	m_pD3DDev->SetTexture( 0,	m_pDecalTexture);
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER,		D3DTEXF_LINEAR);
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER,		D3DTEXF_LINEAR);

	// render non-casters
	if( m_pMeshes != NULL )
	{
		hr = RenderSceneNonCasters( & m_pMeshes->m_NonCasters, &matCameraWVP );
	}


	m_pD3DDev->SetTexture(0,0);//(LPDIRECT3DBASETEXTURE8) m_pLightObjTexture );	
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE, false );

	D3DXVECTOR4  vZero( 0.0f, 0.0f, 0.0f, 0.0f );
	m_pD3DDev->SetVertexShaderConstantF( CV_HEIGHT_FOG_PARAMS, (float*)&vZero, 1 );

	// No culling if model is exploded outward
	if( m_fExplodeDist > 0.0f || m_bNoCull )
	{
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}

	if( m_bRenderCastingObject )
	{
		if( m_pMeshes != NULL )
		{
			hr = RenderSceneCasters( & m_pMeshes->m_BigSceneCasters, &matCameraWVP );
		}

		// If appropriate, redraw zero area tris in a different color
		//  to illustrate them
		if( m_fExplodeDist > 0.0f )
		{
			D3DXVECTOR4  fac1( 0.0f, 0.0f, 0.9f, 0.0f ); // rgba
			D3DXVECTOR4  fac2( 0.0f, 1.0f, 0.0f, 0.0f ); // color switch mask
			m_pD3DDev->SetVertexShaderConstantF( CV_COLOR		, (float*)&fac1, 1 );
			m_pD3DDev->SetVertexShaderConstantF( CV_COLORSWITCH	, (float*)&fac2, 1 );
			m_pD3DDev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
			//@@ call ShadowVolumeVB object's DrawZeroAreaTris()
		}
	}

	// Draw shadow volume from objects casting shadows
	switch( m_eShadeModes[m_nCurrentShadeMode] )
	{
	case STENCIL_SHADOW_TWO_PASS:
		hr = RenderStencilShadow( &matCameraWVP );
		hr = RenderDarkenStencilArea();

		m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
		m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false  );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
		m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
		break;

	case STENCIL_SHADOW_TWOSIDED:
		hr = RenderStencilShadowTwoSided( &matCameraWVP );
		hr = RenderDarkenStencilArea();

		m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
		m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false  );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
		m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
		break;

	case ALPHA_SHADOW_VOLUME:
		RenderShadowVolumeAlphaBlended( &matCameraWVP );
		break;
	case DONT_DRAW_SHADOW :
		break;
	}

	//--------------------------------------------------------------------
	// Set world matrix to light position & render light stand-in object:
	D3DXMatrixTranslation( &matTemp, m_LightPos.x, m_LightPos.y, m_LightPos.z );
	D3DXMatrixMultiply( &matWorld, &matTemp, &matWorld );

	// Setup the transforms
	m_pD3DDev->SetTransform( D3DTS_PROJECTION,	g_Camera.GetProjMatrix() );
	m_pD3DDev->SetTransform( D3DTS_VIEW,		g_Camera.GetViewMatrix() );
	m_pD3DDev->SetTransform( D3DTS_WORLD,		&matWorld );

	m_pD3DDev->SetVertexShader( NULL );
	m_pD3DDev->SetFVF( MESHVERTEX_FVF );

	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, 0xFFFFFFFF );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

	m_pD3DDev->SetTexture( 0, m_pLightObjTexture );

	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,			false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );

	// Draw the light object
	if( m_pLightMesh != NULL )
	{
		m_pLightMesh->Draw();
	}
	return hr;
}



HRESULT StencilShadowDemo::RenderShadowVolumeAlphaBlended( D3DXMATRIX * pMatCameraWVP )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pShaderManager );
	m_pShaderManager->SetShader( m_VSHI_ExtrudeVolume );
	
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	m_pD3DDev->SetTexture( 0, m_pLightObjTexture );	
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE, false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, true );

	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,			0x33FFFFFF );	// argb
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_SELECTARG1 );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,		D3DBLEND_SRCALPHA );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA );
	
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	FALSE );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,         D3DCMP_LESS );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,       TRUE );

	if( m_pMeshes != NULL )
	{
		hr = RenderSceneCasters( & m_pMeshes->m_BigSceneCasters, pMatCameraWVP );
	}
	return( hr );
}


HRESULT StencilShadowDemo::RenderStencilShadow( D3DXMATRIX * pMatCameraWVP )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pMeshes );
	FAIL_IF_NULL( pMatCameraWVP );

	m_pD3DDev->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, false );

	m_pShaderManager->SetShader( m_VSHI_ExtrudeVolume );

	m_pD3DDev->SetRenderState( D3DRS_LIGHTING,			false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE,	false );

	// to avoid depth aliasing when shadow inset value = 0;
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_LESS );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_DIFFUSE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_SELECTARG1 );

	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	
	//render back faces
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CW );
	// do not write Z depth when drawing stencil shadow volumes!
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,				D3DCMP_LESS );

	//increment stencil on z-fail
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		TRUE );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS );
	m_pD3DDev->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_INCRSAT );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_KEEP );
	
	// Render back faces of shadow volume
	RenderSceneCasters( &m_pMeshes->m_BigSceneCasters, pMatCameraWVP );

	// Render front faces of shadow volume
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	//decrement stencil on z-fail
	m_pD3DDev->SetRenderState( D3DRS_STENCILZFAIL,  D3DSTENCILOP_DECRSAT );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFAIL,	D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,	D3DSTENCILOP_KEEP );

	// Render Front faces
	RenderSceneCasters( &m_pMeshes->m_BigSceneCasters, pMatCameraWVP );

	//--------------------------------------------
	// Option to visualize stencil buffer values
	if( m_bRevealStencilAsColor )
	{
		if( g_pTestStencil )
			g_pTestStencil->RevealStencilValues( STENCIL_REVEAL_COLOR, true );

		m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
		m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false  );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
		m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
		return( hr );
	}

	return( hr );
}

HRESULT StencilShadowDemo::RenderStencilShadowTwoSided( D3DXMATRIX * pMatCameraWVP )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMatCameraWVP );
	FAIL_IF_NULL( m_pMeshes );
	FAIL_IF_NULL( m_pD3DDev );

	m_pD3DDev->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, true );

	m_pShaderManager->SetShader( m_VSHI_ExtrudeVolume );

	m_pD3DDev->SetRenderState( D3DRS_LIGHTING,			false );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE,	false );
	// to avoid depth aliasing when shadow inset value = 0;
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,				D3DCMP_LESS );

	// must not render Z depth of shadow volume faces.  Depth test remains on.
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		true );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );

	// CW modes for back faces
	// Increment stencil on z-fail
	m_pD3DDev->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_INCR	);
	m_pD3DDev->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP	);
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_KEEP	);
	m_pD3DDev->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS		);

	// CCW modes for front faces
	// decrement stencil on z-fail
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILZFAIL,	D3DSTENCILOP_DECR	);
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFAIL,	D3DSTENCILOP_KEEP	);
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILPASS,	D3DSTENCILOP_KEEP	);
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFUNC,	D3DCMP_ALWAYS		);

	RenderSceneCasters( &m_pMeshes->m_BigSceneCasters, pMatCameraWVP );

	// Option to visualize stencil buffer values
	if( m_bRevealStencilAsColor )
	{
		m_pD3DDev->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, false );
		if( g_pTestStencil )
			g_pTestStencil->RevealStencilValues( STENCIL_REVEAL_COLOR, true );

		m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
		m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false  );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
		m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		true );
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );
		return( hr );
	}

	return( hr );
}

HRESULT StencilShadowDemo::RenderDarkenStencilArea()
{
	HRESULT hr = S_OK;
	// Render a quad over the entire screen to darken the shadow areas
	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		                                               D3DCOLORWRITEENABLE_RED |
													   D3DCOLORWRITEENABLE_GREEN | 
													   D3DCOLORWRITEENABLE_BLUE );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Render a quad over the viewport to darken the shadow areas
	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,	m_dwShadowAttenValue );	
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		false );

	m_pD3DDev->SetPixelShader( NULL );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1);
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE);

	//alpha
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE);
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_DESTCOLOR );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ZERO );

	//stencil
	m_pD3DDev->SetRenderState( D3DRS_STENCILZFAIL,	D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFAIL,	D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,	D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFUNC,	D3DCMP_NOTEQUAL );
	if( m_bHWSupportsTwoSidedStencil )
	{
		m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILZFAIL,	D3DSTENCILOP_KEEP );
		m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFAIL,	D3DSTENCILOP_KEEP );
		m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILPASS,	D3DSTENCILOP_KEEP );
		m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFUNC,	D3DCMP_NOTEQUAL );
	}

	m_pD3DDev->SetRenderState( D3DRS_STENCILREF,	0 );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );

	if( m_pQuad != NULL )
	{
		m_pQuad->Render();
	}
	else
	{
		FMsg("Quad object not ready!\n");
	}
	return( hr );
}

