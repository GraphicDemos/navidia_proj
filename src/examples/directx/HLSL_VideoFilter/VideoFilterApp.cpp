#define STRICT


#include <DXUT.h>
#include <Sdkmisc.h>
#include "VideoFilterApp.h"
#include "video_flags.h"
// Here we want to define the location for a bunch of the controls, 
// so that it is a little bit easier to make changes quickly
#define PROCAMP_X 380
#define PROCAMP_Y 40 // move up by 40 pixels

#define COMPCTRL_X 380
#define COMPCTRL_Y -240 // let's move it down a little bit

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR lpszCmdParam, int nCmdShow)
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
    DXUTSetCallbackMsgProc        ( MsgProc );
    DXUTSetCallbackKeyboard       ( KeyboardProc );
	DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove      ( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp(lpszCmdParam);

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"Video Filter (DX9.0c) (HLSL)" );

    // Create Device on the Primary Adapter
    DXUTCreateDevice( true, 800, 720);
    // This works for DUALVIEW mode only.  Use "1" for Secondary Adapter, 
    // make resolution the same as the display setting.  false == FULL_SCREEN, true = Windowed
//    DXUTCreateDevice( 1, false, 1280, 1024, IsDeviceAcceptable, ModifyDeviceSettings );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}

void UpdateHiddenControls()
{
	CDXUTListBox *pList;
	CDXUTControl *pControl;
	int select[4] = { 0, 0, 0, 0 };
	bool bEnableCtrl;
	
	// If it's not Post Wipe, then we hide it
	pList = g_HUD.GetListBox( IDC_LB_FILTER1 );
	if (pList) select[0] = pList->GetSelectedIndex();
	pList = g_HUD.GetListBox( IDC_LB_FILTER2 );
	if (pList) select[1] = pList->GetSelectedIndex();
	pList = g_HUD.GetListBox( IDC_LB_COMPOSITE );
	if (pList) select[2] = pList->GetSelectedIndex();

	// Frost Filter
	bEnableCtrl = (bool)(select[0] == VideoFilter::FROST_FILTER || select[1] == VideoFilter::FROST_FILTER); 

	pControl = g_HUD.GetSlider(IDC_DELTAX);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_DELTAXTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	pControl = g_HUD.GetSlider(IDC_DELTAY);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_DELTAYTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	pControl = g_HUD.GetSlider(IDC_FREQ);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_FREQTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	// ORB Filter
	bEnableCtrl = (select[0] == VideoFilter::ORB_FILTER || select[1] == VideoFilter::ORB_FILTER); 

	pControl = g_HUD.GetSlider(IDC_RADIUS);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_RADIUSTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	pControl = g_HUD.GetSlider(IDC_EFFECTSCALE);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_EFFECTSCALETEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	// Radial Blur
	bEnableCtrl = (select[0] == VideoFilter::RADIAL_BLUR || select[1] == VideoFilter::RADIAL_BLUR);

	pControl = g_HUD.GetSlider(IDC_BLURSTART);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_BLURSTARTTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

	pControl = g_HUD.GetSlider(IDC_BLURWIDTH);
	if (pControl) pControl->SetVisible(bEnableCtrl);
	pControl = g_HUD.GetStatic(IDC_BLURWIDTHTEXT);
	if (pControl) pControl->SetVisible(bEnableCtrl);

    // Fade control
	pControl = g_HUD.GetSlider(IDC_FADE);
	if (pControl) pControl->SetVisible(select[2] == 0);
    pControl = g_HUD.GetStatic(IDC_FADETEXT);
	if (pControl) pControl->SetVisible(select[2] == 0);

    // Wipe control
	pControl = g_HUD.GetSlider(IDC_WIPECENTER);
	if (pControl) pControl->SetVisible(select[2] == 1);
    pControl = g_HUD.GetStatic(IDC_WIPECENTERTEXT);
	if (pControl) pControl->SetVisible(select[2] == 1);

    pControl = g_HUD.GetSlider(IDC_WIPESOFT);
    if (pControl) pControl->SetVisible(select[2] == 1);
    pControl = g_HUD.GetStatic(IDC_WIPESOFTTEXT);
    if (pControl) pControl->SetVisible(select[2] == 1);

    pControl = g_HUD.GetSlider(IDC_ANGLE);
    if (pControl) pControl->SetVisible(select[2] == 1);
    pControl = g_HUD.GetStatic(IDC_ANGLETEXT);
    if (pControl) pControl->SetVisible(select[2] == 1);

    pControl = g_HUD.GetSlider(IDC_SLANT);
    if (pControl) pControl->SetVisible(select[2] == 1);
    pControl = g_HUD.GetStatic(IDC_SLANTTEXT);
    if (pControl) pControl->SetVisible(select[2] == 1);
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp(LPSTR lpszCmdParam)
{
    TCHAR sz[100];

    g_pVideoFilterApp = new VideoFilter;
	g_pVideoFilterApp->SetCommandLineFiles(lpszCmdParam);
	
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent );
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Full Screen", 35, 14, 100, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF,        L"Toggle REF (F3)", 35, 38, 100, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE,     L"Device (F2)",     35, 62, 100, 22, VK_F2 );
    g_HUD.AddButton( IDC_RESET,            L"Defaults (F9)",   35, 86, 100, 22 );
    g_HUD.AddButton( IDC_WIREFRAME,        L"(W)ireframe",     35, 110,100, 22 );

    CDXUTListBox* pList = NULL;
    DWORD i, listbox;

    for (listbox=0; listbox < 2; listbox++) {
        i=0;

        g_HUD.AddListBox(((listbox==0) ? IDC_LB_FILTER1 : IDC_LB_FILTER2), 0, 0, 85, 125, 0, &pList );

	    pList->AddItem( L"3x3 Cone",   (void*)(i++));
	    pList->AddItem( L"3x3 Box",    (void*)(i++));
	    pList->AddItem( L"4x4 Box",    (void*)(i++));
	    pList->AddItem( L"Sharpen",    (void*)(i++));
	    pList->AddItem( L"Edge",       (void*)(i++));
	    pList->AddItem( L"Sepia",      (void*)(i++));
        pList->AddItem( L"RadialBlur", (void*)(i++));
//        pList->AddItem( L"lensORB",    (void*)(i++));
//        pList->AddItem( L"Frost",      (void*)(i++));
//        pList->AddItem( L"OldTV",      (void*)(i++));

        pList->SelectItem(0);
    }

    i=0;
    // These are post processing for compositing
    g_HUD.AddListBox(IDC_LB_COMPOSITE, 0, 0, 160, 60, 0, &pList);
    pList->AddItem( L"Post Blend",     (void*)(i++));
    pList->AddItem( L"Post Wipe",      (void*)(i++));
    pList->SelectItem(0);

    // Parameters for ProcAmp (Brightness, Contrast, Hue, Saturation)
    g_HUD.AddSlider( IDC_BRIGHT,           0, 0, 80, 22, 0, 500, 100 );
    _sntprintf( sz, 100, BRIGHT_TEXT, BRIGHT_ALG(0) ); sz[99] = 0;
    g_HUD.AddStatic( IDC_BRIGHTTEXT,   sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_CONTRAST,         0, 0, 80, 22, -500, 500, 100 );
    _sntprintf( sz, 100, CONTRAST_TEXT, CONTRAST_ALG(0) ); sz[99] = 0;
    g_HUD.AddStatic( IDC_CONTRASTTEXT, sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_SATURATE,         0, 0, 80, 22, -500, 500, 100 );
    _sntprintf( sz, 100, SATURATE_TEXT, SATURATE_ALG(0) ); sz[99] = 0;
    g_HUD.AddStatic( IDC_SATURATETEXT, sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_HUE,              0, 0, 80, 22, 0, 360, 0 );
    _sntprintf( sz, 100, HUE_TEXT, HUE_ALG(0) ); sz[99] = 0;
    g_HUD.AddStatic( IDC_HUETEXT,      sz, 0, 0, 80, 22 );

    // Parameters for frosted distortion
    g_HUD.AddSlider( IDC_DELTAX,     0, 0, 80, 22, 10, 300, 73 );
    _sntprintf( sz, 100, DELTAX_TEXT, DELTAX_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_DELTAXTEXT,      sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_DELTAY,     0, 0, 80, 22, 10, 300, 108 );
    _sntprintf( sz, 100, DELTAY_TEXT, DELTAY_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_DELTAYTEXT,      sz, 0, 0, 80, 22 );

	g_HUD.AddSlider( IDC_FREQ,     0, 0, 80, 22, 0, 200, 115 );
    _sntprintf( sz, 100, FREQ_TEXT, FREQ_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_FREQTEXT,      sz, 0, 0, 80, 22 );

	// Parameters for lense ORB distortion
    g_HUD.AddSlider( IDC_RADIUS,     0, 0, 80, 22, 0, 100, 60 );
    _sntprintf( sz, 100, RADIUS_TEXT, RADIUS_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_RADIUSTEXT,      sz, 0, 0, 80, 22 );

	g_HUD.AddSlider( IDC_EFFECTSCALE,     0, 0, 80, 22, 0, 100, 75 );
    _sntprintf( sz, 100, EFFECTSCALE_TEXT, EFFECTSCALE_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_EFFECTSCALETEXT,      sz, 0, 0, 80, 22 );

    // Parameters for Radial Blur
    g_HUD.AddSlider( IDC_BLURSTART,     0, 0, 80, 22, 0, 100, 100 );
    _sntprintf( sz, 100, BLURSTART_TEXT, BLURSTART_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_BLURSTARTTEXT,      sz, 0, 0, 80, 22 );

	g_HUD.AddSlider( IDC_BLURWIDTH,     0, 0, 80, 22, -100, 100, -20 );
    _sntprintf( sz, 100, BLURWIDTH_TEXT, BLURWIDTH_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_BLURWIDTHTEXT,      sz, 0, 0, 80, 22 );

	// Parameters for FADE compositing
    g_HUD.AddSlider( IDC_FADE,     0, 0, 80, 22, 0, 1000, 500 );
    _sntprintf( sz, 100, FADE_TEXT, FADE_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_FADETEXT,      sz, 0, 0, 80, 22 );

	// Parameters for WIPE compositing
    g_HUD.AddSlider( IDC_WIPECENTER,     0, 0, 80, 22, -500, 1500, 500 );
    _sntprintf( sz, 100, WIPECENTER_TEXT, WIPECENTER_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_WIPECENTERTEXT,      sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_WIPESOFT,       0, 0, 80, 22, 40, 500, 70 );
    _sntprintf( sz, 100, WIPESOFT_TEXT,   WIPESOFT_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_WIPESOFTTEXT,      sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_ANGLE,          0, 0, 80, 22, -900, 1800, 0 );
    _sntprintf( sz, 100, ANGLE_TEXT,      ANGLE_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_ANGLETEXT,      sz, 0, 0, 80, 22 );

    g_HUD.AddSlider( IDC_SLANT,          0, 0, 80, 22, -50, 50, 18 );
    _sntprintf( sz, 100, SLANT_TEXT,      SLANT_ALG ); sz[99] = 0;
    g_HUD.AddStatic( IDC_SLANTTEXT,      sz, 0, 0, 80, 22 );

    UpdateHiddenControls();

    // First set of Media Player Buttons
    g_HUD.AddButton( IDC_PLAYPAUSE1, L"Play",   35, 34, 64, 22 );
    g_HUD.AddButton( IDC_STOP1,      L"Stop",    35, 34, 64, 22 );
    g_HUD.AddSlider( IDC_SEEK1,             0, 0, 100, 22, 0, 100, 0 );

    _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->nSeekPos[0]), 
                                    SEEK_ALG(g_pVideoFilterApp->nDuration[0]) ); sz[99] = 0;
    g_HUD.AddStatic  ( IDC_SEEKTEXT1, sz, 0, 0, 100, 22 );
    g_HUD.AddCheckBox( IDC_LOOP1,     L"Enable Looping", 0, 0, 100, 22 );
    g_HUD.GetCheckBox( IDC_LOOP1 )->SetChecked(TRUE);

	// Second set of Media Player Buttons
	g_HUD.AddButton( IDC_PLAYPAUSE2, L"Play",   35, 34, 64, 22 );
    g_HUD.AddButton( IDC_STOP2,      L"Stop",    35, 34, 64, 22 );
    g_HUD.AddSlider( IDC_SEEK2,             0, 0, 100, 22, 0, 100, 0 );

    _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->nSeekPos[0]), 
                                    SEEK_ALG(g_pVideoFilterApp->nDuration[0]) ); sz[99] = 0;
    g_HUD.AddStatic  ( IDC_SEEKTEXT2, sz, 0, 0, 100, 22 );

    g_HUD.AddCheckBox( IDC_LOOP2,     L"Enable Looping", 0, 0, 100, 22 );
    g_HUD.GetCheckBox( IDC_LOOP2 )->SetChecked(TRUE);
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    BOOL bCapsAcceptable = TRUE;

    // Perform checks to see if these display caps are acceptable.
    bCapsAcceptable = (g_pVideoFilterApp->ConfirmDevice(pCaps, 0, AdapterFormat, BackBufferFormat) == S_OK);

    if (!bCapsAcceptable)
        return false;

    return true;
}


//struct DXUTDeviceSettings
//{
//    UINT AdapterOrdinal;
//    D3DDEVTYPE DeviceType;
//    D3DFORMAT AdapterFormat;
//    DWORD BehaviorFlags;
//    D3DPRESENT_PARAMETERS pp;
//};

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
#if REF_NEEDED
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif
    pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_MULTITHREADED;
    // uncomment this if desired to present the video immediately and not wait
    // for vsync.  This improves performance, at the expense of possible tearing
//    pDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

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

    // setup render states
    hr = g_pVideoFilterApp->RestoreDeviceObjects(pd3dDevice);
    if (hr != S_OK) {
        return E_FAIL;
    }

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

	g_HUD.GetControl( IDC_LB_FILTER1 )->SetLocation(pBackBufferSurfaceDesc->Width/2  - 95, 
                                                    pBackBufferSurfaceDesc->Height   - 135 );
	g_HUD.GetControl( IDC_LB_FILTER2 )->SetLocation(pBackBufferSurfaceDesc->Width/2  - 95, 
                                                    pBackBufferSurfaceDesc->Height/2 - 135 );

	g_HUD.GetControl( IDC_LB_COMPOSITE)->SetLocation(pBackBufferSurfaceDesc->Width * 6 / 10,
                                                    pBackBufferSurfaceDesc->Height/2 + 20);

    int iY = -14;
    for(int i = 0; i <= IDC_LAST; i++) {
       g_HUD.GetControl( i )->SetLocation( pBackBufferSurfaceDesc->Width/2 - 100, iY += 24 );
//       g_HUD.GetControl( i )->SetLocation( 10, iY += 24 );
    }

    // These buttons/sliders are used for Media Player Control
    g_HUD.GetControl(IDC_PLAYPAUSE1)->SetLocation   ( 10,
                                                     pBackBufferSurfaceDesc->Height- 54 );
    g_HUD.GetControl(IDC_STOP1)->SetLocation        ( 10,
                                                     pBackBufferSurfaceDesc->Height- 30 );

    g_HUD.GetControl(IDC_SEEK1)->SetLocation        ( 80,
                                                     pBackBufferSurfaceDesc->Height- 60 );
    g_HUD.GetControl(IDC_SEEKTEXT1)->SetLocation    ( 80,
                                                     pBackBufferSurfaceDesc->Height- 45 );
    g_HUD.GetControl(IDC_LOOP1)->SetLocation        ( 80,
                                                     pBackBufferSurfaceDesc->Height- 25 );

    // These buttons/sliders are used for Media Player Control
    g_HUD.GetControl(IDC_PLAYPAUSE2)->SetLocation   ( 10,
                                                     pBackBufferSurfaceDesc->Height/2- 54 );
    g_HUD.GetControl(IDC_STOP2)->SetLocation        ( 10,
                                                     pBackBufferSurfaceDesc->Height/2- 30 );

    g_HUD.GetControl(IDC_SEEK2)->SetLocation        ( 80,
                                                     pBackBufferSurfaceDesc->Height/2- 60 );
    g_HUD.GetControl(IDC_SEEKTEXT2)->SetLocation    ( 80,
                                                     pBackBufferSurfaceDesc->Height/2- 45 );
    g_HUD.GetControl(IDC_LOOP2)->SetLocation        ( 80,
                                                     pBackBufferSurfaceDesc->Height/2- 25);

    // These sliders are used for ProcAmp (Brightness, Contrast, Hue, Saturation)
	UINT width_start = (pBackBufferSurfaceDesc->Width * 3 / 4) + (200);
	UINT height_start= (pBackBufferSurfaceDesc->Height);

    g_HUD.GetControl( IDC_BRIGHT )->SetLocation    ( width_start -(PROCAMP_X + 0 ),
                                                     height_start-(PROCAMP_Y + 20));
    g_HUD.GetControl(IDC_BRIGHTTEXT)->SetLocation  ( width_start -(PROCAMP_X + 0 ),
                                                     height_start-(PROCAMP_Y + 0));

    g_HUD.GetControl( IDC_CONTRAST )->SetLocation  ( width_start -(PROCAMP_X - 100 ),
                                                     height_start-(PROCAMP_Y + 20));
    g_HUD.GetControl(IDC_CONTRASTTEXT)->SetLocation( width_start -(PROCAMP_X - 100 ),
                                                     height_start-(PROCAMP_Y + 0));

    g_HUD.GetControl( IDC_SATURATE )->SetLocation  ( width_start -(PROCAMP_X - 200 ),
                                                     height_start-(PROCAMP_Y + 20));
    g_HUD.GetControl(IDC_SATURATETEXT)->SetLocation( width_start -(PROCAMP_X - 200 ),
                                                     height_start-(PROCAMP_Y + 0));

    g_HUD.GetControl( IDC_HUE )->SetLocation       ( width_start -(PROCAMP_X - 300 ),
                                                     height_start-(PROCAMP_Y + 20));
    g_HUD.GetControl( IDC_HUETEXT )->SetLocation   ( width_start -(PROCAMP_X - 300 ),
                                                     height_start-(PROCAMP_Y + 0));

	// These sliders are used for FADE Final Compositing
    g_HUD.GetControl( IDC_FADE )->SetLocation  ( width_start   - (COMPCTRL_X + 0  ),
                                                       height_start/2- (COMPCTRL_Y + 120));
    g_HUD.GetControl(IDC_FADETEXT)->SetLocation( width_start   - (COMPCTRL_X + 0  ),
                                                       height_start/2- (COMPCTRL_Y + 100 ));

	// These sliders are used for Wipe Final Compositing
    g_HUD.GetControl( IDC_WIPECENTER )->SetLocation  ( width_start   - (COMPCTRL_X + 0  ),
                                                       height_start/2- (COMPCTRL_Y + 120));
    g_HUD.GetControl(IDC_WIPECENTERTEXT)->SetLocation( width_start   - (COMPCTRL_X + 0  ),
                                                       height_start/2- (COMPCTRL_Y + 100 ));

    g_HUD.GetControl( IDC_WIPESOFT )->SetLocation    ( width_start   - (COMPCTRL_X - 100), 
                                                       height_start/2- (COMPCTRL_Y + 120 ));
    g_HUD.GetControl(IDC_WIPESOFTTEXT)->SetLocation  ( width_start   - (COMPCTRL_X - 100), 
                                                       height_start/2- (COMPCTRL_Y + 100 ));

    g_HUD.GetControl( IDC_ANGLE )->SetLocation       ( width_start   - (COMPCTRL_X - 200), 
                                                       height_start/2- (COMPCTRL_Y + 120 ));
    g_HUD.GetControl(IDC_ANGLETEXT)->SetLocation     ( width_start   - (COMPCTRL_X - 200),
                                                       height_start/2- (COMPCTRL_Y + 100 ));

    g_HUD.GetControl( IDC_SLANT )->SetLocation       ( width_start   - (COMPCTRL_X - 300),
                                                       height_start/2- (COMPCTRL_Y + 120));
    g_HUD.GetControl( IDC_SLANTTEXT )->SetLocation   ( width_start   - (COMPCTRL_X - 300),
                                                       height_start/2- (COMPCTRL_Y + 100 ));

	// These sliders are used for frosted effect
	g_HUD.GetControl(IDC_DELTAX)->SetLocation     (  width_start   - (COMPCTRL_X + 0 ),
													 height_start/2- (COMPCTRL_Y + 40 ));
	g_HUD.GetControl(IDC_DELTAXTEXT)->SetLocation (  width_start   - (COMPCTRL_X + 0 ),
													 height_start/2- (COMPCTRL_Y + 20 ));
	g_HUD.GetControl(IDC_DELTAY)->SetLocation     (  width_start   - (COMPCTRL_X - 100 ),
													 height_start/2- (COMPCTRL_Y + 40 ));
	g_HUD.GetControl(IDC_DELTAYTEXT)->SetLocation (  width_start   - (COMPCTRL_X - 100 ),
													 height_start/2- (COMPCTRL_Y + 20 ));
	g_HUD.GetControl(IDC_FREQ)->SetLocation       (  width_start   - (COMPCTRL_X - 200 ),
													 height_start/2- (COMPCTRL_Y + 40 ));
	g_HUD.GetControl(IDC_FREQTEXT)->SetLocation   (  width_start   - (COMPCTRL_X - 200 ),
													 height_start/2- (COMPCTRL_Y + 20 ));


	// These sliders are used for lense ORB distortion effect
	g_HUD.GetControl(IDC_RADIUS)->SetLocation     (  width_start   - (COMPCTRL_X - 200 ),
													  height_start/2-(COMPCTRL_Y + 80 ));
	g_HUD.GetControl(IDC_RADIUSTEXT)->SetLocation (  width_start   - (COMPCTRL_X - 200 ),
													 height_start/2- (COMPCTRL_Y + 60 ));
	g_HUD.GetControl(IDC_EFFECTSCALE)->SetLocation(  width_start   - (COMPCTRL_X - 300 ),
												     height_start/2- (COMPCTRL_Y + 80 ));
	g_HUD.GetControl(IDC_EFFECTSCALETEXT)->SetLocation (  width_start   - (COMPCTRL_X - 300 ),
														  height_start/2- (COMPCTRL_Y + 60 ));

	// These sliders are used for Radial Blur effect 
	g_HUD.GetControl(IDC_BLURSTART)->SetLocation     (  width_start   - (COMPCTRL_X + 0 ),
														height_start/2- (COMPCTRL_Y + 80 ));
	g_HUD.GetControl(IDC_BLURSTARTTEXT)->SetLocation (  width_start   - (COMPCTRL_X + 0 ),
														height_start/2- (COMPCTRL_Y + 60 ));
	g_HUD.GetControl(IDC_BLURWIDTH)->SetLocation     (  width_start   - (COMPCTRL_X - 100 ),
														height_start/2- (COMPCTRL_Y + 80 ));
	g_HUD.GetControl(IDC_BLURWIDTHTEXT)->SetLocation (  width_start   - (COMPCTRL_X - 100 ),
														height_start/2- (COMPCTRL_Y + 60 ));

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
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

    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x00000000, 1.0f, 0L);

    // Set the world matrix
    pd3dDevice->SetTransform(D3DTS_WORLD, g_Camera.GetWorldMatrix());

    // Set the projection matrix
    pd3dDevice->SetTransform(D3DTS_PROJECTION, g_Camera.GetProjMatrix());

	// Set the view matrix
	pd3dDevice->SetTransform(D3DTS_VIEW, g_Camera.GetViewMatrix());

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        // render world
        g_pVideoFilterApp->Render(pd3dDevice);

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
	TCHAR sz[100];

    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
	if( g_bShowUI )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( DXUTGetFrameStats() );
		txtHelper.DrawTextLine( DXUTGetDeviceStats() );
	}

	if( !g_bShowHelp )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 for help" );
	}
	else
	{
        txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height-15*10 );
        txtHelper.SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls:" );

        txtHelper.SetInsertionPos( 20, pd3dsdBackBuffer->Height-15*9 );
        txtHelper.DrawTextLine( L"Cycle Through Filters: +/-, Spacebar" );
		txtHelper.DrawTextLine( L"Press F1 to hide help" );
		txtHelper.DrawTextLine( L"Press ESC to quit" );
		txtHelper.DrawTextLine( L"Toggle UI: H" );
	}

    txtHelper.End();

    g_pVideoFilterApp->nDuration[0] = g_pVideoFilterApp->getGraph(0)->GetDuration();
    g_pVideoFilterApp->nSeekPos[0]  = (g_pVideoFilterApp->getGraph(0)->GetCurrPos() * 100) / g_pVideoFilterApp->nDuration[0];
    g_HUD.GetSlider(IDC_SEEK1)->SetValue(g_pVideoFilterApp->nSeekPos[0]);

    _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->getGraph(0)->GetCurrPos()), 
                                    SEEK_ALG(g_pVideoFilterApp->nDuration[0]) ); sz[99] = 0;
    g_HUD.GetStatic(IDC_SEEKTEXT1)->SetText(sz);
	
    g_pVideoFilterApp->nDuration[1] = g_pVideoFilterApp->getGraph(1)->GetDuration();
    g_pVideoFilterApp->nSeekPos[1]  = (g_pVideoFilterApp->getGraph(1)->GetCurrPos() * 100) / g_pVideoFilterApp->nDuration[1];
    g_HUD.GetSlider(IDC_SEEK2)->SetValue(g_pVideoFilterApp->nSeekPos[1]);

    _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->getGraph(1)->GetCurrPos()), 
                                    SEEK_ALG(g_pVideoFilterApp->nDuration[1]) ); sz[99] = 0;
    g_HUD.GetStatic(IDC_SEEKTEXT2)->SetText(sz);
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

    switch( uMsg )
    {
    case WM_CREATE:
        if (g_pVideoFilterApp) {
            g_pVideoFilterApp->hWnd = hWnd;
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
		g_pVideoFilterApp->mouse_pos[0] = LOWORD(lParam);
		g_pVideoFilterApp->mouse_pos[1] = HIWORD(lParam);
		break;
	}

    if (g_pVideoFilterApp->IsMediaControlPresent()) {
        g_pVideoFilterApp->UpdateVMR9Frame();
    }

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
    int nVideoSurfaces;

	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_F1:
			{
				g_bShowHelp = !g_bShowHelp;
				break;
			}
		case VK_F9:
			{
				g_pVideoFilterApp->SetDefaults();
                for (nVideoSurfaces=0; nVideoSurfaces < MAX_VIDEO_FILES; nVideoSurfaces++)
                    g_HUD.GetListBox( IDC_LB_FILTER1 + nVideoSurfaces )->SelectItem(0);

                g_HUD.GetListBox( IDC_LB_COMPOSITE )->SelectItem(0);
				break;
			}
        case 'W':
        case 'w':
            {
                for (nVideoSurfaces=0; nVideoSurfaces < MAX_VIDEO_FILES; nVideoSurfaces++)
                    g_pVideoFilterApp->mbWireframe[nVideoSurfaces] = !g_pVideoFilterApp->mbWireframe[nVideoSurfaces];
                break;
            }
            break;

		case 'H':
		case 'h':
			{
				if( g_bShowUI = !g_bShowUI )
					for( int i = 0; i < IDC_LASTLAST; i++ )
						g_HUD.GetControl(i)->SetVisible( true );
				else
					for( int i = 0; i < IDC_LASTLAST; i++ )
						g_HUD.GetControl(i)->SetVisible( false );

                if (g_bShowUI) {
                   UpdateHiddenControls();
                }
				break;
			}
        case VK_ADD:
        case VK_SPACE:
            {
                VideoFilter::eFilterOptions selection;
                for (nVideoSurfaces=0; nVideoSurfaces < MAX_VIDEO_FILES; nVideoSurfaces++) {
                    selection = (VideoFilter::eFilterOptions)(((int)g_pVideoFilterApp->meDisplayOption[nVideoSurfaces] + 1) % VideoFilter::NUM_FILTER_OPTIONS);
                    g_pVideoFilterApp->meDisplayOption[nVideoSurfaces] = selection;
                    g_HUD.GetListBox( IDC_LB_FILTER1 + nVideoSurfaces )->SelectItem(0);
                }
                break;
            }
        case VK_SUBTRACT:
            {
                VideoFilter::eFilterOptions selection;
                for (nVideoSurfaces=0; nVideoSurfaces < MAX_VIDEO_FILES; nVideoSurfaces++) {
                    selection = (VideoFilter::eFilterOptions)(((int)g_pVideoFilterApp->meDisplayOption[nVideoSurfaces] - 1) % VideoFilter::NUM_FILTER_OPTIONS);
                    g_pVideoFilterApp->meDisplayOption[nVideoSurfaces] = selection;
                    g_HUD.GetListBox( IDC_LB_FILTER1 + nVideoSurfaces )->SelectItem(0);
                }
                break;
            }
        case VK_HOME :          // reset scene/settings
            {
                for (nVideoSurfaces=0; nVideoSurfaces < MAX_VIDEO_FILES; nVideoSurfaces++) {
                    g_pVideoFilterApp->mbWireframe[nVideoSurfaces] = false;
                    g_pVideoFilterApp->meDisplayOption[nVideoSurfaces] = g_pVideoFilterApp->meInitDisplayOption[nVideoSurfaces];
                    g_HUD.GetListBox( IDC_LB_FILTER1 + nVideoSurfaces )->SelectItem(0);
                }
                g_pVideoFilterApp->mePostOption       = g_pVideoFilterApp->meInitPostOption;
                g_HUD.GetListBox( IDC_LB_COMPOSITE )->SelectItem(0);

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
	VideoFilter::eFilterOptions eTemp;
	TCHAR sz[100];

    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: 
            DXUTToggleFullScreen(); 
            break;
        case IDC_TOGGLEREF:        
            DXUTToggleREF(); 
            break;
        case IDC_CHANGEDEVICE:     
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
            break;
        case IDC_RESET:
            g_pVideoFilterApp->SetDefaults();
            g_HUD.GetListBox( IDC_LB_FILTER1 )->SelectItem(0);
            g_HUD.GetListBox( IDC_LB_FILTER2 )->SelectItem(0);
            g_HUD.GetListBox( IDC_LB_COMPOSITE)->SelectItem(0);

            g_HUD.GetSlider(IDC_BRIGHT)->SetValue  ((int)(g_pVideoFilterApp->fBrightness[0]*100.0f));
            g_HUD.GetSlider(IDC_CONTRAST)->SetValue((int)(g_pVideoFilterApp->fContrast[0]  *100.0f));
            g_HUD.GetSlider(IDC_SATURATE)->SetValue((int)(g_pVideoFilterApp->fSaturate[0]  *100.0f));
            g_HUD.GetSlider(IDC_HUE)->SetValue     ((int)(g_pVideoFilterApp->fHue[0]       *360.0f));

			_sntprintf( sz, 100, BRIGHT_TEXT,   BRIGHT_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_BRIGHTTEXT)->SetText(sz);

            _sntprintf( sz, 100, CONTRAST_TEXT, CONTRAST_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_CONTRASTTEXT)->SetText(sz);

            _sntprintf( sz, 100, SATURATE_TEXT, SATURATE_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_SATURATETEXT)->SetText(sz);

            _sntprintf( sz, 100, HUE_TEXT,      HUE_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_HUETEXT)->SetText(sz);
            break;
        case IDC_WIREFRAME:
            g_pVideoFilterApp->mbWireframe[0] = !g_pVideoFilterApp->mbWireframe[0];
            g_pVideoFilterApp->mbWireframe[1] = !g_pVideoFilterApp->mbWireframe[1];
            break;
        case IDC_LB_FILTER1:
            switch (g_HUD.GetListBox( IDC_LB_FILTER1 )->GetSelectedIndex()) {
                case 0: eTemp = VideoFilter::CONE_FILTER;    break;
                case 1: eTemp = VideoFilter::BOX9_FILTER;    break;
                case 2: eTemp = VideoFilter::BOX16_FILTER;   break;
                case 3: eTemp = VideoFilter::SHARPEN_FILTER; break;
                case 4: eTemp = VideoFilter::LUMINANCE_EDGE; break;
                case 5: eTemp = VideoFilter::SEPIA_FILTER;   break;
                case 6: eTemp = VideoFilter::RADIAL_BLUR;    break;
//                case 7: eTemp = VideoFilter::ORB_FILTER;     break;
//                case 8: eTemp = VideoFilter::FROST_FILTER;   break;
//                case 9: eTemp = VideoFilter::OLDTV_FILTER;   break;
            }
			g_pVideoFilterApp->meDisplayOption[0] = eTemp;
			UpdateHiddenControls();
            break;
        case IDC_LB_FILTER2:
			
            switch (g_HUD.GetListBox( IDC_LB_FILTER2 )->GetSelectedIndex()) {
                case 0: eTemp = VideoFilter::CONE_FILTER;    break;
                case 1: eTemp = VideoFilter::BOX9_FILTER;    break;
                case 2: eTemp = VideoFilter::BOX16_FILTER;   break;
                case 3: eTemp = VideoFilter::SHARPEN_FILTER; break;
                case 4: eTemp = VideoFilter::LUMINANCE_EDGE; break;
                case 5: eTemp = VideoFilter::SEPIA_FILTER;   break;
                case 6: eTemp = VideoFilter::RADIAL_BLUR;    break;
//                case 7: eTemp = VideoFilter::ORB_FILTER;     break;
//                case 8: eTemp = VideoFilter::FROST_FILTER;   break;
//                case 9: eTemp = VideoFilter::OLDTV_FILTER;   break;
            }
			g_pVideoFilterApp->meDisplayOption[1] = eTemp;
			UpdateHiddenControls();
            break;
        case IDC_LB_COMPOSITE:
            switch (g_HUD.GetListBox( IDC_LB_COMPOSITE )->GetSelectedIndex()) {
                case 0: g_pVideoFilterApp->mePostOption = VideoFilter::POST_BLEND;    break;
                case 1: g_pVideoFilterApp->mePostOption = VideoFilter::POST_WIPE;     break;
            }
            UpdateHiddenControls();
            break;

        case IDC_BRIGHT:
			_sntprintf( sz, 100, BRIGHT_TEXT, BRIGHT_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_BRIGHTTEXT)->SetText(sz);
            break;
        case IDC_CONTRAST:
			_sntprintf( sz, 100, CONTRAST_TEXT, CONTRAST_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_CONTRASTTEXT)->SetText(sz);
            break;
        case IDC_SATURATE:
			_sntprintf( sz, 100, SATURATE_TEXT, SATURATE_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_SATURATETEXT)->SetText(sz);
            break;
        case IDC_HUE:
			_sntprintf( sz, 100, HUE_TEXT, HUE_ALG(0)); sz[99] = 0;
			g_HUD.GetStatic(IDC_HUETEXT)->SetText(sz);
            break;
		case IDC_DELTAX:
			_sntprintf( sz, 100, DELTAX_TEXT, DELTAX_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_DELTAXTEXT)->SetText(sz);
			break;
		case IDC_DELTAY:
			_sntprintf( sz, 100, DELTAY_TEXT, DELTAY_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_DELTAYTEXT)->SetText(sz);
			break;
		case IDC_FREQ:
			_sntprintf( sz, 100, FREQ_TEXT, FREQ_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_FREQTEXT)->SetText(sz);
			break;
		case IDC_RADIUS:
			_sntprintf( sz, 100, RADIUS_TEXT, RADIUS_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_RADIUSTEXT)->SetText(sz);
			break;
		case IDC_EFFECTSCALE:
			_sntprintf( sz, 100, EFFECTSCALE_TEXT, EFFECTSCALE_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_EFFECTSCALETEXT)->SetText(sz);
			break;
		case IDC_BLURSTART:
			_sntprintf( sz, 100, BLURSTART_TEXT, BLURSTART_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLURSTARTTEXT)->SetText(sz);
			break;
		case IDC_BLURWIDTH:
			_sntprintf( sz, 100, BLURWIDTH_TEXT, BLURWIDTH_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_BLURWIDTHTEXT)->SetText(sz);
			break;
        case IDC_FADE:
			_sntprintf( sz, 100, FADE_TEXT, FADE_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_FADETEXT)->SetText(sz);
            break;
        case IDC_WIPECENTER:
			_sntprintf( sz, 100, WIPECENTER_TEXT, WIPECENTER_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_WIPECENTERTEXT)->SetText(sz);
            break;
        case IDC_WIPESOFT:
			_sntprintf( sz, 100, WIPESOFT_TEXT, WIPESOFT_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_WIPESOFTTEXT)->SetText(sz);
            break;
        case IDC_ANGLE:
			_sntprintf( sz, 100, ANGLE_TEXT, ANGLE_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_ANGLETEXT)->SetText(sz);
            break;
        case IDC_SLANT:
			_sntprintf( sz, 100, SLANT_TEXT, SLANT_ALG); sz[99] = 0;
			g_HUD.GetStatic(IDC_SLANTTEXT)->SetText(sz);
            break;
        case IDC_PLAYPAUSE1:
            switch (g_pVideoFilterApp->m_ePlayState[0]) 
            {
            case VideoFilter::PS_PLAY:
                g_pVideoFilterApp->getGraph(0)->PauseGraph();
                g_pVideoFilterApp->m_ePlayState[0] = VideoFilter::PS_PAUSE;
                break;
            case VideoFilter::PS_PAUSE:
            case VideoFilter::PS_STOP:
                g_pVideoFilterApp->getGraph(0)->RunGraph();
                g_pVideoFilterApp->m_ePlayState[0] = VideoFilter::PS_PLAY;
                break;
            }
            break;
        case IDC_STOP1:
            g_pVideoFilterApp->getGraph(0)->StopGraph();
            g_pVideoFilterApp->m_ePlayState[0] = VideoFilter::PS_STOP;
            break;
        case IDC_SEEK1:
            g_pVideoFilterApp->nSeekPos[0]  = g_HUD.GetSlider(IDC_SEEK1)->GetValue();
            g_pVideoFilterApp->nDuration[0] = g_pVideoFilterApp->getGraph(0)->GetDuration();

            g_pVideoFilterApp->getGraph(0)->Seek( g_pVideoFilterApp->nSeekPos[0] );
            _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->nSeekPos[0]), 
                                            SEEK_ALG(g_pVideoFilterApp->nDuration[0]) ); sz[99] = 0;
            g_HUD.GetStatic(IDC_SEEKTEXT1)->SetText(sz);
            break;
        case IDC_LOOP1:
            g_pVideoFilterApp->m_bLoopCheck[0] = g_HUD.GetCheckBox(IDC_LOOP1)->GetChecked();
            break;
        case IDC_PLAYPAUSE2:
            switch (g_pVideoFilterApp->m_ePlayState[1]) 
            {
            case VideoFilter::PS_PLAY:
                g_pVideoFilterApp->getGraph(1)->PauseGraph();
                g_pVideoFilterApp->m_ePlayState[1] = VideoFilter::PS_PAUSE;
                break;
            case VideoFilter::PS_PAUSE:
            case VideoFilter::PS_STOP:
                g_pVideoFilterApp->getGraph(1)->RunGraph();
                g_pVideoFilterApp->m_ePlayState[1] = VideoFilter::PS_PLAY;
                break;
            }
            break;
        case IDC_STOP2:
            g_pVideoFilterApp->getGraph(1)->StopGraph();
            g_pVideoFilterApp->m_ePlayState[1] = VideoFilter::PS_STOP;
            break;
        case IDC_SEEK2:
            g_pVideoFilterApp->nSeekPos[1]  = g_HUD.GetSlider(IDC_SEEK2)->GetValue();
            g_pVideoFilterApp->nDuration[1] = g_pVideoFilterApp->getGraph(1)->GetDuration();

            g_pVideoFilterApp->getGraph(1)->Seek( g_pVideoFilterApp->nSeekPos[1] );
            _sntprintf( sz, 100, SEEK_TEXT, SEEK_ALG(g_pVideoFilterApp->nSeekPos[1]), 
                                            SEEK_ALG(g_pVideoFilterApp->nDuration[1]) ); sz[99] = 0;
            g_HUD.GetStatic(IDC_SEEKTEXT2)->SetText(sz);
            break;
        case IDC_LOOP2:
            g_pVideoFilterApp->m_bLoopCheck[1] = g_HUD.GetCheckBox(IDC_LOOP2)->GetChecked();
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

    g_pVideoFilterApp->InvalidateDeviceObjects(DXUTGetD3D9Device());

    if( g_pFont )
        g_pFont->OnLostDevice();

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
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

    SAFE_RELEASE(g_pFont);
}