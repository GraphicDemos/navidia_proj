#define STRICT
#include "nvafx.h"
#include "AnisoDecimationApp.h"
#include <Sdkmisc.h>

const INT UPSAMPLE_FB_HEIGHT = 1200;
const INT UPSAMPLE_FB_WIDTH  = 1600;
const INT DECIMATE_FB_HEIGHT = 256;
const INT DECIMATE_FB_WIDTH  = 256;

const int NUM_TECHNIQUES = 3;
const struct 
{ 
    TCHAR* name; 
    D3DXHANDLE technique; 
    UINT numPasses;
    LPDIRECT3DSURFACE9* targets[2];
    LPDIRECT3DTEXTURE9* textures[2];
} g_Techniques[NUM_TECHNIQUES] = {
    { _T("No Decimation"), "SimpleTechnique", 1, {&g_pDecimateSurface[0], NULL}, {&g_pSrcImage, NULL} },
    { _T("4-tap Pixel Shader Decimation"), "PSDecimateTechnique", 1, {&g_pDecimateSurface[0], NULL}, {&g_pSrcImage, NULL} },
    { _T("2-pass Aniso Decimation"), "AnisoDecimateTechnique", 2, {&g_pDecimateSurface[1], &g_pDecimateSurface[0]}, {&g_pSrcImage, &g_pDstImage[1]} }
};

void DrawFullScreenQuad(LPDIRECT3DDEVICE9 pd3dDevice, float fLeftU, float fTopV, float fRightU, float fBottomV, bool bOffset);
HRESULT SetDecimationTextureCoordinates( UINT SrcWidth, UINT SrcHeight, UINT DstWidth, UINT DstHeight );


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
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true,NULL, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"AnisoDecimation" );
    DXUTCreateDevice( true, DECIMATE_FB_WIDTH, DECIMATE_FB_HEIGHT);

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
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, _T("Toggle full screen"), 35, 34, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, _T("Toggle REF (F3)"), 35, 58, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, _T("Change device (F2)"), 35, 82, 125, 22, VK_F2 );
    g_HUD.AddButton( IDC_SWAPTECHNIQUE, _T("Cycle techniques"), 35, 106, 125, 22 );

}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void *pUserContext )
{
    BOOL bCapsAcceptable = TRUE;

    // TODO: Perform checks to see if these display caps are acceptable.

    if (!bCapsAcceptable)
        return false;

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
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void *pUserContext )
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
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext )
{
    HRESULT hr;


    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         _T("Arial"), &g_pFont ) );

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
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext )
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
    g_HUD.GetControl( IDC_SWAPTECHNIQUE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

    pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

    V_RETURN( pd3dDevice->CreateTexture( UPSAMPLE_FB_WIDTH, UPSAMPLE_FB_HEIGHT,   1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pSrcImage, NULL ) );
    V_RETURN( pd3dDevice->CreateTexture( DECIMATE_FB_WIDTH, UPSAMPLE_FB_HEIGHT/2, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pDstImage[1], NULL ) );
    V_RETURN( pd3dDevice->CreateTexture( DECIMATE_FB_WIDTH, DECIMATE_FB_HEIGHT,   1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pDstImage[0], NULL ) );

    V_RETURN( g_pSrcImage->GetSurfaceLevel(0, &g_pUpsampleSurface) );
    V_RETURN( g_pDstImage[1]->GetSurfaceLevel(0, &g_pDecimateSurface[1]) );
    V_RETURN( g_pDstImage[0]->GetSurfaceLevel(0, &g_pDecimateSurface[0]) );

    V_RETURN( D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA/textures/2d/TestImage.dds")).c_str(), &g_pSrcTexture) );

    V_RETURN( D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA/programs/AnisoDecimation/AnisoDecimation.fx")).c_str(), NULL, NULL, 0, NULL, &g_pEffect, NULL) );

    D3DSURFACE_DESC surfDesc;
    V_RETURN( g_pSrcTexture->GetLevelDesc(0, &surfDesc) );
    SetDecimationTextureCoordinates( UPSAMPLE_FB_WIDTH, UPSAMPLE_FB_HEIGHT, DECIMATE_FB_WIDTH, DECIMATE_FB_HEIGHT );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void *pUserContext )
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
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;
    UINT uPasses;

    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, 0x000000ff, 1.0f, 0L);

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        //  preserve depth/stencil surface
        LPDIRECT3DSURFACE9 pZetaBuffer = NULL;
        LPDIRECT3DSURFACE9 pBackBuffer = NULL;
        pd3dDevice->GetDepthStencilSurface( &pZetaBuffer );
        pd3dDevice->SetDepthStencilSurface( NULL );
        pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );

        {
            //  render to 1600x1200 upsampled image
            pd3dDevice->SetRenderTarget( 0, g_pUpsampleSurface );
            g_pEffect->SetTexture( "SrcTexture", g_pSrcTexture );
            g_pEffect->SetTechnique( "SimpleTechnique" );

            if ( SUCCEEDED(g_pEffect->Begin(&uPasses, 0)) )
            {
                g_pEffect->BeginPass(0);
                DrawFullScreenQuad( pd3dDevice, 0.f, 0.f, 1.f, 1.f, false );
                g_pEffect->EndPass();
                g_pEffect->End();
            }
        }

        for ( UINT decimatePass=0; decimatePass < g_Techniques[g_uiDecimateTechnique].numPasses; decimatePass++ )
        {
            g_pEffect->SetTechnique( g_Techniques[g_uiDecimateTechnique].technique );
            g_pEffect->SetTexture( "SrcTexture", *g_Techniques[g_uiDecimateTechnique].textures[decimatePass] );
            pd3dDevice->SetRenderTarget( 0, *g_Techniques[g_uiDecimateTechnique].targets[decimatePass] );
            if ( SUCCEEDED(g_pEffect->Begin(&uPasses, 0)) )
            {
                g_pEffect->BeginPass(0);
                DrawFullScreenQuad( pd3dDevice, 0.f, 0.f, 1.f, 1.f, true );
                g_pEffect->EndPass();
                g_pEffect->End();
            }
        }

        //  render decimated image to back buffer
        pd3dDevice->SetRenderTarget( 0, pBackBuffer );
        g_pEffect->SetTexture( "SrcTexture", g_pDstImage[0] );
        g_pEffect->SetTechnique( "SimpleTechnique" );

        if ( SUCCEEDED(g_pEffect->Begin(&uPasses, 0)) )
        {
            g_pEffect->BeginPass(0);
            DrawFullScreenQuad( pd3dDevice, 0.f, 0.f, 1.f, 1.f, false );
            g_pEffect->EndPass();
            g_pEffect->End();
        }


        // Render stats and help text  
        RenderText();

		V( g_HUD.OnRender( fElapsedTime ) );

        //  restore depth/stencil surface
        pd3dDevice->SetDepthStencilSurface( pZetaBuffer );
        SAFE_RELEASE( pZetaBuffer );
        SAFE_RELEASE( pBackBuffer );

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
        txtHelper.DrawTextLine( g_Techniques[g_uiDecimateTechnique].name );

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
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext )
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
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void *pUserContext )
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
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
	switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
        case IDC_SWAPTECHNIQUE:    g_uiDecimateTechnique = (g_uiDecimateTechnique+1) % NUM_TECHNIQUES; break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void *pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

    if( g_pFont )
        g_pFont->OnLostDevice();

	SAFE_RELEASE(g_pTextSprite);
    SAFE_RELEASE(g_pSrcTexture);
    SAFE_RELEASE(g_pSrcImage);
    SAFE_RELEASE(g_pDstImage[0]);
    SAFE_RELEASE(g_pDstImage[1]);
    SAFE_RELEASE(g_pUpsampleSurface);
    SAFE_RELEASE(g_pDecimateSurface[0]);
    SAFE_RELEASE(g_pDecimateSurface[1]);
    SAFE_RELEASE(g_pEffect);
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void *pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

    SAFE_RELEASE(g_pFont);
}

//--------------------------------------------------------------------------------------
//
//  SetDecimationTextureCoordinates()
//    Computes an array of 4 2D texture coordinates used for decimation

HRESULT SetDecimationTextureCoordinates( UINT SrcWidth, UINT SrcHeight, UINT DstWidth, UINT DstHeight )
{
    HRESULT hr = S_OK;

    ZeroMemory(g_vCoordArray, sizeof(g_vCoordArray));
    
    FLOAT vStep = (FLOAT(SrcHeight) / FLOAT(DstHeight)) / 2.f;
    FLOAT uStep = (FLOAT(SrcWidth)  / FLOAT(DstWidth) ) / 2.f;
        
    for (int v=0; v<2; v++)
    {
        FLOAT vBias = (FLOAT(v)*vStep + 0.5f) / FLOAT(SrcHeight); 
        for (int u=0; u<2; u++)
        {
            FLOAT uBias = (FLOAT(u)*uStep + 0.5f) / FLOAT(SrcWidth);
            g_vCoordArray[v*2+u] = D3DXVECTOR2(uBias, vBias);
        }
    }

    return S_OK;
}

//-------------------------------------------------------------------------------------------
//
//  DrawFullScreenQuad()
//    Draws a quad with a texture coordinate that fills the screen.

struct ScreenVertex
{
    D3DXVECTOR4 p; // position
    D3DXVECTOR2 t0; // texture coordinate
    D3DXVECTOR2 t1; // texture coordinate
    D3DXVECTOR2 t2; // texture coordinate
    D3DXVECTOR2 t3; // texture coordinate
    static const DWORD FVF;
};
const DWORD ScreenVertex::FVF = D3DFVF_XYZRHW | D3DFVF_TEX4 |
    D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE2(2) | D3DFVF_TEXCOORDSIZE2(3);

void DrawFullScreenQuad(LPDIRECT3DDEVICE9 pd3dDevice, float fLeftU, float fTopV, float fRightU, float fBottomV, bool bOffset)
{
    D3DSURFACE_DESC dtdsdRT;
    PDIRECT3DSURFACE9 pSurfRT;

    // Acquire render target width and height
    pd3dDevice->GetRenderTarget(0, &pSurfRT);
    pSurfRT->GetDesc(&dtdsdRT);
    pSurfRT->Release();

    // Ensure that we're directly mapping texels to pixels by offset by 0.5
    // For more info see the doc page titled "Directly Mapping Texels to Pixels"
    FLOAT fWidth5 = (2.f*(FLOAT)dtdsdRT.Width) ;
    FLOAT fHeight5 = (2.f*(FLOAT)dtdsdRT.Height);

    // Draw the quad
    ScreenVertex svQuad[4];

    float tWidth = fRightU - fLeftU;
    float tHeight = fBottomV - fTopV;

    svQuad[0].p = D3DXVECTOR4(0.f, 0.f, 0.5f, 1.0f);
    svQuad[1].p = D3DXVECTOR4(fWidth5, 0.f, 0.5f, 1.f);
    svQuad[2].p = D3DXVECTOR4(0.f, fHeight5, 0.5f, 1.f);

    FLOAT* coordArray = &svQuad[0].t0.x;
    for ( UINT i=0; i<4; i++, coordArray+=2 )
    {
        coordArray[0] = fLeftU + ((bOffset)?g_vCoordArray[i].x:0);
        coordArray[1] = fTopV + ((bOffset)?g_vCoordArray[i].y:0);
    }

    coordArray = &svQuad[1].t0.x;
    for ( UINT i=0; i<4; i++, coordArray+=2 )
    {
        coordArray[0] = fLeftU + (tWidth*2.f) + ((bOffset)?g_vCoordArray[i].x:0);
        coordArray[1] = fTopV + ((bOffset)?g_vCoordArray[i].y:0);
    }

    coordArray = &svQuad[2].t0.x;
    for ( UINT i=0; i<4; i++, coordArray+=2 )
    {
        coordArray[0] = fLeftU + ((bOffset)?g_vCoordArray[i].x:0);
        coordArray[1] = fTopV + (tHeight*2.f) + ((bOffset)?g_vCoordArray[i].y:0);
    }

    pd3dDevice->SetFVF(ScreenVertex::FVF);
    pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, svQuad, sizeof(ScreenVertex));
}