#ifndef SHADER_Aniso_H
#define SHADER_Aniso_H

#include <TCHAR.H>
typedef std::basic_string<TCHAR> tstring; 

struct AnisoVertex
{
  float x, y, z;
  float nx, ny, nz;
  enum FVF
  {
    FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0
  };
};

HMENU   g_main_menu;
HMENU   g_context_menu;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer;
LPDIRECT3DINDEXBUFFER9  g_pIndexBuffer;
LPDIRECT3DTEXTURE9      g_pShadeTexture;
LPD3DXEFFECT            g_pEffect;

//our matrices
D3DXMATRIX g_world;
D3DXMATRIX g_view;
D3DXMATRIX g_proj;

D3DXATTRIBUTERANGE* g_pAttributes;

tstring g_strFileName;

//number of sections in the mesh
DWORD g_dwNumSections;

//bounding sphere attributes
float g_fRadius;
D3DXVECTOR3 g_vecCenter;

bool g_bWireframe;
bool g_bUseTrilinear;

// methods
HRESULT LoadXFile(const TCHAR* fileName, const DWORD dwFVF, IDirect3DDevice9* pd3dDevice);
HRESULT SetVertexShaderMatrices();
HRESULT ResetLightsAndCamera(IDirect3DDevice9* pd3dDevice);
void    HandleKey(DWORD wParam, int bDown);
void    ToggleWireframe();
void    ShowHelp();

//CAnisoLighting();
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