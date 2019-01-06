//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_BumpyShinyVariations
// File:  BumpyShinyVariations.cpp
// 
// Copyright NVIDIA Corporation 2002-2003
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Note: This code uses the D3D Framework helper library.
// Link to: DX9SDKSampleFramework.lib d3dxof.lib dxguid.lib d3dx9dt.lib d3d9.lib winmm.lib comctl32.lib
//
//-----------------------------------------------------------------------------

#define STRICT

#include "BumpyShinyVariations.h"
#include <shared/GetFilePath.h>
#include <commctrl.h>
#include <string>

HINSTANCE g_hInstance = NULL;

//constants
const tstring BumpyShinyVariations::m_DefaultSceneFilename(_T("MEDIA/models/Rayguns/RayGuns.nvb"));
const tstring BumpyShinyVariations::m_DefaultDiffuseMapFilename(_T("MEDIA/textures/2d/basetex.bmp"));
const tstring BumpyShinyVariations::m_DefaultNormalMapFilename(_T("MEDIA/textures/2d/NMP_Ripples2_512.tga"));
const tstring BumpyShinyVariations::m_DefaultEnvironmentMapFilename(_T("MEDIA/textures/cubemaps/NV-Metal-b.dds"));

// identifiers for items we will add to the context menu: (step 1 of 3 in extending context menu)
#define ID_TOGGLE_FULLSCREEN (USER_ID_START_VALUE+0)
#define ID_QUIT              (USER_ID_START_VALUE+1)
#define ID_SHOW_SPEC         (USER_ID_START_VALUE+2)
#define ID_SHOW_BUMP         (USER_ID_START_VALUE+3)
#define ID_SHOW_ENV          (USER_ID_START_VALUE+4)
#define ID_NEXT              (USER_ID_START_VALUE+5)
#define ID_MORE_BUMP         (USER_ID_START_VALUE+6)
#define ID_LESS_BUMP         (USER_ID_START_VALUE+7)
#define ID_AUTO_ROTATE_WORLD (USER_ID_START_VALUE+8)

//-----------------------------------------------------------------------------
// Name: BumpyShinyVariations()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
BumpyShinyVariations::BumpyShinyVariations():
m_DefaultZFarFactor(10),
m_DefaultZNearFactor(1.0f / 500.0f),
m_DefaultSceneRadius(1),
m_DefaultCameraRadiusFactor(1.5f),
m_DefaultCameraYaw(0),
m_DefaultCameraPitch(0)
{
	m_DefaultSceneCenter[0] = m_DefaultSceneCenter[1] = m_DefaultSceneCenter[2] = 0;

    m_pEffect = NULL;
    m_pSDeclaration = NULL;
	m_DiffuseMap = NULL;
	m_NormalMap = NULL;
	m_CubeMap = NULL;
	m_Light = NULL;
	m_Scene = NULL;
	m_pDeclaration = NULL;
	m_pSimpleTransformEffect = NULL;
	m_LightPosition.x = 0;
	m_LightPosition.y = 1.0f;
	m_LightPosition.z = 0;
	m_LightDirection.x = 0;
	m_LightDirection.y = -1.0f;
	m_LightDirection.z = 0;
	m_AnimTime = 1.0f;
	m_Ambient = 0.1f;

	m_UseDefaultDiffuseMap = false;
	m_UseDefaultNormalMap = false;

	m_DrawNormals = false;
	m_DrawTangentBasis = false;
	m_DrawAxis = false;
	m_DrawLight = false;
	m_Wireframe = false;
	m_PauseLight = true;
	m_PauseCamera = true;
	m_PauseScene = false;
	m_FPS = 0;
	m_PauseFPS = false;
	m_Frame = 0;

	OneTimeSceneInit();
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT BumpyShinyVariations::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if (pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support 
    if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    
    if (!(pCaps->MaxSimultaneousTextures >= 2)) 
        if (!nErrors++) 
            MessageBox(NULL, _T("Device cannot dual texture!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)) 
        if (!nErrors++) 
            MessageBox(NULL, _T("Device cannot handle dotproduct3 operation!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    
    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.  Nothing DX9-related should be allocated
//       or created yet.
//-----------------------------------------------------------------------------
HRESULT BumpyShinyVariations::OneTimeSceneInit()
{
	m_SceneFilename = m_DefaultSceneFilename;
	m_DiffuseMapFilename = m_DefaultDiffuseMapFilename;
	m_NormalMapFilename = m_DefaultNormalMapFilename;
	m_CubeMapFilename = m_DefaultEnvironmentMapFilename;

	m_bBump = 1;
    m_bSpecular = 1;
    m_bEnvMap = false;
    m_fBumpScale = 1.0f;
    m_bAutoRotateWorld = 0;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.  Called at startup, and whenever you resize 
//       the window, switch windowed<->fullscreen, lose the device, etc.
//-----------------------------------------------------------------------------
HRESULT BumpyShinyVariations::RestoreDeviceObjects( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;
    assert(pd3dDevice);

	// Scene
	m_Scene = new NVBScene;
	if (FAILED(hr = m_Scene->Load(m_SceneFilename, pd3dDevice, GetFilePath::GetMediaFilePath))) {
		TCHAR buf[1024];
		_stprintf(buf, _T("Failed to load the scene: %s"), m_Scene->m_ErrorMessage.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return hr;
	}

	// Textures
	if (FAILED(hr = LoadTextures(pd3dDevice))) {
		MessageBox(NULL, _T("Failed to load textures"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return hr;
	}

	// Effect file (shaders)
	hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\SimpleTransform.fx")).c_str(),
		NULL, NULL, 0, NULL, &m_pSimpleTransformEffect, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, _T("Failed to load simple transform effect"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return hr;
	}

	// Assign registers to the relevant vertex attributes
	D3DVERTEXELEMENT9 declaration[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()
	};
	pd3dDevice->CreateVertexDeclaration(declaration, &m_pDeclaration);

	// Check geometry
    if (!m_Scene->m_VertexHasNormal) {
        MessageBox(NULL, _T("Model does not have normals"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
    if (!m_Scene->m_VertexHasTexture) {
        MessageBox(NULL, _T("Model does not have texture coordinates"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
    if (!m_Scene->m_VertexHasTangentBasis) {
        MessageBox(NULL, _T("Model does not have tangent basis"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    // Load effect shaders
    if (FAILED(LoadShaders(pd3dDevice)))
        return E_FAIL;

	// Light position and direction
	m_LightPosition.x = 0;
	m_LightPosition.y = 0.6f * m_Scene->m_Radius;
	m_LightPosition.z = - 0.42f * m_Scene->m_Radius;
	m_LightDirection = - m_LightPosition;
	D3DXVec3Normalize(&m_LightDirection, &m_LightDirection);

	m_Time = m_StartTime = ::timeGetTime()*0.001f;
	m_Frame = 0;
	m_FPS = 30;
	m_LastTime = m_Time;
	m_LastFrame = m_Frame;

	D3DXMatrixIdentity(&CubeRotationMatrix);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Clean up here anything you created in RestoreDeviceObjects().
//       Called on exit, and also whenever you resize the window, switch 
//       windowed<->fullscreen, lose the device, etc.
//-----------------------------------------------------------------------------
HRESULT BumpyShinyVariations::InvalidateDeviceObjects()
{
	ReleaseTextures();
	SAFE_DELETE(m_Scene);
	SAFE_DELETE(m_Light);
	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pSDeclaration);
	SAFE_RELEASE(m_pSimpleTransformEffect);

	SAFE_RELEASE(m_pEffect);
	return S_OK;
}

HRESULT BumpyShinyVariations::LoadShaders( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr = D3D_OK;

    // Load our main Effect file, which contains our shaders.  (note: path is relative to MEDIA\ dir)
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\BumpyShinyVariations.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // Assign registers to the relevant vertex attributes
    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  
        { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },  
        { 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
        { 0, 48, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 }, 
        { 0, 60, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 }, 
        D3DDECL_END()
    };

    pd3dDevice->CreateVertexDeclaration(declaration, &m_pSDeclaration);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT BumpyShinyVariations::Render( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

	// Update time
	m_Time = ::timeGetTime()*0.001f;
	if (m_Frame == 0)
		m_StartTime = m_Time;
	else if (m_Time > m_StartTime && m_Time > m_LastTime) 
	{
		float fps_narrow = (m_Frame - m_LastFrame) / (m_Time - m_LastTime);
		if (m_Time - m_StartTime > 1 && fps_narrow < 1) 
			m_StartTime += m_Time - m_LastTime;     // app was probably frozen due to a context menu or dialog box -> don't let the time lapse mess up the FPS evaluation.
		else if(!m_PauseFPS)
			m_FPS = (float)m_Frame / (m_Time - m_StartTime);
		m_LastTime = m_Time;
		m_LastFrame = m_Frame;
	}

	if (!m_PauseScene || !m_PauseCamera || !m_PauseLight)
		m_AnimTime += 30.0f / m_FPS;

	if (m_Scene)
		m_Scene->Update(m_AnimTime, &m_World, (m_PauseScene ? 0 : NVBScene::MESH) | (m_PauseCamera ? 0 : NVBScene::CAMERA) | (m_PauseLight ? 0 : NVBScene::LIGHT));


	// Update light position
	if (!m_PauseLight)
	{
		if (m_Scene && m_Scene->m_NumLights && m_Scene->m_NumLightKeys) {
			m_LightPosition = m_Scene->m_Lights[0].Position;
			m_LightDirection = m_Scene->m_Lights[0].Direction;
		}
		else {
			float angle = atan2f(m_LightPosition.x, m_LightPosition.y) + 1.0f/m_FPS;
			float lightDistance = - 1.4f * m_LightPosition.z;
			if (m_Scene) {
				m_LightPosition.x = m_Scene->m_Radius * sinf(angle);
				m_LightPosition.y = m_Scene->m_Radius * cosf(angle);
			}
			// if we don't have any nvb scene bu we specified the radius by hand
			else if (m_Scene) {
				m_LightPosition.x = (m_Scene->m_Radius) * sinf(angle);
				m_LightPosition.y = (m_Scene->m_Radius) * cosf(angle);
			}
			else {
				m_LightPosition.x = sinf(angle);
				m_LightPosition.y = cosf(angle);
			}
			m_LightDirection = - m_LightPosition;
			D3DXVec3Normalize(&m_LightDirection, &m_LightDirection);
		}
	}

	m_Frame++;

    // clear the screen
    pd3dDevice->Clear(0, NULL, 
                        D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
                        D3DCOLOR_XRGB(60, 60, 60),
                        1.0f,        // far Z depth
                        0    );        // stencil

    // Fill mode
    pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    // Cull mode
    pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);    // TODO: Kevin to export clean model

    // Render the scene
    if (FAILED(hr = DrawScene(pd3dDevice))) {
        MessageBox(NULL, _T("Failed to draw the scene"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    return S_OK;
}

HRESULT BumpyShinyVariations::DrawEnvironmentCube( IDirect3DDevice9* pd3dDevice )
{
    // Transform
    D3DXMATRIX world;
    float size = 2.0f * m_Scene->m_Radius;
    D3DXMatrixScaling(&world, size, size, size);
	if (m_bAutoRotateWorld)
	{
		D3DXMATRIX tmp;
		D3DXMatrixRotationY( &tmp, D3DXToRadian(6.0f / m_FPS));
		D3DXMatrixMultiply( &CubeRotationMatrix, &tmp, &CubeRotationMatrix);
		world *= CubeRotationMatrix * m_World;
	}
	else
	{
		//CubeRotationMatrix = m_World;
		world *= CubeRotationMatrix * m_World;
	}

    // We used fixed-function pipeline for world environment
    pd3dDevice->SetTransform(D3DTS_WORLD, &world);
    pd3dDevice->SetTransform(D3DTS_VIEW, &m_View);
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_Projection);
    pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    pd3dDevice->SetTexture(0, m_CubeMap);
    HRESULT hr;
    if (FAILED(hr = m_Scene->DrawCube()))
        return hr;
    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

    return S_OK;
}

HRESULT BumpyShinyVariations::DrawScene( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr = S_OK;

    // Render the environment
    if (m_bEnvMap && FAILED(hr = DrawEnvironmentCube( pd3dDevice ))) {
        MessageBox(NULL, _T("Failed to draw the environment"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // select a technique, from our .fx file, to do either Diffuse lighting only, or Diffuse + Specular.
	std::string sTechniqueName;
    if (m_bSpecular) 
    {
        if (m_bBump) 
            sTechniqueName = m_bEnvMap ? "BumpEnvSpecular" : "BumpSpecular";
        else 
            sTechniqueName = m_bEnvMap ? "EnvSpecular" : "Specular";
    }
    else 
    {
        if (m_bBump) 
            sTechniqueName = m_bEnvMap ? "BumpEnv" : "Bump";
        else 
            sTechniqueName = m_bEnvMap ? "Env" : "DiffuseOnly";
    }

    if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
    {
        TCHAR buf[2048];
#ifdef UNICODE
		int len = MultiByteToWideChar(CP_ACP,0,sTechniqueName.c_str(),-1,NULL,NULL);
		TCHAR *tmp = new TCHAR[len];
		MultiByteToWideChar(CP_ACP,0,sTechniqueName.c_str(),-1,tmp,len);
		
		_stprintf(buf, _T("Failed to set '%s' technique in effect file"), tmp);
#else
		_stprintf(buf, _T("Failed to set '%s' technique in effect file"), sTechniqueName.c_str());
#endif
        MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

	m_pEffect->SetValue("Ambient", &m_Ambient, sizeof(float));
    m_pEffect->SetTexture("EnvironmentMap", m_CubeMap);
    
    // Draw
    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

            for (unsigned int i = 0; i < m_Scene->m_NumMeshes; ++i) {
                const NVBScene::Mesh& mesh = m_Scene->m_Meshes[i];
                m_pEffect->SetTexture("DiffuseMap", (mesh.m_DiffuseMap && !m_UseDefaultDiffuseMap) ? mesh.m_DiffuseMap : m_DiffuseMap);
                m_pEffect->SetTexture("NormalMap" , (mesh.m_NormalMap  && !m_UseDefaultNormalMap ) ? mesh.m_NormalMap  : m_NormalMap );
                SetBumpDot3VSConstants(mesh.m_Transform);

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pSDeclaration);

				m_pEffect->CommitChanges();
                if (FAILED(hr = mesh.Draw()))
                    return hr;
            }
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    // Draw normals, tangent basis, and/or coordinate axes, if enabled
    DrawAugmentations();      

    // Draw the light
    if (m_DrawLight && FAILED(hr = DrawDirectionLight(pd3dDevice))) {
        MessageBox(NULL, _T("Failed to draw the light"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    return S_OK;
}

void BumpyShinyVariations::SetBumpDot3VSConstants(const D3DXMATRIX& world)
{
    // Set transform from object space to projection space
    D3DXMATRIX worldViewProj = world * m_View *  m_Scene->m_Cameras[0].m_Projection;
    m_pEffect->SetMatrix("WorldViewProj", &worldViewProj);

    // Send light direction in object space
    D3DXMATRIX world2obj;
    D3DXMatrixInverse(&world2obj, 0, &world);
    D3DXVECTOR3 lightDirectionInObjectSpace;
    D3DXVec3TransformNormal(&lightDirectionInObjectSpace, &m_LightDirection, &world2obj);
    // shader math requires that the light direction vector points towards the light.
    lightDirectionInObjectSpace = - lightDirectionInObjectSpace;
    D3DXVec3Normalize(&lightDirectionInObjectSpace, &lightDirectionInObjectSpace);
    m_pEffect->SetValue("LightVector", &lightDirectionInObjectSpace, sizeof(D3DXVECTOR3));

    // Send eye position in object space
    D3DXMATRIX view2world;
    D3DXMatrixInverse(&view2world, 0, &m_View);
    D3DXVECTOR3 eyePositionInWorldSpace(view2world._41, view2world._42, view2world._43);
    D3DXVECTOR3 eyePositionInObjectSpace;
    D3DXVec3TransformCoord(&eyePositionInObjectSpace, &eyePositionInWorldSpace, &world2obj);
    m_pEffect->SetValue("EyePosition", &eyePositionInObjectSpace, sizeof(D3DXVECTOR3));

    //D3DXMATRIX CubeRotationMatrix = m_World;
    D3DXMATRIX obj2cube;
    D3DXMATRIX world2cube;

    // For environment mapping: transform from object space to cube space
    D3DXMatrixTranspose(&world2cube, &CubeRotationMatrix);
    D3DXMatrixMultiply(&obj2cube, &world, &world2cube);
    //D3DXMatrixTranspose(&obj2cube, &obj2cube);
    m_pEffect->SetMatrix("ObjToCubeSpace", &obj2cube);

    // For environment mapping: Set the bump scale
    m_pEffect->SetValue("BumpScaleEnv", &m_fBumpScale, sizeof(float));

    // For environment mapping: eye position in cube space
    D3DXMatrixInverse(&view2world, 0, &m_View);
    D3DXVECTOR3 eyeWorldPos(view2world._41, view2world._42, view2world._43);
    D3DXVECTOR3 eyeCubePos;
    D3DXMatrixTranspose(&world2cube, &CubeRotationMatrix);
    D3DXVec3TransformCoord(&eyeCubePos, &eyeWorldPos, &world2cube);
    m_pEffect->SetValue("EyePositionEnv", &eyeCubePos, sizeof(D3DXVECTOR3));
}

HRESULT BumpyShinyVariations::DrawAugmentations()
{
	if (m_Scene == 0)
		return S_OK;

	// Draw normals
	if (m_DrawNormals)
		m_Scene->DrawNormals(&m_Projection, &m_View);

	// Draw tangent basis
	if (m_DrawTangentBasis)
		m_Scene->DrawTangentBasis(&m_Projection, &m_View);

	// Draw tangent basis
	if (m_DrawAxis)
		m_Scene->DrawCoordinateAxis(&m_Projection, &m_View);

	return S_OK;
}

HRESULT BumpyShinyVariations::DrawDirectionLight( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;

	if (!m_Light) {
		// Load light model if not done yet
		m_Light = new NVBScene;
		if (FAILED(hr = m_Light->Load(_T("directionLight.nvb"), pd3dDevice, GetFilePath::GetFilePath))) {
			MessageBox(NULL, _T("Failed to load the direction light model"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return hr;
		}
	}

	// Arrow direction in object space
	D3DXVECTOR3 dirArrow = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	// Rotate to make the arrow parallel to the light direction
	D3DXVECTOR3 rotationAxis;
	D3DXVec3Cross(&rotationAxis, &dirArrow, &m_LightDirection);
	D3DXVec3Normalize(&rotationAxis, &rotationAxis);
	D3DXMATRIX world;
	D3DXMatrixRotationAxis(&world, &rotationAxis, acosf(D3DXVec3Dot(&m_LightDirection, &dirArrow)));

	// Scale
	float scale = 0.1f;
	if(m_Scene)
		scale *= m_Scene->m_Radius / m_Light->m_Radius;
	D3DXMATRIX scaling;
	D3DXMatrixScaling(&scaling, scale, scale, scale);
	world *= scaling;

	// Translate to offset from the object
	D3DXMATRIX translation;// = m_World;
	D3DXMatrixIdentity(&translation);
	D3DXMatrixTranslation(&translation, m_LightPosition.x + translation._41, m_LightPosition.y + translation._42, m_LightPosition.z + translation._43);
	world *= translation;

	// Set shader parameters
	D3DXMATRIX worldViewProj =  world * m_View *  m_Scene->m_Cameras[0].m_Projection;
	m_pSimpleTransformEffect->SetMatrix("WorldViewProj", &worldViewProj);
	D3DXVECTOR4 lightColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_pSimpleTransformEffect->SetValue("Color", &lightColor, sizeof(D3DXVECTOR4));

	pd3dDevice->SetVertexDeclaration(m_pDeclaration);

	pd3dDevice->SetFVF(D3DFVF_XYZ);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	// Activate shaders
	if (FAILED(m_pSimpleTransformEffect->SetTechnique("SimpleTransformTechnique")))
	{
		MessageBox(NULL, _T("Unable to set SimpleTransform technique"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return E_FAIL;
	}

	UINT uPasses;
	if (D3D_OK == m_pSimpleTransformEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
		for (UINT uPass = 0; uPass < uPasses; uPass++) {
			// Set the state for a particular pass in a technique.
			m_pSimpleTransformEffect->BeginPass(uPass);

			// Draw the light
			if (FAILED(hr = m_Light->m_Meshes[0].Draw()))
				return hr;
			m_pSimpleTransformEffect->EndPass();
		}
		m_pSimpleTransformEffect->End();
	}

	return S_OK;
}

/*******************************************************************************

Texture management

*******************************************************************************/

HRESULT BumpyShinyVariations::LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DTEXTURE9& texture, D3DPOOL pool, D3DXIMAGE_INFO* info)
{
	SAFE_RELEASE(texture);
	HRESULT hr;
	if (FAILED(hr = D3DXCreateTextureFromFileEx(pd3dDevice, GetFilePath::GetFilePath(filename).c_str(),
		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		0, D3DFMT_A8R8G8B8, pool,
		D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0,
		info, 0, &texture)))
	{
		TCHAR buf[1024];
		_stprintf(buf, _T("Could not create texture %s"), filename.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
	}
	return hr;
}

HRESULT BumpyShinyVariations::LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DCUBETEXTURE9& texture, D3DXIMAGE_INFO* info)
{
	SAFE_RELEASE(texture);
	HRESULT hr;
	if (FAILED(hr = D3DXCreateCubeTextureFromFileEx(pd3dDevice, GetFilePath::GetFilePath(filename).c_str(), 
		D3DX_DEFAULT, 0, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0,
		info, 0, &texture)))
	{
		TCHAR buf[1024];
		_stprintf(buf, _T("Could not create texture %s"), filename.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
	}
	return hr;
}

void BumpyShinyVariations::ReleaseTextures()
{
	SAFE_RELEASE(m_DiffuseMap);
	SAFE_RELEASE(m_NormalMap);
	SAFE_RELEASE(m_CubeMap);
}

HRESULT BumpyShinyVariations::LoadTextures( IDirect3DDevice9* pd3dDevice )
{
	ReleaseTextures();
	HRESULT hr;
	if (m_DiffuseMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_DiffuseMapFilename, m_DiffuseMap)))
			return hr;
	if (m_NormalMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_NormalMapFilename, m_NormalMap)))
			return hr;
	if (m_CubeMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_CubeMapFilename, m_CubeMap)))
			return hr;
	return S_OK;
}
