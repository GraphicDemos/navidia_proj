#include "dxstdafx.h"

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
CDXUTDialog		g_HUD;
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs

enum
{
IDC_TOGGLEFULLSCREEN,
IDC_BOWINTENSE,
IDC_BOWINTENSE_STATIC,
IDC_DROPLETRADIUS,
IDC_DROPLETRADIUS_STATIC,
IDC_RENDSTEPS,
};

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();

#include "nv_RainbowEffect.h"
#include "shared\GetFilePath.h"
#include "resource.h"
#include "HLSL_RainbowFogbow.h"

typedef std::basic_string<TCHAR> tstring; 
#define lookupTextureMinRadius 5.0f//microns
#define lookupTextureMaxRadius 800.0f//microns

INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceFrameMove( OnFrameMove );

	DXUTSetCursorSettings( true, true );

    InitApp();

    DXUTInit( true, true ); // Parse the command line and handle the default hotkeys
    DXUTCreateWindow( L"HLSL_RainbowFogbow" );
    DXUTCreateDevice( true, 512, 512 );

    DXUTMainLoop();

	return DXUTGetExitCode();
}

void InitApp()
{
    g_pFont                     = NULL;
    g_bLoadingApp               = TRUE;

    ZeroMemory(&g_UserInput, sizeof(g_UserInput));

	//NVIDIA effect//////////////////////////////////
	//g_d3dEnumeration.AppMinAlphaChannelBits = 8;	//we need a backbuffer with ALPHA!
	
	g_pSkyBoxMesh						= new CDXUTXFileMesh();
	g_pSkyBoxMoistureMesh				= new CDXUTXFileMesh();
	g_pTerrainMesh						= new CDXUTXFileMesh();
	g_pNVRainbowEffect					= new nv_RainbowEffect();
	g_rainbowDropletRadius				= 0.81f;
	g_rainbowIntensity					= 1.5f;
	g_pObjectEffects					= NULL;
	g_pSkyBoxRainEffect					= NULL;
	g_hTechniqueRenderObjectsNormal		= NULL;
	g_hTechniqueRenderObjectsBlack		= NULL; 
	g_hTechniqueRenderSkyBoxMoisture	= NULL;
	g_VisualizeRenderSteps				= false;

	//NVIDIA effect//////////////////////////////////

	//these vectors are hand tweaked to position the initial viewer nicely,
	//and to match the sun with the sun in the skybox.
	D3DXVECTOR3 vecEye(0.0f, -5.0f, 80.0f);
    D3DXVECTOR3 vecAt (-20.0f, 18.7f,-1.0f);
	g_sunLightDirection = D3DXVECTOR4( -1.0f , -0.69f , -1.1f , 0.0f );

	g_Camera.SetViewParams( &vecEye, &vecAt );

	/////////////////////////////////////////////////

    // Drawing loading status message until app finishes loading
    //SendMessage(g_hWnd, Wg_PAINT, 0, 0);

    g_bLoadingApp = FALSE;

    g_HUD.Init( &g_DialogResourceManager );

    g_HUD.SetFont( 0, L"Arial", 14, 400 );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Full screen", 35, 35, 125, 22 );
	g_HUD.AddSlider(IDC_BOWINTENSE,0,0,150,22,0,1000,(int)g_rainbowIntensity*100);
	g_HUD.AddSlider(IDC_DROPLETRADIUS,0,0,150,22,0,100,(int)g_rainbowDropletRadius*100);
	g_HUD.AddButton(IDC_RENDSTEPS,L"Show All (R)endering Steps",0,0,200,22,L'R');

	TCHAR sz[100];

	_sntprintf( sz, 100, TEXT("Rainbow Intensity: %.3f"), 
		(FLOAT)g_HUD.GetSlider(IDC_BOWINTENSE)->GetValue()/100); 
	sz[99] = 0;
	g_HUD.AddStatic(IDC_BOWINTENSE_STATIC,sz,400,0,220,22);

	_sntprintf( sz, 100, TEXT("Water Droplet Radius: %.3f microns"), 
		((FLOAT)g_HUD.GetSlider(IDC_DROPLETRADIUS)->GetValue()/100)*(lookupTextureMaxRadius - lookupTextureMinRadius)  + lookupTextureMinRadius);
	sz[99] = 0;
	g_HUD.AddStatic(IDC_DROPLETRADIUS_STATIC,sz,400,0,220,22);
}

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	/*
    UNREFERENCED_PARAMETER(pCaps);
    //UNREFERENCED_PARAMETER(dwBehavior);
    UNREFERENCED_PARAMETER(AdapterFormat);
	*/
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if (pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support 
    if (pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
	if(BackBufferFormat != D3DFMT_A8R8G8B8)
		nErrors++;

    return SUCCEEDED((nErrors > nPrevErrors) ? E_FAIL : S_OK);
}

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

	//skybox
	hr = LoadMeshHelperFunction(g_pSkyBoxMesh, TEXT("MEDIA\\models\\RainbowFogbowModels\\rainbowFogBow_skyBox.x"),pd3dDevice);
	if( FAILED( hr ) )
        return hr;

	//skybox with noise textures
	hr = LoadMeshHelperFunction(g_pSkyBoxMoistureMesh, TEXT("MEDIA\\models\\RainbowFogbowModels\\rainbowFogBow_skyBox_Noise.x"),pd3dDevice);
	if( FAILED( hr ) )
        return hr;

	//a ground plane just to show it in the scene
	hr = LoadMeshHelperFunction(g_pTerrainMesh, TEXT("MEDIA\\models\\RainbowFogbowModels\\rainbowFogBow_terrain.x"),pd3dDevice);
	if( FAILED( hr ) )
        return hr;


	//our rainbow effect
	g_pNVRainbowEffect->Create(pd3dDevice);

	tstring path = GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\RainbowFogbowSDKHelpers.cso"), false );
	if(path.empty())
		return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;
	hr = D3DXCreateEffectFromFile(pd3dDevice, path.c_str(), NULL, NULL, 0, NULL, &g_pObjectEffects, NULL );
    if (FAILED(hr))
        g_pObjectEffects = NULL;

	path = GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\RainbowFogbowNoiseScroll.cso"), false );
	if(path.empty())
		return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;
	hr = D3DXCreateEffectFromFile(pd3dDevice, path.c_str(), NULL, NULL, 0, NULL, &g_pSkyBoxRainEffect, NULL );
    if (FAILED(hr))
        g_pSkyBoxRainEffect = NULL;



    // Initialize the font
    HDC hDC = GetDC(NULL);
    int nHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(NULL, hDC);
    if (FAILED(hr = D3DXCreateFont(pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                   DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                                   TEXT("Arial"), &g_pFont)))
        return DXTRACE_ERR(TEXT("D3DXCreateFont"), hr);


    return S_OK;}

HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

    g_HUD.SetLocation( 0, 0 );
	g_HUD.SetSize(pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height);
	g_HUD.GetControl(IDC_TOGGLEFULLSCREEN)->SetLocation(pBackBufferSurfaceDesc->Width-125, 10);

	g_HUD.GetControl(IDC_BOWINTENSE)->SetLocation(pBackBufferSurfaceDesc->Width - 380, pBackBufferSurfaceDesc->Height - 54);
	g_HUD.GetControl(IDC_BOWINTENSE_STATIC)->SetLocation(pBackBufferSurfaceDesc->Width - 220, pBackBufferSurfaceDesc->Height - 54);
	g_HUD.GetControl(IDC_DROPLETRADIUS)->SetLocation(pBackBufferSurfaceDesc->Width - 380, pBackBufferSurfaceDesc->Height - 32);
	g_HUD.GetControl(IDC_DROPLETRADIUS_STATIC)->SetLocation(pBackBufferSurfaceDesc->Width - 220, pBackBufferSurfaceDesc->Height - 32);
	g_HUD.GetControl(IDC_RENDSTEPS)->SetLocation(pBackBufferSurfaceDesc->Width - 210, pBackBufferSurfaceDesc->Height - 76);

	g_Camera.SetProjParams( D3DXToRadian(60.0f), pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height, 1.0f, 505.0f );
	g_Camera.SetScalers(0.01f, 15.0f);

	g_pSkyBoxMesh->RestoreDeviceObjects(pd3dDevice);
	g_pSkyBoxMoistureMesh->RestoreDeviceObjects(pd3dDevice);
	g_pTerrainMesh->RestoreDeviceObjects(pd3dDevice);
	g_pNVRainbowEffect->RestoreDeviceObjects(pd3dDevice);
	if(g_pObjectEffects)
		g_pObjectEffects->OnResetDevice();
	if(g_pSkyBoxRainEffect)
		g_pSkyBoxRainEffect->OnResetDevice();

	g_hTechniqueRenderObjectsNormal     = g_pObjectEffects->GetTechniqueByName("texturedWithFogFactor");
	g_hTechniqueRenderObjectsBlack      = g_pObjectEffects->GetTechniqueByName("blackWithFogFactor");
	g_hTechniqueRenderSkyBoxMoisture	= g_pObjectEffects->GetTechniqueByName("skyBoxMoisture");

	D3DXHANDLE hTechnique = g_pSkyBoxRainEffect->GetTechniqueByName("ScrollingNoise");
	g_pSkyBoxRainEffect->SetTechnique(hTechnique);

    // Set up the textures
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
  
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );



	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Set the world matrix
    D3DXMatrixIdentity( &g_worldTransform );
    pd3dDevice->SetTransform( D3DTS_WORLD,  &g_worldTransform );

    pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
    pd3dDevice->SetTransform( D3DTS_PROJECTION, g_Camera.GetProjMatrix() );


    if (g_pFont)
        g_pFont->OnResetDevice();

    return S_OK;
}

void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove(fElapsedTime);

    // Update the world state according to user input
    pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
    pd3dDevice->SetTransform( D3DTS_PROJECTION, g_Camera.GetProjMatrix() );

	D3DXMATRIX projInv;
	D3DXMatrixInverse(&projInv, NULL, g_Camera.GetProjMatrix());

	g_pNVRainbowEffect->SetViewMatrix((D3DXMATRIX*)g_Camera.GetViewMatrix());
	g_pNVRainbowEffect->SetProjInvMatrix(&projInv);
	g_pNVRainbowEffect->SetDropletRadius(g_rainbowDropletRadius);
	g_pNVRainbowEffect->SetRainbowIntensity(g_rainbowIntensity);
}

void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	/**
		The Rainbow is rendered in a few distinct steps
		1. Generate a moisture texture
		2. Render the scene as you would normally
		3. Render the rainbow effect using the moisture texture
	*/

	g_pObjectEffects->SetFloat("fogNear", 20.0f);
	g_pObjectEffects->SetFloat("fogFar", 300.0f);
	g_pObjectEffects->SetVector( "lightDir", &g_sunLightDirection );
	g_pNVRainbowEffect->SetLightDirection(&g_sunLightDirection);


    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		const D3DSURFACE_DESC*  pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();
		int tmpWidth = pBackBufferDesc->Width;
		int tmpHeight = pBackBufferDesc->Height;

		if(!g_VisualizeRenderSteps)//render normally
		{
			
			g_pNVRainbowEffect->BeginMoistureTextureRendering(pd3dDevice);
			RenderMoisturePass(pd3dDevice, fTime);
			g_pNVRainbowEffect->EndMoistureTextureRendering(pd3dDevice);

			RenderSceneNormally(pd3dDevice);
			
			g_pNVRainbowEffect->RenderRainbow(pd3dDevice);
	        
		}
		else //render steps to different viewports.
		{
			//clear the main viewport
			SetViewPortHelper(0,0,tmpWidth, tmpHeight,pd3dDevice);
			pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
								0x00000000, 1.0f, 0L );


			//lets make a white moisture texture so we can just get a plain rainbow
			g_pNVRainbowEffect->BeginMoistureTextureRendering(pd3dDevice);
			/////////////////////////////////////////////////
			//why does this make things work here?
			//for some reason if I dont do this, then the clear doesn't seem to properly update
			//my viewport...or maybe there is some wierd state that is getting set.
				g_pSkyBoxMesh->Render( pd3dDevice );
			/////////////////////////////////////////////////
			float moistureDensity = 0.5f;
			D3DXCOLOR fogMoisture( moistureDensity , g_rainbowDropletRadius , 0.1f , 1.0f );
			pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
								fogMoisture, 1.0f, 0L);
			g_pNVRainbowEffect->EndMoistureTextureRendering(pd3dDevice);

			//now render just the plain rainbow
			SetViewPortHelper(0,0, tmpWidth/3, tmpHeight/2,pd3dDevice);
			g_pNVRainbowEffect->RenderRainbow(pd3dDevice);


			//update the moisture texture to be the real moisture texture
			g_pNVRainbowEffect->BeginMoistureTextureRendering(pd3dDevice);
			RenderMoisturePass(pd3dDevice, fTime);
			g_pNVRainbowEffect->EndMoistureTextureRendering(pd3dDevice);

			//render the moisture texture (just copy it to the backbuffer is fine.)
			LPDIRECT3DSURFACE9 ppSurface;
			LPDIRECT3DSURFACE9 pBackBuffer;
			RECT viewPortRect;
			viewPortRect.left = tmpWidth/3;
			viewPortRect.top = 0;
			viewPortRect.right = viewPortRect.left+ tmpWidth/3;
			viewPortRect.bottom =tmpHeight/2;
			g_pNVRainbowEffect->GetMoistureTexture()->GetSurfaceLevel(0,&ppSurface);
			pd3dDevice->GetRenderTarget(0, &pBackBuffer);
			pd3dDevice->StretchRect(ppSurface, NULL, pBackBuffer, &viewPortRect, D3DTEXF_NONE);
			ppSurface->Release();
			pBackBuffer->Release();

			//render Moisture X Rainbow
			SetViewPortHelper(2*tmpWidth/3,0, tmpWidth/3, tmpHeight/2,pd3dDevice);
			g_pNVRainbowEffect->RenderRainbow(pd3dDevice);


			//render the normal scene
			SetViewPortHelper(0,tmpHeight/2,tmpWidth/3, tmpHeight/2,pd3dDevice);
			RenderSceneNormally(pd3dDevice);


			//render the normal scene with the rainbow
			SetViewPortHelper(tmpWidth/3,tmpHeight/2, tmpWidth/3, tmpHeight/2,pd3dDevice);
			RenderSceneNormally(pd3dDevice);
			g_pNVRainbowEffect->RenderRainbow(pd3dDevice);
	      
		
		}
		SetViewPortHelper(0,0, tmpWidth, tmpHeight,pd3dDevice);

		g_HUD.OnRender( fElapsedTime );
		pd3dDevice->EndScene();
    }
}

void RenderText()
{
}

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}

void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch(nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		{
			DXUTToggleFullScreen();
			break;
		}
	case IDC_BOWINTENSE:
		{
			TCHAR sz[100];

			g_rainbowIntensity	= (FLOAT)g_HUD.GetSlider(IDC_BOWINTENSE)->GetValue()/100;
			_sntprintf( sz, 100, TEXT("Rainbow Intensity: %.3f"), g_rainbowIntensity); 
			sz[99] = 0;
			g_HUD.GetStatic(IDC_BOWINTENSE_STATIC)->SetText(sz);

			break;
		}
	case IDC_DROPLETRADIUS:
		{
			TCHAR sz[100];

			g_rainbowDropletRadius = (FLOAT)g_HUD.GetSlider(IDC_DROPLETRADIUS)->GetValue()/100;
			_sntprintf( sz, 100, TEXT("Water Droplet Radius: %.3f microns"),
				((FLOAT)g_HUD.GetSlider(IDC_DROPLETRADIUS)->GetValue()/100)*(lookupTextureMaxRadius - lookupTextureMinRadius)  + lookupTextureMinRadius);
			sz[99] = 0;
			g_HUD.GetStatic(IDC_DROPLETRADIUS_STATIC)->SetText(sz);
			break;
		}
	case IDC_RENDSTEPS:
		{
			g_VisualizeRenderSteps = !g_VisualizeRenderSteps;

			if(!g_VisualizeRenderSteps)
				g_HUD.GetButton(IDC_RENDSTEPS)->SetText(L"Show All (R)endering Steps");
			else
				g_HUD.GetButton(IDC_RENDSTEPS)->SetText(L"Show Normal (R)endering Steps");
			break;
		}
	}

}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_pSkyBoxMesh->InvalidateDeviceObjects();
	g_pSkyBoxMoistureMesh->InvalidateDeviceObjects();
	g_pTerrainMesh->InvalidateDeviceObjects();
	g_pNVRainbowEffect->InvalidateDeviceObjects();
	if( g_pObjectEffects)
		g_pObjectEffects->OnLostDevice();
	if( g_pSkyBoxRainEffect )
		g_pSkyBoxRainEffect->OnLostDevice();


	if (g_pFont)
        g_pFont->OnLostDevice();


}

void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
	g_pSkyBoxMesh->Destroy();
	g_pSkyBoxMoistureMesh->Destroy();
	g_pTerrainMesh->Destroy();
	g_pNVRainbowEffect->Destroy();
	SAFE_RELEASE(g_pObjectEffects);
	SAFE_RELEASE(g_pSkyBoxRainEffect);

	SAFE_RELEASE(g_pFont);

	SAFE_RELEASE(g_pObjectEffects);
	SAFE_RELEASE(g_pSkyBoxRainEffect);

	SAFE_DELETE(g_pSkyBoxMesh);
	SAFE_DELETE(g_pSkyBoxMoistureMesh);
	SAFE_DELETE(g_pTerrainMesh);
	SAFE_DELETE(g_pNVRainbowEffect);

}
//helper to set proper directories and find paths and such
HRESULT	LoadMeshHelperFunction(CDXUTXFileMesh* mesh, tstring meshFile, IDirect3DDevice9* pd3dDevice)
{

	HRESULT hr;
	tstring path;
	tstring mediaFileDir;
	tstring::size_type lastSlashPos;

	path = GetFilePath::GetFilePath(meshFile.c_str(), false );
	lastSlashPos = path.find_last_of(TEXT("\\"), path.size());
	if (lastSlashPos != path.npos)
		mediaFileDir = path.substr(0, lastSlashPos);
	else
		mediaFileDir = TEXT(".");

	if(path.empty())
		return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;

	TCHAR currDir[512];
	GetCurrentDirectory(512,currDir);

	//note the mesh needs the current working directory to be set so that it
	//can properly load the textures
	SetCurrentDirectory(mediaFileDir.c_str());
	hr = mesh->Create( pd3dDevice, path.c_str() );
	SetCurrentDirectory(currDir);

	if( FAILED( hr ) )
        return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;
	return S_OK;
}
void SetUpMatriceeesForEffect( D3DXMATRIX* view, D3DXMATRIX* world, D3DXMATRIX* proj)
{
	//set up matrices
	D3DXMATRIX tmp1, tmp2;
	D3DXMatrixMultiply(&tmp1, world,view);
	D3DXMatrixMultiply(&tmp2, &tmp1, proj);
	g_pObjectEffects->SetMatrix("worldViewProj", &tmp2);
	D3DXMatrixInverse(&tmp1, NULL, view);
	g_pObjectEffects->SetMatrix("viewInverse", &tmp1);
	g_pObjectEffects->SetMatrix("world", world);
	D3DXMatrixInverse(&tmp1, NULL, world);
	D3DXMatrixTranspose(&tmp2, &tmp1);
	g_pObjectEffects->SetMatrix("worldInverseTranspose", &tmp2);

}
//helper function to modify the translation of a matrix
//made to move the viewer to be inside the skybox when rendering a skybox
void SetD3DXMatrixTranslationTo(float x, float y, float z, D3DXMATRIX* pMat)
{
	assert(pMat && "null matrix");
	pMat->_41 = x; 
	pMat->_42 = y; 
	pMat->_43 = z;
}

void RenderMoisturePass(IDirect3DDevice9* pd3dDevice, double fTime)
{
	float moistureDensity = 0.5f;
	D3DXCOLOR fogMoisture( moistureDensity , g_rainbowDropletRadius , 0.1f , 1.0f );


	// Clear the viewport
	pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
						fogMoisture, 1.0f, 0L);

	//render objects
	SetUpMatriceeesForEffect((D3DXMATRIX*)g_Camera.GetViewMatrix(), 
						&g_worldTransform ,
						(D3DXMATRIX*)g_Camera.GetProjMatrix()	);
	g_pObjectEffects->SetTechnique( g_hTechniqueRenderObjectsBlack );
	UINT passes;
	UINT i;
	g_pObjectEffects->Begin( &passes , 0 );
	for(i = 0 ; i < passes ; ++i )
	{
		g_pObjectEffects->BeginPass(i);
		g_pTerrainMesh->Render(pd3dDevice);
		g_pObjectEffects->EndPass();
	}
	g_pObjectEffects->End();

	// Render the Skybox

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_DESTALPHA  );
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA  );

	D3DXMATRIXA16 save_matView =  *(g_Camera.GetViewMatrix());
	D3DXMATRIXA16 matView = save_matView;
	SetD3DXMatrixTranslationTo(0.0f, -3.0f, 0.0f, &matView);
	pd3dDevice->SetTransform( D3DTS_VIEW,      &matView );
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	g_pObjectEffects->SetTechnique( g_hTechniqueRenderSkyBoxMoisture );
	SetUpMatriceeesForEffect(&matView,&g_worldTransform,(D3DXMATRIX*)g_Camera.GetProjMatrix());

	// Render the skybox
	g_pObjectEffects->Begin(&passes, 0 );
	for( i = 0 ; i < passes ; ++i)
	{
		g_pObjectEffects->BeginPass(i);
		g_pSkyBoxMesh->Render( pd3dDevice );
		g_pObjectEffects->EndPass();
	}
	g_pObjectEffects->End();

	//if(RENDER_RAIN)
	//render rainsheets noise skybox--------------
	//multiply this noise times our existing moisture
	D3DXMATRIX tmp1, tmp2;
	D3DXMatrixMultiply(&tmp1, &g_worldTransform, &matView);
	D3DXMatrixMultiply(&tmp2, &tmp1, g_Camera.GetProjMatrix());
	g_pSkyBoxRainEffect->SetMatrix( "worldViewProj", &tmp2 );
	g_pSkyBoxRainEffect->SetMatrix( "world", &g_worldTransform );

  	pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );

	g_pSkyBoxRainEffect->SetFloat("speed", 10.0f);
	g_pSkyBoxRainEffect->SetFloat("time", (float)fTime);
	
	D3DXVECTOR4 rainVector( 0.0, -1.0, 0.0, 0.0);
	g_pSkyBoxRainEffect->SetVector( "rainVec", &rainVector );

	g_pSkyBoxRainEffect->Begin(&passes, 0 );
	for( i = 0 ; i < passes ; ++i)
	{
		g_pSkyBoxRainEffect->BeginPass(i);
		g_pSkyBoxMoistureMesh->Render( pd3dDevice );
		g_pSkyBoxRainEffect->EndPass();
	}
	g_pSkyBoxRainEffect->End();


	// Restore the render states
	pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

}

void RenderSceneNormally(IDirect3DDevice9* pd3dDevice)
{

		//since our skybox will blend with based on alpha we have to clear the backbuffer to this alpha value
	D3DXCOLOR fogColor( 0.0f , 0.0f , 0.0f , 1.0f ); 
	pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
						fogColor , 1.0f, 0L);


	SetUpMatriceeesForEffect(	(D3DXMATRIX*)g_Camera.GetViewMatrix(), 
								&g_worldTransform ,
								(D3DXMATRIX*)g_Camera.GetProjMatrix()	);
	g_pObjectEffects->SetTechnique( g_hTechniqueRenderObjectsNormal );

	UINT passes;
	UINT i;
	//render terrain
	g_pObjectEffects->Begin( &passes , 0 );
	for(i = 0 ; i < passes ; ++i )
	{
		g_pObjectEffects->BeginPass(i);
		g_pTerrainMesh->Render(pd3dDevice);
		g_pObjectEffects->EndPass();
	}
	g_pObjectEffects->End();


	// Render the Skybox
		
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_DESTALPHA  );
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA  );

	D3DXMATRIXA16 save_matView =  *(g_Camera.GetViewMatrix());
	D3DXMATRIXA16 matView = save_matView;
	SetD3DXMatrixTranslationTo(0.0f, -3.0f, 0.0f, &matView);

	pd3dDevice->SetTransform( D3DTS_VIEW,      &matView );
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	g_pSkyBoxMesh->Render( pd3dDevice );

	// Restore the render states
	pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	

}
void SetViewPortHelper(DWORD x, DWORD y, DWORD w, DWORD h,IDirect3DDevice9* pd3dDevice)
{
	D3DVIEWPORT9 vp;
	vp.X      = x;
	vp.Y      = y;
	vp.Width  = w;
	vp.Height = h;
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f; 
	pd3dDevice->SetViewport(&vp);
}
