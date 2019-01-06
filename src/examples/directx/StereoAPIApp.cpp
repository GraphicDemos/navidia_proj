#define STRICT
#include "nvafx.h"
#include "..\..\..\..\Libs\inc\StereoI\StereoI.h"
#include "StereoAPIApp.h"
#include "shared\GetFilePath.h"

StereoI* g_pStereoAPI = NULL;

struct MYVERTEX
{
    FLOAT x, y;
	FLOAT u, v;
};

MYVERTEX vertices[] = {
	{-1.0f, 1.0f, 0.0f, 0.0f},
	{ 1.0f, 1.0f, 1.0f, 0.0f},
	{ 1.0f, -1.0f, 1.0f, 1.0f},
	{-1.0f, -1.0f, 0.0f, 1.0f}
};


HRESULT	LoadMeshHelperFunction(CDXUTMesh* mesh, tstring meshFile, IDirect3DDevice9* pd3dDevice);
//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Set the callback functions. These functions allow the sample framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"StereoAPI" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, false, NULL, NULL, IsDeviceAcceptable, ModifyDeviceSettings );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)",			  35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)",     35, 82, 125, 22, VK_F2 );
	g_HUD.AddButton( IDC_STEREOSWITCH, L"Stereo On Off",          35, 106, 125, 22 );
	g_HUD.AddButton( IDC_SCREENSHOT, L"Stereo ScreenShot",          35, 130, 125, 22 );
	g_HUD.AddSlider(IDC_CHANGECONVERGENCE, 0, 0, 300, 30, -100 ,100, 20);
	g_HUD.AddCheckBox(IDC_VIEWCONVERGE, L"Lock convergence to view", 0, 0, 200, 20, true, L' ');
	g_HUD.AddCheckBox(IDC_FOCALPLANE, L"Show Focal Plane", 0, 0, 200, 20, false, L' '); 

	CDXUTListBox* pList;
	g_HUD.AddListBox(IDC_TEXLISTBOX,0,40,150,43,0,&pList);
	if(pList)
	{
		pList->AddItem(L"Behind Car", (void*)0);
		pList->AddItem(L"Behind Wheel", (void*)1);
	}
	g_HUD.GetListBox(IDC_TEXLISTBOX)->SelectItem(0);

	TCHAR sz[100];
	_sntprintf( sz, 100, TEXT("Convergence at %d"), g_HUD.GetSlider(IDC_CHANGECONVERGENCE)->GetValue()); sz[99] = 0;
	g_HUD.AddStatic(IDC_CHANGECONVERGENCE_STATIC,sz,0, 0, 100, 30);

	g_pTerrainMesh = new CDXUTMesh();
	g_pSkyBox = new CDXUTMesh();
	g_pRocketCar = new CDXUTMesh();
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
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
 
    return SUCCEEDED((nErrors > nPrevErrors) ? E_FAIL : S_OK);
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
	pDeviceSettings->pp.BackBufferFormat = D3DFMT_A8R8G8B8;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnCreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Set up our view matrix. A view matrix can be defined given an eye point and
    // a point to lookat. Here, we set the eye five units back along the z-axis and 
	// up three units and look at the origin.
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(5.0f, 0.0f, 30.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, -10.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);

	V_RETURN(LoadMeshHelperFunction(g_pTerrainMesh, TEXT("models\\RainbowFogbowModels\\rainbowFogBow_terrain.x"),pd3dDevice));
	V_RETURN(LoadMeshHelperFunction(g_pSkyBox, TEXT("models\\RainbowFogbowModels\\rainbowFogBow_skyBox.x"),pd3dDevice));
	V_RETURN(LoadMeshHelperFunction(g_pRocketCar, TEXT("models\\RocketCar\\RocketCar.X"),pd3dDevice));

	//Create Stereo
	if(!CreateStereoI(&g_pStereoAPI))
	{
		MessageBox(NULL, _T("This sample requires the NVIDIA 71.83 Stereo driver or greater.  Please visit NVIDIA.COM for details."), _T("StereoI.dll Not Found"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return E_FAIL;
	}

		
	return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnResetDevice() );
    V_RETURN( g_SettingsDlg.OnResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = (FLOAT)pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
	g_HUD.GetControl( IDC_STEREOSWITCH )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
	g_HUD.GetControl( IDC_SCREENSHOT )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

	g_HUD.GetControl( IDC_CHANGECONVERGENCE )->SetLocation( pBackBufferSurfaceDesc->Width - 450, pBackBufferSurfaceDesc->Height - 125);
	g_HUD.GetControl( IDC_CHANGECONVERGENCE_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 150, pBackBufferSurfaceDesc->Height - 125);
	g_HUD.GetControl(IDC_TEXLISTBOX)->SetLocation(pBackBufferSurfaceDesc->Width - 200, pBackBufferSurfaceDesc->Height - 85);
	g_HUD.GetControl( IDC_VIEWCONVERGE )->SetLocation( pBackBufferSurfaceDesc->Width - 400, pBackBufferSurfaceDesc->Height - 55);
	g_HUD.GetControl( IDC_FOCALPLANE )->SetLocation( pBackBufferSurfaceDesc->Width - 400, pBackBufferSurfaceDesc->Height - 25);
	

    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	//Setup meshes
	g_pTerrainMesh->RestoreDeviceObjects(pd3dDevice);
	g_pSkyBox->RestoreDeviceObjects(pd3dDevice);
	g_pRocketCar->RestoreDeviceObjects(pd3dDevice);

	//Rectangle that takes the center 1/5th of the screen
	g_rBBPortion.top = (LONG)(pBackBufferSurfaceDesc->Height * 0.4);
	g_rBBPortion.left = (LONG)(pBackBufferSurfaceDesc->Width * 0.4);
	g_rBBPortion.right = (LONG)(pBackBufferSurfaceDesc->Width * 0.6);
	g_rBBPortion.bottom = (LONG)(pBackBufferSurfaceDesc->Height * 0.6);

	V_RETURN(D3DXCreateEffectFromFile(pd3dDevice,
								  GetFilePath::GetFilePath(TEXT("programs\\StereoAPI\\Stereo.fx")).c_str(),
								  NULL,
								  NULL,
								  NULL,
								  NULL,
								  &g_pEffect,
								  NULL));
		
	if( FAILED( pd3dDevice->CreateVertexBuffer( 4*sizeof(MYVERTEX),
			0, 0, D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	return E_FAIL;

	VOID* pVertices;
	if( FAILED( g_pVB->Lock( 0, sizeof(vertices), &pVertices, 0 ) ) )
		return E_FAIL;
		
	memcpy( pVertices, vertices, 4*sizeof(MYVERTEX) );
		
	g_pVB->Unlock();

    D3DVERTEXELEMENT9 decl[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };	
	pd3dDevice->CreateVertexDeclaration( decl, &g_pVertexDeclaration );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // TODO: update world
    g_Camera.FrameMove( fElapsedTime );
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    static double lastTime = fTime;
	HRESULT hr;

	//Update convergence
	UpdateConvergence();
	
    // Clear Z	
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);
	
	pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, (D3DCOLORWRITEENABLE_ALPHA |D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN |D3DCOLORWRITEENABLE_RED));

	// Begin the scene
	
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		g_pEffect->SetTechnique("RenderScene");
		RenderCar(pd3dDevice, (float)fTime);
		DrawTerrain(pd3dDevice);
		DrawSkyBox(pd3dDevice, 10);
		
		if(g_HUD.GetCheckBox(IDC_FOCALPLANE)->GetChecked())
			DrawFocalPlane(pd3dDevice, g_pStereoAPI->GetConvergence());
		

        // Render stats and help text  
        RenderText();

		V( g_HUD.OnRender( fElapsedTime ) );

        // End the scene.
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 15 );
	if( g_bShowUI )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( DXUTGetFrameStats() );
		txtHelper.DrawTextLine( DXUTGetDeviceStats() );

		// Display any additional information text here

		if( !g_bShowHelp )
		{
			txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
			txtHelper.DrawTextLine( TEXT("F1      - Toggle help text") );
	
			if(g_pStereoAPI->GetStereoState() == STEREO_STATE_DISABLED)
				txtHelper.DrawTextLine(TEXT("Stereo is DISABLED.  Please enable it in the driver control panel"));
		}
	}

	if( g_bShowHelp )
	{
		// Display help text here
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawTextLine( TEXT("F1      - Toggle help text") );
		txtHelper.DrawTextLine( TEXT("H       - Toggle UI") );
		txtHelper.DrawTextLine( TEXT("ESC  - Quit") );

	}
	txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, the sample framework inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_F1:
			g_bShowHelp = !g_bShowHelp;
			break;

		case 'H':
		case 'h':
			g_bShowUI = !g_bShowUI;
			for( int i = 0; i < IDC_LAST; i++ )
				g_HUD.GetControl(i)->SetVisible( g_bShowUI );
			break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	D3DXVECTOR3 vFromPt;
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, -10.0f);

	switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
		case IDC_STEREOSWITCH:	
			if(g_pStereoAPI->GetStereoState() == STEREO_STATE_ENABLED)
				g_pStereoAPI->SetStereoState(1);
			else
				g_pStereoAPI->SetStereoState(0);
			break;
		case IDC_SCREENSHOT: g_pStereoAPI->CaptureStereoImage(IMAGE_JPEG, 75); break;

		case IDC_TEXLISTBOX:
			DXUTListBoxItem *pItem = ((CDXUTListBox *)pControl)->GetItem( ((CDXUTListBox *)pControl)->GetSelectedIndex( -1 ) );
			
			switch((int)pItem->pData)
			{
			case 0:
				g_bBehindWheel = false;
				vFromPt = D3DXVECTOR3(5.0f, 0.0f, 30.0f);
				if(g_HUD.GetCheckBox(IDC_VIEWCONVERGE)->GetChecked())
				{
					g_HUD.GetSlider(IDC_CHANGECONVERGENCE)->SetValue(20);
				}
				break;
			case 1:
				g_bBehindWheel = true;
				vFromPt = D3DXVECTOR3(0.5f, 1.5f, 12.0f);
				if(g_HUD.GetCheckBox(IDC_VIEWCONVERGE)->GetChecked())
				{
					g_HUD.GetSlider(IDC_CHANGECONVERGENCE)->SetValue(3);
				}
				break;
			};
			g_Camera.SetViewParams( &vFromPt, &vLookatPt);
			break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnLostDevice();
    g_SettingsDlg.OnLostDevice();

    if( g_pFont )
        g_pFont->OnLostDevice();

	g_pTerrainMesh->InvalidateDeviceObjects();
	g_pSkyBox->InvalidateDeviceObjects();
	g_pRocketCar->InvalidateDeviceObjects();
	SAFE_RELEASE(g_pTextSprite);

	SAFE_RELEASE(g_pEffect);

	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVertexDeclaration);

}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnDestroyDevice();
    g_SettingsDlg.OnDestroyDevice();

    SAFE_RELEASE(g_pFont);
	g_pTerrainMesh->Destroy();
	g_pSkyBox->Destroy();
	g_pRocketCar->Destroy();

	//Clean up StereoAPI
	delete g_pStereoAPI;
	g_pStereoAPI = NULL;
}

//--------------------------------------------------------------------------------------
//Helper functions
//--------------------------------------------------------------------------------------

void SetUpMatricesForEffect( D3DXMATRIX* view, D3DXMATRIX* world, D3DXMATRIX* proj, LPD3DXEFFECT* pEffect)
{
	//set up matrices
	D3DXMATRIX tmp1, tmp2;

	D3DXMatrixMultiply(&tmp1, world, view);
	D3DXMatrixMultiply(&tmp2, &tmp1, proj);
	(*pEffect)->SetMatrix("WorldViewProjection", &tmp2);

	(*pEffect)->SetMatrix("World", world);

	D3DXMatrixInverse(&tmp1, NULL, world);
	D3DXMatrixTranspose(&tmp2, &tmp1);
	(*pEffect)->SetMatrix("WorldInverseTranspose", &tmp2);
}

HRESULT DrawScene(IDirect3DDevice9* pd3dDevice, LPD3DXEFFECT* pEffect, CDXUTMesh** pMesh)
{
	HRESULT hr;
	UINT iPass, iPasses;
	hr = (*pEffect)->Begin(&iPasses, 0);
	for(iPass = 0; iPass < iPasses; iPass++)
		{
			hr = (*pEffect)->BeginPass(iPass);
			hr = (*pMesh)->Render(pd3dDevice);
			hr = (*pEffect)->EndPass();
		}
	hr = (*pEffect)->End();
	return hr;
}

void RenderCar(IDirect3DDevice9* pd3dDevice, float fTime)
{
	D3DXMATRIX	CarWorldMatrix(
	0.049975902f, 0.00086383166f, 0.0012898464f, 0.00000000f, 
	-0.00092006929f, 0.049943056f, 0.0022009485f, 0.00000000f, 
	-0.0012503546f, -0.0022236225f, 0.049934879f, 0.00000000f, 
	0.00000000f, -0.99984372f, 9.9984369f, 0.99984372f);

	float xspin = sin(fTime*5)/50;
	float yspin = cos(fTime*5)/50;

	D3DXMATRIX tmp1, tmp2;
	D3DXMatrixRotationY(&tmp1, yspin);
	D3DXMatrixRotationX(&tmp2, xspin);
	D3DXMatrixMultiply(&tmp1, &tmp1, &tmp2);
	D3DXMatrixMultiply(&tmp1, &CarWorldMatrix, &tmp1);
	if(!g_bBehindWheel)
		D3DXMatrixMultiply(&tmp1, g_Camera.GetWorldMatrix(), &tmp1);

	SetUpMatricesForEffect((D3DXMATRIX*)g_Camera.GetViewMatrix(), &tmp1, (D3DXMATRIX*)g_Camera.GetProjMatrix(), &g_pEffect);
	DrawScene(pd3dDevice, &g_pEffect, &g_pRocketCar);
}

void DrawTerrain(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX				TerrainWorldMatrix(
	0.10212187f, 0.00000000f, -0.22819099f, 0.00000000f, 
	0.00000000f, 0.25000000f, 0.00000000f, 0.00000000f, 
	0.22819099f, 0.00000000f, 0.10212187f, 0.00000000f, 
	0.00000000f, 0.00000000f, -20.000000f, 1.0000000f);

	if(g_bBehindWheel)
		D3DXMatrixMultiply(&TerrainWorldMatrix, &TerrainWorldMatrix, g_Camera.GetWorldMatrix());

	SetUpMatricesForEffect((D3DXMATRIX*)g_Camera.GetViewMatrix(), &TerrainWorldMatrix, (D3DXMATRIX*)g_Camera.GetProjMatrix(), &g_pEffect);
	DrawScene(pd3dDevice, &g_pEffect, &g_pTerrainMesh);
}

void DrawSkyBox(IDirect3DDevice9* pd3dDevice, float scale)
{
	D3DXMATRIX ScaleMatrix = D3DXMATRIX(
	scale, 0.00000000f, 0.00000000f, 0.00000000f, 
	0.00000000f, scale, 0.00000000f, 0.00000000f, 
	0.00000000f, 0.00000000f, scale, 0.00000000f, 
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	if(g_bBehindWheel)
		D3DXMatrixMultiply(&ScaleMatrix, &ScaleMatrix, g_Camera.GetWorldMatrix());

	SetUpMatricesForEffect((D3DXMATRIX*)g_Camera.GetViewMatrix(), &ScaleMatrix, (D3DXMATRIX*)g_Camera.GetProjMatrix(), &g_pEffect);
	DrawScene(pd3dDevice, &g_pEffect, &g_pSkyBox);
}

void DrawFocalPlane(IDirect3DDevice9* pd3dDevice, float focalpoint)
{
	D3DXMATRIX ScaleBy5 = D3DXMATRIX(
	5.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 
	0.00000000f, 5.00000000f, 0.00000000f, 0.00000000f, 
	0.00000000f, 0.00000000f, 1.00000000f, 0.00000000f, 
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	D3DXMATRIX tmp;
	//Translate focal plane to convergence point
	D3DXMatrixTranslation(&tmp, 0, 0, focalpoint);

	SetUpMatricesForEffect(&ScaleBy5, &tmp, (D3DXMATRIX*)g_Camera.GetProjMatrix(), &g_pEffect);
	
	g_pEffect->SetTechnique("RenderFocalPlane");
	pd3dDevice->SetVertexDeclaration(g_pVertexDeclaration);
	pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(MYVERTEX));
	DrawQuad(pd3dDevice);
}

HRESULT	LoadMeshHelperFunction(CDXUTMesh* mesh, tstring meshFile, IDirect3DDevice9* pd3dDevice)
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
	{
        return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;
	}
	return S_OK;
}

HRESULT DrawQuad(IDirect3DDevice9* pd3dDevice)
{
	pd3dDevice->SetVertexDeclaration(g_pVertexDeclaration);
	pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(MYVERTEX));

	UINT iPass, iPasses;
	g_pEffect->Begin(&iPasses, 0);
	for(iPass = 0; iPass < iPasses; iPass++)
		{
			g_pEffect->BeginPass(iPass);
			pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
			g_pEffect->EndPass();
		}
	g_pEffect->End();
	return S_OK;
}

void UpdateConvergence()
{
	int convergence = g_HUD.GetSlider(IDC_CHANGECONVERGENCE)->GetValue();
	//Set actual convergence
	g_pStereoAPI->SetConvergence((float)convergence);

	//Update stats
	TCHAR sz[100];
	_sntprintf( sz, 100, TEXT("Convergence at %d"), (int)convergence); sz[99] = 0;
	if(CDXUTStatic* convergetext = g_HUD.GetStatic(IDC_CHANGECONVERGENCE_STATIC))
		convergetext->SetText(sz);
}
