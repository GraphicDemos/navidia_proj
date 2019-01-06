#define STRICT
#include "dxstdafx.h"
#include <DXUT/SDKmisc.h>

#include "PracticalPSM.h"
#include "PracticalPSMApp.h"

#define SELECT_TEXT "Shadow Map Type"

#define DELTA_TEXT "Focus Region Distance: %.2f"
#define DELTA_ALG g_pPSM->m_fTSM_Delta = ((float)g_HUD.GetSlider(IDC_DELTA)->GetValue())/200.f

#define SLOPE_TEXT "Slope Scale Bias: %.2f"
#define SLOPE_ALG g_pPSM->m_fBiasSlope = ((float)g_HUD.GetSlider(IDC_SLOPE)->GetValue())/100.0f

#define DEPTH_TEXT "Depth Bias: %8d"
#define DEPTH_ALG g_pPSM->m_iDepthBias = (int)((float)g_HUD.GetSlider(IDC_DEPTH)->GetValue())

#define INFINITYZ_TEXT "InfinityZ: %.3f"
#define INFINITYZ_ALG g_pPSM->m_fMinInfinityZ = ((float)g_HUD.GetSlider(IDC_INFINITYZ)->GetValue())/1000.0f

#define NOPT_TEXT "Nopt Weight: %.3f"
#define NOPT_ALG g_pPSM->m_fLSPSM_NoptWeight = ((float)g_HUD.GetSlider(IDC_NOPT)->GetValue())/1000.f

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
    DXUTInit( true, true,NULL, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"Advanced Shadow Maps 1.0 (DX9) (HLSL)" );
    DXUTCreateDevice(  true, 800, 600 );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Creates/Sets the modal controls
//--------------------------------------------------------------------------------------

void PositionModalControls( int currMode, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    switch ( currMode )
    {
    case (int)SHADOWTYPE_PSM:
        g_HUD.GetControl( IDC_UNITCUBECLIP )->SetLocation( 120, pBackBufferSurfaceDesc->Height - 34 );
        g_HUD.GetControl( IDC_INFINITYZ )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 50);
        g_HUD.GetControl( IDC_INFINITYZ_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 30);
        g_HUD.GetControl( IDC_INFINITYZ_BUTTON )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 80);
        break;
    case (int)SHADOWTYPE_LSPSM:
        g_HUD.GetControl( IDC_NOPT )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 50);
        g_HUD.GetControl( IDC_NOPT_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 30);
        g_HUD.GetControl( IDC_UNITCUBECLIP )->SetLocation( 120, pBackBufferSurfaceDesc->Height - 34 );
        break;
    case (int)SHADOWTYPE_TSM:
        g_HUD.GetControl( IDC_DELTA )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 50);
        g_HUD.GetControl( IDC_DELTA_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 160, pBackBufferSurfaceDesc->Height - 30);
        g_HUD.GetControl( IDC_UNITCUBECLIP )->SetLocation( 120, pBackBufferSurfaceDesc->Height - 34 );
        break;
    case (int)SHADOWTYPE_ORTHO:
        g_HUD.GetControl( IDC_UNITCUBECLIP )->SetLocation( 120, pBackBufferSurfaceDesc->Height - 34 );
        break;
    }
}

void ChangeModalControls( int currMode, int nextMode )
{
    assert( g_pPSM != NULL );
    if ( currMode == nextMode )
        return;

    switch ( currMode )
    {
    case (int)SHADOWTYPE_PSM:
        g_HUD.RemoveControl( IDC_INFINITYZ_STATIC );
        g_HUD.RemoveControl( IDC_INFINITYZ_BUTTON );
        g_HUD.RemoveControl( IDC_INFINITYZ );
        g_HUD.RemoveControl( IDC_UNITCUBECLIP );
        break;
    case (int)SHADOWTYPE_LSPSM:
        g_HUD.RemoveControl( IDC_NOPT_STATIC );
        g_HUD.RemoveControl( IDC_NOPT );
        g_HUD.RemoveControl( IDC_UNITCUBECLIP );
        break;
    case (int)SHADOWTYPE_TSM:
        g_HUD.RemoveControl( IDC_DELTA );
        g_HUD.RemoveControl( IDC_UNITCUBECLIP );
        g_HUD.RemoveControl( IDC_DELTA_STATIC );
        break;
    case (int)SHADOWTYPE_ORTHO:
        g_HUD.RemoveControl( IDC_UNITCUBECLIP );
        break;
    }

    TCHAR sz[100];
    
    switch ( nextMode )
    {
    case (int)SHADOWTYPE_PSM:
	    g_HUD.AddSlider( IDC_INFINITYZ, 0, 0, 150, 22, 1008, 3000, (int)(g_pPSM->m_fMinInfinityZ*1000.f));
	    _sntprintf( sz, 100, TEXT(INFINITYZ_TEXT), INFINITYZ_ALG); sz[99] = 0;
	    g_HUD.AddStatic( IDC_INFINITYZ_STATIC,sz,0,0,140,22);
        g_HUD.AddButton( IDC_INFINITYZ_BUTTON, (g_pPSM->m_bSlideBack)?L"Disable InfinityZ (I)":L"Enable InfinityZ (I)", 0, 0, 150, 22, L'I'); //L"Enable InfinityZ (I)"
        g_HUD.AddButton( IDC_UNITCUBECLIP, (g_pPSM->m_bUnitCubeClip)?L"Disable Unit Cube Clipping (C)":L"Enable Unit Cube Clipping (C)", 0, 0, 190, 22, L'C'); //L"Enable Unit Cube Clipping (C)"
        break;
    case (int)SHADOWTYPE_LSPSM:
        g_HUD.AddSlider( IDC_NOPT, 0, 0, 150, 22, 0, 1000, (int)(g_pPSM->m_fLSPSM_NoptWeight*1000.f) );
        _sntprintf( sz, 100, TEXT(NOPT_TEXT), NOPT_ALG ); sz[99] = 0;
        g_HUD.AddStatic( IDC_NOPT_STATIC, sz, 0, 0, 140, 22 );
        g_HUD.AddButton( IDC_UNITCUBECLIP, (g_pPSM->m_bUnitCubeClip)?L"Disable Unit Cube Clipping (C)":L"Enable Unit Cube Clipping (C)", 0, 0, 190, 22, L'C');
        break;
    case (int)SHADOWTYPE_TSM:
        g_HUD.AddSlider( IDC_DELTA, 0, 0, 150, 22, 0, 200, (int)(g_pPSM->m_fTSM_Delta*200.f) );
        _sntprintf( sz, 100, TEXT(DELTA_TEXT), DELTA_ALG ); sz[99] = 0;
        g_HUD.AddStatic( IDC_DELTA_STATIC, sz, 0, 0, 140, 22 );
        g_HUD.AddButton( IDC_UNITCUBECLIP, (g_pPSM->m_bUnitCubeClip)?L"Disable Unit Cube Clipping (C)":L"Enable Unit Cube Clipping (C)", 0, 0, 190, 22, L'C'); //L"Enable Unit Cube Clipping (C)"
        break;
    case (int)SHADOWTYPE_ORTHO:
        g_HUD.AddButton( IDC_UNITCUBECLIP, (g_pPSM->m_bUnitCubeClip)?L"Disable Unit Cube Clipping (C)":L"Enable Unit Cube Clipping (C)", 0, 0, 190, 22, L'C'); //L"Enable Unit Cube Clipping (C)"
        break;
    }
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_pPSM = new PracticalPSM();
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, 82, 125, 22, VK_F2 );
	g_HUD.AddButton( IDC_RESET, L"Reset Camera (Home)", 0, 0, 125, 22);
	g_HUD.AddButton( IDC_DISPSHADOWMAP, L"Show Shadow Map (P)", 0, 0, 125, 22, L'P'); //L"Hide Shadow Map (P)"
	g_HUD.AddButton( IDC_ANIMATION, L"Pause (Space)", 0, 0, 125, 22, L' '); //L"Play (Space)"
	g_HUD.AddButton( IDC_LIGHTANI, L"Stop Light (L)", 0, 0, 125, 22, L'L'); //L"Start Light (L)"

	TCHAR sz[100];
	g_HUD.AddSlider( IDC_SLOPE, 0, 0, 150, 22, 0, 300, (int)(g_pPSM->m_fBiasSlope*100.f));
	_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
	g_HUD.AddStatic( IDC_SLOPE_STATIC,sz,0,0,140,22);

	g_HUD.AddSlider( IDC_DEPTH, 0, 0, 150, 22, -1000, 1000, g_pPSM->m_iDepthBias);
	_sntprintf( sz, 100, TEXT(DEPTH_TEXT), DEPTH_ALG); sz[99] = 0;
	g_HUD.AddStatic( IDC_DEPTH_STATIC,sz,0,0,140,22);

    g_HUD.AddComboBox( IDC_PERSORTHO, 0, 0, 125, 22, 0, false); //L"Using Ortho Shadow Map (M)"
    g_HUD.GetComboBox( IDC_PERSORTHO )->SetDropHeight(30);
    g_HUD.GetComboBox( IDC_PERSORTHO )->AddItem( L"PSM", (LPVOID)SHADOWTYPE_PSM );
    g_HUD.GetComboBox( IDC_PERSORTHO )->AddItem( L"LiSPSM", (LPVOID)SHADOWTYPE_LSPSM );
    g_HUD.GetComboBox( IDC_PERSORTHO )->AddItem( L"TSM", (LPVOID)SHADOWTYPE_TSM );
    g_HUD.GetComboBox( IDC_PERSORTHO )->AddItem( L"Ortho Shadow Map", (LPVOID)SHADOWTYPE_ORTHO );
    g_HUD.GetComboBox( IDC_PERSORTHO )->SetSelectedByData( (LPVOID)g_pPSM->m_iShadowType );


	g_HUD.AddButton( IDC_SHOWSTATS, L"Show Stats (F4)", 0, 0, 125, 22); //L"Hide Stats (F4)"
	g_HUD.AddButton( IDC_RANDOM, L"Randomize (R)", 0, 0, 125, 22, L'R');

    ChangeModalControls( -1, g_pPSM->m_iShadowType );
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return SUCCEEDED(g_pPSM->ConfirmDevice(pCaps,0,AdapterFormat,BackBufferFormat));
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

    // Set up our view matrix. A view matrix can be defined given an eye point and
    // a point to lookat. Here, we set the eye five units back along the z-axis and 
	// up three units and look at the origin.
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);

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
	float fAspectRatio = float(800) / float(600);
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 10.0f, 800.0f  );

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15*8;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( 5, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( 5, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( 5, iY += 24 );
    g_HUD.GetControl( IDC_SHOWSTATS )->SetLocation( 5, iY += 24 );

	g_HUD.GetControl( IDC_RESET )->SetLocation( 5, iY += 24 );
    g_HUD.GetControl( IDC_ANIMATION )->SetLocation( pBackBufferSurfaceDesc->Width - 135, pBackBufferSurfaceDesc->Height - (iY = 80+34));
    g_HUD.GetControl( IDC_LIGHTANI )->SetLocation( pBackBufferSurfaceDesc->Width - 135, pBackBufferSurfaceDesc->Height - (iY += 24));
    g_HUD.GetControl( IDC_RANDOM )->SetLocation( pBackBufferSurfaceDesc->Width - 135, pBackBufferSurfaceDesc->Height - (iY += 24));
    g_HUD.GetControl( IDC_DISPSHADOWMAP )->SetLocation( pBackBufferSurfaceDesc->Width - 135, pBackBufferSurfaceDesc->Height - (iY += 24) );
	g_HUD.GetControl( IDC_PERSORTHO )->SetLocation( pBackBufferSurfaceDesc->Width - 135, pBackBufferSurfaceDesc->Height - (iY += 24) );

	g_HUD.GetControl( IDC_SLOPE )->SetLocation( pBackBufferSurfaceDesc->Width - 480, pBackBufferSurfaceDesc->Height - 50);
    g_HUD.GetControl( IDC_SLOPE_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 480, pBackBufferSurfaceDesc->Height - 30);

    g_HUD.GetControl( IDC_DEPTH )->SetLocation( pBackBufferSurfaceDesc->Width - 320, pBackBufferSurfaceDesc->Height - 50);
    g_HUD.GetControl( IDC_DEPTH_STATIC )->SetLocation( pBackBufferSurfaceDesc->Width - 320, pBackBufferSurfaceDesc->Height - 30);

    PositionModalControls( g_pPSM->m_iShadowType, pBackBufferSurfaceDesc );
 
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	g_pPSM->RestoreDeviceObjects(pd3dDevice);

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

    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);

	g_pPSM->m_View = *g_Camera.GetViewMatrix();
	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        // TODO: render world
		g_pPSM->Render( pd3dDevice);
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
	}
	if (g_bShowStats)
    {
	    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );

		TCHAR buff[180];

        switch ( g_pPSM->m_iShadowType )
        {
        case (int)SHADOWTYPE_PSM:
            _stprintf (buff, _T("PSM Projection: %s"), (g_pPSM->m_bShadowTestInverted)?_T("Inverted"):_T("Regular"));
            txtHelper.DrawTextLine(buff);
            _stprintf (buff, _T("Near=%.3f, Far=%.3f, ppNear=%.5f, ppFar=%.5f, slideBack=%.3f"), g_pPSM->m_zNear, g_pPSM->m_zFar, 
			    g_pPSM->m_ppNear, g_pPSM->m_ppFar, g_pPSM->m_fSlideBack);
            txtHelper.DrawTextLine(buff);
            _stprintf (buff, _T("Unit Cube Clipping:  %s"), (g_pPSM->m_bUnitCubeClip)?_T("Enabled"):_T("Disabled"));
            txtHelper.DrawTextLine(buff);
            _stprintf (buff, _T("Virtual Slide Back:  %s"), (g_pPSM->m_bSlideBack)?_T("Enabled"):_T("Disabled"));
            txtHelper.DrawTextLine(buff);
            break;
        case (int)SHADOWTYPE_LSPSM:
            txtHelper.DrawTextLine(_T("LSPSM Projection"));
            _stprintf (buff, _T("cos(gamma): %.3f"), g_pPSM->m_fCosGamma);
            txtHelper.DrawTextLine(buff);
            _stprintf (buff, _T("Nopt: %.3f"), g_pPSM->m_fLSPSM_Nopt);
            txtHelper.DrawTextLine(buff);
            _stprintf (buff, _T("Unit Cube Clipping:  %s"), (g_pPSM->m_bUnitCubeClip)?_T("Enabled"):_T("Disabled"));
            txtHelper.DrawTextLine(buff);
            break;
        case (int)SHADOWTYPE_TSM:
            txtHelper.DrawTextLine(_T("Trapezoidal shadow map"));
            _stprintf (buff, _T("Unit Cube Clipping:  %s"), (g_pPSM->m_bUnitCubeClip)?_T("Enabled"):_T("Disabled"));
            txtHelper.DrawTextLine(buff);
            break;
        case (int)SHADOWTYPE_ORTHO:
            txtHelper.DrawTextLine(_T("Uniform shadow map"));
            _stprintf (buff, _T("Unit Cube Clipping:  %s"), (g_pPSM->m_bUnitCubeClip)?_T("Enabled"):_T("Disabled"));
            txtHelper.DrawTextLine(buff);
            break;
        }
    }

	if( !g_bShowHelp )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 for help" );
	}
	else
	{
		// Draw help
		txtHelper.SetInsertionPos( 5, pd3dsdBackBuffer->Height - 120);
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 to hide help" );
		txtHelper.DrawTextLine( L"Press ESC to quit" );
		txtHelper.DrawTextLine( L"Look: Left drag mouse" );
		txtHelper.DrawTextLine( L"Move: A,W,S,D\n" );
		txtHelper.DrawTextLine( L"Strafe up/down: Q,E\n" );
		txtHelper.DrawTextLine( L"Run: Shift" );
		txtHelper.DrawTextLine( L"Toggle UI: H" );
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
	if( bKeyDown)
	{
		switch ( nChar )
		{
		case VK_F1:
			{
				g_bShowHelp = !g_bShowHelp;
				break;
			}
		case VK_F4:
			{
				if(g_bShowStats = !g_bShowStats)
					g_HUD.GetButton(IDC_SHOWSTATS)->SetText(L"Hide Stats (F4)");
				else
					g_HUD.GetButton(IDC_SHOWSTATS)->SetText(L"Show Stats (F4)");
				break;
			}
		case VK_SHIFT:
			{
                g_Camera.SetScalers( 0.01f, 80);
				break;
			}
		case VK_HOME:
			{
				g_Camera.Reset();
				break;
			}
		case L'H':
		case L'h':
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
	else
	{
		g_Camera.SetScalers( 0.01f, 30);

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
		case IDC_RESET:
			{
				g_Camera.Reset();
				break;
			}
		case IDC_DISPSHADOWMAP:
			{
				if( g_pPSM->m_bDisplayShadowMap = !g_pPSM->m_bDisplayShadowMap )
					g_HUD.GetButton(IDC_DISPSHADOWMAP)->SetText(L"Hide Shadow Map (P)");
				else
					g_HUD.GetButton(IDC_DISPSHADOWMAP)->SetText(L"Show Shadow Map (P)");
				break;
			}
		case IDC_ANIMATION:
			{
				if( g_pPSM->m_Paused = !g_pPSM->m_Paused )
					g_HUD.GetButton(IDC_ANIMATION)->SetText(L"Play (Space)");
				else
					g_HUD.GetButton(IDC_ANIMATION)->SetText(L"Pause (Space)");
				break;
			}
		case IDC_LIGHTANI:
			{
				if( g_pPSM->m_bLightAnimation = !g_pPSM->m_bLightAnimation )
					g_HUD.GetButton(IDC_LIGHTANI)->SetText(L"Stop Light (L)");
				else
					g_HUD.GetButton(IDC_LIGHTANI)->SetText(L"Start Light (L)");
				break;
			}
		case IDC_SLOPE:
			{
				_sntprintf( sz, 100, TEXT(SLOPE_TEXT), SLOPE_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_SLOPE_STATIC)->SetText(sz);
				break;
			}
		case IDC_DEPTH:
			{
				_sntprintf( sz, 100, TEXT(DEPTH_TEXT), DEPTH_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_DEPTH_STATIC)->SetText(sz);
				break;
			}
		case IDC_INFINITYZ:
			{
				_sntprintf( sz, 100, TEXT(INFINITYZ_TEXT), INFINITYZ_ALG); sz[99] = 0;
				g_HUD.GetStatic(IDC_INFINITYZ_STATIC)->SetText(sz);
				break;
			}
        case IDC_DELTA:
            {
                _sntprintf( sz, 100, TEXT(DELTA_TEXT), DELTA_ALG ); sz[99] = 0;
                g_HUD.GetStatic(IDC_DELTA_STATIC)->SetText(sz);
                break;
            }
        case IDC_NOPT:
            {
                _sntprintf( sz, 100, TEXT(NOPT_TEXT), NOPT_ALG ); sz[99] = 0;
                g_HUD.GetStatic(IDC_NOPT_STATIC)->SetText(sz);
                break;
            }
		case IDC_INFINITYZ_BUTTON:
			{
				if( g_pPSM->m_bSlideBack = !g_pPSM->m_bSlideBack )
				{
					g_HUD.GetButton(IDC_INFINITYZ_BUTTON)->SetText(L"Disable InfinityZ (I)");
					g_HUD.GetSlider(IDC_INFINITYZ)->SetEnabled(true);
					g_HUD.GetStatic(IDC_INFINITYZ_STATIC)->SetEnabled(true);
				}
				else
				{
					g_HUD.GetButton(IDC_INFINITYZ_BUTTON)->SetText(L"Enable InfinityZ (I)");
					g_HUD.GetSlider(IDC_INFINITYZ)->SetEnabled(false);
					g_HUD.GetStatic(IDC_INFINITYZ_STATIC)->SetEnabled(false);
				}
				break;
			}
		case IDC_PERSORTHO:
			{
                int nextMode = (int)g_HUD.GetComboBox(IDC_PERSORTHO)->GetSelectedData();
                ChangeModalControls( g_pPSM->m_iShadowType, nextMode );
                g_pPSM->m_iShadowType = nextMode;
				PositionModalControls( g_pPSM->m_iShadowType, DXUTGetD3D9BackBufferSurfaceDesc() );
				break;
			}
		case IDC_UNITCUBECLIP:
			{
				if( g_pPSM->m_bUnitCubeClip = !g_pPSM->m_bUnitCubeClip )
					g_HUD.GetButton(IDC_UNITCUBECLIP)->SetText(L"Disable Unit Cube Clipping (C)");
				else
					g_HUD.GetButton(IDC_UNITCUBECLIP)->SetText(L"Enable Unit Cube Clipping (C)");
				break;
			}

		case IDC_SHOWSTATS:
			{
				if(g_bShowStats = !g_bShowStats)
					g_HUD.GetButton(IDC_SHOWSTATS)->SetText(L"Hide Stats (F4)");
				else
					g_HUD.GetButton(IDC_SHOWSTATS)->SetText(L"Show Stats (F4)");
				break;
			}
		case IDC_RANDOM:
			{
				g_pPSM->RandomizeObjects();
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

	g_pPSM->InvalidateDeviceObjects();
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