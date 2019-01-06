#define STRICT
#include "dxstdafx.h"
#include <DXUT/DXUTMesh.h>
#include <DXUT/SDKmisc.h>

#include "MeshMenderSimpleExampleApp.h"
#include "special/nv_meshmender/NVMeshMender.h"
#include <string>
typedef std::basic_string<TCHAR> tstring; 



HRESULT MendTheMesh(	IDirect3DDevice9* pd3dDevice,
	CDXUTXFileMesh* mesh,
					const float minNormalsCreaseCosAngle = 0.0f,
					const float minTangentsCreaseCosAngle = 0.0f ,
					const float minBinormalsCreaseCosAngle = 0.0f,
					const float weightNormalsByArea = 1.0f,
					const MeshMender::NormalCalcOption computeNormals = MeshMender::CALCULATE_NORMALS,
					const MeshMender::ExistingSplitOption respectExistingSplits = MeshMender::DONT_RESPECT_SPLITS,
					const MeshMender::CylindricalFixOption fixCylindricalWrapping = MeshMender::DONT_FIX_CYLINDRICAL);


#include "shared\GetFilePath.h"
HRESULT	LoadMeshHelperFunction(CDXUTXFileMesh* mesh, tstring meshFile, IDirect3DDevice9* pd3dDevice);

WCHAR meshFile[MAX_PATH];
WCHAR textureFile[MAX_PATH];

CDXUTXFileMesh g_TheMesh;
LPD3DXEFFECT g_pEffect;
LPDIRECT3DTEXTURE9 g_TheTexture;


void RemendBasedOnUISettings();
void ReloadTexture();

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
    DXUTCreateWindow( L"MeshMenderSimpleExample" );
    DXUTCreateDevice(  true, 512, 512);

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
	g_HUD.AddButton( IDC_MEND_IT_GO, L"UPDATE MEND", 35, 106, 125, 22);


	g_HUD.AddCheckBox( IDC_CHECKBOX_CALCNORMALS, L"calculate normals (press update mend to recalc mesh)",
                            35, 150, 250, 24, true, 0, false );

	g_HUD.AddCheckBox( IDC_CHECKBOX_RESPECT_SPLITS, L"respect existing splits ",
                            35, 174, 200, 24, false, 0, false );

	g_HUD.AddCheckBox( IDC_CHECKBOX_FIX_CYLINDRICAL, L"fix cylindrical wrapping ",
                            35, 198, 200, 24, false, 0, false );




    g_HUD.AddSlider( IDC_SLIDER_NORMAL_CREASE, 35, 222, 100, 24, -100, 100, 0, false );
	g_HUD.AddStatic( IDC_STATIC_NC,	L"minNormCrease: ",	   135, 222, 150, 24);
	g_HUD.AddSlider( IDC_SLIDER_TANGENT_CREASE, 35, 246, 100, 24, -100, 100, 0, false );
	g_HUD.AddStatic( IDC_STATIC_TC,	L"minTanCrease:",	   135, 246, 150, 24);
	g_HUD.AddSlider( IDC_SLIDER_BINORMAL_CREASE, 35, 270, 100, 24, -100, 100, 0, false );
	g_HUD.AddStatic( IDC_STATIC_BC,	L"minBinormalCrease:",	   135, 270, 150, 24);
	g_HUD.AddSlider( IDC_SLIDER_NORMAL_WEIGHT, 35, 294, 100, 24, 0, 100, 0, false );
	g_HUD.AddStatic( IDC_STATIC_NW,	L"normalAreaWeight: ",	   135, 294, 150, 24);

	g_HUD.AddButton( IDC_BUTTON_CHOOSNEWMESH, L"New Mesh", 135, 350, 125, 22);
	g_HUD.AddButton( IDC_BUTTON_CHOOSNEWNORMMAP, L"New Texture", 35, 350, 100, 22);

	

}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
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
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(0.0f, 0.0f, -555.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);

	swprintf( meshFile, L"MEDIA\\models\\Ufo-03\\UFO-01_NoTexture.X"	);


	


	//a terrain mesh
	hr = LoadMeshHelperFunction(&g_TheMesh, meshFile , pd3dDevice);
	if( FAILED( hr ) )
        return hr;

	hr = MendTheMesh(pd3dDevice,&g_TheMesh,
		0,
		1,
		1,
		1,
		MeshMender::CALCULATE_NORMALS,
		MeshMender::DONT_RESPECT_SPLITS,
		MeshMender::DONT_FIX_CYLINDRICAL	);

	if( FAILED(hr))
		return hr;
	

	//load an effect for our model
	tstring path = GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\MeshMender_perPixelLighting.cso"), false );
	if(path.empty())
		return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;
	hr = D3DXCreateEffectFromFile(pd3dDevice, path.c_str(), NULL, NULL, 0, NULL, &g_pEffect, NULL );
    if (FAILED(hr))
        return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;



	swprintf( textureFile, L"MEDIA\\textures\\2D\\earth_bump.dds"	);

	//load an normalMap for our model
	path = GetFilePath::GetFilePath(textureFile, false );
	if(path.empty())
		return E_FAIL;//D3DAPPERR_MEDIANOTFOUND;

	hr = D3DXCreateTextureFromFileEx( 
		pd3dDevice,
		path.c_str(),
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT ,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		0,
		0,
		&g_TheTexture
	);
	if( FAILED( hr ) )
	{
		
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

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 10000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( 0, 0 );
    g_HUD.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	int iY = 15;
    g_HUD.GetControl( IDC_TOGGLEFULLSCREEN )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY);
    g_HUD.GetControl( IDC_TOGGLEREF )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );
    g_HUD.GetControl( IDC_CHANGEDEVICE )->SetLocation( pBackBufferSurfaceDesc->Width - 135, iY += 24 );

    pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);


	g_TheMesh.RestoreDeviceObjects(pd3dDevice);

	if(g_pEffect)
	{
		g_pEffect->OnResetDevice();
		D3DXHANDLE pTechnique = NULL;
		V_RETURN(g_pEffect->FindNextValidTechnique( NULL, &pTechnique));
		g_pEffect->SetTechnique(pTechnique);
		g_pEffect->SetTexture("Tex0", g_TheTexture);
	}

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

    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);

    // Set the world matrix
    pd3dDevice->SetTransform(D3DTS_WORLD, g_Camera.GetWorldMatrix());

    // Set the projection matrix
    pd3dDevice->SetTransform(D3DTS_PROJECTION, g_Camera.GetProjMatrix());

	// Set the view matrix
	pd3dDevice->SetTransform(D3DTS_VIEW, g_Camera.GetViewMatrix());



	
	//compose worldViewProjection for effects
	const D3DXMATRIX* world = g_Camera.GetWorldMatrix();
	const D3DXMATRIX* view = g_Camera.GetViewMatrix();
	const D3DXMATRIX* proj = g_Camera.GetProjMatrix();

	D3DXMATRIX mWI,mVI;
	float det;
	D3DXMatrixInverse(&mWI,&det, world);
	D3DXMatrixInverse(&mVI,&det, view);


	g_pEffect->SetMatrix("World", world);
	g_pEffect->SetMatrix("WorldInv", &mWI);
	g_pEffect->SetMatrix("View", view);
	g_pEffect->SetMatrix("ViewInv", &mVI);
	
	g_pEffect->SetMatrix("Projection", proj);
	

	g_pEffect->SetTexture("Tex0", g_TheTexture);
	pd3dDevice->SetTexture(0, g_TheTexture);
	
	g_TheMesh.UseMeshMaterials(false);

	// Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        // TODO: render world


		UINT passes;
		g_pEffect->Begin(&passes,NULL);
		for(UINT i = 0 ; i < passes ; ++ i)
		{
			g_pEffect->BeginPass(i);
			g_TheMesh.Render(pd3dDevice);
			g_pEffect->EndPass();
		}
		g_pEffect->End();



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

	if( !g_bShowHelp )
	{
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 for help" );
	}
	else
	{
		txtHelper.SetInsertionPos( 5, pd3dsdBackBuffer->Height - 15*3 - 5);
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
		txtHelper.DrawTextLine( L"Press F1 to hide help" );
		txtHelper.DrawTextLine( L"Press ESC to quit" );
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
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
		case IDC_MEND_IT_GO:
			RemendBasedOnUISettings( );
			break;
		case IDC_SLIDER_NORMAL_CREASE:
			{
				WCHAR wszText[50];
				
				swprintf( wszText, L"minNormalCrease %f", 
					g_HUD.GetSlider(IDC_SLIDER_NORMAL_CREASE)->GetValue() / 100.0f
					);
				g_HUD.GetStatic(IDC_STATIC_NC)->SetText(wszText);
			}
		case IDC_SLIDER_TANGENT_CREASE:
			{
				WCHAR wszText[50];
				
				swprintf( wszText, L"minTangentCrease %f", 
					g_HUD.GetSlider(IDC_SLIDER_TANGENT_CREASE)->GetValue() / 100.0f
					);
				g_HUD.GetStatic(IDC_STATIC_TC)->SetText(wszText);
			}
			break;
		case IDC_SLIDER_BINORMAL_CREASE:
			{
				WCHAR wszText[50];
				
				swprintf( wszText, L"minBinormalCrease %f", 
					g_HUD.GetSlider(IDC_SLIDER_BINORMAL_CREASE)->GetValue() / 100.0f
					);
				g_HUD.GetStatic(IDC_STATIC_BC)->SetText(wszText);
			}
			break;
		case IDC_SLIDER_NORMAL_WEIGHT:
			{
				WCHAR wszText[50];
				
				swprintf( wszText, L"normal Weight %f", 
					g_HUD.GetSlider(IDC_SLIDER_NORMAL_WEIGHT)->GetValue() / 100.0f
					);
				g_HUD.GetStatic(IDC_STATIC_NW)->SetText(wszText);
			}
			break;
		case IDC_BUTTON_CHOOSNEWMESH:
			{
				WCHAR requestedFile[MAX_PATH];
				requestedFile[0] = '\0';

				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = DXUTGetHWND();
				
				ofn.lpstrFilter = L"mesh\0*.x";
				ofn.lpstrFile = requestedFile;
				ofn.lpstrTitle = L"Choose a mesh";
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;


				BOOL ret = GetOpenFileName(&ofn);
				if(ret)
				{
					swprintf( meshFile, requestedFile);
					RemendBasedOnUISettings( );
				}
			}
			break;
		case IDC_BUTTON_CHOOSNEWNORMMAP:
			{
				WCHAR requestedFile[MAX_PATH];
				requestedFile[0] = '\0';

				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = DXUTGetHWND();
				//.bmp, .dds, .dib, .hdr, .jpg, .pfm, .png, .ppm, and .tga.

				ofn.lpstrFilter = L"image files\0*.bmp;*.dds;*.dib;*.hdr;*.jpg;*.pfm;*.png;*.ppm;*.tga\0all files\0*.*";
				ofn.lpstrFile = requestedFile;
				ofn.lpstrTitle = L"Choose a normal Map";
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;


				BOOL ret = GetOpenFileName(&ofn);
				if(ret)
				{
					swprintf( textureFile, requestedFile);
					ReloadTexture();
				}
			}
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

	g_TheMesh.InvalidateDeviceObjects();

	if(g_pEffect)
		g_pEffect->OnLostDevice();

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

    g_TheMesh.Destroy();
	SAFE_RELEASE(g_pEffect);
	SAFE_RELEASE(g_TheTexture);

    SAFE_RELEASE(g_pFont);
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



//helper function to create a tangent basis for CDXUTMesh
HRESULT MendTheMesh(	IDirect3DDevice9* pd3dDevice,
	CDXUTXFileMesh* mesh,
					const float minNormalsCreaseCosAngle ,
					const float minTangentsCreaseCosAngle,
					const float minBinormalsCreaseCosAngle,
					const float weightNormalsByArea ,
					const MeshMender::NormalCalcOption computeNormals ,
					const MeshMender::ExistingSplitOption respectExistingSplits ,
					const MeshMender::CylindricalFixOption fixCylindricalWrapping )
{
	HRESULT hr;

	MeshMender theMender;
	std::vector< MeshMender::Vertex > theVerts;
	std::vector< unsigned int > theIndices;


	//STEP 0:
	//     set the mesh to have a known fvf for this demo, 
	//	   this is done for simplicity mostly since you could query the vertex declaration to find the 
	//	   proper offsets.

	//pos: float, float, float, 
	//norm: float, float, float
	//tex: float, float
	DWORD fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(1); 
	BYTE textureCoordOffset = 6* sizeof(float);
	BYTE normalOffset = 3* sizeof(float);

	hr = mesh->SetFVF(pd3dDevice, fvf);
	if(FAILED(hr))
		return hr;

	
	
	//STEP 1:
	//	 fill up the generic structures that meshmender can work on.
	//	 fill up the vertex data
	DWORD numVerts = mesh->GetMesh()->GetNumVertices();
	void* pVertexData;
	hr = mesh->GetMesh()->LockVertexBuffer(D3DLOCK_NOSYSLOCK , &pVertexData);
	if(FAILED(hr))
		return hr;

	for(DWORD i = 0; i < numVerts; ++i)
	{
		MeshMender::Vertex v;
		BYTE* data = ((BYTE*)pVertexData) + i*D3DXGetFVFVertexSize( fvf);
		D3DXVECTOR3* positionArray = (D3DXVECTOR3*)(data);
		FLOAT* texCoordArray = (float*)(data + textureCoordOffset);
		D3DXVECTOR3* normalArray = (D3DXVECTOR3*)(data + normalOffset);
		v.pos = positionArray[0];
		v.s = texCoordArray[0];
		v.t = texCoordArray[1];
		v.normal = normalArray[0];

		//meshmender will computer tangents and binormals, no need to fill those in.
		theVerts.push_back(v);
	}

	mesh->GetMesh()->UnlockVertexBuffer();

	//fill up the index buffer
	WORD* pIndexData;//ASSUMPTION: the index buffer here is a WORD array
	hr = mesh->GetMesh()->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&pIndexData);
	if(FAILED(hr))
		return hr;
	DWORD numIndices =	mesh->GetMesh()->GetNumFaces()*3;

	for(DWORD ind= 0 ; ind < numIndices; ++ind)
	{
		theIndices.push_back(pIndexData[ind]);
	}

	hr = mesh->GetMesh()->UnlockIndexBuffer();
	if(FAILED(hr))
		return hr;

	//STEP 3:
	//		perform the actual mend
	std::vector< unsigned int > mappingNewToOldVert;
	bool success = theMender.Mend( 
				  theVerts,  
				  theIndices, 
				  mappingNewToOldVert,
				  minNormalsCreaseCosAngle,
				  minTangentsCreaseCosAngle,
				  minBinormalsCreaseCosAngle,
				  weightNormalsByArea,
				  computeNormals,
				  respectExistingSplits,
				  fixCylindricalWrapping);

	if(!success)
		return E_FAIL;


	//STEP 4:
	//		update the mesh with the mended data

	DWORD opts = mesh->GetMesh()->GetOptions();
	mesh->GetMesh()->Release();

	if(FAILED(hr = D3DXCreateMeshFVF(  theIndices.size()/3,
		theVerts.size(),
		opts,
		MeshMender::Vertex::FVF,		
		pd3dDevice,
		&(mesh->m_pMesh)
	)))
	{
		return E_FAIL;
	}

	//fill in our verts
	MeshMender::Vertex* pMenderVerts=0;
	mesh->GetMesh()->LockVertexBuffer(0, (LPVOID*)&pMenderVerts);
	for(unsigned int i = 0; i < theVerts.size(); ++i)
	{
		pMenderVerts[i] = theVerts[i];
	}
	mesh->GetMesh()->UnlockVertexBuffer();
	
	//fill our indices
	mesh->GetMesh()->LockIndexBuffer(0,(LPVOID*)&pIndexData);
	for(unsigned int i = 0 ; i < theIndices.size(); ++i)
	{
		pIndexData[i] = theIndices[i];
	}
	mesh->GetMesh()->UnlockIndexBuffer();

	//now mesh is the mesh
	return S_OK;

}

void RemendBasedOnUISettings(	)
{

	g_TheMesh.InvalidateDeviceObjects();
	g_TheMesh.Destroy();

	//reload the mesh so we start from scratch
	HRESULT hr = LoadMeshHelperFunction(&g_TheMesh, meshFile ,DXUTGetD3D9Device());
	if( FAILED( hr ) )
        return ;



	hr = MendTheMesh(DXUTGetD3D9Device(),&g_TheMesh,
		(g_HUD.GetSlider(IDC_SLIDER_NORMAL_CREASE)->GetValue() / 100.0f),
		(g_HUD.GetSlider(IDC_SLIDER_TANGENT_CREASE)->GetValue() / 100.0f),
		(g_HUD.GetSlider(IDC_SLIDER_BINORMAL_CREASE)->GetValue() / 100.0f),
		(g_HUD.GetSlider(IDC_SLIDER_NORMAL_WEIGHT)->GetValue() / 100.0f),

		g_HUD.GetCheckBox(IDC_CHECKBOX_CALCNORMALS)->GetChecked() ?
		MeshMender::CALCULATE_NORMALS : MeshMender::DONT_CALCULATE_NORMALS ,

		g_HUD.GetCheckBox(IDC_CHECKBOX_RESPECT_SPLITS)->GetChecked() ?
		MeshMender::RESPECT_SPLITS : MeshMender::DONT_RESPECT_SPLITS,

		g_HUD.GetCheckBox(IDC_CHECKBOX_FIX_CYLINDRICAL)->GetChecked() ?
		MeshMender::FIX_CYLINDRICAL : MeshMender::DONT_FIX_CYLINDRICAL 	);




	DWORD numVerts = g_TheMesh.GetMesh()->GetNumVertices();
	
	VOID* pVertexData;
	g_TheMesh.GetMesh()->LockVertexBuffer(D3DLOCK_READONLY, &pVertexData);

	D3DXVECTOR3 center;
	float radius;
	D3DXComputeBoundingSphere( 
		(D3DXVECTOR3*)pVertexData,
		numVerts, 
		D3DXGetFVFVertexSize( g_TheMesh.GetMesh()->GetFVF()),
		&center,	
		&radius);

	g_TheMesh.GetMesh()->UnlockVertexBuffer();

	D3DXVECTOR3 at = center;
	D3DXVECTOR3 eye = center - D3DXVECTOR3(0,0, 2.0*radius);
	g_Camera.SetViewParams(&eye, &at);
	g_Camera.SetAttachCameraToModel(true); //makes it look like the light moves arround it
	



	g_TheMesh.InvalidateDeviceObjects();
	g_TheMesh.RestoreDeviceObjects(DXUTGetD3D9Device());

	if( FAILED(hr))
		return;

}

void ReloadTexture()
{
	g_TheTexture->Release();

	//load an normalMap for our model
	tstring path = GetFilePath::GetFilePath(textureFile, false );
	if(path.empty())
		return ;//D3DAPPERR_MEDIANOTFOUND;

	HRESULT hr = D3DXCreateTextureFromFileEx( 
		DXUTGetD3D9Device(),
		path.c_str(),
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT ,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		0,
		0,
		&g_TheTexture
	);
	if( FAILED( hr ) )
	{
		
		return;
	}


}