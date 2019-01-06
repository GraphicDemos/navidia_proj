#include "dxstdafx.h"

#include "AtlasViewer.h"
#include "resource.h"
#include "shared/NV_Common.h"
#include <shared/GetFilePath.h>
#include <string>
#include <stdlib.h>
#include <DXUT/SDKmisc.h>
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
CDXUTDialog		g_HUD;
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs

enum
{
    IDC_LOAD,
    IDC_SELTEX_U,IDC_SELTEX_D,IDC_SELTEX_S,
    IDC_TEXFILTER,IDC_TEXFILTER_S,
    IDC_DISPLAYMODES,IDC_DISPLAYMODES_S,
    IDC_MAGDIFF,
    IDC_WRAPCLAMP,
    IDC_NUMWRAPS_U,IDC_NUMWRAPS_D,IDC_NUMWRAPS_S,
    IDC_ATLASTEX,
    IDC_NUMQUADS_U,IDC_NUMQUADS_D,IDC_NUMQUADS_S,
    IDC_PERFVIZMODE,
    IDC_TLQ_S,IDC_TRQ_S,IDC_BLQ_S,IDC_BRQ_S,
    IDC_PERF_S,
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

#include <TCHAR.H>
typedef std::basic_string<TCHAR> tstring; 

enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

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
    DXUTCreateWindow( L"AtlasComparisonViewer" );
    DXUTCreateDevice( true, 1024, 768 );

    DXUTMainLoop();

	return DXUTGetExitCode();
}

void GetFormat(D3DFORMAT const format, TCHAR* string)
{
    switch(format)
    {
        case D3DFMT_R8G8B8:
            _stprintf(string, _TEXT("R8G8B8"));
            break;
        case D3DFMT_A8R8G8B8:
            _stprintf(string, _TEXT("A8R8G8B8"));
            break;
        case D3DFMT_X8R8G8B8:
            _stprintf(string, _TEXT("X8R8G8B8"));
            break;
        case D3DFMT_R5G6B5:
            _stprintf(string, _TEXT("R5G6B5"));
            break;
        case D3DFMT_X1R5G5B5:
            _stprintf(string, _TEXT("X1R5G5B5"));
            break;
        case D3DFMT_A1R5G5B5:
            _stprintf(string, _TEXT("A1R5G5B5"));
            break;
        case D3DFMT_A4R4G4B4:
            _stprintf(string, _TEXT("A4R4G4B4"));
            break;
        case D3DFMT_R3G3B2:
            _stprintf(string, _TEXT("R3G3B2"));
            break;
        case D3DFMT_A8:
            _stprintf(string, _TEXT("A8"));
            break;
        case D3DFMT_A8R3G3B2:
            _stprintf(string, _TEXT("A8R3G3B2"));
            break;
        case D3DFMT_X4R4G4B4:
            _stprintf(string, _TEXT("X4R4G4B4"));
            break;
        case D3DFMT_A2B10G10R10:
            _stprintf(string, _TEXT("A2B10G10R10"));
            break;
        case D3DFMT_A8B8G8R8:
            _stprintf(string, _TEXT("A8B8G8R8"));
            break;
        case D3DFMT_X8B8G8R8:
            _stprintf(string, _TEXT("X8B8G8R8"));
            break;
        case D3DFMT_G16R16:
            _stprintf(string, _TEXT("G16R16"));
            break;
        case D3DFMT_A2R10G10B10:
            _stprintf(string, _TEXT("A2R10G10B10"));
            break;
        case D3DFMT_A16B16G16R16:
            _stprintf(string, _TEXT("A16B16G16R16"));
            break;
        case D3DFMT_A8P8:
            _stprintf(string, _TEXT("A8P8"));
            break;
        case D3DFMT_P8:
            _stprintf(string, _TEXT("P8"));
            break;
        case D3DFMT_L8:
            _stprintf(string, _TEXT("L8"));
            break;
        case D3DFMT_A8L8:
            _stprintf(string, _TEXT("A8L8"));
            break;
        case D3DFMT_A4L4:
            _stprintf(string, _TEXT("A4L4"));
            break;
        case D3DFMT_V8U8:
            _stprintf(string, _TEXT("V8U8"));
            break;
        case D3DFMT_L6V5U5:
            _stprintf(string, _TEXT("L6V5U5"));
            break;
        case D3DFMT_X8L8V8U8:
            _stprintf(string, _TEXT("X8L8V8U8"));
            break;
        case D3DFMT_Q8W8V8U8:
            _stprintf(string, _TEXT("Q8W8V8U8"));
            break;
        case D3DFMT_V16U16:
            _stprintf(string, _TEXT("V16U16"));
            break;
        case D3DFMT_A2W10V10U10:
            _stprintf(string, _TEXT("A2W10V10U10"));
            break;
        case D3DFMT_UYVY:
            _stprintf(string, _TEXT("UYVY"));
            break;
        case D3DFMT_R8G8_B8G8:
            _stprintf(string, _TEXT("R8G8_B8G8"));
            break;
        case D3DFMT_YUY2:
            _stprintf(string, _TEXT("YUY2"));
            break;
        case D3DFMT_G8R8_G8B8:
            _stprintf(string, _TEXT("G8R8_G8B8"));
            break;
        case D3DFMT_DXT1:
            _stprintf(string, _TEXT("DXT1"));
            break;
        case D3DFMT_DXT2:
            _stprintf(string, _TEXT("DXT2"));
            break;
        case D3DFMT_DXT3:
            _stprintf(string, _TEXT("DXT3"));
            break;
        case D3DFMT_DXT4:
            _stprintf(string, _TEXT("DXT4"));
            break;
        case D3DFMT_DXT5:
            _stprintf(string, _TEXT("DXT5"));
            break;
        case D3DFMT_D16_LOCKABLE:
            _stprintf(string, _TEXT("D16_LOCKABLE"));
            break;
        case D3DFMT_D32:
            _stprintf(string, _TEXT("D32"));
            break;
        case D3DFMT_D15S1:
            _stprintf(string, _TEXT("D15S1"));
            break;
        case D3DFMT_D24S8:
            _stprintf(string, _TEXT("D24S8"));
            break;
        case D3DFMT_D24X8:
            _stprintf(string, _TEXT("D24X8"));
            break;
        case D3DFMT_D24X4S4:
            _stprintf(string, _TEXT("D24X4S4"));
            break;
        case D3DFMT_D16:
            _stprintf(string, _TEXT("D16"));
            break;
        case D3DFMT_D32F_LOCKABLE:
            _stprintf(string, _TEXT("D32F_LOCKABLE"));
            break;
        case D3DFMT_D24FS8:
            _stprintf(string, _TEXT("D24FS8"));
            break;
        case D3DFMT_L16:
            _stprintf(string, _TEXT("L16"));
            break;
        case D3DFMT_Q16W16V16U16:
            _stprintf(string, _TEXT("Q16W16V16U16"));
            break;
        case D3DFMT_MULTI2_ARGB8:
            _stprintf(string, _TEXT("MULTI2_ARGB8"));
            break;
        case D3DFMT_R16F:
            _stprintf(string, _TEXT("R16F"));
            break;
        case D3DFMT_G16R16F:
            _stprintf(string, _TEXT("G16R16F"));
            break;
        case D3DFMT_A16B16G16R16F:
            _stprintf(string, _TEXT("A16B16G16R16F"));
            break;
        case D3DFMT_R32F:
            _stprintf(string, _TEXT("R32F"));
            break;
        case D3DFMT_G32R32F:
            _stprintf(string, _TEXT("G32R32F"));
            break;
        case D3DFMT_A32B32G32R32F:
            _stprintf(string, _TEXT("A32B32G32R32F"));
            break;
        case D3DFMT_CxV8U8:
            _stprintf(string, _TEXT("CxV8U8"));
            break;
        case D3DFMT_UNKNOWN:    // fall through to default case
        default: 
            _stprintf(string, _TEXT("Unknown format"));
            break;
    }
}

void InitApp()
{
    g_HUD.Init( &g_DialogResourceManager );
	g_HUD.SetFont( 0, L"Arial", 14, 400 );
    g_HUD.SetCallback( OnGUIEvent );
	g_HUD.AddButton(IDC_LOAD,L"Load .tai File",0,0,125,22);

	g_HUD.AddButton(IDC_SELTEX_U,L"--->",400,300,40,22);
	g_HUD.AddButton(IDC_SELTEX_D,L"<---",300,300,40,22);
	g_HUD.AddStatic(IDC_SELTEX_S,L"Select Texture in Atlas",300,300,120,22);

	CDXUTListBox* pList;
	g_HUD.AddStatic(IDC_TEXFILTER_S,L"Filtering Modes:",0,0,80,22);
	g_HUD.AddListBox(IDC_TEXFILTER,0,0,225,100,0,&pList);
	if(pList)	{
		pList->AddItem(L"Trilinar Filtering",(void*)0);
		pList->AddItem(L"2x Aniso Texture- and Linear Map-Filtering",(void*)1);
		pList->AddItem(L"4x Aniso Texture- and Linear Map-Filtering",(void*)2);
		pList->AddItem(L"8x Aniso Texture- and Linear Map-Filtering",(void*)3);
		pList->AddItem(L"16x Aniso Texture- and Linear Map-Filtering",(void*)4);
		pList->AddItem(L"Point Texture- and Mip-Filtering",(void*)5);
		pList->AddItem(L"Bilinar Texture- and Point Mip-Filtering",(void*)6);
	}
	g_HUD.GetListBox(IDC_TEXFILTER)->SelectItem(0);

	g_HUD.AddStatic(IDC_DISPLAYMODES_S,L"Display Modes:",0,0,80,22);
	g_HUD.AddListBox(IDC_DISPLAYMODES,0,0,200,60,0,&pList);
	if(pList)
	{
		pList->AddItem(L"Original Adjusted, Atlas Adjusted",(void*)0);
		pList->AddItem(L"Original NOT Adjusted, Atlas Adjusted",(void*)1);
		pList->AddItem(L"Original NOT Adjusted, Atlas NOT Adjusted",(void*)2);
	}
	g_HUD.GetListBox(IDC_DISPLAYMODES)->SelectItem(0);

	g_HUD.AddButton(IDC_MAGDIFF,L"Show Magnified Difference(Z)",0,0,180,22,L'Z');//"Stop Mag Diff(z)"
	g_HUD.AddButton(IDC_WRAPCLAMP,L"Texture (A)ddress Mode: None",0,0,180,22,L'A');

	g_HUD.AddStatic(IDC_NUMWRAPS_S,L"Number of Wraps",0,0,130,22);
	g_HUD.AddButton(IDC_NUMWRAPS_U,L"+",100,200,40,22);
	g_HUD.AddButton(IDC_NUMWRAPS_D,L"-",125,200,40,22);
	g_HUD.GetControl(IDC_NUMWRAPS_U)->SetEnabled(false);
	g_HUD.GetControl(IDC_NUMWRAPS_D)->SetEnabled(false);

	g_HUD.AddButton(IDC_ATLASTEX,L"Rendering from Texture",0,0,130,22);//"Render from Texture"
	g_HUD.GetButton(IDC_ATLASTEX)->SetVisible(false);

	g_HUD.AddStatic(IDC_NUMQUADS_S,L"# of Quads",0,0,120,22);
	g_HUD.AddStatic(IDC_PERF_S,L"Draw calls",0,0,300,22);
	g_HUD.AddButton(IDC_NUMQUADS_U,L"+",100,200,40,22);
	g_HUD.AddButton(IDC_NUMQUADS_D,L"-",125,200,40,22);
	g_HUD.GetStatic(IDC_NUMQUADS_S)->SetVisible(false);
	g_HUD.GetStatic(IDC_PERF_S)->SetVisible(false);
	g_HUD.GetButton(IDC_NUMQUADS_U)->SetVisible(false);
	g_HUD.GetButton(IDC_NUMQUADS_D)->SetVisible(false);

	g_HUD.AddButton(IDC_PERFVIZMODE,L"Enter (P)erformance Viz Mode",200,50,180,22,L'P');//"Exit performance mode"

	g_HUD.AddStatic(IDC_TLQ_S,L"Original Texture\n",0,0,200,50);
	g_HUD.AddStatic(IDC_TRQ_S,L"From Texture Atlas",0,0,150,22);
	g_HUD.AddStatic(IDC_BLQ_S,L"Difference (abs(A-B))\nBlack means no difference",0,0,200,50);
	g_HUD.AddStatic(IDC_BRQ_S,L"Complete Texture Atlas\n",0,0,200,50);

	g_strWindowTitle = TEXT("AtlasComparisonViewer");
	g_pFont                     = NULL;
    g_bLoadingApp               = TRUE;
    g_bLoadingPerf              = FALSE;
    mbShowHelp                  = FALSE;
    mbHalfTexel                 = FALSE;
    mbPerfVizMode               = FALSE;
    mbAtlasMode                 = FALSE;
	gbShowMag					= FALSE;
    gMode                       = kToedInOrgToedInAtlas;
    mFilter                     = kTrilinear;
    mFilterMax                  = kTrilinear;
    mTexAddress                 = kNone;
    mRepeat                     = kRepeatInit;

    g_pVB                       = NULL;
    g_pPerfVB                   = NULL;
    g_pIndex                    = NULL;


	g_fWorldRotX                = 0.0f;
    g_fWorldRotY                = 0.0f;
    g_fZoom                     = 0.0f;

    g_InputFileName             = TEXT("");
    g_FirstLine                 = 0;
    g_LastLine                  = 0;
    g_CurrentLine               = 0;

    g_pCurrentTexture           = NULL;
    g_pCurrentTextureAtlas      = NULL;
    g_pCurrentVolumeAtlas       = NULL;
    gpAbsDifferenceShader       = NULL;
    gpAbsDifferenceMagShader    = NULL;
    mpTexAtlasHiLite            = NULL;
    mpTexWrap                   = NULL;
    mpTexWrapAbsDiff            = NULL;
    mpTexWrapAbsDiffMag         = NULL;
    mpTexClamp                  = NULL;
    mpTexClampAbsDiff           = NULL;
    mpTexClampAbsDiffMag        = NULL;
    mpTexMirror                 = NULL;
    mpTexMirrorAbsDiff          = NULL;
    mpTexMirrorAbsDiffMag       = NULL;

    g_UnadjusteduMinRangeAtlasTexture = 0.0f;
    g_UnadjustedvMinRangeAtlasTexture = 0.0f;
    g_UnadjusteduMaxRangeAtlasTexture = 0.0f;
    g_UnadjustedvMaxRangeAtlasTexture = 0.0f;

    g_pPerfData        = NULL;
    g_pBatch           = NULL;
    g_NumAtlasBatches  = 0;
    g_Redraws          = kRedrawInit;

	g_bLoadingApp = FALSE;
}

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    UNREFERENCED_PARAMETER(AdapterFormat);

    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))  
        if (!nErrors++) 
            MessageBox(NULL, TEXT("Device does not support 1.1 pixel shaders!"), TEXT("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        
    return (nErrors > nPrevErrors) ? false : true;
}

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    HDC hDC = GetDC(NULL);
    int nHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(NULL, hDC);
    if (FAILED(hr = D3DXCreateFont(pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                   DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                                   TEXT("Arial"), &g_pFont)))
        return DXTRACE_ERR(TEXT("D3DXCreateFont"), hr);

    D3DCAPS9    caps;
    hr = pd3dDevice->GetDeviceCaps(&caps);

    // set mFilterMax according to maximum supported Aniso mode
    if (caps.MaxAnisotropy < 2)
	{
        mFilterMax = kTrilinear;
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(1);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(2);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(3);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(4);
	}
    else if (caps.MaxAnisotropy <= 3)
	{
        mFilterMax = kAniso2x;
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(2);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(3);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(4);
	}
    else if (caps.MaxAnisotropy <= 7)
	{
        mFilterMax = kAniso4x;
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(3);
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(4);
	}
    else if (caps.MaxAnisotropy <= 15)
	{
        mFilterMax = kAniso8x;
		g_HUD.GetListBox(IDC_TEXFILTER)->RemoveItem(4);
	}
    else 
        mFilterMax = kAniso16x;

    return S_OK;
}

HRESULT CreatePixelShaders(IDirect3DDevice9* pd3dDevice)
{
    HRESULT                 hr;
    LPD3DXBUFFER            pShaderToken;
    LPD3DXBUFFER            pErrorMsgs;

    bool bSupportsPS2x = false;
    D3DCAPS9    caps;
    if (   SUCCEEDED( hr = pd3dDevice->GetDeviceCaps(&caps) )
        && (caps.PixelShaderVersion >= D3DPS_VERSION(1,1)) 
        && ((caps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE) != 0)
        && ((caps.PS20Caps.Caps & D3DPS20CAPS_GRADIENTINSTRUCTIONS) != 0)) 
    {
        bSupportsPS2x = true;
    }

    // Hi-lite shader for the atlas
    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\TexAtlasHiLite.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexAtlasHiLite); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    // difference shaders when in address mode kNone
    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\AbsDifference.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &gpAbsDifferenceShader); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\AbsDifferenceMag.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &gpAbsDifferenceMagShader); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    // if the device does not support ps 2x or better, dont attempt to load
    // the address mode shaders: just return and leave those at NULL.
    // Leaving these shader as NULL means address modes will not be available.
    if (! bSupportsPS2x) 
    {
        // Throw a warning up on screen that address modes are only 
        // available on ps2x devices: but only warn once...
        static bool bWarnedAlready = false;

        if (! bWarnedAlready)
            MessageBox( NULL, TEXT("Device does not support ps2x: Texture address modes are thus unavailable."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        bWarnedAlready = true;
        return S_OK;
    }

    // wrap shader and difference shaders when in address mode kWrap
    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\Wrap.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexWrap); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\WrapAbsDiff.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexWrapAbsDiff); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\WrapAbsDiffMag.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexWrapAbsDiffMag); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    // clamp shader and difference shaders when in address mode kClamp
    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\Clamp.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexClamp); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\ClampAbsDiff.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexClampAbsDiff); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\ClampAbsDiffMag.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexClampAbsDiffMag); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    // mirror shader and difference shaders when in address mode kMirror
    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\Mirror.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexMirror); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\MirrorAbsDiff.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexMirrorAbsDiff); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    hr = D3DXAssembleShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA\\programs\\D3D9_BatchingViaTextureAtlases\\MirrorAbsDiffMag.ps")).c_str(), NULL, NULL, 0, &pShaderToken, &pErrorMsgs);
    if (hr != S_OK)
    {
        MessageBox( NULL, TEXT("Unable to load pixel shader."), g_strWindowTitle, MB_ICONERROR|MB_OK );
        return hr;
    }
    hr = pd3dDevice->CreatePixelShader((DWORD *) pShaderToken->GetBufferPointer(), &mpTexMirrorAbsDiffMag); 
    assert(hr == S_OK);
    SAFE_RELEASE(pShaderToken);
    SAFE_RELEASE(pErrorMsgs);

    return S_OK;
}

HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

    g_HUD.SetLocation( 0, 0 );
	g_HUD.SetSize(pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height);
	g_HUD.GetControl(IDC_PERFVIZMODE)->SetLocation(pBackBufferSurfaceDesc->Width-180, 54);

	g_HUD.GetControl(IDC_LOAD)->SetLocation(pBackBufferSurfaceDesc->Width-125, 32);

	g_HUD.GetControl(IDC_SELTEX_U)->SetLocation(pBackBufferSurfaceDesc->Width-50, 185 );
	g_HUD.GetControl(IDC_SELTEX_D)->SetLocation(pBackBufferSurfaceDesc->Width-90, 185 );
	g_HUD.GetControl(IDC_SELTEX_S)->SetLocation(pBackBufferSurfaceDesc->Width-115, 165 );

	g_HUD.GetControl(IDC_TEXFILTER_S)->SetLocation(pBackBufferSurfaceDesc->Width-225, pBackBufferSurfaceDesc->Height/2-72);
	g_HUD.GetControl(IDC_TEXFILTER)->SetLocation(pBackBufferSurfaceDesc->Width-225, pBackBufferSurfaceDesc->Height/2-50);

	g_HUD.GetControl(IDC_DISPLAYMODES_S)->SetLocation(pBackBufferSurfaceDesc->Width-450, pBackBufferSurfaceDesc->Height/2-72);
	g_HUD.GetControl(IDC_DISPLAYMODES)->SetLocation(pBackBufferSurfaceDesc->Width-450, pBackBufferSurfaceDesc->Height/2-50);

	g_HUD.GetControl(IDC_MAGDIFF)->SetLocation(pBackBufferSurfaceDesc->Width-180, 76);
	g_HUD.GetControl(IDC_WRAPCLAMP)->SetLocation(pBackBufferSurfaceDesc->Width-180, 98);

	g_HUD.GetControl(IDC_NUMWRAPS_U)->SetLocation(pBackBufferSurfaceDesc->Width-90, 142  );
	g_HUD.GetControl(IDC_NUMWRAPS_D)->SetLocation(pBackBufferSurfaceDesc->Width-50, 142 );
	g_HUD.GetControl(IDC_NUMWRAPS_S)->SetLocation(pBackBufferSurfaceDesc->Width-115, 120 );

	g_HUD.GetControl(IDC_ATLASTEX)->SetLocation(0, pBackBufferSurfaceDesc->Height-75 );

	g_HUD.GetControl(IDC_NUMQUADS_U)->SetLocation(20 , pBackBufferSurfaceDesc->Height-35 );
	g_HUD.GetControl(IDC_NUMQUADS_D)->SetLocation(60 , pBackBufferSurfaceDesc->Height-35 );
	g_HUD.GetControl(IDC_NUMQUADS_S)->SetLocation(0 , pBackBufferSurfaceDesc->Height-55 );
	g_HUD.GetControl(IDC_PERF_S)->SetLocation(80,pBackBufferSurfaceDesc->Height-70);

	g_HUD.GetControl(IDC_TLQ_S)->SetLocation(0,pBackBufferSurfaceDesc->Height/2-100);
	g_HUD.GetControl(IDC_TRQ_S)->SetLocation(pBackBufferSurfaceDesc->Width/2,pBackBufferSurfaceDesc->Height/2-100);
	g_HUD.GetControl(IDC_BLQ_S)->SetLocation(0,pBackBufferSurfaceDesc->Height-50);
	g_HUD.GetControl(IDC_BRQ_S)->SetLocation(pBackBufferSurfaceDesc->Width/2,pBackBufferSurfaceDesc->Height-50);

    // Create the vertex buffer
    if (FAILED(hr = pd3dDevice->CreateVertexBuffer(3*2*sizeof(CUSTOMVERTEX),
                                                     D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
                                                     D3DPOOL_DEFAULT, &g_pVB, 
                                                     NULL)))
        return DXTRACE_ERR(TEXT("CreateVertexBuffer"), hr);

    // Fill the vertex buffer with 2 triangles
    CUSTOMVERTEX* pVertices;
    if (FAILED(hr = g_pVB->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_DISCARD)))
        return DXTRACE_ERR(TEXT("Lock"), hr);

    pVertices[0].position  = D3DXVECTOR3( -1.0f, -1.0f,  0.0f );
    pVertices[0].texcoord0 = pVertices[0].texcoord1 = pVertices[0].texcoord2 = D3DXVECTOR3(  0.0f,  1.0f, 0.0f );
    pVertices[1].position  = D3DXVECTOR3( -1.0f,  1.0f,  0.0f );
    pVertices[1].texcoord0 = pVertices[1].texcoord1 = pVertices[1].texcoord2 = D3DXVECTOR3(  0.0f,  0.0f, 0.0f );
    pVertices[2].position  = D3DXVECTOR3(  1.0f,  1.0f,  0.0f );
    pVertices[2].texcoord0 = pVertices[2].texcoord1 = pVertices[2].texcoord2 = D3DXVECTOR3(  1.0f,  0.0f, 0.0f );

    pVertices[3].position  = D3DXVECTOR3(  1.0f,  1.0f,  0.0f );
    pVertices[3].texcoord0 = pVertices[3].texcoord1 = pVertices[3].texcoord2 = D3DXVECTOR3(  1.0f,  0.0f, 0.0f );
    pVertices[4].position  = D3DXVECTOR3(  1.0f, -1.0f,  0.0f );
    pVertices[4].texcoord0 = pVertices[4].texcoord1 = pVertices[4].texcoord2 = D3DXVECTOR3(  1.0f,  1.0f, 0.0f );
    pVertices[5].position  = D3DXVECTOR3( -1.0f, -1.0f,  0.0f );
    pVertices[5].texcoord0 = pVertices[5].texcoord1 = pVertices[5].texcoord2 = D3DXVECTOR3(  0.0f,  1.0f, 0.0f );
    g_pVB->Unlock();

    hr = CreatePixelShaders(pd3dDevice);
    assert(hr == S_OK);

    // setup render states
    pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. 
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	g_Camera.SetViewParams( &vFromPt, &vLookatPt);
    pd3dDevice->SetTransform(D3DTS_VIEW, g_Camera.GetViewMatrix());

    D3DXMatrixIdentity( &g_matInitView);
	D3DXMatrixMultiply(&g_matInitView, &g_matInitView, g_Camera.GetViewMatrix());

    // Set the projection matrix
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)pBackBufferSurfaceDesc->Width) / pBackBufferSurfaceDesc->Height;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, fAspect, 0.1f, 10.0f);
	g_Camera.SetProjParams( D3DX_PI/4, fAspect, 0.1f, 10.0f);
	D3DXMatrixIdentity(&matProj);
	D3DXMatrixMultiply(&matProj,&matProj,g_Camera.GetProjMatrix());
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

    if (g_pFont)
        g_pFont->OnResetDevice();

    // get some textures to work with, if we have not done this yet...
    if (g_InputFileName == TEXT(""))
    {
        tstring filename = mkFilename;
        filename += TEXT(".tai");
       
        LoadTAIFile(GetFilePath::GetFilePath(filename).c_str());
    }
    else
        LoadCurrentEntry(pd3dDevice);

	return S_OK;
}

void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    g_Camera.FrameMove( fElapsedTime );
}

void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    SetTextureFilterAndAddressState(pd3dDevice);

    // Clear the viewport
    pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);

    // Begin the scene
    if (SUCCEEDED(pd3dDevice->BeginScene()))
    {
        // Set the world matrix
        D3DXMATRIX  matWorld;
		D3DXMatrixIdentity( &matWorld);
		D3DXMatrixMultiply( &matWorld, &matWorld, g_Camera.GetWorldMatrix());

        pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

        D3DVIEWPORT9    viewport;
        viewport.MinZ   = 0.0f;
        viewport.MaxZ   = 1.0f;

        if ((g_pCurrentTexture != NULL) && !mbPerfVizMode)
        {
            // Setup some state for the 4 following views
            pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
            pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
            pd3dDevice->SetPixelShader(NULL);
            pd3dDevice->SetTexture( 0, g_pCurrentTexture );
            if (g_pCurrentTextureAtlas)
                pd3dDevice->SetTexture( 1, g_pCurrentTextureAtlas);
            else
                pd3dDevice->SetTexture( 1, g_pCurrentVolumeAtlas);

            pd3dDevice->SetTransform(D3DTS_VIEW, g_Camera.GetViewMatrix());
            RenderTopLeftQuadrant(viewport,pd3dDevice);
            RenderTopRightQuadrant(viewport,pd3dDevice);
            RenderBottomLeftQuadrant(viewport,pd3dDevice);
            pd3dDevice->SetTransform(D3DTS_VIEW, &g_matInitView);
            RenderBottomRightQuadrant(viewport,pd3dDevice, fElapsedTime);
        }

        // Render stats and help text  
		const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();
        viewport.X      = 0;
        viewport.Y      = 0;
        viewport.Width  = pBackBufferDesc->Width;
        viewport.Height = pBackBufferDesc->Height;
        pd3dDevice->SetViewport(&viewport);
        pd3dDevice->SetPixelShader(NULL);

        if (mbPerfVizMode)
        {
            RenderPerfViz(matWorld, pd3dDevice);
            RenderStats();
        }
        else
        {
            if (mbShowHelp)
                ;
            else
            {
                RenderStats();
            }
            RenderMode();
        }

		g_HUD.OnRender( fElapsedTime );
        // End the scene.
        pd3dDevice->EndScene();
    }
}

void RenderStats()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, NULL, 15 );

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 15 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	txtHelper.DrawTextLine( DXUTGetFrameStats() );

	txtHelper.End();
}

void RenderMode()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, NULL, 15 );

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 35 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	txtHelper.DrawTextLine( DXUTGetDeviceStats() );

	txtHelper.End();
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
	case IDC_LOAD:
		{
			FileOpen();
			break;
		}
	case IDC_SELTEX_U:
		{
			IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
	        LoadNextEntry(pd3dDevice);
			break;
		}
	case IDC_SELTEX_D:
		{
			IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
	        LoadPreviousEntry(pd3dDevice);
			break;
		}
	case IDC_TEXFILTER:
		{
			DXUTListBoxItem *pItem = ((CDXUTListBox *)pControl)->GetItem( ((CDXUTListBox *)pControl)->GetSelectedIndex( -1 ) );
			switch(reinterpret_cast<long long>(pItem->pData))
			{
    			case 0:
				{
					mFilter = kTrilinear;
					break;
				}
	    		case 1:
				{
					mFilter = kAniso2x;
					break;
				}
		    	case 2:
				{
					mFilter = kAniso4x;
					break;
				}
			    case 3:
				{
					mFilter = kAniso8x;
					break;
				}
			    case 4:
				{
					mFilter = kAniso16x;
					break;
				}
			    case 5:
				{
					mFilter = kPoint;
					break;
				}
			    case 6:
				{
					mFilter = kBilinear;
					break;
				}
			}

			break;
		}
	case IDC_DISPLAYMODES:
		{
			DXUTListBoxItem *pItem = ((CDXUTListBox *)pControl)->GetItem( ((CDXUTListBox *)pControl)->GetSelectedIndex( -1 ) );

			switch(reinterpret_cast<long long>(pItem->pData))
			{
			case 0:
				{
					gMode = kDefaultCoordinateMode;
					IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
					LoadCurrentEntry(pd3dDevice);
					break;
				}
			case 1:
				{
					gMode = kStraightOrgToedInAtlas;
					IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
					LoadCurrentEntry(pd3dDevice);
					break;
			}
			case 2:
			{
				gMode = kStraightOrgStraightAtlas;
				IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
					LoadCurrentEntry(pd3dDevice);
					break;
				}
			}
			break;
		}
	case IDC_MAGDIFF:
		{
			gbShowMag = !gbShowMag;
			break;
		}
	case IDC_WRAPCLAMP:
		{
            // do nothing and early out if ps2x shaders were not loaded
            if (   (mpTexWrap   == NULL) || (mpTexWrapAbsDiff   == NULL) || (mpTexWrapAbsDiffMag   == NULL)
                || (mpTexClamp  == NULL) || (mpTexClampAbsDiff  == NULL) || (mpTexClampAbsDiffMag  == NULL)
                || (mpTexMirror == NULL) || (mpTexMirrorAbsDiff == NULL) || (mpTexMirrorAbsDiffMag == NULL))
                break;

			mTexAddress = static_cast<tAddressMode>(mTexAddress + 1);
            switch (mTexAddress)
            {
                case kNumAddressModes:
				    mTexAddress = kDefaultAddressMode;      // set to none and fall thru to that case

                case kNone:
				    g_HUD.GetButton(IDC_WRAPCLAMP)->SetText(L"Texture (A)ddress Mode: None");
				    g_HUD.GetControl(IDC_NUMWRAPS_U)->SetEnabled(false);
				    g_HUD.GetControl(IDC_NUMWRAPS_D)->SetEnabled(false);
                    break;
                case kClamp:
				    g_HUD.GetButton(IDC_WRAPCLAMP)->SetText(L"Texture (A)ddress Mode: Clamp");
				    g_HUD.GetControl(IDC_NUMWRAPS_U)->SetEnabled(true);
				    g_HUD.GetControl(IDC_NUMWRAPS_D)->SetEnabled(true);
                    break;
                case kWrap:
				    g_HUD.GetButton(IDC_WRAPCLAMP)->SetText(L"Texture (A)ddress Mode: Wrap");
				    g_HUD.GetControl(IDC_NUMWRAPS_U)->SetEnabled(true);
				    g_HUD.GetControl(IDC_NUMWRAPS_D)->SetEnabled(true);
                    break;
                case kMirror:
				    g_HUD.GetButton(IDC_WRAPCLAMP)->SetText(L"Texture (A)ddress Mode: Mirror");
				    g_HUD.GetControl(IDC_NUMWRAPS_U)->SetEnabled(true);
				    g_HUD.GetControl(IDC_NUMWRAPS_D)->SetEnabled(true);
                    break;
                default:
                    assert(false);
                    break;
            }
			IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
			LoadCurrentEntry(pd3dDevice);
			break;
		}
	case IDC_NUMWRAPS_U:
		{
	        ++mRepeat;
			IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
			LoadCurrentEntry(pd3dDevice);
			break;
		}
	case IDC_NUMWRAPS_D:
		{
			if (--mRepeat < 2)
				mRepeat = 2;
			IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
			LoadCurrentEntry(pd3dDevice);
			break;
		}
	case IDC_ATLASTEX:
		{
			mbAtlasMode = !mbAtlasMode;

            int const   kNumDrawCalls = (mbAtlasMode) ? g_NumAtlasBatches : (g_LastLine-g_FirstLine+1);
            TCHAR       string[256];
            _stprintf(string, TEXT("%d Draw calls"), g_Redraws * kNumDrawCalls);
	        g_HUD.GetStatic(IDC_PERF_S)->SetText(string);

            if (mbAtlasMode)
                g_HUD.GetButton(IDC_ATLASTEX)->SetText(L"Rendering from Atlas");
            else
                g_HUD.GetButton(IDC_ATLASTEX)->SetText(L"Rendering from Texture");
			break;
		}
	case IDC_NUMQUADS_U:
		{
	        g_Redraws += 10;
            int const   kNumDrawCalls = (mbAtlasMode) ? g_NumAtlasBatches : (g_LastLine-g_FirstLine+1);
            TCHAR       string[256];
            _stprintf(string, TEXT("%d Draw calls"), g_Redraws * kNumDrawCalls);
	        g_HUD.GetStatic(IDC_PERF_S)->SetText(string);
			break;
		}
	case IDC_NUMQUADS_D:
		{
			if (( g_Redraws -= 10) < 1)
				g_Redraws = 1;
            int const   kNumDrawCalls = (mbAtlasMode) ? g_NumAtlasBatches : (g_LastLine-g_FirstLine+1);
            TCHAR       string[256];
            _stprintf(string, TEXT("%d Draw calls"), g_Redraws * kNumDrawCalls);
	        g_HUD.GetStatic(IDC_PERF_S)->SetText(string);
			break;
		}
	case IDC_PERFVIZMODE:
		{
			if(!mbPerfVizMode)
			{
				IDirect3DDevice9 *pd3dDevice = DXUTGetD3D9Device();
				EnterPerfViz(pd3dDevice);
			}
			else
				ExitPerfViz();

			break;
		}
	}
}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();

    ExitPerfViz();

    if (g_pFont)
        g_pFont->OnLostDevice();
    SAFE_RELEASE(g_pVB);
    SAFE_RELEASE(gpAbsDifferenceShader);
    SAFE_RELEASE(gpAbsDifferenceMagShader);
    SAFE_RELEASE(mpTexAtlasHiLite);
    SAFE_RELEASE(mpTexWrap);
    SAFE_RELEASE(mpTexWrapAbsDiff);
    SAFE_RELEASE(mpTexWrapAbsDiffMag);
    SAFE_RELEASE(mpTexClamp);
    SAFE_RELEASE(mpTexClampAbsDiff);
    SAFE_RELEASE(mpTexClampAbsDiffMag);
    SAFE_RELEASE(mpTexMirror);
    SAFE_RELEASE(mpTexMirrorAbsDiff);
    SAFE_RELEASE(mpTexMirrorAbsDiffMag);
}

void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();

    if ( g_InputFile.is_open() )
        g_InputFile.close();

	SAFE_RELEASE(g_pFont);

    SAFE_RELEASE(g_pCurrentTexture);
    SAFE_RELEASE(g_pCurrentTextureAtlas);
    SAFE_RELEASE(g_pCurrentVolumeAtlas);
}

void LoadTAIFile( TCHAR const *szFilename )
{
    if ( g_InputFile.is_open() )
        g_InputFile.close();

	int nLen = WideCharToMultiByte(CP_ACP, 0, szFilename, -1, NULL, NULL, NULL, NULL);
	LPSTR lpszA = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szFilename, -1, lpszA, nLen, NULL, NULL);

	g_InputFile.open( lpszA );

	delete[] lpszA;

    // Error checking.
    TCHAR strMsg[MAX_PATH];
    _stprintf( strMsg, TEXT("Unable to open file: %s"), szFilename );
    MSG_AND_BREAK_IF( !g_InputFile, strMsg );

    // Skip past comments to the first line of the file with valid data.
	std::string  line;
    int     linecount = 0;
    char    ch = g_InputFile.peek();
    while ( ch == '#' || ch == '\n' )
    {
        // look on the second line for the -halftexel option
        if (++linecount == 2)
        {
			std::string word;
			std::streampos const kMark = g_InputFile.tellg();

            while (g_InputFile.peek() != '\n')
            {
                g_InputFile >> word;  
                if (word.compare("-halftexel") == 0)
                {
                    mbHalfTexel = true;
                    break;
                }
            }
            g_InputFile.seekg( kMark );
        }
        g_InputFile.ignore( 2048, '\n' );
        ch = g_InputFile.peek();
    }

    // Error checking.
    _stprintf( strMsg, TEXT("Invalid .tai file: %s"), szFilename );
    MSG_AND_BREAK_IF( g_InputFile.eof() || line[0] == '#', strMsg );

    // Save the line number the first entry.
    g_FirstLine = linecount;

    // Get/save the line number of the last entry.
    while ( g_InputFile.ignore( 2048, '\n' ) )
        linecount++;

    g_LastLine = linecount-2;

    // Set the file pointer to the first entry.
    g_CurrentLine = g_FirstLine;
    g_InputFile.clear();    // clear eof state
	g_InputFile.seekg( 0, std::ios::beg );
    for ( int i = 0; i < g_CurrentLine; i++ )
        g_InputFile.ignore( 2048, '\n' );

    // Save the filename.
    g_InputFileName = szFilename;

	IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();
    LoadCurrentEntry(pd3dDevice);
}

void LoadNextEntry(IDirect3DDevice9* pd3dDevice)
{
    if ( g_CurrentLine < g_LastLine )
    {
        g_InputFile.ignore( 2048, _T('\n') );
        g_CurrentLine++;
        LoadCurrentEntry(pd3dDevice);
    }
}

void LoadPreviousEntry(IDirect3DDevice9* pd3dDevice)
{
    if ( g_CurrentLine > g_FirstLine )
    {
        g_CurrentLine--;
        g_InputFile.clear();    // clear eof state
		g_InputFile.seekg( 0, std::ios::beg );
        for ( int i = 0; i < g_CurrentLine; i++ )
        {
            g_InputFile.ignore( 2048, '\n' );
        }
        LoadCurrentEntry(pd3dDevice);
    }
}

void LoadCurrentEntry(IDirect3DDevice9* pd3dDevice)
{
	std::string Filename;
    std::string Atlasname;
    std::string AtlasType;
	tstring TAtlasname;
    int AtlasIndex;
    float width, height;
    float wOffset, hOffset, slice;
    char ch;

	if(!pd3dDevice)
		return;
    // save file position
	std::streampos mark = g_InputFile.tellg();

    g_InputFile >> Filename;        // filename
    g_InputFile >> Atlasname;       // atlas name
    g_InputFile >> AtlasIndex;      // texture atlas index
    g_InputFile.get( ch );          // comma
    g_InputFile >> AtlasType;       // atlas type
    g_InputFile >> wOffset;         // woffset
    g_InputFile.get( ch );          // comma
    g_InputFile >> hOffset;         // hoffset
    g_InputFile.get( ch );          // read comma
    g_InputFile >> slice;           // slice (if volume texture or cubemap
    g_InputFile.get( ch );          // read comma
    g_InputFile >> width;           // width
    g_InputFile.get( ch );          // read comma
    g_InputFile >> height;          // height

    // restore file position
    g_InputFile.seekg( mark );

	int len = MultiByteToWideChar(CP_ACP,0,Atlasname.c_str(),-1,NULL,NULL);
	TCHAR *tmp = new TCHAR[len];
	MultiByteToWideChar(CP_ACP,0,Atlasname.c_str(),-1,tmp,len);
	TAtlasname = tstring(tmp);

    // Load the textures.
    SAFE_RELEASE( g_pCurrentTexture );

    HRESULT         hr;
    D3DXIMAGE_INFO  info;

    if (  ((g_pCurrentTextureAtlas == NULL) && (g_pCurrentVolumeAtlas == NULL))
        || AtlasIndex != g_CurrentAtlasIndex || AtlasIndex == -1)
    {
        SAFE_RELEASE( g_pCurrentTextureAtlas );
        SAFE_RELEASE( g_pCurrentVolumeAtlas );

        // get rid of the trailing comma in the filename
        TAtlasname[TAtlasname.length()-1] = '\0';
        hr = D3DXGetImageInfoFromFile(GetFilePath::GetFilePath(TAtlasname).c_str(), &info);

        switch (info.ResourceType)
        {
            case D3DRTYPE_TEXTURE:
                hr = D3DXCreateTextureFromFileEx( pd3dDevice, GetFilePath::GetFilePath(TAtlasname).c_str(), info.Width, info.Height, info.MipLevels, 0, info.Format, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, NULL, &g_pCurrentTextureAtlas);
                break;
            case D3DRTYPE_VOLUMETEXTURE:
                hr = D3DXCreateVolumeTextureFromFileEx( pd3dDevice, GetFilePath::GetFilePath(TAtlasname).c_str(), info.Width, info.Height, info.Depth, info.MipLevels, 0, info.Format, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, NULL, &g_pCurrentVolumeAtlas);
                break;
            default:
                hr = E_FAIL;
                break;
        }
        if ( hr != D3D_OK )
            DisplayTextureErrorMsg( hr, TAtlasname.c_str(), MSG_NONE );
        g_CurrentAtlasIndex = AtlasIndex;
        if (g_pCurrentTextureAtlas)
        {
            hr = g_pCurrentTextureAtlas->GetLevelDesc(0, &mCurrentTextureAtlasDesc);
            // check that created texture is the same as requested texture
            if (   (mCurrentTextureAtlasDesc.Format != info.Format) 
                || (mCurrentTextureAtlasDesc.Width  != info.Width)
                || (mCurrentTextureAtlasDesc.Height != info.Height))
                DisplayTextureErrorMsg( hr, TAtlasname.c_str(), MSG_NONE);
        }
        if (g_pCurrentVolumeAtlas)
        {
            hr = g_pCurrentVolumeAtlas->GetLevelDesc(0, &mCurrentVolumeAtlasDesc);
            if (   (mCurrentVolumeAtlasDesc.Format != info.Format) 
                || (mCurrentVolumeAtlasDesc.Width  != info.Width)
                || (mCurrentVolumeAtlasDesc.Height != info.Height)
                || (mCurrentVolumeAtlasDesc.Depth  != info.Depth))
                DisplayTextureErrorMsg( hr, TAtlasname.c_str(), MSG_NONE);
        }
    }
    // if texture atlas only has a single mip-level (or is volume texture which we know only
    // has single mip) then force single mip for all individual 
    // textures as well.  If it has more than one, then force full mip-chain
    UINT kMipLevels = (g_pCurrentVolumeAtlas) ? 1 : 0;
    if (g_pCurrentTextureAtlas)
        kMipLevels = (g_pCurrentTextureAtlas->GetLevelCount() == 1) ? 1 : 0;

	int nLen = MultiByteToWideChar(CP_ACP, 0, Filename.c_str(), -1, NULL, NULL);
	TCHAR *lpszW = new TCHAR[nLen];
	MultiByteToWideChar(CP_ACP, 0, Filename.c_str(), -1, lpszW, nLen);

    hr = D3DXGetImageInfoFromFile(GetFilePath::GetFilePath(lpszW).c_str(), &info);
    hr = D3DXCreateTextureFromFileEx( pd3dDevice, GetFilePath::GetFilePath(lpszW).c_str(), info.Width, info.Height, kMipLevels, 0, info.Format, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, NULL, &g_pCurrentTexture);
    if ( hr != D3D_OK )
        DisplayTextureErrorMsg( hr, lpszW, MSG_NONE );
    hr = g_pCurrentTexture->GetLevelDesc(0, &mCurrentTextureDesc);

	delete[] lpszW;
    // In DirectX texture coordinates 0 and 1 actually coincide, a quad with 
    // texture coordinates ranging from 0 to 1 thus covers an area 1 texel larger 
    // than the actual texture.  Games typically use tex-coords in the range 0 to 1 
    // to access textures regardless of this fact to maintain texture resolution 
    // in-variance. 
    //
    // These textures thus technically clamp/wrap and several solutions for how to 
    // apply textures atlasing:
    // 1) Correct the original texture coordinates: instead of ranging from 0 to 1 
    //    make all coordinates range from .5/tex-resolution to 1-.5/tex-resolution.
    //    Textures from an atlas use the relative same coordinates.  This is the 
    //    mathematically right thing to do.  It corresponds to the mode
    //    kToedInOrgToedInAtlas.  
    //    This mode has minimal difference errors.
    // 2) Keep the original 0 to 1 texture coordinates and just correctly adjust the 
    //    coordinates of the atlas textures.  It corresponds to the 
    //    kStraightOrgToedInAtlas mode.
    //    This mode shows errors across the whole texture in the difference case, 
    //    since it really is comparing two different mappings. 
    // 3) Keep the original 0 to 1 texture coordinates and let the atlas coordinates 
    //    also range from 0 to 1.  This corresponds to the 
    //    kStraightOrgStraightAtlas mode.
    //    This mode shows errors along the texture edges as unrelated textures
    //    get pulled into display.
    //
    // We initialize the texture coordinates here according to the display mode we are
    // in.  The halftexel bit simply lets us know if the atlas coordinates are already 
    // adjusted for the resolution, ie, if half-texel is false then the atlas coordinates 
    // correspond to a 0 to 1 range.  If half-texel is true then the atlas coordinates
    // correspond to a .5/tex-resolution to 1-.5/tex-resolution range.

    float const kRepeat = (mTexAddress == kNone)? 1.0f : static_cast<float>(mRepeat);
    float       uMinRangeOriginal,     uMaxRangeOriginal; 
    float       vMinRangeOriginal,     vMaxRangeOriginal; 
    float       uMinRangeAtlasTexture, vMinRangeAtlasTexture;
    float       uMaxRangeAtlasTexture, vMaxRangeAtlasTexture;

    // first figure out coordinates for original texture
    if (gMode == kToedInOrgToedInAtlas)
    {
        float const uAdjustment = 0.5f/static_cast<float>(mCurrentTextureDesc.Width);
        float const vAdjustment = 0.5f/static_cast<float>(mCurrentTextureDesc.Height);

        uMinRangeOriginal = uAdjustment;
        vMinRangeOriginal = vAdjustment;
        uMaxRangeOriginal = kRepeat - uAdjustment;
        vMaxRangeOriginal = kRepeat - vAdjustment;
    }
    else
    {
        uMinRangeOriginal = vMinRangeOriginal = 0.0f;
        uMaxRangeOriginal = vMaxRangeOriginal = kRepeat;
    }

    // now figure out coordinates for texture from atlas
    int const descwidth  = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Width  : mCurrentVolumeAtlasDesc.Width;
    int const descheight = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Height : mCurrentVolumeAtlasDesc.Height;
    float const uAdjustment = 0.5f/static_cast<float>(descwidth);
    float const vAdjustment = 0.5f/static_cast<float>(descheight);

    if (gMode == kStraightOrgStraightAtlas)
    {
        uMinRangeAtlasTexture = wOffset          - ((mbHalfTexel) ? uAdjustment : 0.0f);
        vMinRangeAtlasTexture = hOffset          - ((mbHalfTexel) ? vAdjustment : 0.0f);
        uMaxRangeAtlasTexture = wOffset + width  + ((mbHalfTexel) ? uAdjustment : 0.0f);
        vMaxRangeAtlasTexture = hOffset + height + ((mbHalfTexel) ? vAdjustment : 0.0f);

        g_UnadjusteduMinRangeAtlasTexture = uMinRangeAtlasTexture;
        g_UnadjustedvMinRangeAtlasTexture = vMinRangeAtlasTexture;
        g_UnadjusteduMaxRangeAtlasTexture = uMaxRangeAtlasTexture;
        g_UnadjustedvMaxRangeAtlasTexture = vMaxRangeAtlasTexture;

        // factor in the repeat if necessary...
        uMaxRangeAtlasTexture = g_UnadjusteduMinRangeAtlasTexture + 
                                kRepeat * (g_UnadjusteduMaxRangeAtlasTexture-g_UnadjusteduMinRangeAtlasTexture);
        vMaxRangeAtlasTexture = g_UnadjustedvMinRangeAtlasTexture + 
                                kRepeat * (g_UnadjustedvMaxRangeAtlasTexture-g_UnadjustedvMinRangeAtlasTexture);
    }
    else
    {
        uMinRangeAtlasTexture = wOffset          + ((mbHalfTexel) ? 0.0f : uAdjustment);
        vMinRangeAtlasTexture = hOffset          + ((mbHalfTexel) ? 0.0f : vAdjustment);
        uMaxRangeAtlasTexture = wOffset + width  - ((mbHalfTexel) ? 0.0f : uAdjustment);
        vMaxRangeAtlasTexture = hOffset + height - ((mbHalfTexel) ? 0.0f : vAdjustment);

        g_UnadjusteduMinRangeAtlasTexture = uMinRangeAtlasTexture - uAdjustment;
        g_UnadjustedvMinRangeAtlasTexture = vMinRangeAtlasTexture - vAdjustment;
        g_UnadjusteduMaxRangeAtlasTexture = uMaxRangeAtlasTexture + uAdjustment;
        g_UnadjustedvMaxRangeAtlasTexture = vMaxRangeAtlasTexture + vAdjustment;

        uMaxRangeAtlasTexture = g_UnadjusteduMinRangeAtlasTexture
                                + kRepeat * (g_UnadjusteduMaxRangeAtlasTexture-g_UnadjusteduMinRangeAtlasTexture)
                                - uAdjustment;
        vMaxRangeAtlasTexture = g_UnadjustedvMinRangeAtlasTexture 
                                + kRepeat * (g_UnadjustedvMaxRangeAtlasTexture-g_UnadjustedvMinRangeAtlasTexture)
                                - vAdjustment;
    }

    // Update the texture coordinates for the texture atlas geometry.
    CUSTOMVERTEX* pVertices;
    if (FAILED(hr = g_pVB->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_DISCARD)))
        DXTRACE_ERR(TEXT("Lock"), hr);

    pVertices[0].position  = D3DXVECTOR3( -1.0f, -1.0f,  0.0f );
    pVertices[0].texcoord0 = D3DXVECTOR3(  uMinRangeOriginal,      vMaxRangeOriginal, slice);
    pVertices[0].texcoord1 = D3DXVECTOR3(  uMinRangeAtlasTexture,  vMaxRangeAtlasTexture, slice);
    pVertices[0].texcoord2 = D3DXVECTOR3(  0.0f,  1.0f, slice);
    pVertices[1].position  = D3DXVECTOR3( -1.0f,  1.0f,  0.0f );
    pVertices[1].texcoord0 = D3DXVECTOR3(  uMinRangeOriginal,      vMinRangeOriginal, slice);
    pVertices[1].texcoord1 = D3DXVECTOR3(  uMinRangeAtlasTexture,  vMinRangeAtlasTexture, slice);
    pVertices[1].texcoord2 = D3DXVECTOR3(  0.0f,  0.0f, slice);
    pVertices[2].position  = D3DXVECTOR3(  1.0f,  1.0f,  0.0f );
    pVertices[2].texcoord0 = D3DXVECTOR3(  uMaxRangeOriginal,      vMinRangeOriginal, slice);
    pVertices[2].texcoord1 = D3DXVECTOR3(  uMaxRangeAtlasTexture,  vMinRangeAtlasTexture, slice);
    pVertices[2].texcoord2 = D3DXVECTOR3(  1.0f,  0.0f, slice);

    pVertices[3].position  = D3DXVECTOR3(  1.0f,  1.0f,  0.0f );
    pVertices[3].texcoord0 = D3DXVECTOR3(  uMaxRangeOriginal,      vMinRangeOriginal, slice);
    pVertices[3].texcoord1 = D3DXVECTOR3(  uMaxRangeAtlasTexture,  vMinRangeAtlasTexture, slice);
    pVertices[3].texcoord2 = D3DXVECTOR3(  1.0f,  0.0f, slice);
    pVertices[4].position  = D3DXVECTOR3(  1.0f, -1.0f,  0.0f );
    pVertices[4].texcoord0 = D3DXVECTOR3(  uMaxRangeOriginal,      vMaxRangeOriginal, slice);
    pVertices[4].texcoord1 = D3DXVECTOR3(  uMaxRangeAtlasTexture,  vMaxRangeAtlasTexture, slice);
    pVertices[4].texcoord2 = D3DXVECTOR3(  1.0f,  1.0f, slice);
    pVertices[5].position  = D3DXVECTOR3( -1.0f, -1.0f,  0.0f );
    pVertices[5].texcoord0 = D3DXVECTOR3(  uMinRangeOriginal,      vMaxRangeOriginal, slice);
    pVertices[5].texcoord1 = D3DXVECTOR3(  uMinRangeAtlasTexture,  vMaxRangeAtlasTexture, slice);
    pVertices[5].texcoord2 = D3DXVECTOR3(  0.0f,  1.0f, slice);

    g_pVB->Unlock();
}

//-----------------------------------------------------------------------------
// Name: FileOpen()
// Desc: Brings up a File Open dialog for user to select tai file
//-----------------------------------------------------------------------------
void FileOpen()
{
    // simply ignore this request when in perf viz mode
    if (mbPerfVizMode)
        return;

    OPENFILENAME Filename;
    TCHAR         szBuffer[_MAX_PATH];
    TCHAR         szFilename[_MAX_PATH];
    TCHAR         szExtension[4];
    
    wmemcpy( szBuffer, TEXT("TAI files\0*.TAI\0All Files\0*.*\0\0\0"),
                sizeof( TEXT("TAI files\0*.TAI\0All Files\0*.*\0\0\0") ) );
    _stprintf( szExtension, TEXT("tai") );
    _stprintf( szFilename,  TEXT(".\\%s"), mkFilename);

    memset( &Filename, 0, sizeof(Filename) );
    Filename.lStructSize  = sizeof(Filename);
    //Filename.hwndOwner    = hWnd;
    Filename.hInstance    = NULL;
    Filename.lpstrFilter  = szBuffer;
    Filename.nMaxFile     = _MAX_PATH;
    Filename.nFilterIndex = 1;
    Filename.lpstrFile    = szFilename;
    Filename.nMaxFile     = _MAX_PATH;
    Filename.Flags        = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    Filename.lpstrDefExt  = szExtension;

    if ( GetOpenFileName( &Filename ) )
    {
        // Load the TAI file.
        SAFE_RELEASE( g_pCurrentTextureAtlas );
        SAFE_RELEASE( g_pCurrentVolumeAtlas );
        LoadTAIFile( Filename.lpstrFile );
    }
    else
    {
        DWORD err = CommDlgExtendedError();
        MSG_AND_BREAK_IF( (err!=0), TEXT("GetOpenFileName() returned an error.\n") );
    }
}
HRESULT DisplayTextureErrorMsg( HRESULT hr, const TCHAR* filename, DWORD dwType )
{
    static bool s_bFatalErrorReported = false;
    TCHAR strMsg[512];

    // If a fatal error message has already been reported, the app
    // is already shutting down, so don't show more error messages.
    if( s_bFatalErrorReported )
        return hr;

    _tcscpy( strMsg, _T("Failed to load texture: \n") );
    _tcscat( strMsg, filename );
	fprintf( stderr, "Failed to load texture: %s\n", filename );

    switch( hr )
    {
        case D3DERR_NOTAVAILABLE:
            _tcscat( strMsg, _T("This device does not support the required\n")
                             _T("texture format on your Direct3D hardware\n")
                             _T("accelerator.  Try switching to refrast then\n")
                             _T("reload the selected .tai file.\n\n")
                             _T("Press F2 to see a list of devices and modes.\n") );
			fprintf( stderr, "This device does not support the queried technique (i.e. texture format).\n\n" );
            break;

        case D3DERR_OUTOFVIDEOMEMORY:
            _tcscat( strMsg, _T("Microsoft?Direct3D?does not have enough display\n")
                             _T("memory to perform the operation.\n") );
			fprintf( stderr, "Microsoft?Direct3D?does not have enough display memory to perform the operation.\n\n" );
            break;

        case D3DXERR_INVALIDDATA:
            _tcscat( strMsg, _T("File not found or the texture had data that is invalid.\n") );
			fprintf( stderr, "File not found or the texture had data that is invalid.\n\n" );
            break;

        case E_OUTOFMEMORY:
            _tcscat( strMsg, _T("MDirect3D could not allocate sufficient memory to complete the call.\n") );
			fprintf( stderr, "Direct3D could not allocate sufficient memory to complete the call.\n\n" );
            break;

        case D3DERR_INVALIDCALL:
            _tcscat( strMsg, _T("The method call is invalid. For example,\n")
                             _T("a method's parameter may have an invalid value.\n") );
			fprintf( stderr, "The method call is invalid. For example, a method's parameter may have an invalid value.\n\n" );
            break;

        default:
            _tcscat( strMsg, _T("Generic application error. Enable\n")
                             _T("debug output for detailed information.") );
			fprintf( stderr, "Unknown reason for error.\n\n" );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        s_bFatalErrorReported = true;
        _tcscat( strMsg, _T("\n\nThis sample will now exit.") );
        MessageBox( NULL, strMsg, g_strWindowTitle, MB_ICONERROR|MB_OK );

        // Close the window, which shuts down the app
		//if( g_hWnd )
            //SendMessage( g_hWnd, Wg_CLOSE, 0, 0 );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOREF == dwType )
            _tcscat( strMsg, _T("\n\nSwitching to the reference rasterizer,\n")
                             _T("a software device that implements the entire\n")
                             _T("Direct3D feature set, but runs very slowly.") );
        MessageBox( NULL, strMsg, g_strWindowTitle, MB_ICONWARNING|MB_OK );
    }

    return hr;
}
void SetTextureFilterAndAddressState(IDirect3DDevice9* pd3dDevice)
{
    // Set texture-filter based on filter mode:
    // Setup for aniso by default, possibly override for bi- and tri-linear
    pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    switch (mFilter)
    {
        default:
            // fall thru to point
        case kPoint:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
            break;
        case kBilinear:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
            break;
        case kTrilinear:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
            break;
        case kAniso2x:            
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 2);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 2);
            break;
        case kAniso4x:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 4);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 4);
            break;
        case kAniso8x:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 8);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 8);
            break;
        case kAniso16x:
            pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 16);
            pd3dDevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 16);
            break;
    }

    // Now set the texture address mode, based on mTexAddress
    switch (mTexAddress)
    {
        default:
            // fall through to none
        case kNone:
            // fall through to clamp
        case kClamp:
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
            break;
        case kWrap:
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
            break;
        case kMirror:
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
            pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_MIRROR);

            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
            pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSW, D3DTADDRESS_MIRROR);
            break;
    }
}

//-----------------------------------------------------------------------------
// Name: RenderTopLeftQuadrant()
// Desc: renders the original texture on a quad
//-----------------------------------------------------------------------------
void RenderTopLeftQuadrant(D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice)
{
	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    viewport.X      = 0;
    viewport.Y      = 0;
    viewport.Width  = pBackBufferDesc->Width/2;
    viewport.Height = pBackBufferDesc->Height/2;
    pd3dDevice->SetViewport(&viewport);

    // Just select the first texture, ie the original texture
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);

    pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    TCHAR       string[256];
    TCHAR       format[32];
    UINT const mips = g_pCurrentTexture->GetLevelCount();
    GetFormat(mCurrentTextureDesc.Format, format);

    _stprintf(string, TEXT("Original Texture\n(%dx%d, %d Mipmap(s), %s)"), mCurrentTextureDesc.Width, mCurrentTextureDesc.Height, mips, format);
	g_HUD.GetStatic(IDC_TLQ_S)->SetText(string);
}

//-----------------------------------------------------------------------------
// Name: RenderTopRightQuadrant()
// Desc: renders the texture from the atlas on a quad
//-----------------------------------------------------------------------------
void RenderTopRightQuadrant(D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice)
{
	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    viewport.X      = pBackBufferDesc->Width/2;
    viewport.Y      = 0;
    viewport.Width  = pBackBufferDesc->Width/2;
    viewport.Height = pBackBufferDesc->Height/2;
    pd3dDevice->SetViewport(&viewport);

    int const descwidth  = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Width  : mCurrentVolumeAtlasDesc.Width;
    int const descheight = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Height : mCurrentVolumeAtlasDesc.Height;

    if ((mTexAddress != kNone) && (g_pCurrentTextureAtlas))
    {
        // if we are in clamp/wrap/mirror mode compute a bunch of constants that define 
        // - the sub-rectangle in the atlas for the wrapping computations
        // - the sub-rectangle in the atlas to clamp the created coordinates against
        float const kUOffset = g_UnadjusteduMinRangeAtlasTexture;
        float const kVOffset = g_UnadjustedvMinRangeAtlasTexture;
        float const kWidth   = g_UnadjusteduMaxRangeAtlasTexture - g_UnadjusteduMinRangeAtlasTexture;
        float const kHeight  = g_UnadjustedvMaxRangeAtlasTexture - g_UnadjustedvMinRangeAtlasTexture;
        // clamp final repeat coordinates against these min/max values;
        // should use half-texel adjustment -- fudging these, however, 
        // generally yields better results, although they
        // are actually wrong (as visible in very low-res (32x32) textures)
        float const kFudge = 3.0f;
        float const kMinU  = kUOffset + kFudge*0.5f/static_cast<float>(descwidth);
        float const kMinV  = kVOffset + kFudge*0.5f/static_cast<float>(descheight);
        float const kMaxU  = kUOffset + kWidth  - kFudge*0.5f/static_cast<float>(descwidth);
        float const kMaxV  = kVOffset + kHeight - kFudge*0.5f/static_cast<float>(descheight);

        // activate the clamp/wrap/mirror-shader
        pd3dDevice->SetPixelShaderConstantF(0, D3DXVECTOR4( kUOffset,  kVOffset, kWidth, kHeight), 1); 
        pd3dDevice->SetPixelShaderConstantF(1, D3DXVECTOR4( 1.f/kWidth, 1.f/kHeight,  0.5f, 1.0f), 1); 
        pd3dDevice->SetPixelShaderConstantF(2, D3DXVECTOR4( kMinU, kMinV, kMaxU, kMaxV), 1); 
        switch (mTexAddress)
        {
            default:
                assert(false);      // assert and then fall through to reasonable default, ie clamp
            case kClamp:
                pd3dDevice->SetPixelShader(mpTexClamp);
                break;
            case kWrap:
                pd3dDevice->SetPixelShader(mpTexWrap);
                break;
            case kMirror:
                pd3dDevice->SetPixelShader(mpTexMirror);
                break;
        }
    }
    else 
    {
        // Just select the second texture, ie the texture atlas, w/o 
        // changing the texture assignment 
        pd3dDevice->SetPixelShader(NULL);
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    }
    pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
}

//-----------------------------------------------------------------------------
// Name: RenderBottomLeftQuadrant()
// Desc: renders the difference of the orginal texture and the texture from 
//       the atlas 
//-----------------------------------------------------------------------------
void RenderBottomLeftQuadrant(D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice)
{
	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    // render bottom left quadrant (difference between top-left and top-right texturing approaches)
    viewport.X      = 0;
    viewport.Y      = pBackBufferDesc->Height/2;
    viewport.Width  = pBackBufferDesc->Width/2;
    viewport.Height = pBackBufferDesc->Height/2;
    pd3dDevice->SetViewport(&viewport);

    int descwidth  = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Width  : mCurrentVolumeAtlasDesc.Width;
    int descheight = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Height : mCurrentVolumeAtlasDesc.Height;

    if ((mTexAddress != kNone) && (g_pCurrentTextureAtlas))
    {
        // if we are in wrap mode compute a bunch of constants that define 
        // - the sub-rectangle in the atlas for the wrapping computations
        // - the sub-rectangle in the atlas to clamp the created coordinates against
        float const kUOffset = g_UnadjusteduMinRangeAtlasTexture;
        float const kVOffset = g_UnadjustedvMinRangeAtlasTexture;
        float const kWidth   = g_UnadjusteduMaxRangeAtlasTexture - g_UnadjusteduMinRangeAtlasTexture;
        float const kHeight  = g_UnadjustedvMaxRangeAtlasTexture - g_UnadjustedvMinRangeAtlasTexture;
        // clamp final repeat coordinates against these min/max values;
        // should use half-texel adjustment -- fudging these, however, 
        // generally yields better results, although they
        // are actually wrong (as visible in very low-res (32x32) textures)
        float const kFudge = 3.0f;
        float const kMinU  = kUOffset + kFudge*0.5f/static_cast<float>(descwidth);
        float const kMinV  = kVOffset + kFudge*0.5f/static_cast<float>(descheight);
        float const kMaxU  = kUOffset + kWidth  - kFudge*0.5f/static_cast<float>(descwidth);
        float const kMaxV  = kVOffset + kHeight - kFudge*0.5f/static_cast<float>(descheight);

        // strictly speaking these constants do not need to be re-assigned:
        // the top right quadrant ran just before us and set these to the correct values 
        pd3dDevice->SetPixelShaderConstantF(0, D3DXVECTOR4( kUOffset,  kVOffset, kWidth, kHeight), 1); 
        pd3dDevice->SetPixelShaderConstantF(1, D3DXVECTOR4( 1.f/kWidth, 1.f/kHeight,  0.5f, 1.0f), 1); 
        pd3dDevice->SetPixelShaderConstantF(2, D3DXVECTOR4( kMinU, kMinV, kMaxU, kMaxV), 1); 
        pd3dDevice->SetPixelShaderConstantF(3, D3DXVECTOR4( 0.f, 0.f, 0.f, 16.f), 1); 
        switch (mTexAddress)
        {
            default:
                assert(false);      // assert and then fall through to reasonable default, ie clamp
            case kClamp:
                pd3dDevice->SetPixelShader(gbShowMag ? mpTexClampAbsDiffMag : mpTexClampAbsDiff);
                break;
            case kWrap:
                pd3dDevice->SetPixelShader(gbShowMag ? mpTexWrapAbsDiffMag : mpTexWrapAbsDiff);
                break;
            case kMirror:
                pd3dDevice->SetPixelShader(gbShowMag ? mpTexMirrorAbsDiffMag : mpTexMirrorAbsDiff);
                break;
        }
    }
    else
        pd3dDevice->SetPixelShader(gbShowMag ? gpAbsDifferenceMagShader : gpAbsDifferenceShader);

    pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    
    if (gbShowMag)
        g_HUD.GetStatic(IDC_BLQ_S)->SetText(TEXT("Magnified Difference (16*abs(A-B))\nBlack means no difference"));
    else
        g_HUD.GetStatic(IDC_BLQ_S)->SetText(TEXT("Difference (abs(A-B))\nBlack means no difference"));
}

//-----------------------------------------------------------------------------
// Name: RenderBottomRightQuadrant()
// Desc: renders the complete atlas and blinks the currently active sub-texture
//-----------------------------------------------------------------------------
void RenderBottomRightQuadrant(D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice, float fElapsedTime)
{
    // render bottom right quadrant (show the whole atlas)
    int descwidth  = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Width  : mCurrentVolumeAtlasDesc.Width;
    int descheight = (g_pCurrentTextureAtlas) ? mCurrentTextureAtlasDesc.Height : mCurrentVolumeAtlasDesc.Height;

    D3DXMATRIX  matWorld;

    // make this quad slight larger since it is not affected by rotates
    // and non-uniformly scale such that textures have the right aspect ratio
    float const kAspect = static_cast<float>(descwidth)/static_cast<float>(descheight);
    float const kXScale = (kAspect > 1.0f) ? 1.0f         : kAspect;
    float const kYScale = (kAspect > 1.0f) ? 1.0f/kAspect : 1.0f;

    D3DXMatrixScaling(&matWorld, 1.4f * kXScale, 1.4f * kYScale, 1.0f);
    pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetD3D9BackBufferSurfaceDesc();

    viewport.X      = pBackBufferDesc->Width/2;
    viewport.Y      = pBackBufferDesc->Height/2;
    viewport.Width  = pBackBufferDesc->Width/2;
    viewport.Height = pBackBufferDesc->Height/2;
    pd3dDevice->SetViewport(&viewport);

 
    float kBias = -13.0f;      // force top-level mip (at least for up to 8kx8k textures)
    DWORD *pBias = static_cast<DWORD *>(static_cast<void *>(&kBias));
    pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *pBias);
    pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 2);
    pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 2);
    if (g_pCurrentTextureAtlas) 
        pd3dDevice->SetTexture( 0, g_pCurrentTextureAtlas);
    else
        pd3dDevice->SetTexture(0, g_pCurrentVolumeAtlas);

    static float blink = 0.0f;
    static float timer = 0.0f;
    if ((timer += fElapsedTime) > 0.25f)
    {
        timer = 0.0f;
        blink = 1.0f - blink;
    }
    // compute the sub-rectangl;e that needs to be blinked
    float const uAdjustment = 0.5f/static_cast<float>(descwidth);
    float const vAdjustment = 0.5f/static_cast<float>(descheight);

    float const minU = (gMode == kStraightOrgStraightAtlas) ? g_UnadjusteduMinRangeAtlasTexture
                                                            : g_UnadjusteduMinRangeAtlasTexture + uAdjustment;
    float const minV = (gMode == kStraightOrgStraightAtlas) ? g_UnadjustedvMinRangeAtlasTexture
                                                            : g_UnadjustedvMinRangeAtlasTexture + vAdjustment;
    float const maxU = (gMode == kStraightOrgStraightAtlas) ? g_UnadjusteduMaxRangeAtlasTexture
                                                            : g_UnadjusteduMaxRangeAtlasTexture - uAdjustment;
    float const maxV = (gMode == kStraightOrgStraightAtlas) ? g_UnadjustedvMaxRangeAtlasTexture
                                                            : g_UnadjustedvMaxRangeAtlasTexture - vAdjustment;

    pd3dDevice->SetPixelShaderConstantF(0, D3DXVECTOR4( minU,  minV, 0.0f, 0.0f), 1);
    pd3dDevice->SetPixelShaderConstantF(1, D3DXVECTOR4(-maxU, -maxV, 0.0f, 0.0f), 1);
    pd3dDevice->SetPixelShaderConstantF(2, D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f), 1);
    pd3dDevice->SetPixelShaderConstantF(3, D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.5f*blink), 1);
    pd3dDevice->SetPixelShader(mpTexAtlasHiLite);
    pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    TCHAR       string[256];
    TCHAR       format[32];
    GetFormat(mCurrentTextureDesc.Format, format);

    if (g_pCurrentTextureAtlas)
    {
        UINT const mips = (g_pCurrentTextureAtlas) ? g_pCurrentTextureAtlas->GetLevelCount() : 0;
        _stprintf(string, TEXT("Complete Texture Atlas\n(%dx%d, %d Mipmap(s), %s)"), descwidth, descheight, mips, format);
    }
    else
    {
        _stprintf(string, TEXT("Volume Texture Atlas Slice\n(%dx%dx%d, %d Mipmap(s), %s)"), descwidth, descheight, mCurrentVolumeAtlasDesc.Depth, 1, format);
    }
	g_HUD.GetStatic(IDC_BRQ_S)->SetText(string);
    pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, 0);
}
//-----------------------------------------------------------------------------
// Name: EnterPerfViz()
// Desc: finds and references all textures and atlases, creates VBs 
//       for pure-texture rendering and atlas rendering, and creates index
//       lists to be rendered via DrawPrims
//-----------------------------------------------------------------------------
void EnterPerfViz( IDirect3DDevice9* pd3dDevice)
{
    assert(mbPerfVizMode == FALSE);

	g_HUD.GetButton(IDC_PERFVIZMODE)->SetText(L"Exit (P)erformance Mode");

    // Have to force clamp mode: wrapping in atlas mode would require setting
    // up multiple ranges in the pixel shader constants and additional vertex 
    // data to identify which constant-range/texture to use
    mTexAddress = kNone;

    // allocate space for all textures/atlases
    // This can take forever on medium to large size datasets:
    // Thus inserting a "Please Wait" type dialogue.
    g_bLoadingPerf = TRUE;
    int const   kNumTextures = g_LastLine-g_FirstLine+1;
    int const   kNumRows     = static_cast<int>(ceilf(sqrtf(static_cast<float>(kNumTextures))));
    g_pPerfData = new tPerfData[kNumTextures];

    // create a large VB to render the pref data from
    if (FAILED(pd3dDevice->CreateVertexBuffer(kNumTextures * 3*2*sizeof(CUSTOMVERTEX),
                                                D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
                                                D3DPOOL_DEFAULT, &g_pPerfVB, NULL)))
    {
        assert(false);
        return;
    }

    CUSTOMVERTEX* pVertices; 
    if (FAILED(g_pPerfVB->Lock( 0, 0, (VOID**)&pVertices, NULL)))
    {
        assert(false);
        return;
    }

    // read file to beginning of first texture
    int i, j, k;
    g_InputFile.clear();                
	g_InputFile.seekg( 0, std::ios::beg );
    for (i = 0; i < g_FirstLine-1; ++i)
        g_InputFile.ignore( 2048, '\n' );
    // read each texture/atlas from g_pCurrentTexture/g_pCurrentTextureAtlas
    // copy texture pointers, increase ref-count and copy vertex data into VB
    int         index  = 0;
    int         row    = 0;
    int         col    = 0;
    float const kScale = 1.0f/static_cast<float>(kNumRows);
    float const kEps   = 10e-3f;
    D3DXVECTOR3 offset(kScale-1.0f, kScale-1.0f, 0.0f);
    for (i = 0; i < kNumTextures; ++i)
    {
        // SendMessage(g_hWnd, Wg_PAINT, i, kNumTextures);

        g_InputFile.ignore( 2048, '\n' ); 
        LoadCurrentEntry(pd3dDevice);

        g_pCurrentTexture->AddRef();
        if (g_pCurrentTextureAtlas)
            g_pCurrentTextureAtlas->AddRef();
        if (g_pCurrentVolumeAtlas)
            g_pCurrentVolumeAtlas->AddRef();
        g_pPerfData[i].atlasIndex = g_CurrentAtlasIndex;
        g_pPerfData[i].pTexture   = g_pCurrentTexture;
        g_pPerfData[i].pAtlas       = g_pCurrentTextureAtlas;
        g_pPerfData[i].pAtlasVolume = g_pCurrentVolumeAtlas;

        CUSTOMVERTEX* pOldVert;
        if (FAILED(g_pVB->Lock( 0, 0, (VOID**)&pOldVert, NULL)))
        {
            assert(false);
            return;
        }
        for (j = 0; j < 6; ++j)
        {
            pVertices[6*i+j].position  = offset + (kScale-kEps) * pOldVert[j].position;
            pVertices[6*i+j].texcoord0 = pOldVert[j].texcoord0;
            pVertices[6*i+j].texcoord1 = pOldVert[j].texcoord1;
            pVertices[6*i+j].texcoord2 = pOldVert[j].texcoord2;

            g_pPerfData[i].index[j] = index++;
        }
        g_pVB->Unlock();

        offset.x += 2.0f*kScale;
        if (++row >= kNumRows)
        {       
            row = 0;
            ++col;
            offset.x  = kScale-1.0f;
            offset.y += 2.0f*kScale;
        }
    } 
    g_pPerfVB->Unlock();

    assert(index == kNumTextures*3*2);

    // sort data by atlas index
    bool bIsSorted = false;
    while (!bIsSorted)
    {
        bIsSorted = true;
        for (i = 0; i < kNumTextures-1; ++i)
            if (g_pPerfData[i].atlasIndex > g_pPerfData[i+1].atlasIndex)
            {
                tPerfData   swap;

                swap             = g_pPerfData[i];
                g_pPerfData[i]   = g_pPerfData[i+1];
                g_pPerfData[i+1] = swap;

                bIsSorted = false;
            }
    }
    
    // how many distinct atlas pages are there? 
    g_NumAtlasBatches = 1;
    for (i = 1; i < kNumTextures; ++i)
        if (g_pPerfData[i-1].atlasIndex != g_pPerfData[i].atlasIndex)
            ++g_NumAtlasBatches;
    g_pBatch = new tBatchData[g_NumAtlasBatches+1];

    // create index buffer 
    if (FAILED(pd3dDevice->CreateIndexBuffer(kNumTextures * 3*2 * sizeof(WORD),
                                                    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                                    D3DPOOL_DEFAULT, &g_pIndex, NULL)))
    {
        assert(false);
        return;
    }

    WORD* pIndices;
    if (FAILED(g_pIndex->Lock( 0, 0, (VOID**)&pIndices, NULL)))
    {
        assert(false);
        return;
    }

    // copy indices into index buffer
    for (i = 0, j = 0; i < kNumTextures; ++i)
    {
        if ((i == 0) ||
            ((i > 0) && (g_pPerfData[i-1].atlasIndex != g_pPerfData[i].atlasIndex)))
        {
            g_pBatch[j].startIndex   = 6*i;
            g_pBatch[j].pAtlas       = g_pPerfData[i].pAtlas;
            g_pBatch[j].pAtlasVolume = g_pPerfData[i].pAtlasVolume;
            ++j;
        }
        for (k = 0; k < 6; ++k)
            *pIndices++ = g_pPerfData[i].index[k]; 
    }
    g_pIndex->Unlock();
    assert(j == g_NumAtlasBatches);
    g_pBatch[g_NumAtlasBatches].startIndex   = 6*kNumTextures;   // add a sentinel to the array 
    g_pBatch[g_NumAtlasBatches].pAtlas       = NULL;   
    g_pBatch[g_NumAtlasBatches].pAtlasVolume = NULL;   

    mbPerfVizMode  = TRUE;
    mbAtlasMode    = FALSE;
    g_bLoadingPerf = FALSE;

	//Normal Mode Buttons
	g_HUD.GetControl(IDC_LOAD)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_SELTEX_U)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_SELTEX_D)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_SELTEX_S)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_TEXFILTER_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_TEXFILTER)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_DISPLAYMODES_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_DISPLAYMODES)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_MAGDIFF)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_WRAPCLAMP)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_NUMWRAPS_U)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMWRAPS_D)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMWRAPS_S)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_TLQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_TRQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_BLQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_BRQ_S)->SetVisible(!mbPerfVizMode);

	//Perf Viz Mode Buttons
    int const   kNumDrawCalls = (mbAtlasMode) ? g_NumAtlasBatches : (g_LastLine-g_FirstLine+1);
    TCHAR       string[256];
    _stprintf(string, TEXT("%d Draw calls"), g_Redraws * kNumDrawCalls);
	g_HUD.GetStatic(IDC_PERF_S)->SetText(string);
	g_HUD.GetControl(IDC_PERF_S)->SetVisible(mbPerfVizMode);

	g_HUD.GetControl(IDC_ATLASTEX)->SetVisible(mbPerfVizMode);

	g_HUD.GetControl(IDC_NUMQUADS_U)->SetVisible(mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMQUADS_D)->SetVisible(mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMQUADS_S)->SetVisible(mbPerfVizMode);
}

//-----------------------------------------------------------------------------
// Name: ExitPerfViz()
// Desc: release all textures and VBs created in EnterPerfViz()
//-----------------------------------------------------------------------------
void ExitPerfViz()
{
	if (!mbPerfVizMode)
        return;

	g_HUD.GetButton(IDC_PERFVIZMODE)->SetText(L"Enter (P)erformance Viz Mode");

    int const   kNumTextures = g_LastLine-g_FirstLine+1;

    // SAFE_RELEASE all created stuff 
    // This can take forever on medium to large size datasets:
    // Thus inserting a "Please Wait" type dialogue.
    g_bLoadingPerf = TRUE;

    for (int i = 0; i < kNumTextures; ++i)
    {
        //SendMessage(g_hWnd, Wg_PAINT, i, kNumTextures);
        SAFE_RELEASE(g_pPerfData[i].pTexture);
        SAFE_RELEASE(g_pPerfData[i].pAtlas);
        SAFE_RELEASE(g_pPerfData[i].pAtlasVolume);
    }
    SAFE_RELEASE(g_pPerfVB);
    SAFE_RELEASE(g_pIndex);

    // delete all new'ed data
    delete[] g_pPerfData;
    delete[] g_pBatch;
    g_NumAtlasBatches = 0;    

    // need to restore g_InputFile to point at g_Current 
    g_InputFile.clear();                
	g_InputFile.seekg( 0, std::ios::beg );
    for (int i = 0; i < g_CurrentLine; ++i)
        g_InputFile.ignore( 2048, '\n' );
	IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();
    LoadCurrentEntry(pd3dDevice);

    mbPerfVizMode  = FALSE;
    g_bLoadingPerf = FALSE;

	//Normal Mode Buttons
	g_HUD.GetControl(IDC_LOAD)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_SELTEX_U)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_SELTEX_D)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_SELTEX_S)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_TEXFILTER_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_TEXFILTER)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_DISPLAYMODES_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_DISPLAYMODES)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_MAGDIFF)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_WRAPCLAMP)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_NUMWRAPS_U)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMWRAPS_D)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMWRAPS_S)->SetVisible(!mbPerfVizMode);

	g_HUD.GetControl(IDC_TLQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_TRQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_BLQ_S)->SetVisible(!mbPerfVizMode);
	g_HUD.GetControl(IDC_BRQ_S)->SetVisible(!mbPerfVizMode);

	//Perf Viz Mode Buttons
	g_HUD.GetControl(IDC_PERF_S)->SetVisible(mbPerfVizMode);
	g_HUD.GetControl(IDC_ATLASTEX)->SetVisible(mbPerfVizMode);

	g_HUD.GetControl(IDC_NUMQUADS_U)->SetVisible(mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMQUADS_D)->SetVisible(mbPerfVizMode);
	g_HUD.GetControl(IDC_NUMQUADS_S)->SetVisible(mbPerfVizMode);
}

//-----------------------------------------------------------------------------
// Name: RenderPerfViz()
// Desc: parse and render the data prepared in EnterPerfViz()
//-----------------------------------------------------------------------------
void RenderPerfViz(D3DXMATRIX &matWorld, IDirect3DDevice9* pd3dDevice)
{
    if (!mbPerfVizMode) // fail-safe in case something went horribly awry
    {
        assert(false);
        return;
    }

    int const   kNumTextures = g_LastLine-g_FirstLine+1;

    pd3dDevice->SetStreamSource(0, g_pPerfVB, 0, sizeof(CUSTOMVERTEX));
    pd3dDevice->SetIndices(g_pIndex);
    pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
    pd3dDevice->SetPixelShader(NULL);

    int i, j;
    D3DXMATRIX  matTranslate;
    D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, 0.05f); 
    if (mbAtlasMode)        // render w/ atlas textures
    {
        pd3dDevice->SetTexture( 0, NULL);

        pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

        int numPrimitives;
        for (j = 0; j < g_Redraws; ++j)
        {
            for (i = 0; i < g_NumAtlasBatches; ++i)
            {
                if (g_pBatch[i].pAtlas)
                    pd3dDevice->SetTexture( 1, g_pBatch[i].pAtlas);
                else
                    pd3dDevice->SetTexture( 1, g_pBatch[i].pAtlasVolume);
                numPrimitives = (g_pBatch[i+1].startIndex - g_pBatch[i].startIndex)/3;
                pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6*kNumTextures, g_pBatch[i].startIndex, numPrimitives);
            }
            D3DXMatrixMultiply(&matWorld, &matTranslate, &matWorld);
            pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
        }
    }
    else
    {
        pd3dDevice->SetTexture( 1, NULL);
        pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);

        for (j = 0; j < g_Redraws; ++j)
        {
            for (int i = 0; i < kNumTextures; ++i)
            {
                pd3dDevice->SetTexture( 0, g_pPerfData[i].pTexture );
                pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6*kNumTextures, 6*i, 2);
            }
            D3DXMatrixMultiply(&matWorld, &matTranslate, &matWorld);
            pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
        }
    }
}