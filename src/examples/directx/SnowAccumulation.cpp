//-----------------------------------------------------------------------------
// Copyright NVIDIA Corporation 2004
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED 
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS 
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL 
// NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR 
// CONSEQUENTIAL DAMAGES WHATSOEVER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR 
// LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS 
// INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR 
// INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGES.
// 
// File: SnowAccumultation.cpp
// Desc: This sample snows the use of a orthographic rendering into a RT to obtain 
//			the closest distance to the sky, and then use that value to create a snow coverage of the scene.
//		This method is being done in real time in this sample, but it is easily convceivable to preprocess a snow map,
//		and then jsut use this, as long as occluders don't move
//-----------------------------------------------------------------------------
#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/DXUTMesh.h>
#include <DXUT/SDKmisc.h>
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders

#include "nv_skybox.h"
#include <list>
#pragma warning( disable : 4311)	// we cast a void* to an int intentionally

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pSprite = NULL;       // Sprite for batching draw text calls
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
CFirstPersonCamera      g_Camera;               // A model viewing camera
CDXUTDialog             g_HUD;                  // manages the 3D UI
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
bool                    g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader

// NV Vars
#define WORLD_SIZE		1200.0f
int						g_iExposureMapResolution = 512;
nv_SkyBox *				g_skybox = new nv_SkyBox();
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface

LPDIRECT3DSURFACE9			g_pBBSurface = NULL;	// original swap chain surface
LPDIRECT3DSURFACE9			g_pBBDepthSurface = NULL;	// original swap chain depth surface
LPDIRECT3DTEXTURE9			g_pExposureTexture = NULL;
LPDIRECT3DTEXTURE9			g_pExposureTextureRGBA = NULL;
LPDIRECT3DTEXTURE9			g_pExposureTextureR32 = NULL;
LPDIRECT3DSURFACE9			g_pExposureSurface = NULL;
LPDIRECT3DTEXTURE9			g_pExposureDepthTexture = NULL;
LPDIRECT3DSURFACE9			g_pExposureDepthSurface = NULL;	// original swap chain depth surface
LPDIRECT3DVOLUMETEXTURE9	g_pNoiseTexture = NULL;

ID3DXEffect*				g_pExposureCalcEffect = NULL;
D3DXMATRIX					g_pExposureView;
D3DXMATRIX					g_pExposureOrthoProj;

bool bSM30Support;
bool bExposureChanged=true;

/*
	Simple wrapper class that allows me to easily place a couple of meshes around the world, and be able to
	control whether they accumulate snow or are just occluders
*/
class PlacedMesh
{
public:
	CDXUTXFileMesh*				pMesh;			// The mesh to draw
	D3DXMATRIXA16           mWorld;			// Where it is
	bool					bAccumSnow;		// does this object accumulate snow?
	bool					bOccludeSnow;	// does this object block snow from faling on object below it?
	float					fMaxOffset;		// if object accumulates, what is the max vertex offset to use?  Not needed, but makes effect look better.

	PlacedMesh(IDirect3DDevice9*pd3dDevice,LPCWSTR file, D3DXMATRIXA16&world, bool accum, bool occlude,float maxOffset)
	{
		HRESULT hr;
		WCHAR str[MAX_PATH];		
		V( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, file ) );
		pMesh = new CDXUTXFileMesh();
		V( pMesh->Create(pd3dDevice,str));
		mWorld = world;
		bAccumSnow = accum;
		bOccludeSnow = occlude;
		fMaxOffset = maxOffset;
	}
	virtual ~PlacedMesh()
	{
		if(pMesh)
		{
			pMesh->Destroy();
			delete pMesh;
		}
	}
	void InvalidateDeviceObjects() 	{pMesh->InvalidateDeviceObjects();}
	void RestoreDeviceObjects(IDirect3DDevice9*pd3dDevice) {pMesh->RestoreDeviceObjects(pd3dDevice);}
};

std::list<PlacedMesh*> g_SceneMeshList;
PlacedMesh *g_pShip = NULL;

CDXUTDirectionWidget g_LightControl;
float                g_fLightScale;
int                  g_nNumActiveLights;
int                  g_nActiveLight;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_OFFSETAMOUNT		5
#define IDC_SAMPLEDISTANCE		6
#define IDC_SNOWON				7
#define IDC_BASESNOWPCT			9
#define IDC_NORMALDISTORT		10
#define IDC_DOTNORMDISTORT		11
#define IDC_SHOWEXPMAP			12
#define IDC_SNOWBIAS			13

enum E_RENDERMODE
{
	RM_NOSNOW=0,
	RM_SM20=1,
	RM_SM30=2,
};

#define IDC_STATIC_BASE			100


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
void    RenderText( double fTime );
HRESULT RenderLightArrow( D3DXVECTOR3 lightDir, D3DXCOLOR arrowColor );

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
    DXUTCreateWindow( L"SnowAccumulation" );
    DXUTCreateDevice(  true, 640, 480  );

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
    g_bEnablePreshader = true;

    g_LightControl.SetLightDirection( D3DXVECTOR3( 0.5f, 0.8f, 0.5f ) );

    g_nActiveLight = 0;
    g_nNumActiveLights = 1;
    g_fLightScale = 1.0f;

    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );

	int iStaticCount= 0;

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"VTX Offset Scale:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_OFFSETAMOUNT,35,iY += 20,200,22,0,100,20);
	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Depth multi-sample distance:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_SAMPLEDISTANCE,35,iY += 20,200,22,0,300,150);

	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Base Snow %:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_BASESNOWPCT,35,iY += 20,200,22,0,100,40);

	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Snow Bias:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_SNOWBIAS,35,iY += 20,200,22,0,400,200);

	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Normal Noise Scale:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_NORMALDISTORT,35,iY += 20,200,22,0,100,20);

	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Incline Noise Scale:",35,iY += 24,200,22);
	g_SampleUI.AddSlider(IDC_DOTNORMDISTORT,35,iY += 20,200,22,0,100,20);

	g_SampleUI.AddStatic(IDC_STATIC_BASE+(iStaticCount++),L"Technique:",35,iY += 24,100,22);
	g_SampleUI.AddListBox(IDC_SNOWON,35,iY += 20,150,60);
	CDXUTListBox *pListBox = g_SampleUI.GetListBox(IDC_SNOWON);
	pListBox->AddItem(L"No Snow",(void*)RM_NOSNOW);
	pListBox->AddItem(L"SM2.0",(void*)RM_SM20);
	pListBox->AddItem(L"SM3.0",(void*)RM_SM30);
	pListBox->SelectItem(2);

	g_SampleUI.AddCheckBox(IDC_SHOWEXPMAP,L"Show Exposure Map?",35,iY += 62,200,22);
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // No fallback defined by this app, so reject any device that
    // doesn't support at least ps1.1
    if( pCaps->PixelShaderVersion < D3DPS_VERSION(1,1) )
        return false;

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
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
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    else
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }

    // This application is designed to work on a pure device by not using
    // IDirect3D9::Get*() methods, so create a pure device if supported and using HWVP.
    if ((pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 &&
        (pDeviceSettings->d3d9.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 )
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_PUREDEVICE;

    // Debugging vertex shaders requires either REF or software vertex processing
    // and debugging pixel shaders requires REF.
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

	// No SM3.0, so make a note in the selection box
	if(pCaps->VertexShaderVersion < D3DVS_VERSION(3,0))
	{
		bSM30Support = false;
		CDXUTListBox *pListBox = g_SampleUI.GetListBox(IDC_SNOWON);
		pListBox->RemoveItem(2);
		pListBox->AddItem(L"SM3.0(Not Supported)",(void*)RM_SM20);
	}
	else
	{
		bSM30Support = true;
	}

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

    D3DXVECTOR3 vCenter = D3DXVECTOR3(0,0,0);
	float fObjectRadius = 1.0;

    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );
    g_LightControl.SetRadius( fObjectRadius );

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the
    // shader debugger. Debugging vertex shaders requires either REF or software vertex
    // processing, and debugging pixel shaders requires REF.  The
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the
    // shader debugger.  It enables source level debugging, prevents instruction
    // reordering, prevents dead code elimination, and forces the compiler to compile
    // against the next higher available software target, which ensures that the
    // unoptimized shaders do not exceed the shader model limitations.  Setting these
    // flags will cause slower rendering since the shaders will be unoptimized and
    // forced into software.  See the DirectX documentation for more information about
    // using the shader debugger.
    DWORD dwShaderFlags = 0;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Preshaders are parts of the shader that the effect system pulls out of the
    // shader and runs on the host CPU. They should be used if you are GPU limited.
    // The D3DXSHADER_NO_PRESHADER flag disables preshaders.
    if( !g_bEnablePreshader )
        dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

    // Read the D3DX effect file for the snow accum
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\programs\\SnowAccumulation\\Snow.cso" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

	// Seperate fx file for the exposure map calculation, and set its projection matrix
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\programs\\SnowAccumulation\\OrthoDepth.cso" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pExposureCalcEffect, NULL ) );

	// make our view proj matrix for the "snow"'s view.
	// $$ These values control the range for the encoded depths values, so fitting it close to the 
	//		actual range of the possible values will results in better precision utilization.
	// Be careful if your occluder objects fall outside this clipping range, the they will not occlude at all.
	D3DXMatrixOrthoLH(&g_pExposureOrthoProj,WORLD_SIZE,WORLD_SIZE,1.f,1000.f);
	D3DXMatrixLookAtLH(&g_pExposureView,&D3DXVECTOR3(0,540,0),&D3DXVECTOR3(0,0,0),&D3DXVECTOR3(0,0,1));

	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mTrans,mRot,mRot2,mScale;
	PlacedMesh* pPlacedMesh;
	LPDIRECT3DTEXTURE9 pTexture;

	// Load some trees or other interesting objects, and then the terrain.  This is all hard coded since it is just a sample.

	// ------------------
#if 1
	// A rock, gets and occludes snow
	D3DXMatrixTranslation(&mTrans,100,-30,30);
	D3DXMatrixRotationZ(&mRot,-3.141592654f/2);
	D3DXMatrixScaling(&mScale,50,50,50);
	mWorld = mScale*mRot*mTrans;

	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\aster1.x",mWorld,true,true,15.f);
	
	// $$$$ Need to override the texture definition for this one.  It has no textures bound.
	V( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SnowAccumulation\\terrainRock.bmp" ) );
	V( D3DXCreateTextureFromFile(pd3dDevice,str,&pTexture) );
	V(pTexture->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&pPlacedMesh->pMesh->m_pTextures[0] ));	// Obtain the base texture interface
	V(pTexture->Release());// Release the specialized instance

	g_SceneMeshList.push_back(pPlacedMesh);
#endif

	// ------------------
#if 1
	// A Tree, gets and occludes
	D3DXMatrixTranslation(&mTrans,80,-50,90);
	//	D3DXMatrixRotationZ(&mRot,-3.141592654f/2);
	D3DXMatrixScaling(&mScale,25,15,25);
	mWorld = mScale*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\snowAccumulation_tree.x",mWorld,true,true,8.f);

	V( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SnowAccumulation\\bark.tga" ) );
	V( D3DXCreateTextureFromFile(pd3dDevice,str,&pTexture) );

	// $$$$ Need to override the texture definition for this one.  It has no textures bound.
	for(int i=0;i<(int)pPlacedMesh->pMesh->m_dwNumMaterials;i++)
	{
		V(pTexture->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&pPlacedMesh->pMesh->m_pTextures[i] ));	// Obtain the base texture interface
	}
	V(pTexture->Release());// Release the specialized instance

	g_SceneMeshList.push_back(pPlacedMesh);
#endif

	// ------------------
#if 1
	// A Tree, gets and occludes
	D3DXMatrixTranslation(&mTrans,230,10,-185);
	D3DXMatrixScaling(&mScale,35,35,35);
	mWorld = mScale*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\tree_0.x",mWorld,true,true,4.f);
	g_SceneMeshList.push_back(pPlacedMesh);

	D3DXMatrixTranslation(&mTrans,150,20,-200);
	D3DXMatrixScaling(&mScale,35,40,35);
	mWorld = mScale*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\tree_0.x",mWorld,true,true,4.f);
	g_SceneMeshList.push_back(pPlacedMesh);

	D3DXMatrixTranslation(&mTrans,100,20,-280);
	D3DXMatrixScaling(&mScale,45,45,45);
	mWorld = mScale*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\tree_0.x",mWorld,true,true,4.f);
	g_SceneMeshList.push_back(pPlacedMesh);

	D3DXMatrixTranslation(&mTrans,0,35,-190);
	D3DXMatrixScaling(&mScale,55,35,55);
	mWorld = mScale*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"models\\SnowAccumulation\\tree_0.x",mWorld,true,true,4.f);
	g_SceneMeshList.push_back(pPlacedMesh);

#endif

	// ------------------
	// Snowman!!
#if 1
	D3DXMatrixTranslation(&mTrans,-200,-55,150);
	D3DXMatrixRotationZ(&mRot,3.141592654f/20);
	D3DXMatrixRotationY(&mRot2,3.141592654f*1.3f);
	D3DXMatrixScaling(&mScale,20,20,20);
	mWorld = mScale*mRot2*mRot*mTrans;
	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\snowman.x",mWorld,false,true,0.f);
	g_SceneMeshList.push_back(pPlacedMesh);
#endif


	// ---------------
	// Our ground mesh, loaded last since it'll generally be behind everything, so this is "rough front to back sorting"
	D3DXMatrixTranslation( &mTrans, 0.0f, -10.f, 0.f );
	//D3DXMatrixRotationX(&mRot,3.141592654f/6);
	D3DXMatrixScaling(&mScale,5.f,5.f,5.f);
	mWorld = mScale*mTrans;

	pPlacedMesh = new PlacedMesh(pd3dDevice,L"MEDIA\\models\\SnowAccumulation\\snowAccumulation_terrain.x",mWorld,true,true,25.f);

	// $$$$ This is ugly, but i cant edit x files, and these meshes dont have smothed normals and it looks horrible.
	//  so set the alpha for all non smooth objects to 0, so they dont draw...
	for(unsigned int i=0;i<pPlacedMesh->pMesh->m_dwNumMaterials;i++)
	{
		if(i==0) continue;
		pPlacedMesh->pMesh->m_pMaterials[i].Diffuse.a = 0.0f;
	}
	g_SceneMeshList.push_back(pPlacedMesh);

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

    WCHAR str[MAX_PATH];

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );
	if( g_pExposureCalcEffect )
		V_RETURN( g_pExposureCalcEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite ) );

	g_LightControl.OnD3D9ResetDevice( pBackBufferSurfaceDesc  );

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 1.0f, 5000.0f );
	g_Camera.SetRotateButtons(true,false,false);

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye(-15.0f, 30.0f, 440.0f);
	D3DXVECTOR3 vecAt (0.0f, -20.0f, 0.0f);
	g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetScalers(0.01f,95.f);

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-270, 50 );
    g_SampleUI.SetSize( 170, 300 );

	bExposureChanged = true;

	//---------------------------------------------------------------------------
	// initilize our sky box and it's cube map

	LPDIRECT3DCUBETEXTURE9 cubeTex;
	g_skybox->Init(pd3dDevice);
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SnowAccumulation\\CloudyHillsCubemap.dds" ) );

	V(D3DXCreateCubeTextureFromFileEx(pd3dDevice, str,D3DX_DEFAULT,0,0,D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,D3DX_FILTER_LINEAR,
		D3DX_FILTER_LINEAR,0,NULL,NULL,&cubeTex));
	g_skybox->SetCubeMap(cubeTex);

	// Used when we offset in the exposure map
	V( g_pEffect->SetFloat("SceneWidth",(float)pBackBufferSurfaceDesc->Width));
	V( g_pEffect->SetFloat("SceneHeight",(float)pBackBufferSurfaceDesc->Height));

	// Store off BB
	pd3dDevice->GetRenderTarget(0,&g_pBBSurface);
	pd3dDevice->GetDepthStencilSurface(&g_pBBDepthSurface);

	// Make an RGBA RT possibly to store the exposure values
	V(D3DXCreateTexture(pd3dDevice,	g_iExposureMapResolution,g_iExposureMapResolution,
		0,D3DUSAGE_RENDERTARGET|D3DUSAGE_AUTOGENMIPMAP,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
		&g_pExposureTextureRGBA));

	// Need to have both surfaces since can't StretchRect R32 to backbuffer for visualization.  In normal situation, only need one.
	if(bSM30Support)
	{
		// Make a 32bit float RT for storing the exposure depth
		V(D3DXCreateTexture(pd3dDevice,	g_iExposureMapResolution,g_iExposureMapResolution,
			0,D3DUSAGE_RENDERTARGET|D3DUSAGE_AUTOGENMIPMAP,D3DFMT_R32F,D3DPOOL_DEFAULT,
			&g_pExposureTextureR32));

		g_pExposureTextureR32->GetSurfaceLevel(0,&g_pExposureSurface);
		g_pExposureTexture = g_pExposureTextureR32;
	}
	else
	{
		g_pExposureTextureRGBA->GetSurfaceLevel(0,&g_pExposureSurface);
		g_pExposureTexture = g_pExposureTextureRGBA;
	}

	V(D3DXCreateTexture(pd3dDevice,g_iExposureMapResolution,g_iExposureMapResolution,0,D3DUSAGE_DEPTHSTENCIL,D3DFMT_D24X8,D3DPOOL_DEFAULT,&g_pExposureDepthTexture));
	V( g_pExposureDepthTexture->GetSurfaceLevel(0,&g_pExposureDepthSurface));

	// This is our noise volume texture, just has some random values.  Combined in shader using a perlin's method
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SnowAccumulation\\noiseL8_32x32x32.dds" ) );
	V(D3DXCreateVolumeTextureFromFile(pd3dDevice,str,&g_pNoiseTexture));

	for(std::list<PlacedMesh*>::iterator i=g_SceneMeshList.begin();i!= g_SceneMeshList.end();i++)
		(*i)->RestoreDeviceObjects(pd3dDevice);

    return S_OK;
}

/*
	Renders out a R32F RT which contains the closest depth value to the falling snow.  Note that near and far have been
	pre set to tightly fit our scene.  This ensures more precision in our depth values.
*/
void RenderOrthoDepth(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	UINT iPass, cPasses;
	// Render out the closest depth to the sky...
	V( pd3dDevice->SetRenderTarget(0,g_pExposureSurface));
	V( pd3dDevice->SetDepthStencilSurface(g_pExposureDepthSurface));

	// $ Note when clearing we don't clear to black, but this is ok, we clear to 1 for the red component which is enough
	//		the color is handy for visualization in debug mode
	V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(1.0f,0.25f,0.25f,0.55f), 1.0f, 0) );

	V( g_pExposureCalcEffect->SetTechnique("RenderDepthPass") );
	V( g_pExposureCalcEffect->SetMatrix("mView",&g_pExposureView));
	V( g_pExposureCalcEffect->SetMatrix("mProj",&g_pExposureOrthoProj));
	V( g_pExposureCalcEffect->SetMatrix("mViewProj",&(g_pExposureView*g_pExposureOrthoProj)));
	V( g_pExposureCalcEffect->Begin(&cPasses, 0) );
	for (iPass = 0; iPass < cPasses; iPass++)
	{
		V( g_pExposureCalcEffect->BeginPass(iPass) );
		for(std::list<PlacedMesh*>::iterator i=g_SceneMeshList.begin();i!= g_SceneMeshList.end();i++)
		{
			if((*i)->bOccludeSnow)
			{
				D3DXMATRIXA16 mWorld = (*i)->mWorld;
				V( g_pExposureCalcEffect->SetMatrix("mWorldViewProjection",&(mWorld*g_pExposureView*g_pExposureOrthoProj)));
				V( g_pExposureCalcEffect->SetMatrix("mWorld",&mWorld));
				V( g_pExposureCalcEffect->CommitChanges());
				(*i)->pMesh->Render(pd3dDevice,true,false);
			}
		}
		V( g_pExposureCalcEffect->EndPass() );
	}
	V( g_pExposureCalcEffect->End() );

	V(pd3dDevice->SetRenderTarget(0,g_pBBSurface));
	V(pd3dDevice->SetDepthStencilSurface((g_pBBDepthSurface)));
}

void RenderScene(IDirect3DDevice9 *pd3dDevice, bool accum)
{
	HRESULT hr;
	UINT iPass, cPasses;	
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;

	// Get the projection & view matrix from the camera class
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();

	V( g_pEffect->SetValue( "g_LightDir", &g_LightControl.GetLightDirection(), sizeof(D3DXVECTOR3) ) );
	V( g_pEffect->SetValue( "g_LightDiffuse", &(g_fLightScale * D3DXCOLOR(1,1,1,1)), sizeof(D3DXVECTOR4) ) );
	V( g_pEffect->SetMatrix( "g_mProjection", &mProj ) );
	V( g_pEffect->SetMatrix( "g_mViewProj", &(mView*mProj)) );
	D3DXMATRIXA16 mViewInv;
	D3DXMatrixInverse(&mViewInv,NULL,&mView);
	V( g_pEffect->SetMatrix( "g_mViewInverse", &mViewInv) );
	
	V( g_pEffect->SetTexture("g_ExposureDepthMapTexture",g_pExposureTexture));
	V( g_pEffect->SetTexture("g_Noise3D",g_pNoiseTexture));

	D3DXCOLOR vWhite = D3DXCOLOR(1,1,1,1);
	V( g_pEffect->SetValue("g_MaterialDiffuseColor", &vWhite, sizeof(D3DXCOLOR) ) );
	V( g_pEffect->SetValue("g_SnowColor",&D3DXCOLOR(0.90f,0.95f,1.0f,1.0f),sizeof(D3DXCOLOR)));

	V( g_pEffect->SetFloat( "SampleDistance",(float)g_SampleUI.GetSlider(IDC_SAMPLEDISTANCE)->GetValue()/100.f));
	V( g_pEffect->SetFloat( "SpecExpon",(float)300.f));
	V( g_pEffect->SetFloat( "BaseSnowPct",(float)g_SampleUI.GetSlider(IDC_BASESNOWPCT)->GetValue()/100.f));
	V( g_pEffect->SetFloat( "SnowBias",(float)g_SampleUI.GetSlider(IDC_SNOWBIAS)->GetValue()/10000.f));
	V( g_pEffect->SetFloat( "normalDistortionAmount",(float)g_SampleUI.GetSlider(IDC_NORMALDISTORT)->GetValue()/1000.f));
	V( g_pEffect->SetFloat( "dotNormalDistortionAmount",(float)g_SampleUI.GetSlider(IDC_DOTNORMDISTORT)->GetValue()/100.f));
		
	V( g_pEffect->SetVector("g_LightAmbient",&D3DXVECTOR4(0.3f,0.3f,0.3f,0.0)));

	// Apply the technique contained in the effect
	V( g_pEffect->Begin(&cPasses, 0) );
	for (iPass = 0; iPass < cPasses; iPass++)
	{
		V( g_pEffect->BeginPass(iPass) );
		for(std::list<PlacedMesh*>::iterator i=g_SceneMeshList.begin();i!= g_SceneMeshList.end();i++)
		{
			D3DXMATRIX mWorld = (*i)->mWorld;

			if(accum == (*i)->bAccumSnow)
			{
				D3DXMATRIXA16 mInvWorld;
				D3DXMatrixInverse(&mInvWorld,NULL,&mWorld);
				D3DXVECTOR4 mUpObjectSpace;
				D3DXVec3Transform(&mUpObjectSpace,&D3DXVECTOR3(0,1,0),&mInvWorld);
				V( g_pEffect->SetVector("g_upObjectSpace",&mUpObjectSpace));
				V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &(mWorld*mView * mProj)));
				V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );
				D3DXMATRIXA16 mWorldIT;
				D3DXMatrixTranspose(&mWorldIT,D3DXMatrixInverse(&mWorldIT,NULL,&mWorld));
				V( g_pEffect->SetMatrix( "g_mWorldIT", &mWorldIT ) );
				V( g_pEffect->SetMatrix( "g_mWorldView", &(mWorld*mView) ) );
				V( g_pEffect->SetMatrix( "g_mExposureWorldViewOrthoProj",&(mWorld*g_pExposureView*g_pExposureOrthoProj)));
				V( g_pEffect->SetFloat( "OffsetAmount", (*i)->fMaxOffset * (float)g_SampleUI.GetSlider(IDC_OFFSETAMOUNT)->GetValue()/100.f ) );
				V( g_pEffect->CommitChanges() );
				(*i)->pMesh->Render(pd3dDevice,true,false);
			}
		}
		
		V( g_pEffect->EndPass() );
	}
	V( g_pEffect->End() );
}

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not
// intended to contain actual rendering calls, which should instead be placed in the
// OnFrameRender callback.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input
    g_Camera.FrameMove( fElapsedTime );

	// if something moves
	// bExposureChanged = true;

#if 0
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mTrans,mRot,mScale;
	D3DXMatrixTranslation(&mTrans,50*(float)sin((double)fTime*3.141592654/4),10,0);
	D3DXMatrixRotationZ(&mRot,3.141592654f/2);
	D3DXMatrixScaling(&mScale,9,9,9);
	mWorld = mScale*mRot*mTrans;
	g_pShip->mWorld = mWorld;
#endif
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
    D3DXMATRIXA16 mView;

    // Clear the render target and the zbuffer
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.0f,0.25f,0.25f,0.55f), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		CDXUTListBox *pListBox = g_SampleUI.GetListBox(IDC_SNOWON);
		DXUTListBoxItem *pItem = pListBox->GetSelectedItem();
		int renderMode = pItem?(int)pItem->pData:0;
		// Renders all possible snow occluding objects and generates a distance from the sky surface, used in exposure calculation
		if(bExposureChanged && (renderMode == RM_SM20 || renderMode == RM_SM30))
		{
			RenderOrthoDepth(pd3dDevice);
			bExposureChanged = false;
		}

		// Draw us a nice sky box
		mView = *g_Camera.GetViewMatrix();	// using the view makes the box move around when we move.
		g_skybox->Render(pd3dDevice,mView);

		// $$$ Need to have skybox not render Z, or something...
		V( pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DXCOLOR(0.0f,0.25f,0.25f,0.55f), 1.0f, 0) );

		// render all non snowed surfaces first.
		V( g_pEffect->SetTechnique( "RenderScene_NoSnow" ) );
		RenderScene(pd3dDevice,false);

		// Render all snow accumulated surfaces
		switch(renderMode)
		{
			case RM_NOSNOW:
			default:
				V( g_pEffect->SetTechnique( "RenderScene_NoSnow" ) );
				break;
			case RM_SM20:
				V( g_pEffect->SetTechnique( "RenderScene_SM20" ) );
				break;
			case RM_SM30:
				V( g_pEffect->SetTechnique( "RenderScene_SM30" ) );
				break;
		}
		RenderScene(pd3dDevice,true);

		// $$ This is just for the visualization of the map, not needed really.
		if(g_SampleUI.GetCheckBox(IDC_SHOWEXPMAP)->GetChecked())
		{
			// visualize the depth surface, only works with RGBA depth surface, not R32F
			RECT destRect;
			D3DSURFACE_DESC desc;
			g_pBBSurface->GetDesc(&desc);
			destRect.left = 0;
			destRect.right = g_iExposureMapResolution/2;
			destRect.top = desc.Height-(g_iExposureMapResolution/2);
			destRect.bottom = desc.Height;

			if(g_pExposureTextureR32 != NULL)
			{
				g_pExposureSurface->Release();
				g_pExposureTextureRGBA->GetSurfaceLevel(0,&g_pExposureSurface);
				RenderOrthoDepth(pd3dDevice);
				V(pd3dDevice->StretchRect(g_pExposureSurface,NULL,g_pBBSurface,&destRect,D3DTEXF_NONE));
				g_pExposureSurface->Release();
				g_pExposureTextureR32->GetSurfaceLevel(0,&g_pExposureSurface);
			}
			else
			{
				V(pd3dDevice->StretchRect(g_pExposureSurface,NULL,g_pBBSurface,&destRect,D3DTEXF_NONE));
			}

			CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );
			txtHelper.Begin();
			txtHelper.SetInsertionPos( 2, 0 );
			txtHelper.SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 1.0f, 1.0f ) );
			txtHelper.DrawTextLine( DXUTGetFrameStats() );
			txtHelper.End();
		}


        g_HUD.OnRender( fElapsedTime );
        g_SampleUI.OnRender( fElapsedTime );

        RenderText( fTime );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText( double fTime )
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 0.0f, 0.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats() );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );


/*    No HELP for YOU!
// Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls:" );

        txtHelper.SetInsertionPos( 20, pd3dsdBackBuffer->Height-15*5 );
        txtHelper.DrawTextLine( L"Snow Depth Map Resolution is \n"
                                L"Two Line\n");
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }*/
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
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

	// $$ Uncomment to use right mouse to move the light.
	//    g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

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
            case VK_F1: g_bShowHelp = !g_bShowHelp; break;
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

    CDXUTDirectionWidget::StaticOnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
	if(g_pExposureCalcEffect)
		g_pExposureCalcEffect->OnLostDevice();

	g_skybox->Destroy();

	for(std::list<PlacedMesh*>::iterator i=g_SceneMeshList.begin();i!= g_SceneMeshList.end();i++)
	{
		(*i)->InvalidateDeviceObjects();
	}

	SAFE_RELEASE(g_pExposureTextureR32);
	SAFE_RELEASE(g_pExposureTextureRGBA);
	SAFE_RELEASE(g_pExposureSurface);
	SAFE_RELEASE(g_pBBSurface);
	SAFE_RELEASE(g_pExposureDepthTexture);
	SAFE_RELEASE(g_pExposureDepthSurface);
	SAFE_RELEASE(g_pBBDepthSurface);
	SAFE_RELEASE(g_pNoiseTexture);

    SAFE_RELEASE(g_pSprite);

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
    CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();
    SAFE_RELEASE(g_pEffect);
    SAFE_RELEASE(g_pFont);
	while(!g_SceneMeshList.empty())
	{
		PlacedMesh *pPlacedMesh = g_SceneMeshList.back();
		g_SceneMeshList.pop_back();
		delete pPlacedMesh;
	}

	SAFE_RELEASE(g_pExposureCalcEffect);
}



