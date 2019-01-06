#ifndef NVTRISTRIPTEST_H
#define NVTRISTRIPTEST_H

#include <TCHAR.H>
typedef std::basic_string<TCHAR> tstring; 


//forward declare
struct PrimitiveGroup;

struct VertexType
{
  float x, y, z;
  float nx, ny, nz;
  float u, v;
  enum FVF
  {
    FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
  };
};

enum RenderMode
{
	RM_NOOPT,
	RM_OPTLIST,
	RM_OPTSTRIP
};

HMENU   g_main_menu;
HMENU   g_context_menu;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer;
LPDIRECT3DINDEXBUFFER9  g_pIndexBufferUnOpt;
LPDIRECT3DINDEXBUFFER9  g_pIndexBufferOptStrip;
LPDIRECT3DINDEXBUFFER9  g_pIndexBufferOptList;
LPDIRECT3DTEXTURE9      g_pSkinTexture;
LPD3DXEFFECT            g_pEffect;

//our matrices
D3DXMATRIX g_world;
D3DXMATRIX g_view;
D3DXMATRIX g_proj;

RenderMode g_renderMode;

tstring g_strFileName;

DWORD g_dwNumVerts;
DWORD g_dwNumFacesList, g_dwNumFacesStrip;

//bounding sphere attributes
float g_fRadius;
D3DXVECTOR3 g_vecCenter;

bool g_bWireframe;
bool g_bUseTrilinear;

//primitive groups from NvTriStrip
PrimitiveGroup* g_pPrimitiveGroupsList;
PrimitiveGroup* g_pPrimitiveGroupsStrip;


// methods
HRESULT LoadXFile(const TCHAR* fileName, const DWORD dwFVF, IDirect3DDevice9* pd3dDevice);
HRESULT SetVertexShaderMatrices(const D3DXMATRIX& worldMat);
HRESULT ResetLightsAndCamera(IDirect3DDevice9* pd3dDevice);
void    HandleKey(DWORD wParam, int bDown);
void    ToggleWireframe();
void    ShowHelp();

LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// implemented virtual functions:
HRESULT OneTimeSceneInit();

//HRESULT InitDeviceObjects();       // called once @ startup
//HRESULT DeleteDeviceObjects();     // called once @ exit
HRESULT InvalidateDeviceObjects(); // called just before device is Reset
HRESULT RestoreDeviceObjects();    // called when device is restored
HRESULT Render();
HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
HRESULT FinalCleanup();

ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
#endif