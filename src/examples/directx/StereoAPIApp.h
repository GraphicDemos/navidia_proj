#pragma once
//--------------------------------------------------------------------------------------
// View defines
//--------------------------------------------------------------------------------------
typedef int VIEWCHOICE;
const VIEWCHOICE BEHIND_WHEEL = 1, BEHIND_CAR = 0, ABOVE_CAR = 2;
#define DEPTH_RING 4
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
IDC_TOGGLEFULLSCREEN,
IDC_TOGGLEREF,
IDC_CHANGEDEVICE,
IDC_CHANGECONVERGENCE,
IDC_STEREOSWITCH,
IDC_CHANGECONVERGENCE_STATIC,
IDC_TEXLISTBOX,
IDC_FOCALPLANE,
IDC_VIEWCONVERGE,
IDC_SCREENSHOT,
IDC_LAST,
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
CModelViewerCamera      g_Camera;               // A model viewing camera
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
bool					g_bShowHelp = false;
bool					g_bShowUI = true;
bool					g_bBehindWheel = false;

VIEWCHOICE				g_CurrentViewChoice = BEHIND_CAR;
RECT					g_rBBPortion;

LPD3DXEFFECT			g_pEffect = NULL;
CDXUTMesh*				g_pTerrainMesh = NULL;
CDXUTMesh*				g_pSkyBox = NULL;
CDXUTMesh*				g_pRocketCar = NULL;

IDirect3DVertexBuffer9* g_pVB;
IDirect3DVertexDeclaration9* g_pVertexDeclaration;


//--------------------------------------------------------------------------------------
//Helper functions
//--------------------------------------------------------------------------------------
void SetUpMatricesForEffect( D3DXMATRIX* view, D3DXMATRIX* world, D3DXMATRIX* proj, LPD3DXEFFECT* pEffect);
HRESULT DrawScene(IDirect3DDevice9* pd3dDevice, LPD3DXEFFECT* pEffect, CDXUTMesh** pMesh);
HRESULT DrawQuad(IDirect3DDevice9* pd3dDevice);

void UpdateConvergence();

void RenderCar(float fTime);
void DrawTerrain(IDirect3DDevice9* pd3dDevice);
void DrawSkyBox(IDirect3DDevice9* pd3dDevice, float scale);
void DrawFocalPlane(IDirect3DDevice9* pd3dDevice, float FocalPoint);
void RenderCar(IDirect3DDevice9* pd3dDevice, float fTime);