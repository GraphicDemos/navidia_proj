	#define STRICT
#include "nvafx.h"
#include "AntiAliasingWithTransparencyApp.h"
#include "shared\GetFilePath.h"

#include <Sdkmisc.h>


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
	DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
	DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
	DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceFrameMove(OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"AntiAliasingWithTransparency" );
    DXUTCreateDevice( true, 512, 512 );

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
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );
	g_HUD.AddSlider( IDC_ALPHASCALE, 100, 10, 160, 22, 0, 4000,2000);
	TCHAR sz[100];

	_sntprintf( sz, 100, TEXT("Alpha Scale: %.2f"), (float)(g_HUD.GetSlider(IDC_ALPHASCALE)->GetValue() / 1000.0)); sz[99] = 0;

	g_HUD.AddStatic(IDC_ALPHASCALETEXT,sz,400,0,100,22);
	g_HUD.AddCheckBox(IDC_CLOSEUP, L"Close Up", 0, 0, 200, 20, false, L' ');
	g_HUD.AddCheckBox(IDC_ATOC, L"Transparency Multisampiling", 0, 0, 200, 20, false, L' ');
	g_HUD.AddCheckBox(IDC_SSAA, L"Transparency Supersampiling", 0, 0, 200, 20, false, L' ');
	g_HUD.AddCheckBox(IDC_ALPHABLENDING, L"Alpha Blending", 0, 0, 200, 20, false, L' ');
	g_HUD.AddCheckBox(IDC_ANIMATE, L"Animate", 0, 0, 200, 20, true, L' ');
	g_HUD.AddCheckBox(IDC_ALPHATEST, L"Alpha Test", 0, 0, 200, 20, true, L' ');
	g_HUD.AddCheckBox(IDC_AA, L"4x AA", 0, 0, 200, 20, true, L' ');

}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{

    // TODO: Perform checks to see if these display caps are acceptable.

    // check pixel shader support 
    if (pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
	{
            MessageBox(NULL, _T("Device does not support 2.0 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return false;
	}

	if (pCaps->VertexShaderVersion < D3DVS_VERSION(3,0))
	{
            MessageBox(NULL, _T("Device does not support 3.0 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return false;
	}

    return true;
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
	pDeviceSettings->d3d9.pp.MultiSampleQuality = 0;
	pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D24S8;


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

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Set up our view matrix. A view matrix can be defined given an eye point and
    // a point to lookat.
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(-20.0f, 98.0f, -40.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(-20.0f, 98.0f, 100.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
	g_Camera.SetModelCenter(D3DXVECTOR3(0.0f, 0.0f, 0.0f));

	g_Rotations[0] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, -0.99318486f, -0.11655048f, 0.00000000f,
	0.00000000f, 0.11655048f, -0.99318486f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[1] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, -0.61987460f, -0.78470099f, 0.00000000f,
	0.00000000f, 0.78470099f, -0.61987460f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[2] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, 0.11654922f, -0.99318504f, 0.00000000f,
	0.00000000f, 0.99318504f, 0.11654922f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[3] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, 0.78470021f, -0.61987561f, 0.00000000f,
	0.00000000f, 0.61987561f, 0.78470021f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[4] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, 0.99318516f, 0.11654797f, 0.00000000f,
	0.00000000f, -0.11654797f, 0.99318516f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[5] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, 0.61987662f, 0.78469944f, 0.00000000f,
	0.00000000f, -0.78469944f, 0.61987662f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[6] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, -0.11654669f, 0.99318540f, 0.00000000f,
	0.00000000f, -0.99318540f, -0.11654669f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	g_Rotations[7] = D3DXMATRIX(
	1.0000000f, 0.00000000f, 0.00000000f, 0.00000000f,
	0.00000000f, -0.78469872f, 0.61987770f, 0.00000000f,
	0.00000000f, -0.61987770f, -0.78469872f, 0.00000000f,
	0.00000000f, 0.00000000f, 0.00000000f, 1.0000000f);

	IDirect3D9* pd3d;
	pd3dDevice->GetDirect3D(&pd3d);
	//Check for support of high quality AA modes
	g_bATOCSupport = (pd3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')) == S_OK);
	g_bSSAASupport = (pd3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK);
	SAFE_RELEASE(pd3d);
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

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

	iY = pBackBufferSurfaceDesc->Height;
	g_HUD.GetControl( IDC_ALPHASCALETEXT )->SetLocation( pBackBufferSurfaceDesc->Width - 260, iY -= 24);
	g_HUD.GetControl( IDC_ALPHASCALE )->SetLocation( pBackBufferSurfaceDesc->Width - 160, iY);

	g_HUD.GetControl( IDC_ATOC )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_SSAA )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_ALPHABLENDING )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_ALPHATEST )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_ANIMATE )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_AA )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
	g_HUD.GetControl( IDC_CLOSEUP )->SetLocation( pBackBufferSurfaceDesc->Width - 170, iY -= 24);
				
    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pRenderer = new AntiAliasingWithTransparency(pd3dDevice);

	g_pSkybox->Init(pd3dDevice);
	V(D3DXCreateCubeTextureFromFileEx(pd3dDevice, GetFilePath::GetFilePath(TEXT("MEDIA/models/SnowAccumulation/CloudyHillsCubemap.dds")).c_str(),D3DX_DEFAULT,0,0,D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,D3DX_FILTER_LINEAR,
		D3DX_FILTER_LINEAR,0,NULL,NULL,&g_pCubeTex));
	g_pSkybox->SetCubeMap(g_pCubeTex);
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
	if(g_iReset)
	{
		IDirect3DSurface9* pBB = NULL;
		D3DSURFACE_DESC BBDesc;
		pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB);
		pBB->GetDesc(&BBDesc);
		SAFE_RELEASE(pBB);

		IDirect3DSwapChain9 *pSwapChain = NULL;
		D3DPRESENT_PARAMETERS PresentParams;
		pd3dDevice->GetSwapChain(0, &pSwapChain);
		pSwapChain->GetPresentParameters(&PresentParams);
		SAFE_RELEASE(pSwapChain);
		
		(g_iReset == 2) ? PresentParams.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES:PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
		g_iReset = 0;

		OnLostDevice(pUserContext);
		pd3dDevice->Reset(&PresentParams);
		OnResetDevice(pd3dDevice, &BBDesc, pUserContext);
	}
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{

	D3DFORMAT AAFmt = D3DFMT_UNKNOWN;

	float fAlphaScale = 1.0f;
	bool buseATOC = (g_HUD.GetCheckBox(IDC_ATOC)->GetChecked());

	if(buseATOC)
	{
		AAFmt = (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C');
		fAlphaScale = (float)(g_HUD.GetSlider(IDC_ALPHASCALE)->GetValue() / 1000.0);
	}
	else if(g_HUD.GetCheckBox(IDC_SSAA)->GetChecked())
		AAFmt = (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A');

	g_HUD.GetControl(IDC_ALPHASCALE)->SetVisible(buseATOC);
	g_HUD.GetControl(IDC_ALPHASCALETEXT)->SetVisible(buseATOC);

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

	static double myTime = fTime;
	float myElapsedTime;
	if(g_HUD.GetCheckBox(IDC_ANIMATE)->GetChecked())
	{
		myTime = fTime;
		myElapsedTime = fElapsedTime;
	}
	else
	{
		myElapsedTime = 0.0f;
	}

    HRESULT hr;
    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER  | D3DCLEAR_STENCIL,
                        0xFFFFFFFF, 1.0f, 0L);

	D3DXMATRIX view;
	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	D3DXMatrixMultiply(&view, &id, (D3DXMATRIX*)g_Camera.GetViewMatrix());

	// Begin the scene
	bool bCloseUp = g_HUD.GetCheckBox(IDC_CLOSEUP)->GetChecked();


    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		if(bCloseUp)
			g_pRenderer->RenderAlphaTested(AAFmt, g_HUD.GetCheckBox(IDC_ALPHATEST)->GetChecked(), g_HUD.GetCheckBox(IDC_ALPHABLENDING)->GetChecked(), fAlphaScale, myTime, bCloseUp);
		else
		{
			static D3DXMATRIX constantrot = id;
			D3DXMATRIX rotation = id;
			D3DXMatrixRotationX(&rotation, -myElapsedTime / 8.0f);
			D3DXMatrixMultiply(&constantrot, &constantrot, &rotation);
			rotation = constantrot;

			g_pSkybox->Render(pd3dDevice, (D3DXMATRIXA16)view);
			for(int i = 0; i < 8; ++i)
			{
				D3DCULL cullmode = D3DCULL_CCW;
				D3DXMATRIX rot;
				D3DXMatrixMultiply(&rot, &g_Rotations[i], &rotation);
				D3DXMATRIX temp, temp2;
				temp2 = id;
				if(i % 2 ==  0)
				{
					temp2._13 =- temp2._13;
					temp2._23 =- temp2._23;
					temp2._33 =- temp2._33;
					cullmode = D3DCULL_CW;
				}
				D3DXMatrixTranslation(&temp, 0.0f, 24.49f, 0.0f);
				D3DXMatrixMultiply(&temp, &temp2, &temp);
				D3DXMatrixMultiply(&temp, &temp, &rot);

				g_pRenderer->UpdateMatrices(&temp, &view, (D3DXMATRIX*)g_Camera.GetProjMatrix());
				g_pRenderer->RenderOpaque(cullmode);
				g_pRenderer->RenderAlphaTested(AAFmt, g_HUD.GetCheckBox(IDC_ALPHATEST)->GetChecked(), g_HUD.GetCheckBox(IDC_ALPHABLENDING)->GetChecked(), fAlphaScale, myTime, bCloseUp);
			}
		}


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
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

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
	TCHAR sz[100];

	switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;


		case IDC_ALPHATEST:
			g_HUD.GetCheckBox(IDC_ALPHABLENDING)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_ATOC)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_SSAA)->SetChecked(false);
			break;

		case IDC_ALPHABLENDING:
			g_HUD.GetCheckBox(IDC_ALPHATEST)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_ATOC)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_SSAA)->SetChecked(false);
			break;


		case IDC_ALPHASCALE:
			_sntprintf( sz, 100, TEXT("Alpha Scale: %.2f"), (float)(g_HUD.GetSlider(IDC_ALPHASCALE)->GetValue() / 1000.0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_ALPHASCALETEXT)->SetText(sz);
			break;


		case IDC_ATOC: 
			if(!g_bATOCSupport || !g_HUD.GetCheckBox(IDC_AA)->GetChecked())
			{
				g_HUD.GetCheckBox(IDC_ATOC)->SetChecked(false);
				break;
			}
			g_HUD.GetCheckBox(IDC_ALPHATEST)->SetChecked(true);
			g_HUD.GetCheckBox(IDC_ALPHABLENDING)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_SSAA)->SetChecked(false);
			break;

		case IDC_SSAA: 
			if(!g_bSSAASupport || !g_HUD.GetCheckBox(IDC_AA)->GetChecked())
			{
				g_HUD.GetCheckBox(IDC_SSAA)->SetChecked(false);
				break;
			}
			g_HUD.GetCheckBox(IDC_ALPHATEST)->SetChecked(true);
			g_HUD.GetCheckBox(IDC_ALPHABLENDING)->SetChecked(false);
			g_HUD.GetCheckBox(IDC_ATOC)->SetChecked(false);
			break;

		case IDC_AA:
			if(!g_HUD.GetCheckBox(IDC_AA)->GetChecked())
			{
				g_HUD.GetCheckBox(IDC_ATOC)->SetChecked(false);
				g_HUD.GetCheckBox(IDC_SSAA)->SetChecked(false);
				g_iReset = 1;
			}
			else
				g_iReset = 2;
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
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

    if( g_pFont )
        g_pFont->OnLostDevice();

	g_pSkybox->Destroy();
	SAFE_DELETE(g_pRenderer);
	SAFE_RELEASE(g_pTextSprite);
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

    SAFE_RELEASE(g_pFont);
}