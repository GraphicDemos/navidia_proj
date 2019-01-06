#pragma once
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
IDC_TOGGLEFULLSCREEN,
IDC_TOGGLEREF,
IDC_CHANGEDEVICE,
IDC_SWAPTECHNIQUE,
IDC_LAST
};
#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTcamera.h>
#include <DXUTSettingsDlg.h>
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void *pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void *pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext );
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void *pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void *pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
void    CALLBACK OnLostDevice( void *pUserContext );
void    CALLBACK OnDestroyDevice( void *pUserContext );

void    InitApp();
void    RenderText();

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CModelViewerCamera      g_Camera;                // A model viewing camera
CDXUTDialog             g_HUD;                   // dialog for standard controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
ID3DXFont*              g_pFont = NULL;          // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;    // Sprite for batching draw text calls
LPD3DXEFFECT            g_pEffect = NULL;

LPDIRECT3DTEXTURE9      g_pSrcTexture = NULL;    // just a texture
LPDIRECT3DTEXTURE9      g_pSrcImage = NULL;      // source texture for decimation (full-screen version of srcTexture)
LPDIRECT3DTEXTURE9      g_pDstImage[2] = {NULL,NULL}; // result texture for decimation

LPDIRECT3DSURFACE9      g_pUpsampleSurface = NULL;
LPDIRECT3DSURFACE9      g_pDecimateSurface[2] = { NULL, NULL };

D3DXVECTOR2             g_vCoordArray[4];

bool					g_bShowHelp = false;
bool					g_bShowUI = true;
unsigned int            g_uiDecimateTechnique = 0; // technique to use