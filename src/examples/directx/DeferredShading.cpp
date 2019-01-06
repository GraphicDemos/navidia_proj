#include "DeferredShading.h"

//constants
const tstring DeferredShading::m_DefaultSceneFilename(_T("adobe_mar13.NVB"));
const tstring DeferredShading::m_DefaultDiffuseMapFilename(_T("MEDIA/textures/2d/basetex.bmp"));
const tstring DeferredShading::m_DefaultNormalMapFilename(_T("MEDIA/textures/2d/wavy_normal.dds"));
const tstring DeferredShading::m_DefaultEnvironmentMapFilename(_T("MEDIA\\textures\\cubemaps\\NV-Metal-b.dds"));
const float	  DeferredShading::m_DefaultAmbient = 0.1f;

#define ID_QUIT              (USER_ID_START_VALUE + 0)


//-----------------------------------------------------------------------------
// Name: DeferredShading()
// Desc: Application constructor.   Paired with ~CDeferredShading()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
DeferredShading::DeferredShading() :
m_bLoadingApp(true),
m_bCanTonemap(true),
m_pEffect(NULL),
m_pDeclaration0(NULL),
m_pDeclaration1(NULL),
m_pBackBuffer(NULL),
m_pDepthBuffer(NULL),
m_pQuadVB(NULL),
m_pLightMesh(NULL),
m_bAnimateLights(false),
m_bDoScissor(true),
m_bDebugScissor(false),
m_currRenderMode(RM_NORMAL),
m_bTonemap(true),
m_fTonemapScale(0.22f),
m_bBlur(true),
m_bBlurFirst(false),
m_fDepthBias(0.0002f),
m_fBiasSlope(0.1f),
m_bShowHelp(false),
m_Scene(0),
m_DiffuseMap(NULL),
m_HeightMap(NULL),
m_NormalMap(NULL),
m_CubeMap(NULL),
m_Ambient(m_DefaultAmbient),
m_AnimTime(1),
m_FPS(1),
m_Wireframe(false),
m_UseDefaultDiffuseMap(false),
m_UseDefaultNormalMap(false)
{
}

//-----------------------------------------------------------------------------
// Name: ~CDeferredShading()
// Desc: Application destructor.  Paired with CDeferredShading()
//-----------------------------------------------------------------------------
DeferredShading::~DeferredShading()
{
}

//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT DeferredShading::OneTimeSceneInit(HWND m_hWnd)
{
	srand(0);

	// set these params here so that if user resizes window after they loaded
	// a new scene or changed other settings, they won't be reset:
	m_SceneFilename = m_DefaultSceneFilename;
	m_DiffuseMapFilename = m_DefaultDiffuseMapFilename;
	m_NormalMapFilename = m_DefaultNormalMapFilename;
	m_CubeMapFilename = m_DefaultEnvironmentMapFilename;

	m_PauseCamera = true;
	m_PauseLight  = true;

    // Drawing loading status message until app finishes loading
    SendMessage(m_hWnd, WM_PAINT, 0, 0);
 
    m_bLoadingApp = FALSE;

	//setup quad matrix to render a fullscreen quad
    D3DXMATRIX matWorld;
	D3DXMATRIX matView;
    D3DXMATRIX matProj;
    D3DXMATRIX matViewProj;
    
    D3DXVECTOR3 const vEyePt      = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 const vLookatPt   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 const   vUp       = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    // Set World, View, Projection, and combination matrices.
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
    D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);

    D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

    D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
    D3DXMatrixMultiply(&m_quadMat, &matWorld, &matViewProj);

	//
	m_matDefaultView._11 = -0.987f;
	m_matDefaultView._12 = -0.047f;
	m_matDefaultView._13 = -0.153f;
	m_matDefaultView._14 =  0.000f;
	m_matDefaultView._21 =  0.003f;
	m_matDefaultView._22 =  0.950f;
	m_matDefaultView._23 = -0.312f;
	m_matDefaultView._24 =  0.000f;
	m_matDefaultView._31 =  0.160f;
	m_matDefaultView._32 = -0.308f;
	m_matDefaultView._33 = -0.938f;
	m_matDefaultView._34 =  0.000f;
	m_matDefaultView._41 =  1.042f;
	m_matDefaultView._42 = 51.501f;
	m_matDefaultView._43 =  9.156f;
	m_matDefaultView._44 =  1.000f;
	//
	
	//
	m_matAlternateView._11 =  0.980f;
	m_matAlternateView._12 =  0.044f;
	m_matAlternateView._13 = -0.192f;
	m_matAlternateView._14 =  0.000f;
	m_matAlternateView._21 = -0.075f;
	m_matAlternateView._22 =  0.984f;
	m_matAlternateView._23 = -0.159f;
	m_matAlternateView._24 =  0.000f;
	m_matAlternateView._31 =  0.182f;
	m_matAlternateView._32 =  0.170f;
	m_matAlternateView._33 =  0.969f;
	m_matAlternateView._34 =  0.000f;
	m_matAlternateView._41 = -5.186f;
	m_matAlternateView._42 = 55.511f;
	m_matAlternateView._43 = -4.751f;
	m_matAlternateView._44 =  1.000f;
	//

    return S_OK;
}

bool DeferredShading::CanTonemap(LPDIRECT3D9 m_pD3D, D3DDEVTYPE devType)
{
	HRESULT hr = D3D_OK;
	hr = m_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, devType, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
			D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F);

	return (hr == D3D_OK);
}

void DeferredShading::InitializeLights()
{
	m_Lights.clear();

    //first light
	LightType currLight;
    ZeroMemory(&currLight, sizeof(LightType));

	currLight.Type      = D3DLIGHT_DIRECTIONAL;
    currLight.Color     = D3DXVECTOR3(12.8f, 12.8f, 12.1f);
	currLight.Direction = D3DXVECTOR3(0.7f, 1.0f, 0.5f);
	m_Lights.push_back(currLight);

	//this is the light we use for shadowing
	D3DXVECTOR3 lightPos;
	lightPos.x = m_Scene->m_Center.x;// + (m_SceneMgr.m_Scene->m_Radius / 4.0f) + 40.0f;
	lightPos.y = m_Scene->m_Center.y - 100.0f;// + (m_SceneMgr.m_Scene->m_Radius / 4.0f) - 10.0f;
	lightPos.z = m_Scene->m_Center.z;// + (m_SceneMgr.m_Scene->m_Radius / 4.0f) - 60.0f;

	//setup matrices for shadowmap
    D3DXVECTOR3 eye, lookAt, up;
    
    lookAt.x = lightPos.x - (m_Lights[0].Direction.x); 
    lookAt.y = lightPos.y - (m_Lights[0].Direction.y);
    lookAt.z = lightPos.z - (m_Lights[0].Direction.z);
    
    up.x     = 0.0f;          up.y     = 1.0f;          up.z     = 0.0f;
    
    D3DXMATRIX lightView, lightProj;
    D3DXMatrixLookAtLH(&lightView, &lightPos, &lookAt, &up);
	D3DXMatrixOrthoLH(&lightProj, 35, 35, 2.0f, 1000.0f);

    m_lightMat = lightView * lightProj;

    ZeroMemory(&currLight, sizeof(LightType));
    currLight.Type      = D3DLIGHT_POINT;
    currLight.Color     = D3DXVECTOR3(1.8f, 1.7f, 1.8f);
    currLight.Position  = D3DXVECTOR3(1.31f, -55.96f, -0.85f);
	currLight.Range     = 4.0f;
	m_Lights.push_back(currLight);
}

void DeferredShading::AnimateLights()
{
    D3DXMATRIX transformMat;
    D3DXMatrixRotationY(&transformMat, D3DXToRadian(1.0f));
    D3DXVECTOR4 tempVec;
    D3DXVec3Transform(&tempVec, &m_Lights[1].Position, &transformMat);
    m_Lights[1].Position.x = tempVec.x;
    m_Lights[1].Position.y = tempVec.y;
    m_Lights[1].Position.z = tempVec.z;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the display device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT DeferredShading::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior,
                                         D3DFORMAT Format,D3DFORMAT Format2)
{
    UNREFERENCED_PARAMETER(dwBehavior);
    UNREFERENCED_PARAMETER(Format);

    bool bCapsAcceptable = true;
 
	//surface support
	//bCapsAcceptable = CanTonemap(pCaps->DeviceType);

	if((pCaps->VertexShaderVersion < D3DVS_VERSION(2,0)))
	{
		bCapsAcceptable = false;
	}

	if((pCaps->PixelShaderVersion < D3DPS_VERSION(2,0)))
	{
		bCapsAcceptable = false;
	}

	//need MRT support
	if (pCaps->NumSimultaneousRTs < 4)
	{
		bCapsAcceptable = false;
	}

	return (bCapsAcceptable ? S_OK : E_FAIL);
}

HRESULT DeferredShading::CreateQuad( IDirect3DDevice9* pd3dDevice, IDirect3DVertexBuffer9* &quadVB, unsigned int width, unsigned int height)
{
	HRESULT hr = S_OK;

    // create vertex buffer 
    hr = pd3dDevice->CreateVertexBuffer( 4 * sizeof(QuadVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &quadVB, NULL);
    if (FAILED(hr))
        return hr;

    QuadVertex *pBuff;

    // account for DirectX's texel center standard:
    float u_adjust = 0.5f / width;
    float v_adjust = 0.5f / height;

    if (quadVB)
    {
        hr = quadVB->Lock(0, 0,(void**)&pBuff, 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Couldn't lock quad buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }

        for (int i = 0; i < 4; ++i)
        {
            pBuff->Position = D3DXVECTOR3((i==0 || i==1) ? -1.0f : 1.0f,
                                           (i==0 || i==3) ? -1.0f : 1.0f,
                                          0.0f);
            pBuff->Texture  = D3DXVECTOR2(((i==0 || i==1) ? 0.0f : 1.0f) + u_adjust, 
                                           ((i==0 || i==3) ? 1.0f : 0.0f) + v_adjust);
            pBuff++; 
        }
        quadVB->Unlock();
    }

	return hr;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT DeferredShading::RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;

	assert(pd3dDevice);

	if(FAILED(CheckResourceFormatSupport(pd3dDevice, D3DFMT_D24S8, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL)))
    {
        MessageBox(NULL, _T("Device/driver does not support hardware shadow maps."), _T("Message"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        m_bSupportsHWShadowMaps = false;
    }
    else
		m_bSupportsHWShadowMaps = true;

	//grab the primary rendertarget
	if (hr = FAILED(pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer)))
		return hr;

	if (hr = FAILED(pd3dDevice->GetDepthStencilSurface(&m_pDepthBuffer)))
		return hr;

    //
    //create MRT textures
    //

    //albedo
	if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_MRTs[0].tex, NULL)))
		return hr;

    //normal
	if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_MRTs[1].tex, NULL)))
		return hr;

    //depth
    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_MRTs[2].tex, NULL)))
        return hr;

    for (int i = 0; i < NUM_MRT; ++i)
    {
		if (FAILED(hr = (m_MRTs[i].tex)->GetSurfaceLevel(0, &m_MRTs[i].surf)))
		    return hr;
    }

    //
    //create light transport buffers
    //
    D3DFORMAT lightFormat = D3DFMT_A8R8G8B8;
    if (m_bCanTonemap)
        lightFormat = D3DFMT_A16B16G16R16F;

    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_LTSurfaces.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_LTSurfaces.tex)->GetSurfaceLevel(0, &m_LTSurfaces.surf)))
        return hr;

    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_LTSurfacesFinal.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_LTSurfacesFinal.tex)->GetSurfaceLevel(0, &m_LTSurfacesFinal.surf)))
        return hr;

    //
    //create luminance / log luminance buffers
    //
    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth, m_iBBHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_G16R16F, D3DPOOL_DEFAULT, &m_LuminanceSurfaces.tex, NULL)))
        return hr;
    if (FAILED(hr = (m_LuminanceSurfaces.tex)->GetSurfaceLevel(0, &m_LuminanceSurfaces.surf)))
        return hr;

    //
    //create shadow map
    //
	if( m_bSupportsHWShadowMaps )
	{
		//dummy texture, because you can't set the render target to NULL
		if (FAILED(hr = pd3dDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_ShadowMapDummy.tex, NULL)))
			return hr;
		if (FAILED(hr = (m_ShadowMapDummy.tex)->GetSurfaceLevel(0, &m_ShadowMapDummy.surf)))
			return hr;

		if (FAILED(hr = pd3dDevice->CreateTexture(512, 512, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24X8, D3DPOOL_DEFAULT, &m_ShadowMap.tex, NULL)))
			return hr;
	}
	else
	{
		if (FAILED(hr = pd3dDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_ShadowMap.tex, NULL)))
			return hr;
	}

	if (FAILED(hr = (m_ShadowMap.tex)->GetSurfaceLevel(0, &m_ShadowMap.surf)))
        return hr;

	if (m_bCanTonemap)
	{
		unsigned int numMipLevelsH = static_cast<int>((logf(static_cast<float>(m_iBBHeight)) / logf(2.0f))) + 1;
		unsigned int numMipLevelsW = static_cast<int>((logf(static_cast<float>(m_iBBWidth)) / logf(2.0f))) + 1;
		unsigned int numMipLevels = numMipLevelsH > numMipLevelsW ? numMipLevelsH : numMipLevelsW;

		m_TonemapSurfaces.resize(numMipLevels - 1);
		m_QuadVec.resize(numMipLevels - 1);

		//we create rendertargets to the 1x1 level, but we don't create one for the top level
		for (unsigned int i = 0; i < numMipLevels - 1; ++i)
		{
			unsigned int mipHeight = m_iBBHeight >> (i + 1);
			mipHeight = (mipHeight == 0) ? 1 : mipHeight;
			unsigned int mipWidth = m_iBBWidth >> (i + 1);
			mipWidth = (mipWidth == 0) ? 1 : mipWidth;

			if (FAILED(hr = pd3dDevice->CreateTexture(mipWidth, mipHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, 
				&m_TonemapSurfaces[i].tex, NULL)))
				return hr;

			if (FAILED(hr = (m_TonemapSurfaces[i].tex)->GetSurfaceLevel(0, &m_TonemapSurfaces[i].surf)))
			    return hr;

			if (FAILED(hr = CreateQuad(pd3dDevice, m_QuadVec[i], mipWidth, mipHeight)))
				return hr;
		}
		
	}


	//
    //create blur surfaces
    //
    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth/2, m_iBBHeight/2, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_BlurSurfaces1.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_BlurSurfaces1.tex)->GetSurfaceLevel(0, &m_BlurSurfaces1.surf)))
        return hr;

    if (FAILED(hr = pd3dDevice->CreateTexture(m_iBBWidth/2, m_iBBHeight/2, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_BlurSurfaces2.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_BlurSurfaces2.tex)->GetSurfaceLevel(0, &m_BlurSurfaces2.surf)))
        return hr;

	//
	//final luminance surfaces, for adaptation effect
	//
    if (FAILED(hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_LastLuminance.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_LastLuminance.tex)->GetSurfaceLevel(0, &m_LastLuminance.surf)))
        return hr;

    if (FAILED(hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, lightFormat, D3DPOOL_DEFAULT, &m_CurrLuminance.tex, NULL)))
        return hr;
	if (FAILED(hr = (m_CurrLuminance.tex)->GetSurfaceLevel(0, &m_CurrLuminance.surf)))
        return hr;

	//init both to black
	pd3dDevice->ColorFill(m_LastLuminance.surf, NULL, 0);
	pd3dDevice->ColorFill(m_CurrLuminance.surf, NULL, 0);

	if (FAILED(hr = InitializeScene(pd3dDevice)))
		return hr;

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

	if (FAILED(hr = CreateQuad(pd3dDevice, m_pQuadVB, m_iBBWidth, m_iBBHeight)))
		return hr;

    InitializeLights();

    //create light geometry
    if (FAILED(hr = D3DXCreateSphere(pd3dDevice, 0.12f, 18, 18, &m_pLightMesh, NULL)))
        return hr;

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT DeferredShading::Render(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr = S_OK;

	// Update time
	/*
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
	
    m_AnimTime += 30.0f / m_FPS;
*/
	/**/
	m_World._11 = 1.00000000f;
	m_World._12 = 0.00000000f;
	m_World._13 = 0.00000000f;
	m_World._14 = 0.00000000f;
	m_World._21 = 0.00000000f;
	m_World._22 = 1.00000000f;
	m_World._23 = 0.00000000f;
	m_World._24 = 0.00000000f;
	m_World._31 = 0.00000000f;
	m_World._32 = 0.00000000f;
	m_World._33 = 1.00000000f;
	m_World._34 = 0.00000000f;
	m_World._41 = 0.18972015f;
	m_World._42 = -57.538067f;
	m_World._43 = -0.64709091f;
	m_World._44 = 1.00000000f;
	/**
	m_View._11 = -0.98699999;
	m_View._12 = -0.046999998;
	m_View._13 = -0.15300000;
	m_View._14 = 0.00000000;
	m_View._21 = 0.0030000000;
	m_View._22 = 0.94999999;
	m_View._23 = -0.31200001;
	m_View._24 = 0.00000000;
	m_View._31 = 0.16000000;
	m_View._32 = -0.30800000;
	m_View._33 = -0.93800002;
	m_View._34 = 0.00000000;
	m_View._41 = 1.0420001;
	m_View._42 = 51.500999;
	m_View._43 = 9.1560001;
	m_View._44 = 1.0000000;
	/**
	m_Projection._11 = 1.7320508;
	m_Projection._12 = 0.00000000;
	m_Projection._13 = 0.00000000;
	m_Projection._14 = 0.00000000;
	m_Projection._21 = 0.00000000;
	m_Projection._22 = 1.7320508;
	m_Projection._23 = 0.00000000;
	m_Projection._24 = 0.00000000;
	m_Projection._31 = 0.00000000;
	m_Projection._32 = 0.00000000;
	m_Projection._33 = 1.0011734;
	m_Projection._34 = 1.0000000;
	m_Projection._41 = 0.00000000;
	m_Projection._42 = 0.00000000;
	m_Projection._43 = -1.5017601;
	m_Projection._44 = 0.00000000;
	/**/

	if (m_Scene)
		m_Scene->Update(m_AnimTime, &m_World, (m_PauseScene ? 0 : NVBScene::MESH) | (m_PauseCamera ? 0 : NVBScene::CAMERA) | (m_PauseLight ? 0 : NVBScene::LIGHT));

	/**/
    if (m_bAnimateLights)
        AnimateLights();

	//
	//first, let's render to the shadow map
	//
    if (FAILED(hr = DrawSceneToShadowMap(pd3dDevice))) 
	{
        MessageBox(NULL, _T("Failed to draw the scene to shadow map"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

	//
	//next, we render to the MRT buffers
	//
    if (FAILED(hr = DrawSceneToMRT(pd3dDevice))) 
	{
        MessageBox(NULL, _T("Failed to draw the scene to MRT"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

	//
    //Now we render light transport to a buffer
	//
    if (FAILED(hr = DrawLightTransport(pd3dDevice))) 
    {
        MessageBox(NULL, _T("Failed to draw light transport"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

	//
	//Blur light transport surfaces if required
	//
	if (m_bBlur)
	{
		BlurSurface(pd3dDevice, m_LTSurfaces.tex);
	}

	//
    //draw to final scene
	//
	if (FAILED(hr = DrawFinalScene(pd3dDevice))) 
	{
        MessageBox(NULL, _T("Failed to draw final scene to fb"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    return S_OK;
}


//blurred result ends up in m_BlurSurface1
HRESULT DeferredShading::BlurSurface(IDirect3DDevice9* pd3dDevice,IDirect3DTexture9* sourceTex)
{
	HRESULT hr;

	//first, simply 2x2 decimate, and threshold
	pd3dDevice->SetRenderTarget(0, m_BlurSurfaces1.surf);
	pd3dDevice->SetDepthStencilSurface(NULL);

	if (FAILED(hr = m_pEffect->SetTechnique("FilteredThreshold")))
	{
		return hr;
	}

	UINT uPasses;
	if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++) 
		{
			// Set the state for a particular pass in a technique.
			m_pEffect->BeginPass(uPass);

			m_pEffect->SetTexture("LightTransport", sourceTex);

			m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

			pd3dDevice->SetFVF(NULL);
			pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

			pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

			m_pEffect->CommitChanges();
			if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
				return hr;

			m_pEffect->EndPass();
		}
		m_pEffect->End();
	}

	//blur horizontal
	pd3dDevice->SetRenderTarget(0, m_BlurSurfaces2.surf);
	if (FAILED(hr = m_pEffect->SetTechnique("Blur")))
	{
		return hr;
	}

	if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++)
		{
			// Set the state for a particular pass in a technique.
			m_pEffect->BeginPass(uPass);

			m_pEffect->SetTexture("BlurTexture", m_BlurSurfaces1.tex);

			m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

			int width = m_iBBWidth / 2;

			D3DXVECTOR2 blurOffset;
			blurOffset.y = 0.0f;

			//blurOffset.x = -3.5f / width;
			blurOffset.x = -4.0f / width;
			m_pEffect->SetValue("BlurOffset[0]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = -2.5f / width;
			blurOffset.x = -3.0f / width;
			m_pEffect->SetValue("BlurOffset[1]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = -1.5f / width;
			blurOffset.x = -2.0f / width;
			m_pEffect->SetValue("BlurOffset[2]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = -0.5f / width;
			blurOffset.x = -1.0f / width;
			m_pEffect->SetValue("BlurOffset[3]", &blurOffset, sizeof(D3DXVECTOR2));

			//blurOffset.x = 0.5f / width;
			blurOffset.x = 1.0f / width;
			m_pEffect->SetValue("BlurOffset[4]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = 1.5f / width;
			blurOffset.x = 2.0f / width;
			m_pEffect->SetValue("BlurOffset[5]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = 2.5f / width;
			blurOffset.x = 3.0f / width;
			m_pEffect->SetValue("BlurOffset[6]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.x = 3.5f / width;
			blurOffset.x = 4.0f / width;
			m_pEffect->SetValue("BlurOffset[7]", &blurOffset, sizeof(D3DXVECTOR2));

			pd3dDevice->SetFVF(NULL);
			pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

			pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

			m_pEffect->CommitChanges();
			if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
				return hr;

			m_pEffect->EndPass();
		}
		m_pEffect->End();
	}

	//blur vertical
	pd3dDevice->SetRenderTarget(0, m_BlurSurfaces1.surf);
	if (FAILED(hr = m_pEffect->SetTechnique("Blur")))
	{
		return hr;
	}

	if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++) 
		{
			// Set the state for a particular pass in a technique.
			m_pEffect->BeginPass(uPass);

			m_pEffect->SetTexture("BlurTexture", m_BlurSurfaces2.tex);

			m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

			int height = m_iBBHeight / 2;
			
			D3DXVECTOR2 blurOffset;
			blurOffset.x = 0.0f;

			//blurOffset.y = -3.5f / height;
			blurOffset.y = -4.0f / height;
			m_pEffect->SetValue("BlurOffset[0]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = -2.5f / height;
			blurOffset.y = -3.0f / height;
			m_pEffect->SetValue("BlurOffset[1]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = -1.5f / height;
			blurOffset.y = -2.0f / height;
			m_pEffect->SetValue("BlurOffset[2]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = -0.5f / height;
			blurOffset.y = -1.0f / height;
			m_pEffect->SetValue("BlurOffset[3]", &blurOffset, sizeof(D3DXVECTOR2));

			//blurOffset.y = 0.5f / height;
			blurOffset.y = 1.0f / height;
			m_pEffect->SetValue("BlurOffset[4]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = 1.5f / height;
			blurOffset.y = 2.0f / height;
			m_pEffect->SetValue("BlurOffset[5]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = 2.5f / height;
			blurOffset.y = 3.0f / height;
			m_pEffect->SetValue("BlurOffset[6]", &blurOffset, sizeof(D3DXVECTOR2));
			//blurOffset.y = 3.5f / height;
			blurOffset.y = 4.0f / height;
			m_pEffect->SetValue("BlurOffset[7]", &blurOffset, sizeof(D3DXVECTOR2));

			pd3dDevice->SetFVF(NULL);
			pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

			pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

			m_pEffect->CommitChanges();
			if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
				return hr;

			m_pEffect->EndPass();
		}
		m_pEffect->End();
	}
	
	return S_OK;
}

RECT DeferredShading::DetermineClipRect(const D3DXVECTOR3& position, const float range)
{
	//compute 3D bounding box of light in world space
	D3DXVECTOR3 bbox3D[8];
	bbox3D[0].x = position.x - range;  bbox3D[0].y = position.y + range;  bbox3D[0].z = position.z - range;
	bbox3D[1].x = position.x + range;  bbox3D[1].y = position.y + range;  bbox3D[1].z = position.z - range;
	bbox3D[2].x = position.x - range;  bbox3D[2].y = position.y - range;  bbox3D[2].z = position.z - range;
	bbox3D[3].x = position.x + range;  bbox3D[3].y = position.y - range;  bbox3D[3].z = position.z - range;
	bbox3D[4].x = position.x - range;  bbox3D[4].y = position.y + range;  bbox3D[4].z = position.z + range;
	bbox3D[5].x = position.x + range;  bbox3D[5].y = position.y + range;  bbox3D[5].z = position.z + range;
	bbox3D[6].x = position.x - range;  bbox3D[6].y = position.y - range;  bbox3D[6].z = position.z + range;
	bbox3D[7].x = position.x + range;  bbox3D[7].y = position.y - range;  bbox3D[7].z = position.z + range;

	//project coordinates
	D3DXMATRIX viewProjMat = m_View * m_Projection;
	D3DXVECTOR2 projBox[8];
	for (int i = 0; i < 8; ++i)
	{
		D3DXVECTOR4 projPoint;
		D3DXVec3Transform(&projPoint, &bbox3D[i], &viewProjMat);
		projBox[i].x = projPoint.x / projPoint.w;  
		projBox[i].y = projPoint.y / projPoint.w;

		//clip to extents
		if (projBox[i].x < -1.0f)
			projBox[i].x = -1.0f;
		else if (projBox[i].x > 1.0f)
			projBox[i].x = 1.0f;
		if (projBox[i].y < -1.0f)
			projBox[i].y = -1.0f;
		else if (projBox[i].y > 1.0f)
			projBox[i].y = 1.0f;

		//go to pixel coordinates
		projBox[i].x = ((projBox[i].x + 1.0f) / 2.0f) * m_iBBWidth;
		projBox[i].y = ((-projBox[i].y + 1.0f) / 2.0f) * m_iBBHeight;
	}

	//compute 2D bounding box of projected coordinates
	unsigned int minX = 0xFFFFFFFF;
	unsigned int maxX = 0x00000000;
	unsigned int minY = 0xFFFFFFFF;
	unsigned int maxY = 0x00000000;
	for (int i = 0; i < 8; ++i)
	{
		unsigned int x = static_cast<unsigned int>(projBox[i].x);
		unsigned int y = static_cast<unsigned int>(projBox[i].y);
		if (x < minX)
			minX = x;
		if (x > maxX)
			maxX = x;
		if (y < minY)
			minY = y;
		if (y > maxY)
			maxY = y;
	}
	RECT bbox2D;
	bbox2D.top    = minY;
	bbox2D.bottom = maxY;
	bbox2D.left   = minX;
	bbox2D.right  = maxX;

	return bbox2D;
}


HRESULT DeferredShading::DrawFinalScene(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;

	pd3dDevice->SetRenderTarget(0, m_pBackBuffer);
	pd3dDevice->SetDepthStencilSurface(NULL);

	//apply blur, if applicable
	if (m_bBlur && m_bBlurFirst)
	{
		pd3dDevice->SetRenderTarget(0, m_LTSurfacesFinal.surf);

		pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		if (FAILED(hr = m_pEffect->SetTechnique("AddBlur")))
		{
			return hr;
		}
		UINT uPasses;
		if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
		{  
			for (UINT uPass = 0; uPass < uPasses; uPass++) 
			{
				// Set the state for a particular pass in a technique.
				m_pEffect->BeginPass(uPass);

				m_pEffect->SetTexture("LightTransport", m_LTSurfaces.tex);
				m_pEffect->SetTexture("BlurTexture", m_BlurSurfaces1.tex);

				m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

				pd3dDevice->SetFVF(NULL);
				pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

				pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

				m_pEffect->CommitChanges();
				if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
					return hr;
				m_pEffect->EndPass();
			}
			m_pEffect->End();
		}
	}

    if (m_bCanTonemap)
    {
        //
        //convert to luminance, log luminance
		//
        pd3dDevice->SetRenderTarget(0, m_LuminanceSurfaces.surf);

        pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

        if (FAILED(hr = m_pEffect->SetTechnique("ConvertToLuminance")))
        {
            return hr;
        }
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
        {  
            for (UINT uPass = 0; uPass < uPasses; uPass++) 
            {
                // Set the state for a particular pass in a technique.
                m_pEffect->BeginPass(uPass);

				if (m_bBlur && m_bBlurFirst)
					m_pEffect->SetTexture("LightTransport", m_LTSurfacesFinal.tex);
				else
					m_pEffect->SetTexture("LightTransport", m_LTSurfaces.tex);

                m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

				m_pEffect->CommitChanges();

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

                pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));
                if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
                    return hr;
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }

        //
        //create filtered surface representing average luminance (or log luminance)
		//
        if (FAILED(hr = m_pEffect->SetTechnique("PassThroughFiltered")))
        {
            return hr;
        }

		if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
		{  
			for (UINT uPass = 0; uPass < uPasses; uPass++) 
			{
				// Set the state for a particular pass in a technique.
				m_pEffect->BeginPass(uPass);

				for (unsigned int i = 0; i < m_TonemapSurfaces.size(); ++i)
				{
					pd3dDevice->SetRenderTarget(0, m_TonemapSurfaces[i].surf);

					pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

					if (i==0)
						m_pEffect->SetTexture("LightTransport", m_LuminanceSurfaces.tex);
					else
						m_pEffect->SetTexture("LightTransport", m_TonemapSurfaces[i - 1].tex);

					m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

					pd3dDevice->SetFVF(NULL);
					pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

					pd3dDevice->SetStreamSource(0, m_QuadVec[i], 0, sizeof(QuadVertex));

					m_pEffect->CommitChanges();
					if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
						return hr;
				}
				m_pEffect->EndPass();
			}
			m_pEffect->End();
		}

		//combine this frame's luminance with previous frame's
		
		//swap 'em
		PDIRECT3DTEXTURE9 tempTex = m_LastLuminance.tex;
		PDIRECT3DSURFACE9 tempSurf = m_LastLuminance.surf;
		m_LastLuminance = m_CurrLuminance;
		m_CurrLuminance.tex = tempTex;
		m_CurrLuminance.surf = tempSurf;

		pd3dDevice->SetRenderTarget(0, m_CurrLuminance.surf);

		if (FAILED(hr = m_pEffect->SetTechnique("Adaptation")))
		{
			return hr;
		}

		if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
		{  
			for (UINT uPass = 0; uPass < uPasses; uPass++) 
			{
				// Set the state for a particular pass in a technique.
				m_pEffect->BeginPass(uPass);

				m_pEffect->SetTexture("CurrentLum", m_TonemapSurfaces[m_TonemapSurfaces.size() - 1].tex);
				m_pEffect->SetTexture("PreviousLum", m_LastLuminance.tex);

				m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

				pd3dDevice->SetFVF(NULL);
				pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

				pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

				m_pEffect->CommitChanges();
				if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
					return hr;
				m_pEffect->EndPass();
			}
			m_pEffect->End();
		}


		//now render to final fb using filtered value for tonemapping
		if (m_bTonemap)
		{
			pd3dDevice->SetRenderTarget(0, m_pBackBuffer);
            pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
        
            if (FAILED(hr = m_pEffect->SetTechnique("Tonemap")))
			{
				return hr;
			}

			if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
			{  
				for (UINT uPass = 0; uPass < uPasses; uPass++) 
				{
					// Set the state for a particular pass in a technique.
					m_pEffect->BeginPass(uPass);

					if (m_bBlur && m_bBlurFirst)
						m_pEffect->SetTexture("LightTransport", m_LTSurfacesFinal.tex);
					else
						m_pEffect->SetTexture("LightTransport", m_LTSurfaces.tex);

					m_pEffect->SetTexture("AvgLuminance", m_CurrLuminance.tex);
					m_pEffect->SetTexture("Luminance", m_LuminanceSurfaces.tex);

					m_pEffect->SetValue("TonemapScale", &m_fTonemapScale, sizeof(float));

					if (m_bBlurFirst)
						m_pEffect->SetTexture("BlurTexture", NULL);
					else
						m_pEffect->SetTexture("BlurTexture", m_BlurSurfaces1.tex);

					m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

					pd3dDevice->SetFVF(NULL);
					pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

					pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

					m_pEffect->CommitChanges();
					if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
						return hr;
					m_pEffect->EndPass();
				}
				m_pEffect->End();
			}
		}
		else
		{
			pd3dDevice->SetRenderTarget(0, m_pBackBuffer);
            pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

			if (FAILED(hr = m_pEffect->SetTechnique("PassThrough")))
			{
				return hr;
			}

			if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
			{  
				for (UINT uPass = 0; uPass < uPasses; uPass++) 
				{
					// Set the state for a particular pass in a technique.
					m_pEffect->BeginPass(uPass);
					if (m_bBlur && m_bBlurFirst)
						m_pEffect->SetTexture("LightTransport", m_LTSurfacesFinal.tex);
					else
						m_pEffect->SetTexture("LightTransport", m_LTSurfaces.tex);
					
					m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

					pd3dDevice->SetFVF(NULL);
					pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

					pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));
					m_pEffect->CommitChanges();
					if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
						return hr;
					m_pEffect->EndPass();
				}
				m_pEffect->End();
			}
		}
    }
    else
    {
        if (FAILED(hr = m_pEffect->SetTechnique("PassThrough")))
        {
            return hr;
        }

        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
        {  
            for (UINT uPass = 0; uPass < uPasses; uPass++) 
            {
                // Set the state for a particular pass in a technique.
                m_pEffect->BeginPass(uPass);

				if (m_bBlur && m_bBlurFirst)
					m_pEffect->SetTexture("LightTransport", m_LTSurfacesFinal.tex);
				else
					m_pEffect->SetTexture("LightTransport", m_LTSurfaces.tex);

                m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

                pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

				m_pEffect->CommitChanges();
                if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
                    return hr;
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }

    return S_OK;
}


HRESULT DeferredShading::DrawLightTransport(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;

	pd3dDevice->SetRenderTarget(0, m_LTSurfaces.surf);

	//make sure no mrts are set
	for (int i = 1; i < NUM_MRT; ++i)
		pd3dDevice->SetRenderTarget(i, NULL);

	pd3dDevice->SetDepthStencilSurface(NULL);

	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    // select a technique, from our .fx file, to do either Diffuse lighting only, or Diffuse + Specular.
    float ambient = 0.1f;
    m_pEffect->SetValue("Ambient", &ambient, sizeof(float));

    if (m_Lights.size() > 1)
    {
        //we need to add into fb
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }

    RECT defaultClipRect;
    defaultClipRect.top    = 0;
    defaultClipRect.bottom = m_iBBHeight;
    defaultClipRect.right  = m_iBBWidth;
    defaultClipRect.left   = 0;

    if (m_bDoScissor)
        pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    else
        pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    // Draw
    for (unsigned int i = 0; i < m_Lights.size(); ++i)
    {
        pd3dDevice->SetScissorRect(&defaultClipRect);

        std::string sTechniqueName;
        if (m_Lights[i].Type == D3DLIGHT_DIRECTIONAL)
        {
            sTechniqueName = "DirLighting";
            D3DXVECTOR3 lightInViewSpace;
            D3DXVec3TransformNormal(&lightInViewSpace, &m_Lights[i].Direction, &m_View);
            D3DXVec3Normalize(&lightInViewSpace, &lightInViewSpace);
            m_pEffect->SetValue("LightVector", &lightInViewSpace, sizeof(D3DXVECTOR3));
        }
        else if (m_Lights[i].Type == D3DLIGHT_POINT)
        {
            sTechniqueName = "PointLighting";
            D3DXVECTOR4 lightInViewSpace;
            D3DXVec3Transform(&lightInViewSpace, &m_Lights[i].Position, &m_View);
            D3DXVECTOR3 tempVec;
            tempVec.x = lightInViewSpace.x;
            tempVec.y = lightInViewSpace.y;
            tempVec.z = lightInViewSpace.z;
            m_pEffect->SetValue("LightPosition", &tempVec, sizeof(D3DXVECTOR3));
            m_pEffect->SetValue("LightRange", &m_Lights[i].Range, sizeof(float));

            RECT rect = DetermineClipRect(m_Lights[i].Position, m_Lights[i].Range);
            pd3dDevice->SetScissorRect(&rect);
        }

        m_pEffect->SetValue("LightColor", &m_Lights[i].Color, sizeof(D3DXVECTOR3));

        if (m_bDebugScissor)
            sTechniqueName = "DebugScissor";

        if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
        {
            char buf[2048];
            sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
			WCHAR lpszW[2048];
			MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

			MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			

            return E_FAIL;
        }

        float specExp = 30.0f;
        m_pEffect->SetValue("SpecularExponent", &specExp, sizeof(float));

        if (m_currRenderMode != RM_NORMAL)
        {
            switch(m_currRenderMode)
            {
                case RM_CHANNEL0:
                    sTechniqueName = "ShowChannel0";
                    break;
                case RM_CHANNEL1:
                    sTechniqueName = "ShowChannel1";
                    break;
                case RM_CHANNEL2:
                    sTechniqueName = "ShowChannel2";
                    break;
            }

            if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
            {
                char buf[2048];
                sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
				WCHAR lpszW[2048];
				MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

				MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
				
				return E_FAIL;
            }
            
            pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }

        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
        {  
            for (UINT uPass = 0; uPass < uPasses; uPass++) 
            {
                // Set the state for a particular pass in a technique.
                m_pEffect->BeginPass(uPass);

				for (int i = 0; i < NUM_MRT; ++i)
				{
					char buf[8];
					sprintf(buf, "MRT%d", i);
					m_pEffect->SetTexture(buf, m_MRTs[i].tex);
				}

                m_pEffect->SetMatrix("WorldViewProj", &m_quadMat);
                m_pEffect->SetMatrix("InvProj", &m_matInvProj);
				m_pEffect->SetMatrix("InvViewProj", &m_matInvViewProj);
				m_pEffect->SetMatrix("InvView", &m_matInvView);

				//set special texture matrix for shadow mapping
				float fOffsetX = 0.5f + (0.5f / 512.0f);
				float fOffsetY = 0.5f + (0.5f / 512.0f);

				D3DXMATRIX texScaleBiasMat;
				if(m_bSupportsHWShadowMaps)
				{
					texScaleBiasMat = D3DXMATRIX( 0.5f,     0.0f,     0.0f,    0.0f,
												0.0f,    -0.5f,     0.0f,    0.0f,
												0.0f,     0.0f,     1.0f,    0.0f,
												fOffsetX, fOffsetY, 0.0f,    1.0f );
				}
				else
				{
					texScaleBiasMat = D3DXMATRIX( 0.5f,     0.0f,     0.0f,    0.0f,
												0.0f,    -0.5f,     0.0f,    0.0f,
												0.0f,     0.0f,     0.0f,    0.0f,
												fOffsetX, fOffsetY, 0.0f,    1.0f );
				}
				D3DXMATRIX shadowMat;
				shadowMat = m_matInvView * m_lightMat * texScaleBiasMat;
				m_pEffect->SetMatrix("ShadowMat", &shadowMat);
				m_pEffect->SetTexture("ShadowMap", m_ShadowMap.tex);

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pDeclaration1);

                pd3dDevice->SetStreamSource(0, m_pQuadVB, 0, sizeof(QuadVertex));

				m_pEffect->CommitChanges();
                if (FAILED(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)))
                    return hr;
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }

    if (m_Lights.size() > 1)
    {
        //reset state
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    return S_OK;
}

HRESULT DeferredShading::DrawSceneToShadowMap(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr = S_OK;

	if(m_bSupportsHWShadowMaps)
	{
		pd3dDevice->SetRenderTarget(0, m_ShadowMapDummy.surf);
		pd3dDevice->SetDepthStencilSurface(m_ShadowMap.surf);
	}
	else
	{
		pd3dDevice->SetRenderTarget(0, m_ShadowMap.surf);
		pd3dDevice->SetDepthStencilSurface(NULL);
	}
	DWORD clearFlags = D3DCLEAR_TARGET;
#ifdef USE_HWSHADOWMAP
	clearFlags |= D3DCLEAR_ZBUFFER;
#endif

	pd3dDevice->Clear(0, NULL, clearFlags, D3DCOLOR_ARGB(0, 255, 255, 255), 1.0f, 0);

	// select a technique, from our .fx file, to do either Diffuse lighting only, or Diffuse + Specular.
    std::string sTechniqueName = "RenderToShadowMap";

    if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
    {
        char buf[2048];
        sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
		WCHAR lpszW[2048];
		MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

		MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		
        return E_FAIL;
    }

	if(m_bSupportsHWShadowMaps)
	{
		pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&m_fDepthBias);
		pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);
	}

    // Draw
    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++) 
		{
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

            for (unsigned int i = 0; i < m_Scene->m_NumMeshes; ++i) 
			{
                const NVBScene::Mesh& mesh = m_Scene->m_Meshes[i];

				SetBumpDot3VSConstantsSM(mesh.m_Transform, m_lightMat);

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pDeclaration0);

				m_pEffect->CommitChanges();

				if ( (mesh.m_Material == "stucko_Outside") || (mesh.m_Material == "stucko_Inside") 
					|| (mesh.m_Material == "logs") || (mesh.m_Material == "old_wood") || (mesh.m_Material == "Front_door"))
				{
					if (FAILED(hr = mesh.Draw()))
						return hr;
				}
            }
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

	if(m_bSupportsHWShadowMaps)
	{
		float zero = 0.0f;
		pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&zero);
		pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&zero);
	}

	return hr;
}

void DeferredShading::AddPointLight(float x, float y, float z)
{
	bool bFound = false;
	for (unsigned int i = 0; i < m_Lights.size(); ++i)
	{
		if ( (abs(m_Lights[i].Position.x - x) < 0.0001f) &&
			 (abs(m_Lights[i].Position.y - y) < 0.0001f) &&
			 (abs(m_Lights[i].Position.z - z) < 0.0001f))
		{
			 bFound = true;
			 break;
		}
	}

	if (!bFound)
	{
		float randColor = (float)rand() / ((float)RAND_MAX * 4.0f);
		float randRange = 2.0f * (float)rand() / (float)RAND_MAX;
		LightType newLight;
		ZeroMemory(&newLight, sizeof(LightType));
		newLight.Type      = D3DLIGHT_POINT;
		newLight.Color     = D3DXVECTOR3(0.8f - randColor, 0.7f + randColor, 0.8f - randColor);
		newLight.Position  = D3DXVECTOR3(x, y, z);
		newLight.Range     = 3.5f - randRange;
		m_Lights.push_back(newLight);
	}
}

HRESULT DeferredShading::DrawSceneToMRT(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;

	for (int i = 0; i < NUM_MRT; ++i)
		pd3dDevice->SetRenderTarget(i, m_MRTs[i].surf);

	pd3dDevice->SetDepthStencilSurface(m_pDepthBuffer);

	// clear (this clears all MRTs)
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	// Fill mode
	pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    // select a technique, from our .fx file, to do either Diffuse lighting only, or Diffuse + Specular.
    //std::string sTechniqueName = "CreateMRTPerPixelNormal";
	std::string sTechniqueName = "CreateMRTInterpolatedNormal";

    if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
    {
        char buf[2048];
        sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
		WCHAR lpszW[2048];
		MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

		MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		
        return E_FAIL;
    }

	NVBScene::Mesh* emissiveMesh = NULL;

    // Draw
    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++) 
		{
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

			bool bDone = false;
            for (unsigned int i = 0; i < m_Scene->m_NumMeshes; ++i) 
			{
                const NVBScene::Mesh& mesh = m_Scene->m_Meshes[i];
                m_pEffect->SetTexture("DiffuseMap", (mesh.m_DiffuseMap && !m_UseDefaultDiffuseMap) ? mesh.m_DiffuseMap : m_DiffuseMap);
                m_pEffect->SetTexture("NormalMap" , (mesh.m_NormalMap  && !m_UseDefaultNormalMap ) ? mesh.m_NormalMap  : m_NormalMap );
                SetBumpDot3VSConstants(mesh.m_Transform);

                pd3dDevice->SetFVF(NULL);
                pd3dDevice->SetVertexDeclaration(m_pDeclaration0);

				float zero = 0.0f;
				float one  = 1.0f;
				if (IsShadowedMesh(mesh.m_Material))
					m_pEffect->SetValue("NotShadowed", &zero, sizeof(float));
				else
					m_pEffect->SetValue("NotShadowed", &one, sizeof(float));

				//this is the emissive mesh
				if (mesh.m_Material == "rampfun1")
					emissiveMesh = &m_Scene->m_Meshes[i];
				else
				{
					m_pEffect->CommitChanges();
					if (FAILED(hr = mesh.Draw()))
						return hr;
				}

				//while we're going through the mesh bits, place faint lights at the flame tips
				if (mesh.m_Material == "CandleFlameMap")
				{
					//save transform
					D3DXMATRIX tempMat = mesh.m_Transform;
					//compute midpoint
					D3DXVECTOR3 tempPos3 = (mesh.m_Vertices[0].m_Position + mesh.m_Vertices[1].m_Position + mesh.m_Vertices[2].m_Position + mesh.m_Vertices[3].m_Position) / 4.0f;;
					D3DXVECTOR4 tempPos;
					D3DXVec3Transform(&tempPos, &tempPos3, &mesh.m_Transform);
					AddPointLight(tempPos.x, tempPos.y, tempPos.z);
				}
            }
			m_pEffect->EndPass();

        }
        m_pEffect->End();
    }

	//now render emissive bits
    if (FAILED(m_pEffect->SetTechnique("CreateMRTEmissive")))
    {
        char buf[2048];
        sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
		WCHAR lpszW[2048];
		MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

		MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		
        return E_FAIL;
    }

	// Draw
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
	{  
		for (UINT uPass = 0; uPass < uPasses; uPass++) 
		{
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

            m_pEffect->SetTexture("DiffuseMap", (emissiveMesh->m_DiffuseMap && !m_UseDefaultDiffuseMap) ? emissiveMesh->m_DiffuseMap : m_DiffuseMap);
            m_pEffect->SetTexture("NormalMap" , (emissiveMesh->m_NormalMap  && !m_UseDefaultNormalMap ) ? emissiveMesh->m_NormalMap  : m_NormalMap );
            SetBumpDot3VSConstants(emissiveMesh->m_Transform);
            
			float emissiveFactor = 10.0f;
			m_pEffect->SetValue("EmissiveFactor", &emissiveFactor, sizeof(float));

            pd3dDevice->SetFVF(NULL);
            pd3dDevice->SetVertexDeclaration(m_pDeclaration0);

			m_pEffect->CommitChanges();

			if (FAILED(hr = emissiveMesh->Draw()))
				return hr;
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    //render light mesh
    sTechniqueName = "CreateMRTLightMesh";

    if (FAILED(m_pEffect->SetTechnique(sTechniqueName.c_str())))
    {
        char buf[2048];
        sprintf(buf, "Failed to set '%s' technique in effect file", sTechniqueName.c_str());
		WCHAR lpszW[2048];
		MultiByteToWideChar(CP_ACP, 0, buf, 2048, lpszW, 2048);

		MessageBox(NULL, lpszW, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		
        return E_FAIL;
    }

    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) 
    {  
        for (UINT uPass = 0; uPass < uPasses; uPass++) 
        {
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

            //place light at actual light's position
            D3DXMATRIX transMat;
            D3DXMatrixTranslation(&transMat, m_Lights[1].Position.x, m_Lights[1].Position.y, m_Lights[1].Position.z);
            SetBumpDot3VSConstants(transMat);

			m_pEffect->CommitChanges();
            if (FAILED(hr = m_pLightMesh->DrawSubset(0)))
                return hr;
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }
    else
        return E_FAIL;
	

    return S_OK;
}

bool DeferredShading::IsShadowedMesh(const std::string meshName)
{
	if(m_bSupportsHWShadowMaps)
		return true;      //everything shadows with hw shadowmaps

	if ( (meshName == "GroundA_shader") ||
		 (meshName == "GroundB_shader") ||
		 (meshName == "stucko_Inside")  ||
		 (meshName == "lambert1")        ||
		 (meshName == "rug")||
		 (meshName == "DreamCatcher")||
		 (meshName == "DreamCatch_Center")||
		 (meshName == "feather")||
		 (meshName == "Red_Mat1")||
		 (meshName == "Blue_mat1")||
		 (meshName == "dirtymetal")||
		 (meshName == "old_wood")||
		 (meshName == "CandleFlameMap")||
		 (meshName == "Mountains_shader")||
		 (meshName == "lambert79_file562"))
		return true;
	else
		return false;
}


void DeferredShading::SetBumpDot3VSConstantsSM(const D3DXMATRIX& world, const D3DXMATRIX& viewProj)
{
    D3DXMATRIX worldViewProj = world * viewProj;
    m_pEffect->SetMatrix("WorldViewProj", &worldViewProj);
}

void DeferredShading::SetBumpDot3VSConstants(const D3DXMATRIX& world)
{
    // Set transform from object space to projection space
    D3DXMATRIX worldView = world * m_View;
    m_pEffect->SetMatrix("WorldView", &worldView);
    D3DXMATRIX worldViewIT;
    D3DXMatrixInverse(&worldViewIT, NULL, &worldView);
    D3DXMatrixTranspose(&worldViewIT, &worldViewIT);
    m_pEffect->SetMatrix("WorldViewIT", &worldViewIT);

    D3DXMATRIX worldViewProj = worldView *  m_Projection;
    m_pEffect->SetMatrix("WorldViewProj", &worldViewProj);

    D3DXMatrixInverse(&m_matInvProj, NULL, &m_Projection);

	D3DXMATRIX viewProj = m_View * m_Projection;
	D3DXMatrixInverse(&m_matInvViewProj, NULL, &viewProj);

	D3DXMatrixInverse(&m_matInvView, NULL, &m_View);
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT DeferredShading::InvalidateDeviceObjects()
{
	ReleaseTextures();
    SAFE_RELEASE(m_pDeclaration0);
	SAFE_RELEASE(m_pDeclaration1);
    SAFE_RELEASE(m_pEffect);

	SAFE_RELEASE(m_pBackBuffer);
    SAFE_RELEASE(m_pDepthBuffer);
    
	SAFE_RELEASE(m_pQuadVB);
	SAFE_DELETE (m_Scene);

	for (unsigned int i = 0; i < m_QuadVec.size(); ++i)
    {
		SAFE_RELEASE(m_QuadVec[i]);
    }

    SAFE_RELEASE(m_pLightMesh);

    for (unsigned int i = 0; i < m_TonemapSurfaces.size(); ++i)
    {
        SAFE_RELEASE(m_TonemapSurfaces[i].surf);
        SAFE_RELEASE(m_TonemapSurfaces[i].tex);
    }

	SAFE_RELEASE(m_BlurSurfaces1.surf);
	SAFE_RELEASE(m_BlurSurfaces1.tex);

	SAFE_RELEASE(m_BlurSurfaces2.surf);
	SAFE_RELEASE(m_BlurSurfaces2.tex);

    for (int i = 0; i < NUM_MRT; ++i)
    {
        SAFE_RELEASE(m_MRTs[i].surf);
        SAFE_RELEASE(m_MRTs[i].tex);
    }

    SAFE_RELEASE(m_LuminanceSurfaces.surf);
    SAFE_RELEASE(m_LuminanceSurfaces.tex);

    SAFE_RELEASE(m_LTSurfaces.surf);
    SAFE_RELEASE(m_LTSurfaces.tex);

    SAFE_RELEASE(m_LTSurfacesFinal.surf);
    SAFE_RELEASE(m_LTSurfacesFinal.tex);

    SAFE_RELEASE(m_ShadowMap.surf);
    SAFE_RELEASE(m_ShadowMap.tex);

#if USE_HWSHADOWMAP
	SAFE_RELEASE(m_ShadowMapDummy.surf);
	SAFE_RELEASE(m_ShadowMapDummy.tex);
#endif

	SAFE_RELEASE(m_LastLuminance.surf);
	SAFE_RELEASE(m_LastLuminance.tex);
	SAFE_RELEASE(m_CurrLuminance.surf);
	SAFE_RELEASE(m_CurrLuminance.tex);
	
    return S_OK;
}

HRESULT DeferredShading::LoadShaders(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = D3D_OK;

    // Load our main Effect file, which contains our shaders.  (note: path is relative to MEDIA\ dir)
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\DeferredShading.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // Assign registers to the relevant vertex attributes
    D3DVERTEXELEMENT9 declaration0[] =
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

    D3DVERTEXELEMENT9 declaration1[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    if (hr = FAILED(pd3dDevice->CreateVertexDeclaration(declaration0, &m_pDeclaration0)))
		return hr;
	
	if (hr = FAILED(pd3dDevice->CreateVertexDeclaration(declaration1, &m_pDeclaration1)))
		return hr;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Paired with OneTimeSceneInit()
//       Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT DeferredShading::FinalCleanup()
{
    return S_OK;
}

HRESULT DeferredShading::InitializeScene(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

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

	return S_OK;
}

/*******************************************************************************

Texture management

*******************************************************************************/

HRESULT DeferredShading::LoadTexture(IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DTEXTURE9& texture, D3DPOOL pool, D3DXIMAGE_INFO* info)
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

HRESULT DeferredShading::LoadTexture(IDirect3DDevice9* pd3dDevice, const tstring& filename, LPDIRECT3DCUBETEXTURE9& texture, D3DXIMAGE_INFO* info)
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

void DeferredShading::ReleaseTextures()
{
	SAFE_RELEASE(m_DiffuseMap);
	SAFE_RELEASE(m_HeightMap);
	SAFE_RELEASE(m_NormalMap);
	SAFE_RELEASE(m_CubeMap);
}

HRESULT DeferredShading::LoadTextures(IDirect3DDevice9* pd3dDevice)
{
	ReleaseTextures();
	HRESULT hr;
	if (m_DiffuseMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_DiffuseMapFilename, m_DiffuseMap)))
			return hr;
	if (m_HeightMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_HeightMapFilename, m_HeightMap)))
			return hr;
	if (m_NormalMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_NormalMapFilename, m_NormalMap)))
			return hr;
	if (m_CubeMapFilename != _T(""))
		if (FAILED(hr = LoadTexture(pd3dDevice, m_CubeMapFilename, m_CubeMap)))
			return hr;
	return S_OK;
}
HRESULT DeferredShading::CheckResourceFormatSupport(IDirect3DDevice9* pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
{
    HRESULT hr = S_OK;
    IDirect3D9* tempD3D = NULL;
    pd3dDevice->GetDirect3D(&tempD3D);
    D3DCAPS9 devCaps;
    pd3dDevice->GetDeviceCaps(&devCaps);
    
    D3DDISPLAYMODE displayMode;
    tempD3D->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);
    
    hr = tempD3D->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);
    
    tempD3D->Release(), tempD3D = NULL;
    
    return hr;
}
