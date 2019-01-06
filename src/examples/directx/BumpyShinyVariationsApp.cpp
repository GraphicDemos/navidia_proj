#define STRICT
#include "BumpyShinyVariations.h"
#include "BumpyShinyVariationsApp.h"
#include <shared/NVFileDialog.h>

#include <DXUT/SDKmisc.h>
#define TOGGLE(p) p = !p
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
	DXUTSetCallbackD3D9DeviceFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( _T("Bumpy Shiny Variations 1.0 (DX9) (HLSL)") );
    DXUTCreateDevice(  true, 640, 576 );

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
	g_pBSV = new BumpyShinyVariations();

    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 140, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 140, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 140, 22, VK_F2 );

	g_HUD.AddButton( IDC_CYCLE, L"Cycle through combinations", 0, 0, 150, 22 );
	g_HUD.AddButton( IDC_TOGGLE_SPEC, L"Toggle Specular Highlights", 0, 0, 150, 22 );
	g_HUD.AddButton( IDC_TOGGLE_BUMP, L"Toggle Bump Mapping", 0, 0, 150, 22 );
	g_HUD.AddButton( IDC_TOGGLE_REFL, L"Toggle Reflection Mapping", 0, 0, 150, 22 );

	g_HUD.AddButton( IDC_TOGGLE_ROTATE, L"Toggle Rotate World (R)", 0, 0, 140, 22, L'R' );
	g_HUD.AddButton( IDC_TOGGLE_WIRE, L"Toggle Wireframe (W)", 0, 0, 140, 22, L'W' );

	g_HUD.AddButton( IDC_PAUSE_SCENE, L"Pause Scene (Space)", 0, 0, 140, 22, L' ' );

	g_HUD.AddButton( IDC_DRAW_NORMALS, L"Draw Normals (N)", 0, 0, 140, 22, L'N' );
	g_HUD.AddButton( IDC_DRAW_TANGENT, L"Draw Tangent Basis (T)", 0, 0, 140, 22, L'T' );
	g_HUD.AddButton( IDC_DRAW_COORDS, L"Draw Coordinate Axis (X)", 0, 0, 140, 22, L'X' );
	g_HUD.AddButton( IDC_DRAW_LIGHT, L"Draw Light (G)", 0, 0, 140, 22, L'G' );

	g_HUD.AddButton( IDC_LOAD_DIFFUSE, L"Load Diffuse Map", 0, 0, 125, 22 );
	g_HUD.AddButton( IDC_LOAD_NORMAL, L"Load Normal Map", 0, 0, 125, 22 );
	g_HUD.AddButton( IDC_LOAD_CUBE, L"Load Cube Map", 0, 0, 125, 22 );
	g_HUD.AddButton( IDC_LOAD_SCENE, L"Load Scene", 0, 0, 125, 22 );

	g_HUD.AddButton( IDC_DEFAULT_DIFFUSE, L"Load Default Diffuse Map", 0, 0, 145, 22 );
	g_HUD.AddButton( IDC_DEFAULT_NORMAL, L"Load Default Normal Map", 0, 0, 145, 22 );

	g_HUD.AddSlider( IDC_SLIDER_BUMP, 0, 0, 170, 22, 0, 200, 100 );
	TCHAR sz[33];
	_sntprintf( sz, 33, L"Environment Map Bump Scale: %.2f", g_pBSV->m_fBumpScale = ((float)g_HUD.GetSlider(IDC_SLIDER_BUMP)->GetValue()/100.0f));
	sz[32] = 0;
	g_HUD.AddStatic( IDC_STATIC_BUMP, sz, 0, 0, 170, 22 );

}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return SUCCEEDED(g_pBSV->ConfirmDevice( pCaps, 0, AdapterFormat, BackBufferFormat ));
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


    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );
	g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );

	g_HUD.GetControl( IDC_PAUSE_SCENE )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 + 24 );
	g_HUD.GetControl( IDC_TOGGLE_ROTATE )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );

	g_HUD.GetControl( IDC_DRAW_NORMALS )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 + 24 );
	g_HUD.GetControl( IDC_DRAW_TANGENT )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );
	g_HUD.GetControl( IDC_DRAW_COORDS )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );
	g_HUD.GetControl( IDC_DRAW_LIGHT )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );
	g_HUD.GetControl( IDC_TOGGLE_WIRE )->SetLocation( pBackBufferSurfaceDesc->Width - 150, iY += 24 );

	g_HUD.GetControl( IDC_LOAD_SCENE )->SetLocation( 5, pBackBufferSurfaceDesc->Height - (iY = 27) );
	g_HUD.GetControl( IDC_LOAD_CUBE )->SetLocation( 5, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_LOAD_NORMAL )->SetLocation( 5, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_DEFAULT_NORMAL )->SetLocation( 5 + 135, pBackBufferSurfaceDesc->Height - iY );
	g_HUD.GetControl( IDC_LOAD_DIFFUSE )->SetLocation( 5, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_DEFAULT_DIFFUSE )->SetLocation( 5 + 135, pBackBufferSurfaceDesc->Height - iY );

	g_HUD.GetControl( IDC_TOGGLE_REFL )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - (iY = 27) );
	g_HUD.GetControl( IDC_SLIDER_BUMP )->SetLocation( pBackBufferSurfaceDesc->Width - 180 - 160 - 10, pBackBufferSurfaceDesc->Height - iY );
	g_HUD.GetControl( IDC_STATIC_BUMP )->SetLocation( pBackBufferSurfaceDesc->Width - 180 - 160 - 10, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_TOGGLE_BUMP )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - (iY) );
	g_HUD.GetControl( IDC_TOGGLE_SPEC )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_CYCLE )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - (iY += 24) );

	g_HUD.GetControl( IDC_SLIDER_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
	g_HUD.GetControl( IDC_STATIC_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
	
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pBSV->RestoreDeviceObjects(pd3dDevice);

	// Set up our view matrix. A view matrix can be defined given an eye point and
	// a point to lookat. Here, we set the eye five units back along the z-axis and 
	// up three units and look at the origin.
	D3DXVECTOR3 vFromPt   = D3DXVECTOR3(10.938984f, -1.3918809f, -21.788483f);//(0.0f, 0.0f, 1.0f) * (-1.5f*g_pBSV->m_Scene->m_Radius);
	D3DXVECTOR3 vLookatPt = g_pBSV->m_Scene->m_Center;
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
	g_pBSV->m_View = *g_Camera.GetViewMatrix();

	// Setup the camera's projection parameters
	float zFar = 15.0f*g_pBSV->m_Scene->m_Radius;
	float zNear = 0.5f;
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DXToRadian(45.0f), fAspectRatio, zNear, zFar );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_pBSV->m_Projection = *g_Camera.GetProjMatrix();

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
    
    HRESULT hr;

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
		g_pBSV->m_View = *g_Camera.GetViewMatrix();
		g_pBSV->m_World = *g_Camera.GetWorldMatrix();

		g_pBSV->Render(pd3dDevice);

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

    // Output statistics
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
		txtHelper.DrawTextLine( TEXT("H        - Toggle UI") );
		txtHelper.DrawTextLine( TEXT("ESC   - Quit") );
		txtHelper.DrawTextLine( TEXT("Left Mouse  - Move Scene") );
		txtHelper.DrawTextLine( TEXT("Right Mouse - Move Camera") );
		txtHelper.DrawTextLine( TEXT("Mouse Wheel - Zoom") );
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
			{
				g_bShowHelp = !g_bShowHelp;
				break;
			}
		case 'H':
		case 'h':
			{
				if( g_bShowUI = !g_bShowUI )
					for( int i = 0; i < IDC_LAST; i++ )
						g_HUD.GetControl(i)->SetVisible( true );
				else
					for( int i = 0; i < IDC_LAST; i++ )
						g_HUD.GetControl(i)->SetVisible( false );
				break;
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:	DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:			DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:      g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
        case IDC_TOGGLE_SPEC:		TOGGLE(g_pBSV->m_bSpecular); break;
        case IDC_TOGGLE_BUMP:		
			{
				TOGGLE(g_pBSV->m_bBump); 
				g_HUD.GetControl( IDC_SLIDER_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				g_HUD.GetControl( IDC_STATIC_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				break;
			}
        case IDC_TOGGLE_REFL:
			{
				TOGGLE(g_pBSV->m_bEnvMap); 
				g_HUD.GetControl( IDC_SLIDER_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				g_HUD.GetControl( IDC_STATIC_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				break;
			}
        case IDC_TOGGLE_ROTATE:		TOGGLE(g_pBSV->m_bAutoRotateWorld); break;
		case IDC_TOGGLE_WIRE:		TOGGLE(g_pBSV->m_Wireframe); break;
        case IDC_PAUSE_SCENE:		TOGGLE(g_pBSV->m_PauseScene); break;
        case IDC_DRAW_NORMALS:		TOGGLE(g_pBSV->m_DrawNormals); break;
        case IDC_DRAW_TANGENT:		TOGGLE(g_pBSV->m_DrawTangentBasis); break;
		case IDC_DRAW_COORDS:		TOGGLE(g_pBSV->m_DrawAxis); break;
        case IDC_DRAW_LIGHT:		TOGGLE(g_pBSV->m_DrawLight); break;
        case IDC_CYCLE:
			{
				int t = ((int)g_pBSV->m_bEnvMap << 2) | ((int)g_pBSV->m_bBump << 1) | ((int)g_pBSV->m_bSpecular << 0);
				t = (t + 1) % 8;
				g_pBSV->m_bEnvMap   = (bool)((t >> 2) & 1);
				g_pBSV->m_bBump     = (bool)((t >> 1) & 1);
				g_pBSV->m_bSpecular = (bool)((t >> 0) & 1);
				g_HUD.GetControl( IDC_SLIDER_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				g_HUD.GetControl( IDC_STATIC_BUMP )->SetEnabled(g_pBSV->m_bEnvMap && g_pBSV->m_bBump);
				break;
			}
        case IDC_LOAD_DIFFUSE:
			{
				LPDIRECT3DDEVICE9 tmpd3dDevice = DXUTGetD3D9Device();
				NVTextureFileDialog dialog;
				tstring filename;
				LPDIRECT3DTEXTURE9 diffuseMap = 0;
				if (dialog.Open(filename))
					if (FAILED(g_pBSV->LoadTexture(tmpd3dDevice, filename, diffuseMap)))
						FDebug(_T("Diffuse map %s not loaded\n"), filename.c_str());
					else {
						SAFE_RELEASE(g_pBSV->m_DiffuseMap);
						g_pBSV->m_DiffuseMap= diffuseMap;
						g_pBSV->m_DiffuseMapFilename = filename;
						FDebug(_T("Diffuse map %s loaded\n"), g_pBSV->m_DiffuseMapFilename.c_str());
					}
				break;
			}
        case IDC_LOAD_NORMAL:
			{
				LPDIRECT3DDEVICE9 tmpd3dDevice = DXUTGetD3D9Device();
				NVTextureFileDialog dialog;
				tstring filename;
				LPDIRECT3DTEXTURE9 normalMap = 0;
				if (dialog.Open(filename))
					if (FAILED(g_pBSV->LoadTexture(tmpd3dDevice, filename, normalMap)))
						FDebug(_T("Normal map %s loaded\n"), filename.c_str());
					else {
						SAFE_RELEASE(g_pBSV->m_NormalMap);
						g_pBSV->m_NormalMap = normalMap;
						g_pBSV->m_NormalMapFilename = filename;
						FDebug(_T("Normal map %s loaded\n"), g_pBSV->m_NormalMapFilename.c_str());
					}
				break;
			}
        case IDC_LOAD_CUBE:
			{
				LPDIRECT3DDEVICE9 tmpd3dDevice = DXUTGetD3D9Device();
				NVTextureFileDialog dialog;
				tstring filename;
				LPDIRECT3DCUBETEXTURE9 cubeMap = 0;
				if (dialog.Open(filename))
					if (FAILED(g_pBSV->LoadTexture(tmpd3dDevice, filename, cubeMap)))
						FDebug(_T("Cube map %s loaded\n"), filename.c_str());
					else {
						SAFE_RELEASE(g_pBSV->m_CubeMap);
						g_pBSV->m_CubeMap = cubeMap;
						g_pBSV->m_CubeMapFilename = filename;
						FDebug(_T("Cube map %s loaded\n"), g_pBSV->m_CubeMapFilename.c_str());
					}
				break;
			}
        case IDC_LOAD_SCENE:
			{
				LPDIRECT3DDEVICE9 tmpd3dDevice = DXUTGetD3D9Device();
				NVFileDialog dialog;
				NVBScene* scene = new NVBScene;
				tstring filename;
				if (dialog.Open(filename))
					if (FAILED(scene->Load(filename, tmpd3dDevice, GetFilePath::GetFilePath))) {
						FDebug(_T("Scene %s not loaded: %s\n"), filename.c_str(), scene->m_ErrorMessage.c_str());
						delete scene;
					}
					else {
						delete g_pBSV->m_Scene;
						g_pBSV->m_Scene = scene;
						g_pBSV->m_SceneFilename = filename;
						g_pBSV->RestoreDeviceObjects(tmpd3dDevice);
						FDebug(_T("Scene %s loaded\n"), g_pBSV->m_SceneFilename.c_str());
					}
				break;
			}
        case IDC_DEFAULT_DIFFUSE:	TOGGLE(g_pBSV->m_UseDefaultDiffuseMap); break;
        case IDC_DEFAULT_NORMAL:	TOGGLE(g_pBSV->m_UseDefaultNormalMap); break;
        case IDC_SLIDER_BUMP:
			{
				TCHAR sz[33];
				_sntprintf( sz, 33, L"Environment Map Bump Scale: %.2f", g_pBSV->m_fBumpScale = ((float)g_HUD.GetSlider(IDC_SLIDER_BUMP)->GetValue()/100.0f));
				sz[32] = 0;
				g_HUD.GetStatic( IDC_STATIC_BUMP )->SetText(sz);
				break;
			}
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

	SAFE_RELEASE(g_pTextSprite);
	
	g_pBSV->InvalidateDeviceObjects();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

    SAFE_RELEASE(g_pFont);
}
