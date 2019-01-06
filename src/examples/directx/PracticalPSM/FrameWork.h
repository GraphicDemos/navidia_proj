//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT PracticalPSM::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
         

    if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support mipmaps!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    
    if(pCaps->MaxSimultaneousTextures < 2)
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support two simultaneous textures!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if(!(pCaps->RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support slope scale depth bias!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//  The device exists, but was just lost or reset, and is now being restored.  
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here.  Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT PracticalPSM::RestoreDeviceObjects(IDirect3DDevice9* m_pd3dDevice)
{
    HRESULT hr;

    assert(m_pd3dDevice);

    //  hardware shadow maps are enabled by creating a texture with a depth format (D16, D24X8, D24S8),
    //  with usage DEPTHSTENCIL set.
    //  set this texture as the depth/stencil buffer when rendering the shadow map, and as a texture
    //  when performing the shadow comparison.

    D3DFORMAT zFormat = D3DFMT_D24S8;
    m_bitDepth = 24;

    if(FAILED(CheckResourceFormatSupport(m_pd3dDevice, zFormat, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL)))
    {
        MessageBox(NULL, _T("Device/driver does not support hardware shadow maps, using R32F."), _T("Message"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        m_bSupportsHWShadowMaps = false;
    }
    else
		m_bSupportsHWShadowMaps = true;

    D3DCAPS9 deviceCaps;
    m_pd3dDevice->GetDeviceCaps( &deviceCaps );

    m_bSupportsPixelShaders20 = (deviceCaps.PixelShaderVersion >= D3DPS_VERSION(2,0));

    //setup buffers
    if(FAILED(m_pd3dDevice->GetRenderTarget(0, &m_pBackBuffer)))
        return E_FAIL;
    if(FAILED(m_pd3dDevice->GetDepthStencilSurface(&m_pZBuffer)))
        return E_FAIL;

    if(!m_pBackBuffer || !m_pZBuffer)
        return E_FAIL;


    D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
    
    int shadowTexWidth = (m_bSupportsPixelShaders20) ? TEXDEPTH_WIDTH_20 : TEXDEPTH_SIZE_11;
    int shadowTexHeight = (m_bSupportsPixelShaders20) ? TEXDEPTH_HEIGHT_20 : TEXDEPTH_SIZE_11;


	if (m_bSupportsHWShadowMaps)
	{
		if(FAILED(m_pd3dDevice->CreateTexture(shadowTexWidth, shadowTexHeight, 1, D3DUSAGE_RENDERTARGET, colorFormat, 
			D3DPOOL_DEFAULT, &m_pSMColorTexture, NULL)))
			return E_FAIL;
		if(FAILED(m_pd3dDevice->CreateTexture(shadowTexWidth, shadowTexHeight, 1, D3DUSAGE_DEPTHSTENCIL, zFormat, 
			D3DPOOL_DEFAULT, &m_pSMZTexture, NULL)))
			return E_FAIL;
		if(!m_pSMColorTexture || !m_pSMZTexture )
			return E_FAIL;

		// Retrieve top-level surfaces of our shadow buffer (need these for use with SetRenderTarget)
		if(FAILED(m_pSMColorTexture->GetSurfaceLevel(0, &m_pSMColorSurface)))
			return E_FAIL;
		if(FAILED(m_pSMZTexture->GetSurfaceLevel(0, &m_pSMZSurface)))
			return E_FAIL;
		if(!m_pSMColorSurface || !m_pSMZSurface)
			return E_FAIL;
	}
	else
	{
        //  use R32F & shaders instead of depth textures
		m_pSMZTexture = NULL;
		if(FAILED(m_pd3dDevice->CreateTexture(shadowTexWidth, shadowTexHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, 
			D3DPOOL_DEFAULT, &m_pSMColorTexture, NULL)))
			return E_FAIL;
		if(FAILED(m_pd3dDevice->CreateDepthStencilSurface(shadowTexWidth, shadowTexHeight, zFormat, 
			D3DMULTISAMPLE_NONE, 0, FALSE, &m_pSMZSurface, NULL)))
			return E_FAIL;
			
		if(!m_pSMColorTexture || !m_pSMZSurface)
        {
            MessageBox(NULL, _T("Device/driver does not support R32F textures."), _T("Message"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return E_FAIL;
        }

		// Retrieve top-level surfaces of our shadow buffer (need these for use with SetRenderTarget)
		if(FAILED(m_pSMColorTexture->GetSurfaceLevel(0, &m_pSMColorSurface)))
			return E_FAIL;

		if(!m_pSMColorSurface )
			return E_FAIL;	
	}


    // Assign registers to the relevant vertex attributes
    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  
        D3DDECL_END()
    };

    m_pd3dDevice->CreateVertexDeclaration(declaration, &m_pDeclaration);

    const char* profileOpts[] = 
    {
        "-profileopts", "dcls", NULL,
    };

    DWORD tempFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0;

    hr = CreateQuad(m_pd3dDevice, &m_smQuad);
    tstring fileNameRock(L"SambavaRockChunk.nvb");
    tstring fileNameClawBot(L"ClawBot.nvb");

    m_pRockChunk = new NVBScene;
    hr = m_pRockChunk->Load(fileNameRock, m_pd3dDevice, GetFilePath::GetMediaFilePath);
    if(FAILED(hr))
        return hr;
    m_pClawBot = new NVBScene;
    
    if (!m_pRockChunk->m_VertexHasNormal) {
        MessageBox(NULL, _T("Model does not have normals"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    hr = m_pClawBot->Load(fileNameClawBot, m_pd3dDevice, GetFilePath::GetMediaFilePath);
    if(FAILED(hr))
        return hr;

    if (!m_pClawBot->m_VertexHasNormal) {
        MessageBox(NULL, _T("Model does not have normals"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
  
    //set render states
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
     
    // Load our Effect file
    // note: path is relative to MEDIA\ dir
    hr = D3DXCreateEffectFromFile(m_pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\PracticalPSM.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // Mouse UI
    RECT rect;
    rect.left = rect.top = 0;

    // Set view matrix (m_View) and apply to m_UICamera
    RandomizeObjects();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//  The device exists, but is about to be Reset(), so we should release some things. 
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here. Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT PracticalPSM::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_smQuad.pVB);
    SAFE_RELEASE(m_smQuad.pIB);

    SAFE_RELEASE(m_pSMColorTexture);
    SAFE_RELEASE(m_pSMZTexture);
    SAFE_RELEASE(m_pSMColorSurface);
    SAFE_RELEASE(m_pSMZSurface);

    SAFE_RELEASE(m_pBackBuffer);
    SAFE_RELEASE(m_pZBuffer);

    SAFE_RELEASE(m_pDeclaration);

    SAFE_DELETE(m_pRockChunk);
    SAFE_DELETE(m_pClawBot);
    SAFE_RELEASE(m_pEffect);
    
    for (unsigned int i=0; i<m_Scenes.size(); i++)
    {
        delete m_Scenes[i];
    }
    m_Scenes.clear();

    return S_OK;
}

HRESULT PracticalPSM::CheckResourceFormatSupport(IDirect3DDevice9* m_pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
{
    HRESULT hr = S_OK;
    IDirect3D9* tempD3D = NULL;
    m_pd3dDevice->GetDirect3D(&tempD3D);
    D3DCAPS9 devCaps;
    m_pd3dDevice->GetDeviceCaps(&devCaps);
    
    D3DDISPLAYMODE displayMode;
    tempD3D->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);
    
    hr = tempD3D->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);
    
    tempD3D->Release(), tempD3D = NULL;
    
    return hr;
}
void PracticalPSM::RandomizeObjects()
{
    unsigned int i;
    for (i=0; i<m_Scenes.size(); i++)
    {
        delete m_Scenes[i];
    }
    m_Scenes.clear();
       
    for (i=0; i<NUM_OBJECTS; i++)
    {
        int objectType = rand() & 1;
        ObjectInstance* cbInstance = new ObjectInstance;
        cbInstance->scene = (objectType)?m_pClawBot : m_pRockChunk;
        cbInstance->translation.x = (float)(rand()%1600)-800;
        cbInstance->translation.z = (float)(rand()%1600)-800;
        cbInstance->translation.y = (float)((objectType)?-10:-5);
        cbInstance->aabb = (objectType)?&m_ClawBotLocalBB : &m_RockChunkLocalBB;
        cbInstance->aaBoxen = (objectType)?&m_vClawBotLocalBBs : &m_vRockChunkLocalBBs;
        m_Scenes.push_back(cbInstance);
    }
}