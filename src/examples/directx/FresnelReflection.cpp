//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_FresnelReflection
// File:  FresnelReflection.cpp
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

#include "FresnelReflection.h"

HINSTANCE g_hInstance = NULL;

//constants
const tstring FresnelReflection::m_DefaultSceneFilename = _T("RocketCar.nvb");

// identifiers for items we will add to the context menu: (step 1 of 3 in extending context menu)
#define ID_TOGGLE_FULLSCREEN (USER_ID_START_VALUE+0)
#define ID_QUIT              (USER_ID_START_VALUE+1)
#define ID_NO_FRESNEL        (USER_ID_START_VALUE+2)
#define ID_1DOT3             (USER_ID_START_VALUE+3)
#define ID_VS                (USER_ID_START_VALUE+4)
#define ID_TEXLOOKUP         (USER_ID_START_VALUE+5)
#define ID_REGCOMB           (USER_ID_START_VALUE+6)
#define ID_WATER             (USER_ID_START_VALUE+7)
#define ID_PLEX              (USER_ID_START_VALUE+8)
#define ID_GLASS             (USER_ID_START_VALUE+9)
#define ID_ZIRCON            (USER_ID_START_VALUE+10)
#define ID_DIAMOND           (USER_ID_START_VALUE+11)
#define ID_PLUS_KEY          (USER_ID_START_VALUE+12)
#define ID_MINUS_KEY         (USER_ID_START_VALUE+13)

float const FresnelReflection::kRefractIndex[] = 
{
    1.333f,     // REFRACT_WATER/air
    1.51f,      // REFRACT_PLEXIGLAS/air
    1.66f,      // REFRACT_DFLINTGLASS/air
    1.922f,     // REFRACT_ZIRCON/air
    2.416f      // REFRACT_DIAMOND/air
};

const float FresnelReflection::m_DefaultAmbient = 0.1f;

//-----------------------------------------------------------------------------
// Name: FresnelReflection()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
FresnelReflection::FresnelReflection() :
	m_DefaultSceneRadius(1),
	m_DefaultCameraRadiusFactor(1.5f),
	m_DefaultCameraYaw(0),
	m_DefaultCameraPitch(0),
	m_DefaultZFarFactor(10),
	m_DefaultZNearFactor(1.0f / 500.0f),
	m_Scene(0),
	m_Light(0),
	m_DiffuseMap(0),
	m_HeightMap(0),
	m_NormalMap(0),
	m_CubeMap(0),
	m_Wireframe(false),
	m_DrawAxis(false),
	m_UseDefaultDiffuseMap(false),
	m_UseDefaultNormalMap(false),
	m_DrawNormals(false),
	m_DrawTangentBasis(false),
	m_AnimTime(1),
	m_pDeclaration(NULL),
	m_PauseScene(false),
	m_PauseCamera(true),
	m_PauseLight(false),
	m_Ambient(m_DefaultAmbient),
	m_main_menu(0),
	m_context_menu(0),
	m_pSimpleTransformEffect(0),
	m_FPS(0),
	m_PauseFPS(false),
	m_Frame(0)
{
    m_pEffect = NULL;
    m_pDeclaration = NULL;
	m_pSDeclaration = NULL;
    mpCubeTexture = NULL;
    mpFresnelFunctionMap = NULL;
    mpWorldBoxVertices = NULL;
    mpWorldBoxIndices = NULL;
    meApproximationOption = APPROX_VERTEX;
    meRefractionOption = REFRACT_PLEXIGLAS;
    mRefractionIndex = kRefractIndex[REFRACT_PLEXIGLAS];

	m_DefaultSceneCenter[0] = m_DefaultSceneCenter[1] = m_DefaultSceneCenter[2] = 0;
	m_LightPosition.x = 0;
	m_LightPosition.y = 1.0f;
	m_LightPosition.z = 0;
	m_LightDirection.x = 0;
	m_LightDirection.y = -1.0f;
	m_LightDirection.z = 0;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT FresnelReflection::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    if(pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 2.0 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support cubemaps!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 3 element texture coordinates!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.  Nothing DX9-related should be allocated
//       or created yet.
//-----------------------------------------------------------------------------
HRESULT FresnelReflection::OneTimeSceneInit()
{
    // set these params here so that if user resizes window after they loaded
    // a new scene or changed other settings, they won't be reset:
    m_SceneFilename = m_DefaultSceneFilename;
    m_PauseScene = true;
    m_PauseLight = true;
    m_PauseCamera = true;
    //...

    // IMPORTANT: you must set this if your demo will be calling GetRenderState() at any point!  
    // Otherwise, the D3D app framework will create a pure device and GetRenderState will fail.
    //m_d3dSettings.Windowed_VertexProcessingType = HARDWARE_VP;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.  Called at startup, and whenever you resize 
//       the window, switch windowed<->fullscreen, lose the device, etc.
//-----------------------------------------------------------------------------
HRESULT FresnelReflection::RestoreDeviceObjects( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    assert(pd3dDevice);

    // Load our main Effect file, which contains our shaders.  (note: path is relative to MEDIA\ dir)
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\FresnelReflection.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // all VBs used are going to look like this
    const D3DVERTEXELEMENT9 vertex_definition[] = {
        { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL  , 0 },
        { 0, 6 * sizeof(float), D3DDECLTYPE_D3DCOLOR,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_COLOR   , 0 },
        { 0, 7 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 9 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
        { 0, 12* sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 }, 
        { 0, 15* sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 }, 
        D3DDECL_END()
    };

    pd3dDevice->CreateVertexDeclaration(vertex_definition, &m_pDeclaration);
    pd3dDevice->SetVertexDeclaration(m_pDeclaration);
    
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
	pd3dDevice->CreateVertexDeclaration(declaration, &m_pSDeclaration);
	
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


    // Create world-cube
    hr = pd3dDevice->CreateVertexBuffer(24 * sizeof(WorldBoxVertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0), D3DPOOL_MANAGED, &mpWorldBoxVertices, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create vertex buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    hr = pd3dDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mpWorldBoxIndices, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create index buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    WorldBoxVertex  *pVertices = NULL;
    WORD            *pIndices  = NULL;
    hr = mpWorldBoxVertices->Lock(0, 24*sizeof(WorldBoxVertex),(void**)&pVertices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not lock vertex buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    hr = mpWorldBoxIndices->Lock(0, 36*sizeof(WORD),(void**)&pIndices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not lock vertex buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    CreateCube(pVertices, pIndices);
    mpWorldBoxVertices->Unlock();
    mpWorldBoxIndices->Unlock();

    // allocate environment map (cube texture)
    hr = D3DXCreateCubeTextureFromFileEx(pd3dDevice, 
        GetFilePath::GetMediaFilePath(_T("InCloudsCubemap.dds")).c_str(),
        D3DX_DEFAULT,
        0,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_FILTER_LINEAR,
        D3DX_FILTER_LINEAR,
        0,
        NULL,
        NULL,
        &mpCubeTexture);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create cube environment texture map"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = pd3dDevice->CreateTexture(kResolution, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mpFresnelFunctionMap, NULL);
    if (FAILED(hr))
        return hr;
    hr = BuildPerPixelFresnelCurve(mRefractionIndex);
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < 4; ++i)
    {
        pd3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        pd3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        pd3dDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    }

	// Size
	float radius = m_DefaultSceneRadius;
	if (m_Scene)
		radius = m_Scene->m_Radius;

	// Center
	float center[3];
	memcpy(center, m_Scene ? m_Scene->m_Center : m_DefaultSceneCenter, 3 * sizeof(float));

	// Animation
	if (!m_Scene || m_Scene->m_NumMeshKeys == 0)
		m_PauseScene = true;
	if (!m_Scene || !m_Scene->m_NumCameras || (m_Scene->m_NumCameraKeys == 0))
		m_PauseCamera = true;
	if (!m_Scene || m_Scene->m_NumLights && (m_Scene->m_NumLightKeys == 0))
		m_PauseLight = true;

	// Light position and direction
	m_LightPosition.x = 0;
	m_LightPosition.y = 0.6f * radius;
	m_LightPosition.z = - 0.42f * radius;
	m_LightDirection = - m_LightPosition;
	D3DXVec3Normalize(&m_LightDirection, &m_LightDirection);    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Clean up here anything you created in RestoreDeviceObjects().
//       Called on exit, and also whenever you resize the window, switch 
//       windowed<->fullscreen, lose the device, etc.
//-----------------------------------------------------------------------------
HRESULT FresnelReflection::InvalidateDeviceObjects()
{
    SAFE_RELEASE(mpCubeTexture);
    SAFE_RELEASE(mpFresnelFunctionMap);
    SAFE_RELEASE(mpWorldBoxVertices);
    SAFE_RELEASE(mpWorldBoxIndices);
	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pSDeclaration);
    SAFE_RELEASE(m_pEffect);
	SAFE_DELETE(m_Scene);
	SAFE_DELETE(m_Light);
	SAFE_RELEASE(m_pSimpleTransformEffect);

    return S_OK;
}

HRESULT FresnelReflection::BuildPerPixelFresnelCurve(float refractionIndex)
{
    LPDIRECT3DTEXTURE9 pTexture;
    mpFresnelFunctionMap->QueryInterface(IID_IDirect3DTexture9, (void**)&pTexture);

    D3DLOCKED_RECT lockedRect;
    pTexture->LockRect(0, &lockedRect, NULL, 0);


    // this is a 1D texture, so we only need to loop in x (although we could 
    // encode a whole range of refraction indices into the map and possibly
    // vary it across a polygon...  Hmm.)
    float           rs, rp, result;
    float const     kEps = 10e-4f;
//    float const     kCritical = asinf(1.0f/refractionIndex);
    for (int x = 0; x < kResolution; ++x)
    {
        // we are pumping cos(angle) into tex coordinates, so  
        // need to first undo the cos() part before applying Fresnel formula.

        float const kCosAngle     = static_cast<float>(x)/static_cast<float const>(kResolution-1);
        float const kAngle        = acosf(kCosAngle);
        float const kRefractAngle = asinf(sinf(kAngle)/refractionIndex);

        if (kCosAngle <= kEps)
        {
            result  = (refractionIndex - 1.0f)/(refractionIndex + 1.0f);
            result *= result;
        }
        else 
        {
            rs     = sinf(kAngle - kRefractAngle)/sinf(kAngle + kRefractAngle);
            rp     = tanf(kAngle - kRefractAngle)/tanf(kAngle + kRefractAngle);
            result = (rs*rs + rp*rp)/2.0f;
        }

        result = 1 - result;

        DWORD const kOutput = static_cast<DWORD>(255.0f * result);
        *(((DWORD*)lockedRect.pBits) + x) = 0xFF000000 &  (kOutput << 24);
    }
    pTexture->UnlockRect(0);
    SAFE_RELEASE(pTexture);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT FresnelReflection::Render( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr = S_OK;
    // clear the screen
    pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0x0A, 0x0A, 0xAA ), 1.0, 0);

    pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    float const rzero = (float)pow((mRefractionIndex - 1.0f)/(mRefractionIndex + 1.0f), 2.0f);
    D3DXVECTOR4 fresnelVec(rzero, 100.0f, 1.0f, 1.0f);    // y comp is spec power, z comp is spec contribution, w is constant EdotN for when fresnel is off
    D3DXVECTOR4 modulationCol(1.0f, 1.0f, 1.0f, rzero);

    if (meApproximationOption == APPROX_REG_COMBINERS)
        modulationCol.x = modulationCol.y = modulationCol.z = modulationCol.w;

    D3DXVECTOR4 lightPos  (1.0f, 1.0f, -1.0f, 0.0f);    
    D3DXVECTOR4 lightColor(1.0f, 1.0f,  0.9f, 0.2f);    // alpha of light-color is ambient term!

    // draw the world cube first
    hr = DrawCube(pd3dDevice);

    pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    pd3dDevice->SetVertexDeclaration(m_pDeclaration);

    // Select pixel and vertex shader based on user choice.
    // But this choice may be overridden later based on per-mesh 
    // Material type.
    std::string sTechniqueName;
    switch (meApproximationOption)
    {
        case APPROX_VERTEX:
            sTechniqueName = "Fresnel";
            break;
        case APPROX_TEX_LOOKUP:
            sTechniqueName = "TexLookup";
            break;
        case APPROX_REG_COMBINERS:
            sTechniqueName = "RegCombiners";
            break;
        case APPROX_ONE_DOT3:
            sTechniqueName = "Dot3";
            break;
        case APPROX_NONE:
        default:
            sTechniqueName = "NoFresnel";
            break;
    }

    // set constants that won't change from object to object:
    m_pEffect->SetValue("ModulationColor", &modulationCol, sizeof(D3DXVECTOR4));
    m_pEffect->SetTexture("EnvironmentMap", mpCubeTexture);
    m_pEffect->SetTexture("FresnelFunc", mpFresnelFunctionMap);

    // write light cam-space position and color into vertex constant memory
    m_pEffect->SetValue("LightPos", &lightPos, sizeof(D3DXVECTOR3));
    m_pEffect->SetValue("LightColor", &lightColor, sizeof(D3DXVECTOR4));

    // pass through the list of objects twice...
    //   pass 0: draw all objects but the glass
    //   pass 1: draw the glass last (since it's transparent)
    for (int pass=0; pass<2; pass++)
    {
        // for each object in the scene...
        for (unsigned int i = 0; i < m_Scene->m_NumMeshes; ++i) 
        {
            const NVBScene::Mesh& mesh = m_Scene->m_Meshes[i];

            // don't draw the Glass on pass 0!
            if (mesh.m_Material == "Glass" && pass==0)
                continue;

            // don't draw other objects (besides glass) on pass 1!
            if (mesh.m_Material != "Glass" && pass==1)
                continue;

            std::string sActualTechnique = sTechniqueName;

            if (mesh.m_Material == "Glass")
            {
                fresnelVec.y        = 10.0f;
                fresnelVec.z        = 5.0f;
            }
            else if (mesh.m_Material == "metal" || 
                    mesh.m_Material == "BlueMetal" ||
                    mesh.m_Material == "Interior" ||
                    mesh.m_Material == "Dull" ||
                    mesh.m_Material == "Hose")
            {
                sActualTechnique = "NoMaterial";
            }
            else // presumably the car body:
            {
                fresnelVec.y        = 30.0f;
                fresnelVec.z        = 1.0f;
            }

            // Activate the appropriate HLSL technique, from the .fx file
            if (FAILED(hr = m_pEffect->SetTechnique(sActualTechnique.c_str())))
            {
                MessageBox(NULL, _T("Failed to set technique"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                return E_FAIL;
            }

            // set matrices & constants
            {
                D3DXMATRIX matWorldLocal = mesh.m_Transform * m_World;
                D3DXMATRIX matWorldViewLocal;
                D3DXMATRIX matWorldViewProjLocal;
                D3DXMATRIX matWorldViewITLocal;

                // Worldview transform (model -> world space)
                D3DXMatrixMultiply(&matWorldViewLocal,     &matWorldLocal,     &m_View);
                m_pEffect->SetMatrix("WorldView", &matWorldViewLocal);

                // Projection to clip space (model -> world -> eye space)
                D3DXMatrixMultiply(&matWorldViewProjLocal, &matWorldViewLocal, &m_Projection);
                m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProjLocal);

                // Worldview IT transform (model -> world -> eye space, but without translation; for transforming normals)
                D3DXMatrixInverse( &matWorldViewITLocal,    NULL,              &matWorldViewLocal);
                D3DXMatrixTranspose(&matWorldViewITLocal, &matWorldViewITLocal);
                m_pEffect->SetMatrix("WorldViewIT", &matWorldViewITLocal);

                // object diffuse texture
                m_pEffect->SetTexture("DiffuseMap", mesh.m_DiffuseMap);

                // compute R(0) and write to vertex constant memory
                m_pEffect->SetValue("FresnelConstants", &fresnelVec, sizeof(D3DXVECTOR4));
            }

            // render mesh using the technique
            UINT uPasses;
            if (D3D_OK == m_pEffect->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            {
                for (UINT uPass = 0; uPass < uPasses; uPass++) 
                {
                    m_pEffect->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                    if (FAILED(hr = mesh.Draw()))
                        return hr;
					m_pEffect->EndPass();
                }
                m_pEffect->End();
            }
        }

        if (pass==0)
        {
            // get ready to draw the glass in the next pass
            pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);      // double-sided
            pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);      // alpha blend on
            pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);   
            pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
        }
        else if (pass==1)
        {
            // done drawing glass -> reset state
            pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        }
    }

    // Draw normals, tangent basis, and/or coordinate axes, if enabled
    DrawAugmentations();      

    return S_OK;
}

HRESULT FresnelReflection::CreateCube(WorldBoxVertex* pVertices, WORD* pIndices)
{
    // Set up the vertices for the cube. Note: to prevent tiling problems,
    // the u/v coords are knocked slightly inwards.
    float const kEpsMult = 1.9999f;
    for (int ix = 0; ix < 2; ++ix)
        for (int iy = 0; iy < 2; ++iy)
            for (int iz = 0; iz < 2; ++iz)
            {
                float const x  = 2000.0f*(static_cast<float>(ix) - 0.5f);
                float const y  = 2000.0f*(static_cast<float>(iy) - 0.5f);
                float const z  = 2000.0f*(static_cast<float>(iz) - 0.5f);

                for (int iv = 0; iv < 3; ++iv)
                {
                    float const tx = ((iv == 0) ? 2.0f : kEpsMult) * (static_cast<float>(ix) - 0.5f);
                    float const ty = ((iv == 1) ? 2.0f : kEpsMult) * (static_cast<float>(iy) - 0.5f);
                    float const tz = ((iv == 2) ? 2.0f : kEpsMult) * (static_cast<float>(iz) - 0.5f);

                    *pVertices++ = WorldBoxVertex(D3DXVECTOR3(x, y, z), D3DXVECTOR3(tx, ty, tz));
                }
            }
    // Set up the indices for the cube
    // no offset in x-coords
    *pIndices++ =  0;     *pIndices++ =  6;     *pIndices++ =  3;
    *pIndices++ =  3;     *pIndices++ =  6;     *pIndices++ =  9;
    *pIndices++ = 12;     *pIndices++ = 15;     *pIndices++ = 21;
    *pIndices++ = 12;     *pIndices++ = 21;     *pIndices++ = 18;
    
    // no offset in y-coords
    *pIndices++ =  9+1;   *pIndices++ =  6+1;   *pIndices++ = 21+1;
    *pIndices++ =  6+1;   *pIndices++ = 18+1;   *pIndices++ = 21+1;
    *pIndices++ =  0+1;   *pIndices++ = 15+1;   *pIndices++ = 12+1;
    *pIndices++ =  0+1;   *pIndices++ =  3+1;   *pIndices++ = 15+1;
    
    // no offset in z-coords
    *pIndices++ =  3+2;   *pIndices++ =  9+2;   *pIndices++ = 15+2;
    *pIndices++ =  9+2;   *pIndices++ = 21+2;   *pIndices++ = 15+2;
    *pIndices++ =  0+2;   *pIndices++ = 18+2;   *pIndices++ =  6+2;
    *pIndices++ =  0+2;   *pIndices++ = 12+2;   *pIndices++ = 18+2;

    return S_OK;
}

HRESULT FresnelReflection::DrawCube( IDirect3DDevice9* pd3dDevice )
{
    HRESULT     hr = S_OK;

    hr = pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    hr = pd3dDevice->SetPixelShader(0);    
    hr = pd3dDevice->SetVertexShader(0);

    // note: setting FVF clobbers our current vertex declaration, so be sure 
    // that the caller re-establishes the vertex declaration on return!
    hr = pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) );

    hr = pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
    hr = pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
    hr = pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
    hr = pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);

    hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
    hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE | D3DTTFF_COUNT3 );    
    
    hr = pd3dDevice->SetStreamSource(0, mpWorldBoxVertices, 0, sizeof(WorldBoxVertex));
    hr = pd3dDevice->SetIndices(mpWorldBoxIndices);

    hr = pd3dDevice->SetTexture(0, mpCubeTexture);
    hr = pd3dDevice->SetTexture(1, NULL);


    D3DXMATRIX id, matProj;
    D3DXMatrixIdentity(&id);
    D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(90.0f), 1.0f, 0.1f, 2000.0f);
    pd3dDevice->SetTransform(D3DTS_VIEW, &id);
    pd3dDevice->SetTransform(D3DTS_WORLD, &id);
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj); 

    hr = pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);

    hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);    

    return hr;
}

/*******************************************************************************

Texture management

*******************************************************************************/

HRESULT FresnelReflection::LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DTEXTURE9& texture, D3DPOOL pool, D3DXIMAGE_INFO* info)
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

HRESULT FresnelReflection::LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DCUBETEXTURE9& texture, D3DXIMAGE_INFO* info)
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

void FresnelReflection::ReleaseTextures()
{
	SAFE_RELEASE(m_DiffuseMap);
	SAFE_RELEASE(m_NormalMap);
	SAFE_RELEASE(m_CubeMap);
}

HRESULT FresnelReflection::LoadTextures( IDirect3DDevice9* pd3dDevice )
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

HRESULT FresnelReflection::DrawAugmentations()
{
	if (m_Scene == 0)
		return S_OK;

	// Draw normals
	if (m_DrawNormals)
		m_Scene->DrawNormals(&m_Projection, &m_View, &m_World);

	// Draw tangent basis
	if (m_DrawTangentBasis)
		m_Scene->DrawTangentBasis(&m_Projection, &m_View, &m_World);

	// Draw tangent basis
	if (m_DrawAxis)
		m_Scene->DrawCoordinateAxis(&m_Projection, &m_View, &m_World);

	return S_OK;
}
FresnelReflection::~FresnelReflection()
{
	ReleaseTextures();
	SAFE_DELETE(m_Scene);
	SAFE_DELETE(m_Light);
	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pSimpleTransformEffect);
}
