/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\Membrane\
File:  MembraneDemo.cpp

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

#include "MembraneDemo.h"
#include "MEDIA\Programs\D3D9_Membrane\MembraneConstants.h"
#include "shared/NV_Common.h"
#include <DXUT/DXUTcamera.h>



#define XFILE_TO_LOAD			TEXT("bigship1.x")
#define FILE_CHECKERBOARD		TEXT("MEDIA\\textures\\2D\\checker_single16.dds")

#define FILE_YELLOWMATALIC		TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_YellowMetalic.png")		
#define FILE_BLUE				TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_blue.png")
#define FILE_PURPLEHIGHLIGHT	TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_PurpleHighlight.png")
#define FILE_RAINBOWMETALIC		TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_RainbowMetalic.png")
#define FILE_BLUEREDEDGE		TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_BlueRedEdge.png")
#define FILE_BLUEPURPINV		TEXT("MEDIA\\textures\\2d\\Gradients2\\Membrane_Green_BluePurpInv.png")

#define VSHN_MembraneShader "MEDIA\\programs\\D3D9_Membrane\\Membrane_vs11.vsh"

extern CModelViewerCamera	g_Camera;

//-----------------------------------------
MembraneDemo::MembraneDemo()
{
	SetAllNull();
}
MembraneDemo::~MembraneDemo()
{
	Free();
	SetAllNull();
}
void MembraneDemo::SetAllNull()
{
	m_pD3DDev			= NULL;
	m_pShaderManager	= NULL;
	m_pLoadXFile		= NULL;
	m_pTextureDisplay	= NULL;
}

HRESULT MembraneDemo::Free()
{
	SAFE_DELETE( m_pLoadXFile );
	SAFE_DELETE( m_pShaderManager );
	SAFE_DELETE( m_pTextureDisplay );
	SAFE_RELEASE( m_pD3DDev );	
	return( S_OK );
}

HRESULT MembraneDemo::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pDev );
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_pLoadXFile = new LoadXFile;
	MSG_AND_RET_VAL_IF( m_pLoadXFile == NULL, TEXT("Couldn't create xfile loader!\n"), E_FAIL );
	hr = m_pLoadXFile->Initialize( m_pD3DDev, GetFilePath::GetMediaFilePath );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't initialize LoadXFile class\n"), E_FAIL );
	hr = m_pLoadXFile->LoadFile( XFILE_TO_LOAD, false );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load X-file mesh\n"), E_FAIL );	
//	m_pLoadXFile->ListMeshInfo();

	// Set the class's matrix to transform the object to the given bounding box
	m_pLoadXFile->SetMatrixToXFormMesh( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), LoadXFile::KEEP_ASPECT_RATIO );
	// Set the camera to view the object
	float dist = 1.5f;
	g_Camera.SetViewParams( &D3DXVECTOR3( -dist, dist*3.0f/5.0f, -dist ), &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );
	g_Camera.SetProjParams( 3.14159f * 0.25f, viewport.Width / ((float)viewport.Height), 0.1f, 15.0f );

	// Load color ramp textures for the membrane effect
	// Use the LoadXFile class's TextureManager
	TextureFactory **	ppTF;
	ppTF = m_pLoadXFile->GetTextureFactoryPP();
	MSG_AND_RET_VAL_IF( ppTF == NULL, TEXT("no ppTF\n"), E_FAIL );
	MSG_AND_RET_VAL_IF( *ppTF == NULL, TEXT("no *ppTF\n"), E_FAIL );

	// load textures
	// make list of textures to load
	TextureFilenamePair tfp;
	m_vGradientTextures.clear();
	tfp.m_ppTexture = NULL;
	tfp.m_tsFilename = FILE_YELLOWMATALIC;		m_vGradientTextures.push_back( tfp );
	tfp.m_tsFilename = FILE_BLUEPURPINV;		m_vGradientTextures.push_back( tfp );
	tfp.m_tsFilename = FILE_BLUEREDEDGE;		m_vGradientTextures.push_back( tfp );
	tfp.m_tsFilename = FILE_RAINBOWMETALIC;		m_vGradientTextures.push_back( tfp );
	tfp.m_tsFilename = FILE_PURPLEHIGHLIGHT;	m_vGradientTextures.push_back( tfp );
	tfp.m_tsFilename = FILE_BLUE;				m_vGradientTextures.push_back( tfp );
	// load the textures
	size_t i;
	for( i=0; i < m_vGradientTextures.size(); i++ )
	{
		m_vGradientTextures.at(i).m_ppTexture = (*ppTF)->CreateTextureFromFile( m_pD3DDev, m_vGradientTextures.at(i).m_tsFilename.c_str());
		if( m_vGradientTextures.at(i).m_ppTexture == NULL )
		{
			FMsg(TEXT("MembraneDemo couldn't load texture: %s\n"), m_vGradientTextures.at(i).m_tsFilename.c_str() );
		}
	}
	RET_VAL_IF( m_vGradientTextures.size() == 0, E_FAIL );
	m_ppTexCurrent = m_vGradientTextures.at(0).m_ppTexture;
	m_ptstrCurrentTextureFilename = &(m_vGradientTextures.at(0).m_tsFilename);
	m_uTexCurrent = 0;

	// Load the vertex shader for the membrane effect.  This shader converts the dot product of 
	// view direction and surface normal to a texture coordinate.
	m_pShaderManager = new ShaderManager;
	MSG_AND_RET_VAL_IF( m_pShaderManager == NULL, TEXT("Couldn't create ShaderManager\n"), E_FAIL );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );

	hr = m_pShaderManager->LoadAndAssembleShader( TEXT(VSHN_MembraneShader), SM_SHADERTYPE_VERTEX, &m_VSHI_MembraneShader );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") TEXT(VSHN_MembraneShader) TEXT(" shader\n"), E_FAIL );

	// TexureDisplay is used only to display the color ramp texture in the lower part of the screen
	m_pTextureDisplay = new TextureDisplay2;
	MSG_AND_RET_VAL_IF( m_pTextureDisplay==NULL, TEXT("Couldn't create TextureDisplay2\n"), E_FAIL );
	hr = m_pTextureDisplay->Initialize( m_pD3DDev );
	FRECT frect = FRECT( 0.01f, 0.90f, 0.5f, 0.99f );		// bottom left of the screen
	IDirect3DTexture9 ** ppTexChecker;
	ppTexChecker = (*ppTF)->CreateTextureFromFile( m_pD3DDev, FILE_CHECKERBOARD );
	MSG_AND_RET_VAL_IF( ppTexChecker == NULL, FILE_CHECKERBOARD, E_FAIL );

	m_pTextureDisplay->AddTexture( &m_TID_Checkerboard, ppTexChecker, frect );
	// Make texture coordinates for the checkerboard pattern so that it repeats.
	// Correct for the aspect ratio of the screen space position
	FRECT chktx = FRECT( 0.0f, 0.0f, 0.0f, 0.0f );
	chktx.right = 20;
	chktx.bottom = floor( (( chktx.right * (frect.bottom - frect.top) / (frect.right - frect.left) + 0.5f )));
	m_pTextureDisplay->SetTextureCoords( m_TID_Checkerboard, chktx );

	m_pTextureDisplay->AddTexture( &m_TID_RampTexture,  m_ppTexCurrent, frect );

	return( hr );
}

HRESULT MembraneDemo::ReloadTextures()
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pLoadXFile == NULL, E_FAIL );
	TextureFactory **	ppTF;
	ppTF = m_pLoadXFile->GetTextureFactoryPP();
	MSG_AND_RET_VAL_IF( ppTF == NULL, TEXT("no ppTF\n"), E_FAIL );
	MSG_AND_RET_VAL_IF( *ppTF == NULL, TEXT("no *ppTF\n"), E_FAIL );

	hr = (*ppTF)->ReloadTexturesFromDisk( true );
	return(hr);
}

HRESULT MembraneDemo::UseNextColorRamp()
{
	HRESULT hr = S_OK;
	m_uTexCurrent ++;
	m_uTexCurrent = m_uTexCurrent % m_vGradientTextures.size();
	if( m_vGradientTextures.size() > 0 )
	{
		m_ppTexCurrent = m_vGradientTextures.at( m_uTexCurrent ).m_ppTexture;
		m_ptstrCurrentTextureFilename = &(m_vGradientTextures.at( m_uTexCurrent ).m_tsFilename);
	}
	return( hr );
}

HRESULT MembraneDemo::Render()
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pLoadXFile == NULL, E_FAIL );
	RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	RET_VAL_IF( m_ppTexCurrent == NULL, E_FAIL );
	RET_VAL_IF( *m_ppTexCurrent == NULL, E_FAIL );

    // Clear the viewport
    m_pD3DDev->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x00000000,			// black
						1.0f, 0L);

	D3DXMATRIX matWorld;
	D3DXMatrixMultiply( &matWorld, m_pLoadXFile->GetMatrixP(), g_Camera.GetWorldMatrix() );
	D3DXMATRIX matWVP, matWVP_T;
	D3DXMatrixMultiply( &matWVP, &matWorld, g_Camera.GetViewMatrix() );
	D3DXMatrixMultiply( &matWVP, &matWVP, g_Camera.GetProjMatrix() );
	D3DXMatrixTranspose( &matWVP_T, &matWVP );

	// enable additive alpha blending
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ONE );
	// no backface culling
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_NONE );
	// no depth testing
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		D3DZB_FALSE );

	// Transform eye position into object space
	D3DXVECTOR3 EyeObjsp;
	D3DXMATRIX matInvWorld;
	D3DXMatrixInverse( &matInvWorld, NULL, &matWorld );
	D3DXVec3TransformCoord( &EyeObjsp, g_Camera.GetEyePt(), &matInvWorld ); 

	D3DXVECTOR4 Tc0_Offset = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );

	// Set shader constants
	m_pD3DDev->SetVertexShaderConstantF( MAT_WVP_0, (float*)(&matWVP_T), 4 );
	m_pD3DDev->SetVertexShaderConstantF( EYE_POS_OBJ_SPACE, (float*)&EyeObjsp, 1 );
	m_pD3DDev->SetVertexShaderConstantF( TXCRD_OFFSET_0, (float*)Tc0_Offset, 1 );

	m_pShaderManager->SetShader( m_VSHI_MembraneShader );

	// Set the color ramp texture
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1 );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	// no texture coordinate wrapping
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );		// trilinear
	m_pD3DDev->SetTexture( 0, *m_ppTexCurrent );

	m_pLoadXFile->Render( false, false );

	// Render the color ramp texture to a rectangle in the lower left of the screen
	// Render it on top of a checkerboard texture
	m_pTextureDisplay->Render( m_TID_Checkerboard );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA );
	m_pTextureDisplay->SetTexture( m_TID_RampTexture, m_ppTexCurrent );
	m_pTextureDisplay->Render( m_TID_RampTexture );
	return( hr );
}

