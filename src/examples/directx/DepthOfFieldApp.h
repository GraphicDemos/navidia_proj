#pragma once
#include "DepthOfField.h"
#include <DXUT/DXUTcamera.h>
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
IDC_TOGGLEFULLSCREEN,
IDC_TOGGLEREF,
IDC_CHANGEDEVICE,
IDC_TOGGLEWIREFRAME,
IDC_FSTOP,
IDC_FSTOPTEXT,
IDC_FOCALLENGTH,
IDC_FOCALLENGTHTEXT,
IDC_FOCUSDISTANCE,
IDC_FOCUSDISTANCETEXT,
IDC_RADIO_COLORS,
IDC_RADIO_DEPTH,
IDC_RADIO_BLUR,
IDC_LAST
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
void    RenderText();

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CFirstPersonCamera      g_Camera;               // A model viewing camera
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
bool                    g_bShowUI = true;      // Whether or not to draw the HUD
bool                    g_bShowHelp = true;     // Whether or not to draw the help
DepthOfField            *g_pDepthOfFieldApp;    // Class containing all sample-specific code