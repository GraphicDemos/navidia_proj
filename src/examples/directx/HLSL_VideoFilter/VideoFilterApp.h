#pragma once
#include "GraphBuilder.h"
#include "VideoFilter.h"


#include <DXUTgui.h>
#include <DXUTcamera.h>
#include <DXUTSettingsDlg.h>
// Controls for the frosted effect
#define DELTAX_TEXT      L"DeltaX: %.3f"
#define DELTAX_ALG       g_pVideoFilterApp->m_deltax       = ((float)g_HUD.GetSlider(IDC_DELTAX)->GetValue()/1000.0f)
#define DELTAY_TEXT      L"DeltaY: %.3f"
#define DELTAY_ALG       g_pVideoFilterApp->m_deltay       = ((float)g_HUD.GetSlider(IDC_DELTAY)->GetValue()/1000.0f)
#define FREQ_TEXT        L"Freq: %.3f"
#define FREQ_ALG         g_pVideoFilterApp->m_freq         = ((float)g_HUD.GetSlider(IDC_FREQ)->GetValue()/1000.0f)

// Controls for the lense ORB distortion effect
#define RADIUS_TEXT      L"Radius: %.2f"
#define RADIUS_ALG       g_pVideoFilterApp->m_radius       = ((float)g_HUD.GetSlider(IDC_RADIUS)->GetValue()/100.0f)
#define EFFECTSCALE_TEXT L"Effect: %.2f"
#define EFFECTSCALE_ALG  g_pVideoFilterApp->m_effect_scale = ((float)g_HUD.GetSlider(IDC_EFFECTSCALE)->GetValue()/100.0f)

// Controls for the Radial Blur effect
#define BLURSTART_TEXT   L"Blur Start: %.2f"
#define BLURSTART_ALG    g_pVideoFilterApp->m_blurstart   = ((float)g_HUD.GetSlider(IDC_BLURSTART)->GetValue()/100.0f)
#define BLURWIDTH_TEXT   L"Blur Width: %.2f"
#define BLURWIDTH_ALG    g_pVideoFilterApp->m_blurwidth   = ((float)g_HUD.GetSlider(IDC_BLURWIDTH)->GetValue()/100.0f)

// Controls for the FADE Compositing effect
#define FADE_TEXT        L"Fade: %.3f"
#define FADE_ALG         g_pVideoFilterApp->m_fade        = ((float)g_HUD.GetSlider(IDC_FADE)->GetValue()/1000.0f)

// Controls for the WIPE Compositing effect
#define WIPECENTER_TEXT  L"Wipe: %.3f"
#define WIPECENTER_ALG   g_pVideoFilterApp->m_wipe         = ((float)g_HUD.GetSlider(IDC_WIPECENTER)->GetValue()/1000.0f)
#define WIPESOFT_TEXT    L"Softness: %.3f"
#define WIPESOFT_ALG     g_pVideoFilterApp->m_wipesoft     = ((float)g_HUD.GetSlider(IDC_WIPESOFT)->GetValue()/1000.0f)
#define ANGLE_TEXT       L"Angle: %.1f"
#define ANGLE_ALG        g_pVideoFilterApp->m_angle        = ((float)g_HUD.GetSlider(IDC_ANGLE)->GetValue()/10.0f)
#define SLANT_TEXT       L"Slant: %.2f"
#define SLANT_ALG        g_pVideoFilterApp->m_slant        = ((float)g_HUD.GetSlider(IDC_SLANT)->GetValue()/100.0f)

// Final pass for Video Proc Amp Controls
#define BRIGHT_TEXT      L"Brightness: %.2f"
#define BRIGHT_ALG(n)    g_pVideoFilterApp->fBrightness[n] = ((float)g_HUD.GetSlider(IDC_BRIGHT)->GetValue()/100.0f)
#define CONTRAST_TEXT    L"Contrast: %.2f"
#define CONTRAST_ALG(n)  g_pVideoFilterApp->fContrast[n]   = ((float)g_HUD.GetSlider(IDC_CONTRAST)->GetValue()/100.0f)
#define SATURATE_TEXT    L"Saturation: %.2f"
#define SATURATE_ALG(n)  g_pVideoFilterApp->fSaturate[n]   = ((float)g_HUD.GetSlider(IDC_SATURATE)->GetValue()/100.0f)
#define HUE_TEXT         L"Hue: %1.0f"
#define HUE_ALG(n)       g_pVideoFilterApp->fHue[n]        = ((float)g_HUD.GetSlider(IDC_HUE)->GetValue())


#define SEEK_TEXT     L"%02d:%02d:%02d / %02d:%02d:%02d"
#define SEEK_ALG(x)   (x / 3600), (x / 60), (x % 60)

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
    IDC_TOGGLEFULLSCREEN,
    IDC_TOGGLEREF,
    IDC_CHANGEDEVICE,
    IDC_RESET,
    IDC_WIREFRAME,
    IDC_LB_FILTER1,
    IDC_LB_FILTER2,
    IDC_LB_COMPOSITE,

    IDC_BRIGHT,
    IDC_BRIGHTTEXT,
    IDC_CONTRAST,
    IDC_CONTRASTTEXT,
    IDC_SATURATE,
    IDC_SATURATETEXT,
    IDC_HUE,
    IDC_HUETEXT,

	// These are for the frosted effect
	IDC_DELTAX,
	IDC_DELTAXTEXT,
	IDC_DELTAY,
	IDC_DELTAYTEXT,
	IDC_FREQ,
	IDC_FREQTEXT,

	// These are for lens distortion ORB
	IDC_RADIUS,
	IDC_RADIUSTEXT,
	IDC_EFFECTSCALE,
	IDC_EFFECTSCALETEXT,

	// These are for Radial Blur
	IDC_BLURSTART,
	IDC_BLURSTARTTEXT,
	IDC_BLURWIDTH,
	IDC_BLURWIDTHTEXT,

	// These are for PostFade & PostWipe
    IDC_FADE,
    IDC_FADETEXT,
    IDC_WIPECENTER,
    IDC_WIPECENTERTEXT,
    IDC_WIPESOFT,
    IDC_WIPESOFTTEXT,
    IDC_ANGLE,
    IDC_ANGLETEXT,
    IDC_SLANT,
    IDC_SLANTTEXT,

    IDC_PLAYPAUSE1,
    IDC_SEEK1,
    IDC_SEEKTEXT1,
    IDC_STOP1,
    IDC_LOOP1,
    IDC_PLAYPAUSE2,
    IDC_SEEK2,
    IDC_SEEKTEXT2,
    IDC_STOP2,
    IDC_LOOP2,
    IDC_LASTLAST
};

#define IDC_LAST IDC_WIREFRAME
#define IDC_FIRST_POST IDC_HUE_TEXT+1
#define IDC_LAST_POST  IDC_PLAYPAUSE1-1


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp(LPSTR lpszCmdParam);
void    RenderText();

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CModelViewerCamera      g_Camera;               // A model viewing camera
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
bool					g_bShowHelp = false;
bool					g_bShowUI = true;
VideoFilter              *g_pVideoFilterApp;