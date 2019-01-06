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
// File: AtlasViewer.h
// Desc: Declaration of Viewer application class.
//-----------------------------------------------------------------------------

#include <TCHAR.H>
#include <iostream>
#include <fstream>
#include <DXUT/DXUTcamera.h>
typedef std::basic_string<TCHAR> tstring; 

// Custom D3D vertex format used by the vertex buffer
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE3(0)|D3DFVF_TEXCOORDSIZE3(1)|D3DFVF_TEXCOORDSIZE3(2))
struct CUSTOMVERTEX
{
    D3DXVECTOR3 position;       // vertex position
    D3DXVECTOR3 texcoord0;       // (u,v) texture coorinates
    D3DXVECTOR3 texcoord1;       // (u,v) texture coorinates
    D3DXVECTOR3 texcoord2;       // (u,v) texture coorinates
};

// Struct to store the current input state
struct UserInput
{
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
    BOOL bZoomIn;
    BOOL bZoomOut;
    BOOL bReset;
    BOOL bShowMag;

    BOOL bPageUpWasPressed;
    BOOL bPageDownWasPressed;
    BOOL bHWasPressed;
    BOOL bMWasPressed;
    BOOL bFWasPressed;
    BOOL bAWasPressed;
    BOOL bRWasPressed;
    BOOL bTWasPressed;
    BOOL bPWasPressed;
    BOOL bBWasPressed;
};

struct tPerfData
{
    LPDIRECT3DTEXTURE9        pTexture;
    LPDIRECT3DTEXTURE9        pAtlas;
    LPDIRECT3DVOLUMETEXTURE9  pAtlasVolume;
    int                       atlasIndex;
    int                       index[6];
};

struct tBatchData
{
    int                       startIndex;
    LPDIRECT3DTEXTURE9        pAtlas;
    LPDIRECT3DVOLUMETEXTURE9  pAtlasVolume;
};


//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------

    void GetFormat(D3DFORMAT const format, TCHAR* string);
	void RenderStats();
    void RenderMode();
    HRESULT DisplayTextureErrorMsg( HRESULT hr, const TCHAR* filename, DWORD dwType );
    HRESULT CreatePixelShaders(IDirect3DDevice9* pd3dDevice);

    void LoadTAIFile( TCHAR const * szFilename );
    void LoadCurrentEntry(IDirect3DDevice9* pd3dDevice);
    void LoadNextEntry(IDirect3DDevice9* pd3dDevice);
    void LoadPreviousEntry(IDirect3DDevice9* pd3dDevice);
    void FileOpen();
    void SetTextureFilterAndAddressState(IDirect3DDevice9* pd3dDevice);

    void RenderTopLeftQuadrant    (D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice);
    void RenderTopRightQuadrant   (D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice);
    void RenderBottomLeftQuadrant (D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice);
    void RenderBottomRightQuadrant(D3DVIEWPORT9 &viewport, IDirect3DDevice9* pd3dDevice, float fElapsedTime);

    void EnterPerfViz(IDirect3DDevice9* pd3dDevice);
    void RenderPerfViz(D3DXMATRIX &matWorld, IDirect3DDevice9* pd3dDevice);
    void ExitPerfViz();

    static TCHAR const mkFilename[_MAX_PATH]=TEXT("MEDIA\\tai\\Default"); 

    enum {
        kRepeatInit =  4,
        kRedrawInit = 21,
    };

    enum tCoordinateMode {
        kDefaultCoordinateMode = 0,
        kToedInOrgToedInAtlas  = 0,
        kStraightOrgToedInAtlas,
        kStraightOrgStraightAtlas,
        kNumCoordinateModes
    };

    enum tFilterMode {
        kDefaultFilterMode = 0,
        kPoint             = 0,
        kBilinear,
        kTrilinear, 
        kAniso2x,
        kAniso4x,
        kAniso8x,
        kAniso16x,
        kNumFilterModes
    };

    enum tAddressMode {
        kDefaultAddressMode = 0,
        kNone               = 0,
        kWrap, 
        kClamp,
        kMirror,
        kNumAddressModes
    };

    BOOL                    g_bLoadingApp;          // TRUE, if the app is loading
    BOOL                    g_bLoadingPerf;         // TRUE, if the app is entering/exiting perf-mode
    BOOL                    mbShowHelp;             // Show help screen if true
    BOOL                    mbHalfTexel;            // tai file was generated w/ -halftexel option
    bool                    mbPerfVizMode;          // in perf vizualiztion mode
	BOOL					gbShowMag;
    BOOL                    mbAtlasMode;            // rendering from atlas in perf viz mode
    tCoordinateMode         gMode;
    tFilterMode             mFilter;
    tFilterMode             mFilterMax;
    tAddressMode            mTexAddress;
    int                     mRepeat;

    LPDIRECT3DVERTEXBUFFER9 g_pVB;                  // Vextex buffer 
    LPDIRECT3DVERTEXBUFFER9 g_pPerfVB;              // Vertex buffer for perf viz mode
    LPDIRECT3DINDEXBUFFER9  g_pIndex;               // Index buffer for perf viz mode
    ID3DXFont*              g_pFont;                // D3DX font    
    CModelViewerCamera      g_Camera;            // Struct for storing user input 
    
    FLOAT                   g_fWorldRotX;           // World rotation state X-axis
    FLOAT                   g_fWorldRotY;           // World rotation state Y-axis
    FLOAT                   g_fZoom;

    tstring					g_InputFileName;        // Name of the currently open .tai file
	std::ifstream			g_InputFile;            // Pointer to the currently open .tai file
    int                     g_FirstLine;            // First line of .tai file w/ texture info
    int                     g_LastLine;             // Last line of .tai file w/ texture info
    int                     g_CurrentLine;          // Current line of .tai file

    LPDIRECT3DTEXTURE9       g_pCurrentTexture;      // The current texture.       
    LPDIRECT3DTEXTURE9       g_pCurrentTextureAtlas; // The current texture atlas.
    LPDIRECT3DVOLUMETEXTURE9 g_pCurrentVolumeAtlas;  // The current texture volume atlas.

    int                     g_CurrentAtlasIndex;     // The index of the current texture atlas.
    D3DSURFACE_DESC         mCurrentTextureDesc;
    D3DSURFACE_DESC         mCurrentTextureAtlasDesc;
    D3DVOLUME_DESC          mCurrentVolumeAtlasDesc;
    IDirect3DPixelShader9  *gpAbsDifferenceShader;
    IDirect3DPixelShader9  *gpAbsDifferenceMagShader;
    IDirect3DPixelShader9  *mpTexAtlasHiLite;
    IDirect3DPixelShader9  *mpTexWrap;
    IDirect3DPixelShader9  *mpTexWrapAbsDiff;
    IDirect3DPixelShader9  *mpTexWrapAbsDiffMag;
    IDirect3DPixelShader9  *mpTexClamp;
    IDirect3DPixelShader9  *mpTexClampAbsDiff;
    IDirect3DPixelShader9  *mpTexClampAbsDiffMag;
    IDirect3DPixelShader9  *mpTexMirror;
    IDirect3DPixelShader9  *mpTexMirrorAbsDiff;
    IDirect3DPixelShader9  *mpTexMirrorAbsDiffMag;
    float                   g_UnadjusteduMinRangeAtlasTexture;
    float                   g_UnadjustedvMinRangeAtlasTexture;
    float                   g_UnadjusteduMaxRangeAtlasTexture;
    float                   g_UnadjustedvMaxRangeAtlasTexture;

    tPerfData              *g_pPerfData;
    tBatchData             *g_pBatch;
    int                     g_NumAtlasBatches;
    int                     g_Redraws;
    TCHAR *					g_strWindowTitle;

    D3DXMATRIX              g_matInitView;
