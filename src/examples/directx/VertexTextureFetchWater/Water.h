#pragma once

#include "Grid.h"
#include "HeightMap.h"
#include "Vertex.h"
#include "Viewer.h"

#include <string>
#include <vector>
#include <windows.h>


#define CHECK_RETURN_CODE(text, hr) \
if(FAILED((hr)))\
{\
  return(DXTRACE_ERR((text), (hr)));\
}\


////////////////////////////////////////////////////////////////////////////////
//
//                         Water class declaration
//
////////////////////////////////////////////////////////////////////////////////
class Water
{
public:
                          Water(void);
  virtual                ~Water();

  // Callbacks for 9.0c sample framework
  static bool    CALLBACK IsDeviceAcceptableCallback(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
  static bool    CALLBACK ModifyDeviceSettingsCallback(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext);
  static HRESULT CALLBACK OnCreateDeviceCallback(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
  static HRESULT CALLBACK OnResetDeviceCallback(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
  static void    CALLBACK OnFrameMoveCallback(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime, void* pUserContext);
  static void    CALLBACK OnFrameRenderCallback(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime, void* pUserContext);
  static LRESULT CALLBACK MsgProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
  static void    CALLBACK KeyboardProcCallback(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
  static void    CALLBACK OnGUIEventCallback(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
  static void    CALLBACK OnLostDeviceCallback(void* pUserContext);
  static void    CALLBACK OnDestroyDeviceCallback(void* pUserContext);

  static LPDIRECT3DVERTEXBUFFER9 m_quadVertexBuffer;

protected:
  enum RenderSelection
    {
      RENDER_LAND        = 0x00000001,
      RENDER_BOAT        = 0x00000002,
      RENDER_SUNKEN_BOAT = 0x00000004,
      RENDER_ALL         = 0xFFFFFFFF,
    };

  
  static Water*           m_Water;


  void                    CopySimulationTexture(LPDIRECT3DTEXTURE9 simulationSourceTexture, LPDIRECT3DSURFACE9 currentRenderTarget);
  bool                    CreatePixelShader(LPDIRECT3DDEVICE9 device, const char* filename, D3DXMACRO* defines, const char* function,
                                            const char* compileTarget, LPDIRECT3DPIXELSHADER9& pixelShader);
  bool                    CreateVertexShader(LPDIRECT3DDEVICE9 device, const char* filename, const char* function, 
                                             const char* compileTarget, LPDIRECT3DVERTEXSHADER9& vertexShader);
  bool                    CreateVertexShader(LPDIRECT3DDEVICE9 device, const char* assemblyFilename, LPDIRECT3DVERTEXSHADER9& vertexShader);

  bool                    CreateVertexShaderFromPreCompiledFile(LPDIRECT3DDEVICE9 device, const WCHAR* preCompiledFile, LPDIRECT3DVERTEXSHADER9& vertexShader);
  bool                    CreatePixelShaderFromPreCompiledFile(LPDIRECT3DDEVICE9 device, const WCHAR* preCompiledFile, LPDIRECT3DPIXELSHADER9& vertexShader);

  bool                    DestroyShaders(void);
  bool                    FindFile(const std::string& filename, std::string& fullpath, bool exitOnFail = true) const;
  void                    InitGUI(void);
  bool                    InitShaders(void);
  bool                    KeyIsDown(int virtualKeyCode);
  LPDIRECT3DTEXTURE9      LoadTexture(const char* filename);
  int                     Printf(const char* formatString, ...);
  void                    RenderBackbuffer(void);
  void                    RenderInformation(void);
  void                    RenderReflectionTexture(void);
  void                    RenderRefractionTexture(void);
  void                    RenderScene(unsigned int sceneObjectSelection = RENDER_ALL, bool disableStateSetup = false);
  void                    RenderSceneBoat(bool disableStateSetup);
  void                    RenderSceneLand(bool disableStateSetup);
  void                    RenderSceneSunkenBoat(bool disableStateSetup);
  void                    RenderSkybox(void);
  void                    RenderWater(void);
  HRESULT                 RestoreWaterRenderingObjects(void);
  HRESULT                 RestoreWaterSimulationObjects(void);
  void                    SetMatrices(void);
  void                    SetMatrix(int constantIndex, const D3DXMATRIX& matrix);
  void                    SimulateWaveEquation(void);

  // 9.0c sample framework methods
  bool                    IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed);
  bool                    ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps);
  HRESULT                 OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
  HRESULT                 OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
  void                    OnFrameMove(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime);
  void                    OnFrameRender(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime);
  LRESULT                 MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
  void                    KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown);
  void                    OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl);
  void                    OnLostDevice(void);
  void                    OnDestroyDevice(void);


private:
  LPDIRECT3DDEVICE9        m_pd3dDevice;
  BOOL                     m_loadingApp;           // TRUE, if the app is loading
  ID3DXFont*               m_font;                 // D3DX font   
  int                      m_deviceWidth;
  int                      m_deviceHeight;

  D3DXMATRIX               m_projectionMatrix;
  D3DXMATRIX               m_reflectionProjectionMatrix;
  D3DXMATRIX               m_refractionProjectionMatrix;
  D3DXMATRIX               m_worldViewProjectionInverseTransposeMatrix;

  D3DXVECTOR3              m_lightPositionWorld;

  LPDIRECT3DTEXTURE9       m_waveSimulationTexture[3];
  LPDIRECT3DSURFACE9       m_waveSimulationSurface[3];
  LPDIRECT3DTEXTURE9       m_waveSimulationVertexShaderTexture;
  LPDIRECT3DSURFACE9       m_waveSimulationVertexShaderSurface;
  LPDIRECT3DTEXTURE9       m_waveSimulationDampeningTexture;
  Grid                     m_waveSimulationGrid;
  LPDIRECT3DVERTEXBUFFER9  m_waveSimulationVertexBuffer;
  LPDIRECT3DINDEXBUFFER9   m_waveSimulationIndexBuffer;
  LPDIRECT3DVERTEXSHADER9  m_waveSimulationVertexShader;
  LPDIRECT3DPIXELSHADER9   m_waveSimulationPixelShader;
  LPDIRECT3DVERTEXSHADER9  m_waveSimulationPerturbationVertexShader;
  LPDIRECT3DPIXELSHADER9   m_waveSimulationPerturbationPixelShader;
  int                      m_waveSimulationIndex;
  int                      m_waveSimulationWidth;
  int                      m_waveSimulationHeight;

  int                      m_reflectionTextureWidth;
  int                      m_reflectionTextureHeight;
  int                      m_refractionTextureWidth;
  int                      m_refractionTextureHeight;
  LPDIRECT3DTEXTURE9       m_waveReflectionTexture;
  LPDIRECT3DSURFACE9       m_waveReflectionSurface;
  LPDIRECT3DTEXTURE9       m_waveRefractionTexture;
  LPDIRECT3DSURFACE9       m_waveRefractionSurface;
  LPDIRECT3DTEXTURE9       m_waveReflectionNearTexture;
  LPDIRECT3DSURFACE9       m_waveReflectionNearSurface;
  LPDIRECT3DTEXTURE9       m_waveRefractionNearTexture;
  LPDIRECT3DSURFACE9       m_waveRefractionNearSurface;
  LPDIRECT3DPIXELSHADER9   m_waveRenderingPixelShader;
  LPDIRECT3DPIXELSHADER9   m_waveRenderingRefractionPixelShader;
  LPDIRECT3DPIXELSHADER9   m_waveRenderingReflectionPixelShader;
  LPDIRECT3DPIXELSHADER9   m_waveRenderingFresnelPixelShader;
  LPDIRECT3DPIXELSHADER9   m_waveRenderingNormalsPixelShader;
  LPDIRECT3DVERTEXSHADER9  m_waveRenderingVertexShader;
  LPDIRECT3DTEXTURE9       m_fadeTexture;


  LPDIRECT3DSURFACE9       m_backBufferRenderTarget;

  LPDIRECT3DTEXTURE9       m_underwaterWorldRenderTargetTexture;
  LPDIRECT3DSURFACE9       m_underwaterWorldRenderTargetSurface;
  LPDIRECT3DPIXELSHADER9   m_underwaterPixelShader;

  LPDIRECT3DVERTEXSHADER9  m_causticVertexShader;
  LPDIRECT3DPIXELSHADER9   m_causticPixelShader;

  LPDIRECT3DVERTEXSHADER9  m_diffuseVertexShader;
  LPDIRECT3DPIXELSHADER9   m_diffusePixelShader;

  LPDIRECT3DVERTEXSHADER9  m_singleTextureVertexShader;
  LPDIRECT3DPIXELSHADER9   m_singleTexturePixelShader;
  
  LPDIRECT3DTEXTURE9       m_skyBoxTextures[6];

  LPDIRECT3DTEXTURE9       m_wakeTexture;

  LPD3DXMATRIXSTACK        m_worldMatrixStack;
  LPD3DXMATRIXSTACK        m_viewMatrixStack;
  LPD3DXMATRIXSTACK        m_projectionMatrixStack;
  bool                     m_reflectViewMatrix;
  
  HeightMap*               m_land;
  LPDIRECT3DTEXTURE9       m_landTexture;
  float                    m_landScale[3];

  Mesh                     m_boatMesh;
  Mesh                     m_sunkenBoatMesh;
  LPDIRECT3DTEXTURE9       m_boatTexture;

  Viewer                   m_viewer;
  bool                     m_walkOnGround;
  float                    m_time;
  float                    m_tLastFrame;
  float                    m_dt; 
  float                    m_fieldOfView;
  float                    m_height;
  D3DXPLANE                m_refractionClipPlaneAboveWater;
  D3DXPLANE                m_reflectionClipPlaneAboveWater;
  D3DXPLANE                m_refractionClipPlaneBelowWater;
  D3DXPLANE                m_reflectionClipPlaneBelowWater;

  float                    m_aspectRatio;
  float                    m_refractionMapOverdraw;
  float                    m_reflectionMapOverdraw;
  bool                     m_underwater;
  bool                     m_mouseIsClipped;
  RECT                     m_mouseClipRect;
  int                      m_textureOverlayIndex;
  bool                     m_displayHelp;
  bool                     m_freezeSimulation;
  bool                     m_stopPertubingWater;
  bool                     m_enableWaterGhost;
  bool                     m_renderRefractionOnly;
  bool                     m_renderReflectionOnly;
  bool                     m_renderFresnelOnly;
  bool                     m_renderNormalsOnly;
  bool                     m_renderWireframeWater;
  bool                     m_renderText;
  bool                     m_renderGUI;
  bool                     m_renderSunkenBoat;
  D3DCOLOR                 m_fontColor;
  POINT                    m_fontPosition;
  std::vector<std::string> m_path;

  // GUI related stuff
  CDXUTDialog              m_HUD;
  CDXUTDialog              m_SampleUI;
  CD3DSettingsDlg          m_SettingsDlg;
  CDXUTDialogResourceManager m_DialogResourceManager;

  enum GUI_IDS
    {
      IDC_TOGGLE_FULLSCREEN,
      IDC_TOGGLE_REF,
      IDC_CHANGE_DEVICE,
      IDC_RENDER_MODE,
      IDC_RENDER_MODE_STATIC,
      IDC_INTERMEDIATE_RENDER_TARGET,
      IDC_INTERMEDIATE_RENDER_TARGET_STATIC,
      IDC_ENABLE_SIMULATION,
      IDC_ENABLE_PERTURBATION,
      IDC_ENABLE_WATER_GHOST,
      IDC_WIREFRAME_WATER,
      IDC_WALK_ON_GROUND
    };
};



//
// Global function prototypes
//
int      dprintf(const char* formatString, ...);
void     dprintf(const D3DXMATRIX& matrix);
void     dprintf(const char* description, const D3DXMATRIX& matrix);
HRESULT  drawQuad(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3& position0, D3DCOLOR color0, const D3DXVECTOR2& texCoord0,
                  const D3DXVECTOR3& position1, D3DCOLOR color1, const D3DXVECTOR2& texCoord1,
                  const D3DXVECTOR3& position2, D3DCOLOR color2, const D3DXVECTOR2& texCoord2,
                  const D3DXVECTOR3& position3, D3DCOLOR color3, const D3DXVECTOR2& texCoord3);
void     fatalError(const char* formatString, ...);
D3DCOLOR randomColor(void);
WCHAR*   toUnicode(const char* string);
char*    fromUnicode(const WCHAR* string);
