
////////////////////////////////////////////////////////////////////////////////
//
//                              Questions?
//  Author: Jeremy Zelsnack
//  Contact sdkfeedback@nvidia.com
//
////////////////////////////////////////////////////////////////////////////////

// The Dec 2004 and Feb 2005 DXSDK's seem to have a bug that prevents the
// vertex shader from compiling.
#define FXC_WORK_AROUND 1

#include "dxstdafx.h"

#include "Mesh.h"
#include "registers.h"
#include "Vertex.h"
#include "Viewer.h"
#include "Water.h"

#if _MSC_VER == 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

// Evil globals
LPDIRECT3DDEVICE9 gDirect3DDevice9 = NULL;

// Static Water variables
LPDIRECT3DVERTEXBUFFER9 Water::m_quadVertexBuffer = NULL;
Water* Water::m_Water = NULL;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
  Water water;

  // Set the callback functions. These functions allow the sample framework to notify
  // the application about device changes, user input, and windows messages.  The 
  // callbacks are optional so you need only set callbacks for events you're interested 
  // in. However, if you don't handle the device reset/lost callbacks then the sample 
  // framework won't be able to reset your device since the application must first 
  // release all device resources before resetting.  Likewise, if you don't handle the 
  // device created/destroyed callbacks then the sample framework won't be able to 
  // recreate your device resources.
  DXUTSetCallbackD3D9DeviceCreated(Water::OnCreateDeviceCallback);
  DXUTSetCallbackD3D9DeviceReset(Water::OnResetDeviceCallback);
  DXUTSetCallbackD3D9DeviceLost(Water::OnLostDeviceCallback);
  DXUTSetCallbackD3D9DeviceDestroyed(Water::OnDestroyDeviceCallback);
  DXUTSetCallbackMsgProc(Water::MsgProcCallback);
  DXUTSetCallbackKeyboard(Water::KeyboardProcCallback);
  DXUTSetCallbackD3D9FrameRender(Water::OnFrameRenderCallback);
  DXUTSetCallbackD3D9DeviceFrameMove(Water::OnFrameMoveCallback);

  // Show the cursor and clip it when in full screen
  DXUTSetCursorSettings(true, true);


  // Initialize the sample framework and create the desired Win32 window and Direct3D 
  // device for the application. Calling each of these functions is optional, but they
  // allow you to set several options which control the behavior of the framework.
  DXUTInit(true, true,NULL, true); // Parse the command line, handle the default hotkeys, and show msgboxes
  DXUTCreateWindow(L"VertexTextureFetchWater");
  DXUTCreateDevice( true, 800, 600);

  // Pass control to the sample framework for handling the message pump and 
  // dispatching render calls. The sample framework will call your FrameMove 
  // and FrameRender callback when there is idle time between handling window messages.
  DXUTMainLoop();

  // Perform any application-level cleanup here. Direct3D device resources are released within the
  // appropriate callback functions and therefore don't require any cleanup code here.

  // These callbacks get called after water has been destructed. That's not good.
  // OnLostDevice() and OnDestroyDevice() are called in Water::~Water for clean-up.
  DXUTSetCallbackD3D9DeviceLost(NULL);
  DXUTSetCallbackD3D9DeviceDestroyed(NULL);


  return(DXUTGetExitCode());
}

//-----------------------------------------------------------------------------
// Name: Water()
// Desc: Application constructor.   Paired with ~Water()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
Water::Water(void)
{
  // Record this to a static variable. Only 1 instance of Water can exist.
  assert(m_Water == NULL);
  m_Water = this;

  // Initialize with default values
  m_pd3dDevice                        = NULL;
  m_font                              = NULL;
  m_loadingApp                        = TRUE;
  m_deviceWidth                       = 1;
  m_deviceHeight                      = 1;
  m_land                              = NULL;
  m_height                            = 5.0f;
  m_walkOnGround                      = false;
  m_underwater                        = false;
  m_waveSimulationWidth               = 128;
  m_waveSimulationHeight              = 128;
  m_waveSimulationIndex               = 0;
  m_mouseIsClipped                    = false;
  m_textureOverlayIndex               = 0;
  m_displayHelp                       = false;
  m_freezeSimulation                  = false;
  m_stopPertubingWater                = false;
  m_enableWaterGhost                  = true;
  m_renderRefractionOnly              = false;
  m_renderReflectionOnly              = false;
  m_renderFresnelOnly                 = false;
  m_renderNormalsOnly                 = false;
  m_renderWireframeWater              = false;
  m_renderText                        = true;
  m_renderGUI                         = true;
  m_renderSunkenBoat                  = true;
  m_refractionMapOverdraw             = 1.25f;
  m_reflectionMapOverdraw             = 1.5f;
  m_reflectionTextureWidth            = 512;
  m_reflectionTextureHeight           = 512;
  m_refractionTextureWidth            = 512;
  m_refractionTextureHeight           = 512;
  m_fontColor                         = D3DCOLOR_ARGB(255, 255, 255, 127);
  m_fontPosition.x                    = 0;
  m_fontPosition.y                    = 0;
  m_backBufferRenderTarget            = NULL;
  m_aspectRatio                       = 1.0f;
  m_fieldOfView                       = 60.0f * D3DX_PI / 180.0f; 
  m_time                              = 0.0f;
  m_tLastFrame                        = -1.0f;
  m_dt                                = 0.0f;


  // Initialize clip planes
  m_refractionClipPlaneAboveWater = D3DXPLANE(0.0f, -1.0f, 0.0f, 0.5f);
  m_reflectionClipPlaneAboveWater = D3DXPLANE(0.0f,  1.0f, 0.0f, 0.5f);
  m_refractionClipPlaneBelowWater = D3DXPLANE(0.0f,  1.0f, 0.0f, 0.5f);
  m_reflectionClipPlaneBelowWater = D3DXPLANE(0.0f, -1.0f, 0.0f, 0.0f);


  // Set up the path
  m_path.push_back(std::string("./"));
  m_path.push_back(std::string("src/VertexTextureFetchWater/"));
  m_path.push_back(std::string("DEMOS/Direct3D9/src/VertexTextureFetchWater/"));
  m_path.push_back(std::string("MEDIA/programs/VertexTextureFetchWater/"));
  m_path.push_back(std::string("MEDIA/models/"));
  m_path.push_back(std::string("MEDIA/textures/2D/"));

  // Set up the GUI
  InitGUI();
}

//-----------------------------------------------------------------------------
// Name: ~Water()
// Desc: Application destructor.  Paired with Water()
//-----------------------------------------------------------------------------
Water::~Water()
{
  OnLostDevice();
  OnDestroyDevice();
}





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//               Callbacks for 9.0c sample framework
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Water::IsDeviceAcceptableCallback( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
  assert(m_Water != NULL);

  return(m_Water->IsDeviceAcceptable(pCaps, AdapterFormat, BackBufferFormat, bWindowed));
}

bool Water::ModifyDeviceSettingsCallback( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
  assert(m_Water != NULL);
  
  return(m_Water->ModifyDeviceSettings(pDeviceSettings, pCaps));
}

HRESULT Water::OnCreateDeviceCallback( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
  assert(m_Water != NULL);

  return(m_Water->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
}

HRESULT Water::OnResetDeviceCallback( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
  assert(m_Water != NULL);

  return(m_Water->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc));
}

void Water::OnFrameMoveCallback( IDirect3DDevice9* pd3dDevice, double time, float elapsedTime, void* pUserContext )
{
  assert(m_Water != NULL);

  m_Water->OnFrameMove(pd3dDevice, time, elapsedTime);
}

void Water::OnFrameRenderCallback( IDirect3DDevice9* pd3dDevice, double time, float elapsedTime, void* pUserContext )
{
  assert(m_Water != NULL);

  m_Water->OnFrameRender(pd3dDevice, time, elapsedTime);
}

LRESULT Water::MsgProcCallback( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
  if(m_Water != NULL)
    {
      return(m_Water->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing));
    }
  
  return(0);
}

void Water::KeyboardProcCallback( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
  assert(m_Water != NULL);

  m_Water->KeyboardProc(nChar, bKeyDown, bAltDown);
}

void Water::OnGUIEventCallback( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
  assert(m_Water != NULL);

  m_Water->OnGUIEvent(nEvent, nControlID, pControl);
}

void Water::OnLostDeviceCallback(void* pUserContext)
{
  assert(m_Water != NULL);

  m_Water->OnLostDevice();
}

void Water::OnDestroyDeviceCallback(void* pUserContext)
{
  assert(m_Water != NULL);

  m_Water->OnDestroyDevice();
}





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//                       Water's own methods
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
// 
// Copies the simulation texture from fp16 texture to fp32 texture. This is
// necessary because vertex texture fetch only works on fp32 formats.
//        
//-----------------------------------------------------------------------------
void Water::CopySimulationTexture(LPDIRECT3DTEXTURE9 simulationSourceTexture, LPDIRECT3DSURFACE9 currentRenderTarget)
{
  D3DXMATRIX identity;
  HRESULT    hr;

  D3DXMatrixIdentity(&identity);

  // Copy the simulation from the D3DFMT_A16B16G16R16F texture to D3DFMT_A32B32G32R32F texture
  hr = m_pd3dDevice->SetRenderTarget(0, m_waveSimulationVertexShaderSurface);
  assert(hr == D3D_OK);
  
  // Set up state
  m_pd3dDevice->SetTexture(0, simulationSourceTexture);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  m_pd3dDevice->SetTransform(D3DTS_WORLD, &identity);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, &identity);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &identity);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  // Draw the quad to do the simulation
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3(-1.0f, -1.0f,  0.0f), randomColor(), D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3( 1.0f, -1.0f,  0.0f), randomColor(), D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3( 1.0f,  1.0f,  0.0f), randomColor(), D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3(-1.0f,  1.0f,  0.0f), randomColor(), D3DXVECTOR2(0.0f, 1.0f));

  // Reset state
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

  // Reset the matrix state
  SetMatrices();


  hr = m_pd3dDevice->SetRenderTarget(0, currentRenderTarget);
  assert(hr == D3D_OK);  
}


//-----------------------------------------------------------------------------
// 
// Loads a pixel shader from disk and creates it.
//        
//-----------------------------------------------------------------------------
bool Water::CreatePixelShader(LPDIRECT3DDEVICE9 device, const char* filename, D3DXMACRO* defines, const char* function, 
                              const char* compileTarget, LPDIRECT3DPIXELSHADER9& pixelShader)
{
  assert(filename != NULL);
  assert(function != NULL);
  assert(compileTarget != NULL);

  HRESULT      hr;
  LPD3DXBUFFER compiledShader;
  std::string  fullpath;

  pixelShader = NULL;

  // Find the file
  FindFile(std::string(filename), fullpath);

  // Compile the HLSL function
  hr = D3DXCompileShaderFromFile(toUnicode(fullpath.c_str()), defines, NULL, function, compileTarget, 0, &compiledShader, NULL, NULL);
  if(hr != D3D_OK)
    {
      DXTRACE_ERR(L"CreatePixelShader() D3DXCompileShaderFromFile", hr);
      dprintf("CreatePixelShader failed    shader = \"%s:%s\"\n", filename, function); 
      SAFE_RELEASE(compiledShader);

      return(false);
    }

  // Create the pixel shader
  hr = device->CreatePixelShader((DWORD*)compiledShader->GetBufferPointer(), &pixelShader);
  SAFE_RELEASE(compiledShader);
  if(hr != D3D_OK)
    {
      DXTRACE_ERR(L"CreatePixelShader() CreatePixelShader", hr);

      pixelShader = NULL;
      return(false);
    }

  return(true);
}


//-----------------------------------------------------------------------------
// 
// Loads a vertex shader from disk and creates it.
//        
//-----------------------------------------------------------------------------
bool Water::CreateVertexShader(LPDIRECT3DDEVICE9 device, const char* filename, const char* function, 
                               const char* compileTarget, LPDIRECT3DVERTEXSHADER9& vertexShader)
{
  assert(filename != NULL);
  assert(function != NULL);
  assert(compileTarget != NULL);

  HRESULT      hr;
  LPD3DXBUFFER compiledShader;
  std::string  fullpath;

  vertexShader = NULL;

  // Find the file
 // FindFile(std::string(filename), fullpath);

  // Compile the HLSL function
  hr = D3DXCompileShaderFromFile(toUnicode(filename), NULL, NULL, function, compileTarget, 0, &compiledShader, NULL, NULL);
  if(hr != D3D_OK)
    {
      DXTRACE_ERR(L"CreateVertexShader() D3DXCompileShaderFromFile", hr);
      dprintf("CreateVertexShader failed     shader = \"%s:%s\"\n", filename, function); 
      SAFE_RELEASE(compiledShader);

      return(false);
    }

  // Create the vertex shader
  hr = device->CreateVertexShader((DWORD*)compiledShader->GetBufferPointer(), &vertexShader);
  SAFE_RELEASE(compiledShader);
  if(hr != D3D_OK)
    {
      DXTRACE_ERR(L"CreateVertexShader() CreateVertexShader", hr);

      vertexShader = NULL;
      return(false);
    }

  return(true);
}


//-----------------------------------------------------------------------------
// 
// Loads a vertex shader from disk and creates it.
//        
//-----------------------------------------------------------------------------
bool Water::CreateVertexShader(LPDIRECT3DDEVICE9 device, const char* filename, LPDIRECT3DVERTEXSHADER9& vertexShader)
{
  assert(filename != NULL);

  HRESULT      hr;
  LPD3DXBUFFER compiledShader;
  std::string  fullpath;

  // Find the file
  FindFile(std::string(filename), fullpath);

  // Assemble the file
  hr = D3DXAssembleShaderFromFile(toUnicode(fullpath.c_str()), NULL, NULL, 0, &compiledShader, NULL);
  if(hr != D3D_OK)
    {
      dprintf("CreateVertexShader failed    shader = \"%s\"\n", filename);
      SAFE_RELEASE(compiledShader);
      return(false);
    }

  // Create the vertex shader
  hr = m_pd3dDevice->CreateVertexShader((DWORD*)compiledShader->GetBufferPointer(), &vertexShader);
  SAFE_RELEASE(compiledShader);
  if(hr != D3D_OK)
    {
      dprintf("CreateVertexShader::CreateVertexShader() failed    shader = \"%s\"\n", filename);

      vertexShader = NULL;
      return(false);
    }

  return(true);
}


bool Water::CreateVertexShaderFromPreCompiledFile(LPDIRECT3DDEVICE9 device, const WCHAR* preCompiledFile, LPDIRECT3DVERTEXSHADER9& vertexShader)
{
	assert(preCompiledFile != NULL);

	HRESULT hr = E_FAIL;
	DWORD dwBytesRead = 0;

	// Open the file
	HANDLE hFile = CreateFile(preCompiledFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return DXUTERR_MEDIANOTFOUND;

	LARGE_INTEGER FileSize;
	GetFileSizeEx(hFile, &FileSize);
	UINT cBytes = FileSize.LowPart;

	//allocate
	BYTE * data = new BYTE[(size_t)(cBytes)];
	
	LARGE_INTEGER liMove;
	liMove.QuadPart = 0;
	if (!SetFilePointerEx(hFile, liMove, NULL, FILE_BEGIN))
		return false;

	if (!ReadFile(hFile, data,cBytes , &dwBytesRead, NULL))
	{
		return false;
	}

	vertexShader = NULL;
	// Create the vertex shader
	hr = device->CreateVertexShader((DWORD*)data, &vertexShader);

	if (hr != D3D_OK)
	{
		DXTRACE_ERR(L"CreateVertexShader() CreateVertexShader", hr);
		vertexShader = NULL;
		return(false);
	}

	CloseHandle(hFile);
	return(true);
}

bool Water::CreatePixelShaderFromPreCompiledFile(LPDIRECT3DDEVICE9 device, const WCHAR* preCompiledFile, LPDIRECT3DPIXELSHADER9& pixelShader)
{
	assert(preCompiledFile != NULL);

	HRESULT hr = E_FAIL;
	DWORD dwBytesRead = 0;

	// Open the file
	HANDLE hFile = CreateFile(preCompiledFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return DXUTERR_MEDIANOTFOUND;

	LARGE_INTEGER FileSize;
	GetFileSizeEx(hFile, &FileSize);
	UINT cBytes = FileSize.LowPart;

	//allocate
	BYTE * data = new BYTE[(size_t)(cBytes)];

	LARGE_INTEGER liMove;
	liMove.QuadPart = 0;
	if (!SetFilePointerEx(hFile, liMove, NULL, FILE_BEGIN))
		return false;

	if (!ReadFile(hFile, data, cBytes, &dwBytesRead, NULL))
	{
		return false;
	}

	pixelShader = NULL;
	// Create the vertex shader
	hr = device->CreatePixelShader((DWORD*)data, &pixelShader);

	if (hr != D3D_OK)
	{
		DXTRACE_ERR(L"CreateVertexShader() CreateVertexShader", hr);
		pixelShader = NULL;
		return(false);
	}

	CloseHandle(hFile);
	return(true);
}

//-----------------------------------------------------------------------------
// 
// Destroy all of the vertex and pixel shaders
//        
//-----------------------------------------------------------------------------
bool Water::DestroyShaders(void)
{
  SAFE_RELEASE(m_waveSimulationVertexShader);
  SAFE_RELEASE(m_waveSimulationPixelShader);
  SAFE_RELEASE(m_waveSimulationPerturbationVertexShader);
  SAFE_RELEASE(m_waveSimulationPerturbationPixelShader);
  SAFE_RELEASE(m_waveRenderingVertexShader);
  SAFE_RELEASE(m_waveRenderingPixelShader);
  SAFE_RELEASE(m_waveRenderingRefractionPixelShader);
  SAFE_RELEASE(m_waveRenderingReflectionPixelShader);
  SAFE_RELEASE(m_waveRenderingFresnelPixelShader);
  SAFE_RELEASE(m_waveRenderingNormalsPixelShader);
  SAFE_RELEASE(m_underwaterPixelShader);
  SAFE_RELEASE(m_causticVertexShader);
  SAFE_RELEASE(m_causticPixelShader);
  SAFE_RELEASE(m_diffuseVertexShader);
  SAFE_RELEASE(m_diffusePixelShader);
  SAFE_RELEASE(m_singleTextureVertexShader);
  SAFE_RELEASE(m_singleTexturePixelShader);  

  return(true);
}


//-----------------------------------------------------------------------------
// 
// Searches paths to find the full path of a given file
//        
//-----------------------------------------------------------------------------
bool Water::FindFile(const std::string& filename, std::string& fullpath, bool exitOnFail) const
{
  FILE*       fin;
  int         i;
  int         j;
  std::string prefixPath;
  std::string path;
  
  for(j=0; j<(int)m_path.size(); j++)
    {
      prefixPath = m_path[j];
      for(i=0; i<10; i++)
        {
          path = prefixPath + filename;

          if((fin = fopen(path.c_str(), "r")) != NULL)
            {
              fullpath = path;
              dprintf("found \"%s\"\n", fullpath.c_str());
              fclose(fin);
              return(true);
            }

          prefixPath = std::string("../") + prefixPath;
        }
    }

  if(exitOnFail)
    {
      fatalError("Water::FindFile() failed for \"%s\"", filename.c_str());
    }

  return(false);
}

//-----------------------------------------------------------------------------
// 
// Initialize all of the gui related stuff
//        
//-----------------------------------------------------------------------------
void Water::InitGUI(void)
{
  // Initialize dialogs
  m_SettingsDlg.Init( &m_DialogResourceManager );
  m_SampleUI.Init( &m_DialogResourceManager );
  m_HUD.Init( &m_DialogResourceManager );

  int y;
  static int intermediateRenderTargets[] = {0, 1, 2, 3, 5};

  y = 10;
  m_HUD.SetCallback(OnGUIEventCallback);
  m_HUD.AddButton(IDC_TOGGLE_FULLSCREEN, L"Toggle Fullscreen", 35, y, 125, 22);
  m_HUD.AddButton(IDC_TOGGLE_REF, L"Toggle REF (F3)", 35, y += 24, 125, 22, VK_F3);
  m_HUD.AddButton(IDC_CHANGE_DEVICE, L"Chance Device (F2)", 35, y += 24, 125, 22, VK_F2);


  y = 10;
  m_SampleUI.SetCallback(OnGUIEventCallback);
  m_SampleUI.AddStatic(IDC_RENDER_MODE_STATIC, L"Render Mode", 5, y += 24, 75, 22);
  m_SampleUI.AddComboBox(IDC_RENDER_MODE, 5, y += 15, 150, 24); 
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->AddItem(L"Regular", NULL);
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->AddItem(L"Refraction", (void*)&m_renderRefractionOnly);
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->AddItem(L"Reflection", (void*)&m_renderReflectionOnly);
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->AddItem(L"Fresnel", (void*)&m_renderFresnelOnly);
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->AddItem(L"Normals", (void*)&m_renderNormalsOnly);
  m_SampleUI.GetComboBox(IDC_RENDER_MODE)->SetSelectedByIndex(0);
  y += 10;

  m_SampleUI.AddStatic(IDC_INTERMEDIATE_RENDER_TARGET_STATIC, L"Intermediate Render Target", 5, y += 24, 140, 22 );
  m_SampleUI.AddComboBox(IDC_INTERMEDIATE_RENDER_TARGET, 5, y += 15, 150, 24); 
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->AddItem(L"None", (void*)(&intermediateRenderTargets[0]));
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->AddItem(L"Refraction", (void*)(&intermediateRenderTargets[1]));
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->AddItem(L"Reflection", (void*)(&intermediateRenderTargets[2]));
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->AddItem(L"Refraction Near", (void*)(&intermediateRenderTargets[3]));
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->AddItem(L"Simulation", (void*)(&intermediateRenderTargets[4]));
  m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->SetSelectedByIndex(0);
  y += 40;

  m_SampleUI.AddCheckBox(IDC_ENABLE_SIMULATION, L"Enable Simulation (1)", 5, y += 24, 150, 22, !m_freezeSimulation, '1');
  m_SampleUI.AddCheckBox(IDC_ENABLE_PERTURBATION, L"Enable Peturbation (2)", 5, y += 24, 150, 22, !m_stopPertubingWater, '2');
  m_SampleUI.AddCheckBox(IDC_ENABLE_WATER_GHOST, L"Enable Water Ghost (3)", 5, y += 24, 150, 22, m_enableWaterGhost, '3');
  m_SampleUI.AddCheckBox(IDC_WALK_ON_GROUND, L"Walk on Ground (4)", 5, y += 24, 150, 22, m_walkOnGround, '4');  
  m_SampleUI.AddCheckBox(IDC_WIREFRAME_WATER, L"Wireframe Water (5)", 5, y += 24, 150, 22, m_renderWireframeWater, '5');  
}


//-----------------------------------------------------------------------------
// 
// Initialize all of the vertex and pixel shaders
//        
//-----------------------------------------------------------------------------
bool Water::InitShaders(void)
{
  assert(m_pd3dDevice != NULL);

  D3DXMACRO renderRefractionIncludes[] = {{"RETURN_REFRACTION", "1"}, NULL};
  D3DXMACRO renderReflectionIncludes[] = {{"RETURN_REFLECTION", "1"}, NULL};
  D3DXMACRO renderFresnelIncludes[] = {{"RETURN_FRESNEL", "1"}, NULL};
  D3DXMACRO renderNormalsIncludes[] = {{"RETURN_NORMALS", "1"}, NULL};

  //simulation
  CreateVertexShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\simulation.vertexShader", m_waveSimulationVertexShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\simulation.pixelShader", m_waveSimulationPixelShader);
  //perturbSimulation
  CreateVertexShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\perturbSimulation.vertexShader", m_waveSimulationPerturbationVertexShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\perturbSimulation.pixelShader", m_waveSimulationPerturbationPixelShader);
  //caustic
  CreateVertexShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\caustic.vertexShader", m_causticVertexShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\caustic.pixelShader", m_causticPixelShader);
  //diffuse
  CreateVertexShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\diffuse.vertexShader", m_diffuseVertexShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\diffuse.pixelShader", m_diffusePixelShader);
  //singleTexture
  CreateVertexShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\singleTexture.vertexShader", m_singleTextureVertexShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\singleTexture.pixelShader", m_singleTexturePixelShader);

  //render
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\render.pixelShader", m_waveRenderingPixelShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\render.pixelRefractionShader", m_waveRenderingRefractionPixelShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\render.pixelReflectionlShader", m_waveRenderingReflectionPixelShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\render.pixelFresnelShader", m_waveRenderingFresnelPixelShader);
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\render.pixelNormalsShader", m_waveRenderingNormalsPixelShader);

  //underwater
  CreatePixelShaderFromPreCompiledFile(m_pd3dDevice, L"MEDIA\\programs\\VertexTextureFetchWater\\underwater.pixelShader", m_underwaterPixelShader);

  /*
  CreateVertexShader(m_pd3dDevice, "simulation.hlsl", "vertexShader", "vs_1_1", m_waveSimulationVertexShader);
  CreatePixelShader(m_pd3dDevice, "simulation.hlsl", NULL, "pixelShader", "ps_2_0", m_waveSimulationPixelShader);
  CreateVertexShader(m_pd3dDevice, "perturbSimulation.hlsl", "vertexShader", "vs_2_0", m_waveSimulationPerturbationVertexShader);
  CreatePixelShader(m_pd3dDevice, "perturbSimulation.hlsl", NULL, "pixelShader", "ps_2_0", m_waveSimulationPerturbationPixelShader);

  CreatePixelShader(m_pd3dDevice, "underwater.hlsl", NULL, "pixelShader", "ps_2_0", m_underwaterPixelShader);
  CreateVertexShader(m_pd3dDevice, "caustic.hlsl", "vertexShader", "vs_1_1", m_causticVertexShader);
  CreatePixelShader(m_pd3dDevice, "caustic.hlsl", NULL, "pixelShader", "ps_2_0", m_causticPixelShader);
  CreateVertexShader(m_pd3dDevice, "diffuse.hlsl", "vertexShader", "vs_1_1", m_diffuseVertexShader);
  CreatePixelShader(m_pd3dDevice, "diffuse.hlsl", NULL, "pixelShader", "ps_1_1", m_diffusePixelShader);
  CreateVertexShader(m_pd3dDevice, "singleTexture.hlsl", "vertexShader", "vs_1_1", m_singleTextureVertexShader);
  CreatePixelShader(m_pd3dDevice, "singleTexture.hlsl", NULL, "pixelShader", "ps_1_1", m_singleTexturePixelShader);
  CreatePixelShader(m_pd3dDevice, "render.hlsl", NULL, "pixelShader", "ps_3_0", m_waveRenderingPixelShader);

  CreatePixelShader(m_pd3dDevice, "render.hlsl", renderRefractionIncludes, "pixelShader", "ps_3_0", m_waveRenderingRefractionPixelShader);
  CreatePixelShader(m_pd3dDevice, "render.hlsl", renderReflectionIncludes, "pixelShader", "ps_3_0", m_waveRenderingReflectionPixelShader);
  CreatePixelShader(m_pd3dDevice, "render.hlsl", renderFresnelIncludes, "pixelShader", "ps_3_0", m_waveRenderingFresnelPixelShader);
  CreatePixelShader(m_pd3dDevice, "render.hlsl", renderNormalsIncludes, "pixelShader", "ps_3_0", m_waveRenderingNormalsPixelShader);

  */
#ifdef FXC_WORK_AROUND
  CreateVertexShader(m_pd3dDevice, "renderVertexShader.vsh", m_waveRenderingVertexShader);
#else
  CreateVertexShader(m_pd3dDevice, "render.hlsl", "vertexShader", "vs_3_0", m_waveRenderingVertexShader);
#endif
 

  
  
  return(true);
}


//-----------------------------------------------------------------------------
// 
// Returns asyncrhonously whether or not a key is down
//        
//-----------------------------------------------------------------------------
bool Water::KeyIsDown(int virtualKeyCode)
{
  return((GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0);
}


//-----------------------------------------------------------------------------
// 
// Load a texture from disk
//        
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 Water::LoadTexture(const char* filename)
{
  assert(filename != NULL);
  assert(m_pd3dDevice != NULL);

  HRESULT            hr;
  LPDIRECT3DTEXTURE9 texture;
  std::string        fullpath;

  FindFile(filename, fullpath);

  hr = D3DXCreateTextureFromFile(m_pd3dDevice, toUnicode(fullpath.c_str()), &texture);
  assert(hr == D3D_OK);  

  return(texture);
}



//-----------------------------------------------------------------------------
// 
// Render text to screen
//
//-----------------------------------------------------------------------------
int Water::Printf(const char* formatString, ...)
{
  assert(formatString != NULL);

  // Don't render text if desired
  if(m_renderText == false)
    {
      return(0);
    }


  va_list argumentList;
  int     returnValue;
  char    outputString[4096];
  RECT    textRect;

  va_start(argumentList, formatString);
  returnValue = vsprintf(outputString, formatString, argumentList);
  va_end(argumentList);


  textRect.left = m_fontPosition.x;
  textRect.right = textRect.left + 4096;
  textRect.top = m_fontPosition.y;
  textRect.bottom = textRect.top + 4096;
  m_font->DrawText(NULL, toUnicode(outputString), -1, &textRect, DT_NOCLIP, m_fontColor);

  m_fontPosition.y += 20;

  return(returnValue);
}


//-----------------------------------------------------------------------------
// 
// Render the normal geometry to the backbuffer. If we're underwater, apply
// a full-screen wavy effect.
//
//-----------------------------------------------------------------------------
void Water::RenderBackbuffer(void)
{
  HRESULT hr;

  if(m_underwater)
    {
      // Set up fog
      float fogDensity = 0.02f;

      m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, 1);
      m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_RGBA(0, 40, 120, 255));
      m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, (*(DWORD*)(&fogDensity)));
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);


      // If we're under water, we actually render to texture. This way we can 
      // apply a full-screen wave effect on it.
      hr = m_pd3dDevice->SetRenderTarget(0, m_underwaterWorldRenderTargetSurface);
      assert(hr == D3D_OK);
    }
  else
    {
      hr = m_pd3dDevice->SetRenderTarget(0, m_backBufferRenderTarget);
      assert(hr == D3D_OK);          
    }

  // Do the actual rendering of the world
  m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);
  RenderScene();
  RenderSkybox();
  m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, 0);




  if(m_underwater)
    {
      //
      // Render caustics on top off all the world geometry
      // This is blended into what is already in the render target
      //
      m_pd3dDevice->SetVertexShader(m_causticVertexShader);
      m_pd3dDevice->SetPixelShader(m_causticPixelShader);

      m_pd3dDevice->SetTexture(0, m_waveSimulationTexture[(m_waveSimulationIndex + 0) % 3]);

      assert(hr == D3D_OK);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      RenderScene(RENDER_LAND | RENDER_SUNKEN_BOAT, true);
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

      m_pd3dDevice->SetVertexShader(NULL);
      m_pd3dDevice->SetPixelShader(NULL);
      
      m_pd3dDevice->SetTexture(0, NULL);
          

      //
      //
      //  Render the full-screen wavy effect.
      //
      //

      //
      // We were rendering to texture so we could do the full-screen wavy effect.
      // Now, really use the backbuffer as the render target
      //
      m_pd3dDevice->SetRenderTarget(0, m_backBufferRenderTarget);

      m_worldMatrixStack->Push();
      m_worldMatrixStack->LoadIdentity();
          
      m_viewMatrixStack->Push();
      m_viewMatrixStack->LoadIdentity();
          
      m_projectionMatrixStack->Push();
      m_projectionMatrixStack->LoadIdentity();

      SetMatrices();
          
      m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
      m_pd3dDevice->SetPixelShader(m_underwaterPixelShader);
      m_pd3dDevice->SetPixelShaderConstantF(PS_TIME, (const float*)&D3DXVECTOR4(m_time, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetTexture(0, m_underwaterWorldRenderTargetTexture);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

      drawQuad(m_pd3dDevice, 
               D3DXVECTOR3(-1.0f, -1.0f,  0.0f), randomColor(), D3DXVECTOR2(0.0f, 1.0f),
               D3DXVECTOR3(-1.0f,  1.0f,  0.0f), randomColor(), D3DXVECTOR2(0.0f, 0.0f),
               D3DXVECTOR3( 1.0f,  1.0f,  0.0f), randomColor(), D3DXVECTOR2(1.0f, 0.0f),
               D3DXVECTOR3( 1.0f, -1.0f,  0.0f), randomColor(), D3DXVECTOR2(1.0f, 1.0f));
      m_pd3dDevice->SetPixelShader(NULL);
      m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, true);
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
      m_pd3dDevice->SetTexture(0, NULL);

      m_worldMatrixStack->Pop();
      m_viewMatrixStack->Pop();
      m_projectionMatrixStack->Pop();
      SetMatrices();
    }
}

//-----------------------------------------------------------------------------
// 
// Render the informational text.
//
//-----------------------------------------------------------------------------
void Water::RenderInformation(void)
{  
  // Set up the transform state for text
  m_worldMatrixStack->Push();
  m_worldMatrixStack->LoadIdentity();
      
  m_viewMatrixStack->Push();
  m_viewMatrixStack->LoadIdentity();
      
  m_projectionMatrixStack->Push();
  m_projectionMatrixStack->LoadIdentity();

  SetMatrices();


  // Render various render targets if desired
  if(m_textureOverlayIndex > 0)
    {
      LPDIRECT3DTEXTURE9 texture;
      char*              description0;
      char*              description1;

      switch(m_textureOverlayIndex)
        {
        case 1:
          {
            texture = m_waveRefractionTexture;
            description0 = "refraction texture";
            description1 = "";
            break;
          }
        case 2:
          {
            texture = m_waveReflectionTexture;
            description0 = "reflection texture";
            description1 = "";
            break;
          }
        case 3:
          {
            texture = m_waveRefractionNearTexture;
            description0 = "reflection texture for";
            description1 = "objects penetrating the water";
            break;
          }
        case 4:
          {
            texture = m_waveReflectionNearTexture;
            description0 = "refraction texture for";
            description1 = "objects penetrating the water";
            break;
          }
        case 5:
          {            
            texture = m_waveSimulationTexture[(m_waveSimulationIndex + 0) % 3];
            description0 = "wave equation simulation";
            description1 = "height = r,   normal = gba";
            break;
          }
        default:
          {
            assert("Shouldn't be here" == NULL);
            description0 = "";
            description1 = "";
            break;
          }
        }



      // Draw the texture
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      m_pd3dDevice->SetTexture(0, texture);
      drawQuad(m_pd3dDevice, 
               D3DXVECTOR3( 0.00f,  0.00f, 0.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
               D3DXVECTOR3( 1.00f,  0.00f, 0.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
               D3DXVECTOR3( 1.00f, -1.00f, 0.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
               D3DXVECTOR3( 0.00f, -1.00f, 0.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
      m_pd3dDevice->SetTexture(0, NULL);

      // Render the text description
      m_fontPosition.x = m_deviceWidth / 2;
      m_fontPosition.y = m_deviceHeight / 2;
      Printf(description0);
      Printf(description1);
    }

  // Render info about rendering
  m_fontPosition.x = 10;
  m_fontPosition.y = 10;
  Printf(fromUnicode(DXUTGetFrameStats()));
  Printf(fromUnicode(DXUTGetDeviceStats()));

  // Render info about current state
  m_fontPosition.x = m_deviceWidth / 2;
  m_fontPosition.y = 20;  

  // Render help blurb and help
  m_fontPosition.x = 10 + (int)(50.0f + 50.0f * sin(m_time));
  m_fontPosition.y = m_deviceHeight - 30;
  Printf("Press 'F1' to toggle help");

  if(m_displayHelp)
    {
      m_fontPosition.x = 10;
      m_fontPosition.y = 100;

      Printf("Drag with left button down to rotate camera");
      Printf("'M' to toggle mouse camera control");      
      Printf("WASD to move");
      Printf("'E' to move eye up");
      Printf("'Q' to move eye down");
      Printf("'Esc' to quit");
    }

  // Reset the transform state
  m_worldMatrixStack->Pop();
  m_viewMatrixStack->Pop();
  m_projectionMatrixStack->Pop();
  SetMatrices();
}


//-----------------------------------------------------------------------------
// 
// Render the scene into the reflection texture
//
//-----------------------------------------------------------------------------
void Water::RenderReflectionTexture(void)
{
  // Render the water using the vertex shader with texture fetch
  D3DXPLANE clipPlane;
  D3DXPLANE transformedClipPlane;
  float     reflectionMapFieldOfView;

  // Render to the reflection map
  m_pd3dDevice->SetRenderTarget(0, m_waveReflectionSurface);

  // Compute the field of view and use it
  reflectionMapFieldOfView = 2.0f * atan((tan(m_fieldOfView / 2.0f)) * m_reflectionMapOverdraw);
  D3DXMatrixPerspectiveFovRH(&m_reflectionProjectionMatrix, reflectionMapFieldOfView, m_aspectRatio, 1.0f, 100.0f);

  m_projectionMatrixStack->Push();
  m_projectionMatrixStack->LoadMatrix(&m_reflectionProjectionMatrix);
  m_reflectViewMatrix = true;
  SetMatrices();

  // Set up the clip plane to draw the correct geometry
  if(m_underwater)
    {
      clipPlane = m_reflectionClipPlaneBelowWater;
    }
  else
    {
      clipPlane = m_reflectionClipPlaneAboveWater;
    }
  D3DXPlaneTransform(&transformedClipPlane, &clipPlane, &m_worldViewProjectionInverseTransposeMatrix);
  m_pd3dDevice->SetClipPlane(0, transformedClipPlane);


  // Clear
  m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);

  // Render the actual scene
  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
  RenderScene(RENDER_LAND | RENDER_SUNKEN_BOAT);
  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
  RenderSkybox();
  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);

#ifdef CODE_IS_NOT_USED   
  // Render the reflection map for objects penetrating the water
  // This is not used because it looks ugly
  m_pd3dDevice->SetRenderTarget(0, m_waveReflectionNearSurface);
  m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);
  RenderScene(RENDER_BOAT);
#endif


  // Reste state
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  m_projectionMatrixStack->Pop();
  m_reflectViewMatrix = false;

  SetMatrices();
}


//-----------------------------------------------------------------------------
// 
// Render the scene into the refraction texture
//
//-----------------------------------------------------------------------------
void Water::RenderRefractionTexture(void)
{
  D3DXPLANE clipPlane;
  D3DXPLANE transformedClipPlane;
  float     refractionMapFieldOfView;

  m_pd3dDevice->SetRenderTarget(0, m_waveRefractionSurface);

  refractionMapFieldOfView = 2.0f * atan((tan(m_fieldOfView / 2.0f)) * m_refractionMapOverdraw);
  D3DXMatrixPerspectiveFovRH(&m_refractionProjectionMatrix, refractionMapFieldOfView, m_aspectRatio, 1.0f, 100.0f);

  m_worldMatrixStack->LoadIdentity();
  m_projectionMatrixStack->Push();
  m_projectionMatrixStack->LoadMatrix(&m_refractionProjectionMatrix);
  SetMatrices();


  if(m_underwater)
    {
      clipPlane = m_refractionClipPlaneBelowWater;
    }
  else
    {
      clipPlane = m_refractionClipPlaneAboveWater;
    }
  D3DXPlaneTransform(&transformedClipPlane, &clipPlane, &m_worldViewProjectionInverseTransposeMatrix);
  m_pd3dDevice->SetClipPlane(0, transformedClipPlane);

  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);

  RenderScene(RENDER_LAND | RENDER_SUNKEN_BOAT);

  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

  RenderSkybox();

  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);

  m_pd3dDevice->SetRenderTarget(0, m_waveRefractionNearSurface);
  m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);
  RenderScene(RENDER_BOAT);

  m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

  // Use the default matrix setup
  m_projectionMatrixStack->Pop();
  SetMatrices();
}


//-----------------------------------------------------------------------------
// 
// Render the scene
// Selectively renders bit and pices depending on what the scene is
// being rendered for
//        
//-----------------------------------------------------------------------------
void Water::RenderScene(unsigned int objectSelection, bool disableStateSetup)
{
  if(objectSelection & RENDER_LAND)
    {
      RenderSceneLand(disableStateSetup);
    }

  if(objectSelection & RENDER_BOAT)
    {
      RenderSceneBoat(disableStateSetup);
    }

  if(objectSelection & RENDER_SUNKEN_BOAT)
    {
      RenderSceneSunkenBoat(disableStateSetup);
    }
}


//-----------------------------------------------------------------------------
// 
// Renders the boat in the scene
//        
//-----------------------------------------------------------------------------
void Water::RenderSceneBoat(bool disableStateSetup)
{
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  m_worldMatrixStack->Push();
  m_worldMatrixStack->LoadMatrix(&m_boatMesh.getWorldMatrix());
  SetMatrices();
  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(m_singleTextureVertexShader);
      m_pd3dDevice->SetPixelShader(m_singleTexturePixelShader);
      m_pd3dDevice->SetTexture(0, m_boatTexture);
    }
  m_boatMesh.render(disableStateSetup);
  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(NULL);
      m_pd3dDevice->SetPixelShader(NULL);
    }

  m_worldMatrixStack->Pop();
  SetMatrices();
}


//-----------------------------------------------------------------------------
// 
// Renders the land in the scene
//        
//-----------------------------------------------------------------------------
void Water::RenderSceneLand(bool disableStateSetup)
{
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  m_worldMatrixStack->Push();
  m_worldMatrixStack->LoadIdentity();
  SetMatrices();

  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(m_singleTextureVertexShader);
      m_pd3dDevice->SetPixelShader(m_singleTexturePixelShader);
    }

  m_land->render(m_pd3dDevice, disableStateSetup);

  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(NULL);
      m_pd3dDevice->SetPixelShader(NULL);
    }
      
  m_worldMatrixStack->Pop();
  SetMatrices();

  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}


//-----------------------------------------------------------------------------
// 
// Renders the sunken boat in the scene
//        
//-----------------------------------------------------------------------------
void Water::RenderSceneSunkenBoat(bool disableStateSetup)
{
  // Render the boat
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      
  m_worldMatrixStack->Push();
  m_worldMatrixStack->LoadMatrix(&m_sunkenBoatMesh.getWorldMatrix());
  SetMatrices();
  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(m_diffuseVertexShader);
      m_pd3dDevice->SetPixelShader(m_diffusePixelShader);
    }

  if(m_renderSunkenBoat)
    {
      m_sunkenBoatMesh.render(disableStateSetup);
    }

  if(disableStateSetup == false)
    {
      m_pd3dDevice->SetVertexShader(NULL);
      m_pd3dDevice->SetPixelShader(NULL);
    }

  m_worldMatrixStack->Pop();
  SetMatrices();

  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}


//-----------------------------------------------------------------------------
// 
// Render the skybox
//        
//-----------------------------------------------------------------------------
void Water::RenderSkybox(void)
{
  D3DXMATRIX worldMatrix;

  // Don't need culling for skybox
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  // Scale and translate the unit cube
  m_worldMatrixStack->Push();
  m_worldMatrixStack->Translate(1.0f, 0.0f, 1.0f);
  m_worldMatrixStack->Scale(25.0f, 25.0f, 25.0f);
  SetMatrices();

  // Set up rendering state
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);

  // Draw the skybox
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[0]);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3(-1.0f, -1.0f,  1.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3( 1.0f, -1.0f,  1.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3( 1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3(-1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[1]);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3( 1.0f, -1.0f,  1.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3( 1.0f, -1.0f, -1.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3( 1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3( 1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[2]);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3( 1.0f, -1.0f, -1.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3(-1.0f, -1.0f, -1.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3(-1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3( 1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[3]);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3(-1.0f, -1.0f, -1.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3(-1.0f, -1.0f,  1.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3(-1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3(-1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[4]);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3( 1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3(-1.0f,  1.0f, -1.0f), 0, D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3(-1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3( 1.0f,  1.0f,  1.0f), 0, D3DXVECTOR2(0.0f, 1.0f));
  m_pd3dDevice->SetTexture(0, m_skyBoxTextures[5]);


  // Reset the rendering state
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  m_worldMatrixStack->Pop();

  // Reset the world transform
  SetMatrices();
}




//-----------------------------------------------------------------------------
// 
// Render the surface of the water. This uses the refraction and reflection
// textures that have already been rendered for this frame.
//
//-----------------------------------------------------------------------------
void Water::RenderWater(void)
{
  HRESULT hr;

  const float etaWater = 1.33f;    // Index of refraction of water
  const float etaAir   = 1.00f;    // Index of refraction of air
  float etaRatio;                  // Ratio of indices of refraction of two media

  // Set up the vertex shader texture
  CopySimulationTexture(m_waveSimulationTexture[(m_waveSimulationIndex + 0) % 3], m_backBufferRenderTarget);

  hr = m_pd3dDevice->SetTexture(D3DVERTEXTEXTURESAMPLER0, m_waveSimulationVertexShaderTexture);
  m_pd3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
  m_pd3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


  // Set up the vertex shader
  m_pd3dDevice->SetVertexShader(m_waveRenderingVertexShader);
  SetMatrices();

  // Set up the pixel shader
  if(m_renderRefractionOnly)
    {
      m_pd3dDevice->SetPixelShader(m_waveRenderingRefractionPixelShader);
    }
  else if(m_renderReflectionOnly)
    {
      m_pd3dDevice->SetPixelShader(m_waveRenderingReflectionPixelShader);
    }
  else if(m_renderFresnelOnly)
    {
      m_pd3dDevice->SetPixelShader(m_waveRenderingFresnelPixelShader);
    }
  else if(m_renderNormalsOnly)
    {
      m_pd3dDevice->SetPixelShader(m_waveRenderingNormalsPixelShader);
    }
  else
    {
      m_pd3dDevice->SetPixelShader(m_waveRenderingPixelShader);
    }

  // Set up coefficients that depend on whether the viewer is above or below the water
  if(m_underwater)
    {
      etaRatio = etaWater / etaAir;

      m_pd3dDevice->SetVertexShaderConstantF(VS_ETA_RATIO, (const float*)&D3DXVECTOR4(etaRatio, etaRatio * etaRatio, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetVertexShaderConstantF(VS_NORMAL_SIGN, (const float*)&D3DXVECTOR4(-1.0f, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_FRESNEL_R0, (const float*)&D3DXVECTOR4(0.20f, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_UNDER_WATER, (const float*)&D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_TOTAL_INTERNAL_REFLECTION_SLOPE_BIAS, (const float*)&D3DXVECTOR4(5.0f, 0.65f, 0.65f, 0.0f), 1);
    }
  else
    {
      etaRatio = etaAir / etaWater;

      m_pd3dDevice->SetVertexShaderConstantF(VS_ETA_RATIO, (const float*)&D3DXVECTOR4(etaRatio, etaRatio * etaRatio, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetVertexShaderConstantF(VS_NORMAL_SIGN, (const float*)&D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_FRESNEL_R0, (const float*)&D3DXVECTOR4(0.0977f, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_UNDER_WATER, (const float*)&D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
      m_pd3dDevice->SetPixelShaderConstantF(PS_TOTAL_INTERNAL_REFLECTION_SLOPE_BIAS, (const float*)&D3DXVECTOR4(1.0f, 0.0f, 1.0f, 0.0f), 1);
    }

  // Set the eye position in world space
  m_pd3dDevice->SetPixelShaderConstantF(PS_EYE_POSITION_WORLD, (const float*)(&(m_viewer.getPosition())), 1);

  // Set the refraction map overdraw
  m_pd3dDevice->SetVertexShaderConstantF(VS_REFRACTION_TEXCOORD_SCALE_OFFSET, 
                                         (const float*)&D3DXVECTOR4((1.0f /  m_refractionMapOverdraw), (1.0f / m_refractionMapOverdraw),
                                                                    .5f * (1.0f - (1.0f / m_refractionMapOverdraw)),
                                                                    .5f * (1.0f - (1.0f / m_refractionMapOverdraw))), 1);
  m_pd3dDevice->SetVertexShaderConstantF(VS_REFLECTION_TEXCOORD_SCALE_OFFSET, 
                                         (const float*)&D3DXVECTOR4((1.0f /  m_reflectionMapOverdraw), 
                                                                    (1.0f / m_reflectionMapOverdraw),
                                                                    .5f * (1.0f - (1.0f / m_reflectionMapOverdraw)),
                                                                    .5f * (1.0f - (1.0f / m_reflectionMapOverdraw))), 1);



  // Set up the pixel shader textures
  m_pd3dDevice->SetTexture(0, m_waveRefractionTexture);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetTexture(1, m_waveReflectionTexture);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetTexture(2, m_waveRefractionNearTexture);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetTexture(3, m_waveReflectionNearTexture);
  m_pd3dDevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
  m_pd3dDevice->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
  hr = m_pd3dDevice->SetTexture(4, m_fadeTexture);
  assert(hr == D3D_OK);
  m_pd3dDevice->SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(4, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  if(m_renderWireframeWater)
    {
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }

  // Set renderstates
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  // Set up vertices, indices and render
  m_pd3dDevice->SetStreamSource(0, m_waveSimulationVertexBuffer, 0, sizeof(Vertex));
  m_pd3dDevice->SetIndices(m_waveSimulationIndexBuffer);
  m_pd3dDevice->SetFVF(Vertex::FVF);
  m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_waveSimulationGrid.size(), 0, m_waveSimulationGrid.primitiveCount());

  // Reset state
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetTexture(1, NULL);
  m_pd3dDevice->SetTexture(2, NULL);
  m_pd3dDevice->SetTexture(3, NULL);
  m_pd3dDevice->SetTexture(4, NULL);
  m_pd3dDevice->SetVertexShader(NULL);
  m_pd3dDevice->SetPixelShader(NULL);
  m_pd3dDevice->SetTexture(D3DVERTEXTEXTURESAMPLER0, NULL);
  if(m_renderWireframeWater)
    {
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
}


//-----------------------------------------------------------------------------
// 
// Restore all of the objects related to rendering the water
// This is called by RestoreDeviceObjects()
//
//-----------------------------------------------------------------------------
HRESULT Water::RestoreWaterRenderingObjects(void)
{
  HRESULT hr;
  
  // Create the wave reflection and refraction textures for objects
  // that do not penetrate the surface of the water.
  hr = m_pd3dDevice->CreateTexture(min((int)m_deviceWidth, m_reflectionTextureWidth), 
                                   min((int)m_deviceHeight, m_reflectionTextureHeight), 
                                   1, D3DUSAGE_RENDERTARGET, 
                                   D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveReflectionTexture, NULL);
  CHECK_RETURN_CODE(L"CreateTexture m_waveReflectionTexture", hr);  

  hr = m_waveReflectionTexture->GetSurfaceLevel(0, &m_waveReflectionSurface);
  CHECK_RETURN_CODE(L"GetSurfaceLevel m_waveReflectionSurface", hr);
  
  hr = m_pd3dDevice->CreateTexture(min((int)m_deviceWidth, m_refractionTextureWidth), 
                                   min((int)m_deviceHeight, m_refractionTextureHeight), 
                                   1, D3DUSAGE_RENDERTARGET, 
                                   D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveRefractionTexture, NULL);
  CHECK_RETURN_CODE(L"CreateTexture m_waveRefractionTexture", hr);

  hr = m_waveRefractionTexture->GetSurfaceLevel(0, &m_waveRefractionSurface);
  CHECK_RETURN_CODE(L"GetSurfaceLevel m_waveRefractionSurface", hr);

  
  // Create the second set of refraction and reflection render targets for objects
  // that are penetrating the surface of the water.
  hr = m_pd3dDevice->CreateTexture(min((int)m_deviceWidth, m_reflectionTextureWidth), 
                                   min((int)m_deviceHeight, m_reflectionTextureHeight), 
                                   1, D3DUSAGE_RENDERTARGET, 
                                   D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveReflectionNearTexture, NULL);
  CHECK_RETURN_CODE(L"CreateTexture m_waveReflectionNearTexture", hr);  

  hr = m_waveReflectionNearTexture->GetSurfaceLevel(0, &m_waveReflectionNearSurface);
  CHECK_RETURN_CODE(L"GetSurfaceLevel m_waveReflectionNearSurface", hr);
  
  hr = m_pd3dDevice->CreateTexture(min((int)m_deviceWidth, m_refractionTextureWidth), 
                                   min((int)m_deviceHeight, m_refractionTextureHeight), 
                                   1, D3DUSAGE_RENDERTARGET, 
                                   D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveRefractionNearTexture, NULL);
  CHECK_RETURN_CODE(L"CreateTexture m_waveRefractionNearTexture", hr);

  hr = m_waveRefractionNearTexture->GetSurfaceLevel(0, &m_waveRefractionNearSurface);
  CHECK_RETURN_CODE(L"GetSurfaceLevel m_waveRefractionNearSurface", hr);

  return(D3D_OK);
}


//-----------------------------------------------------------------------------
// 
// Restore all of the objects related to simulating the water
// This is called by RestoreDeviceObjects()
//
//-----------------------------------------------------------------------------
HRESULT Water::RestoreWaterSimulationObjects(void)
{
  int         i;
  HRESULT     hr;
  std::string fullpath;


  // Create the texture/render targets where the wave simulation takes place
  for(i=0; i<3; i++)
    {
      // Create the texture
      hr = D3DXCreateTexture(m_pd3dDevice, m_waveSimulationWidth, m_waveSimulationHeight, 1, D3DUSAGE_RENDERTARGET, 
                             D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_waveSimulationTexture[i]);
      assert(hr == D3D_OK);

      // Get the surface from the texture so we can use it as a render target
      hr = m_waveSimulationTexture[i]->GetSurfaceLevel(0, &m_waveSimulationSurface[i]);
      assert(hr == D3D_OK);

      // Clear the surface
      m_pd3dDevice->SetRenderTarget(0, m_waveSimulationSurface[i]);
      m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);
      m_pd3dDevice->SetRenderTarget(0, m_backBufferRenderTarget);
    }


  // Create the wave simulation texture and surface for the vertex shader
  hr = D3DXCreateTexture(m_pd3dDevice, m_waveSimulationWidth, m_waveSimulationHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, 
                         &m_waveSimulationVertexShaderTexture);
  assert(hr == D3D_OK);

  hr = m_waveSimulationVertexShaderTexture->GetSurfaceLevel(0, &m_waveSimulationVertexShaderSurface);
  assert(hr == D3D_OK);
  

  // Load the simulation dampening texture from a file
  FindFile("dampening.tga", fullpath);
  hr = D3DXCreateTextureFromFileEx(m_pd3dDevice, toUnicode(fullpath.c_str()), m_waveSimulationWidth, m_waveSimulationHeight, 1, 0, 
                                   D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, NULL, NULL, &m_waveSimulationDampeningTexture);
  assert(hr == D3D_OK);

  return(D3D_OK);
}



//-----------------------------------------------------------------------------
// 
// Setup up the transforms for fixed function.
// Setup constants for vertex shaders.        
//
//-----------------------------------------------------------------------------
void Water::SetMatrices(void)
{
  D3DXMATRIX worldViewMatrix;
  D3DXMATRIX worldInverseMatrix;
  D3DXMATRIX worldInverseTransposeMatrix;
  D3DXMATRIX viewMatrix;
  D3DXMATRIX viewProjectionMatrix;
  D3DXMATRIX worldViewProjectionMatrix;

  // Reflect the view matrix if asked
  if(m_reflectViewMatrix)
    {
      D3DXMATRIX reflectionMatrix;

      D3DXMatrixReflect(&reflectionMatrix, &D3DXPLANE(0.0f, 1.0f, 0.0f, 0.0f));

      viewMatrix = reflectionMatrix * (*m_viewMatrixStack->GetTop());
    }
  else
    {
      viewMatrix = (*m_viewMatrixStack->GetTop());
    }

  // Compute various matrices
  worldViewMatrix = (*m_worldMatrixStack->GetTop()) * viewMatrix;
  worldViewProjectionMatrix = (*m_worldMatrixStack->GetTop()) * viewMatrix * (*m_projectionMatrixStack->GetTop());
  viewProjectionMatrix = viewMatrix * (*m_projectionMatrixStack->GetTop());
  D3DXMatrixInverse(&worldInverseMatrix, NULL, m_worldMatrixStack->GetTop());
  D3DXMatrixTranspose(&worldInverseTransposeMatrix, &worldInverseMatrix);

    
  // Set the matrices for fixed function
  m_pd3dDevice->SetTransform(D3DTS_WORLD, m_worldMatrixStack->GetTop());
  m_pd3dDevice->SetTransform(D3DTS_VIEW, &viewMatrix);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m_projectionMatrixStack->GetTop());


  // Set the vertex shader constants for the matrices
  SetMatrix(VS_WORLD_MATRIX, (*m_worldMatrixStack->GetTop()));
  SetMatrix(VS_WORLD_INVERSE_TRANSPOSE_MATRIX, worldInverseTransposeMatrix);
  SetMatrix(VS_WORLD_VIEW_MATRIX, worldViewMatrix);
  SetMatrix(VS_WORLD_VIEW_PROJECTION_MATRIX, worldViewProjectionMatrix);
  SetMatrix(VS_PROJECTION_MATRIX, (*m_projectionMatrixStack->GetTop()));
  SetMatrix(VS_VIEW_PROJECTION_MATRIX, viewProjectionMatrix); 

  // Compute _worldViewProjectionInverseTransposeMatrix so that we can transform clip planes for 
  // geometry that is rendered without the fixed function pipeline
  D3DXMatrixInverse(&m_worldViewProjectionInverseTransposeMatrix, NULL, &worldViewProjectionMatrix);
  D3DXMatrixTranspose(&m_worldViewProjectionInverseTransposeMatrix, &m_worldViewProjectionInverseTransposeMatrix);

  // Set up some vertex shader constants
  m_pd3dDevice->SetVertexShaderConstantF(VS_LIGHT_POSITION_WORLD, (float*)&m_lightPositionWorld, 1);
  m_pd3dDevice->SetVertexShaderConstantF(VS_EYE_POSITION_WORLD, (float*)&m_viewer.getPosition(), 1);
  m_pd3dDevice->SetVertexShaderConstantF(VS_LIGHT_DIFFUSE, (float*)&D3DXVECTOR4(0.7f, 0.7f, 0.7f, 1.0f), 1);
  m_pd3dDevice->SetVertexShaderConstantF(VS_LIGHT_AMBIENT, (float*)&D3DXVECTOR4(0.7f, 0.7f, 0.7f, 1.0f), 1);
  
}




//-----------------------------------------------------------------------------
// 
// Setup the transpose of a matrix in the vertex shader constants
//
//-----------------------------------------------------------------------------
void Water::SetMatrix(int constantIndex, const D3DXMATRIX& matrix)
{
  assert(constantIndex >= 0);

  D3DXMATRIX transposeMatrix;

  D3DXMatrixTranspose(&transposeMatrix, &matrix);

  m_pd3dDevice->SetVertexShaderConstantF(constantIndex, (float*)&transposeMatrix, 4);
}




//-----------------------------------------------------------------------------
// 
// Do the simulation of the wave equation by rendering to an off-screen
// render target using the simulation pixel shader. Also, render pertubations
// in the simulation to account for things in the water.
//
//-----------------------------------------------------------------------------
void Water::SimulateWaveEquation(void)
{
  HRESULT hr;
  float   hackFactor;
  float   x, y;
  float   u[2];
  float   v[2];
  float   wakeTime;

  // Check to see if simulation is frozen
  if(m_freezeSimulation)
    {
      m_tLastFrame = -1.0f;
      return;
    }
      
  // Set up the samplers. We need to use the previous two simulation results
  // to compute the new simulation result.
  m_pd3dDevice->SetTexture(0, m_waveSimulationTexture[(m_waveSimulationIndex + 1) % 3]);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetTexture(1, m_waveSimulationTexture[(m_waveSimulationIndex + 2) % 3]);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  // There is a dampening texture to allow an artist to control how much the
  // water is dampened in areas. This is mostly useful near the shore.
  m_pd3dDevice->SetTexture(2, m_waveSimulationDampeningTexture);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


  // This is the simulation result we're going to compute
  m_pd3dDevice->SetRenderTarget(0, m_waveSimulationSurface[(m_waveSimulationIndex + 0) % 3]);
  m_pd3dDevice->SetVertexShader(m_waveSimulationVertexShader);
  m_pd3dDevice->SetPixelShader(m_waveSimulationPixelShader);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

  // Set the constants
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_TEX_COORD_DELTA_X0Y1, 
                                        (const float*)&D3DXVECTOR4(-1.0f / m_waveSimulationWidth,  0.0f / m_waveSimulationHeight, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_TEX_COORD_DELTA_X2Y1, 
                                        (const float*)&D3DXVECTOR4( 1.0f / m_waveSimulationWidth,  0.0f / m_waveSimulationHeight, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_TEX_COORD_DELTA_X1Y0, 
                                        (const float*)&D3DXVECTOR4( 0.0f / m_waveSimulationWidth, -1.0f / m_waveSimulationHeight, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_TEX_COORD_DELTA_X1Y2, 
                                        (const float*)&D3DXVECTOR4( 0.0f / m_waveSimulationWidth,  1.0f / m_waveSimulationHeight, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_POSITION_WEIGHTING, 
                                        (const float*)&D3DXVECTOR4(1.99f, 0.99f, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_WAVE_SPEED_SQUARED, 
                                        (const float*)&D3DXVECTOR4(10.0f, 0.0f, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_ONE_HALF_TIMES_DELTA_TIME_SQUARED, 
                                        (const float*)&D3DXVECTOR4(.01f, 0.0f, 0.0f, 0.0f), 1);

  // The hackFactor is just an empirically derived value that makes the water look
  // ok for the given average framerate. Currently, the simulation is not very
  // sophisticated and runs on a fixed timestep. If the framerate increases, the
  // simulation rate increases. This is clearly not the right thing to do, but
  // it is illustrative and simpler to understand. This factor was also selected for
  // stability.
  hackFactor = 15.0f;                      
  
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_ONE_HALF_TIMES_DELTA_TIME_SQUARED, 
                                        (const float*)&D3DXVECTOR4(hackFactor * m_dt * m_dt, 0.0f, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_ONE_HALF_TIMES_DELTA_TIME_SQUARED, 
                                        (const float*)&D3DXVECTOR4(0.001f, 0.0f, 0.0f, 0.0f), 1);
  m_pd3dDevice->SetPixelShaderConstantF(PS_SIMULATION_GRID_SIZE, 
                                        (const float*)&D3DXVECTOR4(m_landScale[0] / m_waveSimulationWidth, 
                                                                   0.0f,
                                                                   m_landScale[1] / m_waveSimulationHeight, 
                                                                   0.0f), 1);


  // Draw a quad to do the simulation; this is where the work gets done
  srand(10);
  drawQuad(m_pd3dDevice, 
           D3DXVECTOR3(-1.0f,  1.0f, 0.0f), randomColor(), D3DXVECTOR2(0.0f, 0.0f),
           D3DXVECTOR3( 1.0f,  1.0f, 0.0f), randomColor(), D3DXVECTOR2(1.0f, 0.0f),
           D3DXVECTOR3( 1.0f, -1.0f, 0.0f), randomColor(), D3DXVECTOR2(1.0f, 1.0f),
           D3DXVECTOR3(-1.0f, -1.0f, 0.0f), randomColor(), D3DXVECTOR2(0.0f, 1.0f));

      
  // Be nice and reset state for the simulation
  m_pd3dDevice->SetVertexShader(NULL);
  m_pd3dDevice->SetPixelShader(NULL);
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetTexture(1, NULL);
  m_pd3dDevice->SetTexture(2, NULL);


  //
  // Render perturbations into the simulation texture
  // This is used to put the wake from the boat and the water ghost
  // into the water.
  //
  if(m_stopPertubingWater == false)
    {
      // Calculate some hackety hack hack animation
      wakeTime = m_time - .55f;

      x = 0.5f * sin(wakeTime);
      y = -0.5f * cos(wakeTime);
      v[0] = -0.1f * cos(wakeTime);
      v[1] = -0.1f * sin(wakeTime);
      u[0] = -0.1f * -sin(wakeTime);
      u[1] = -0.1f * cos(wakeTime);

      
      // Set up the texture for the wake
      m_pd3dDevice->SetTexture(0, m_wakeTexture);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
      m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

      // Set up to blend the wake directly into the floating point simulation texture
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      
      m_pd3dDevice->SetVertexShader(m_waveSimulationPerturbationVertexShader);
      m_pd3dDevice->SetPixelShader(m_waveSimulationPerturbationPixelShader);

      m_pd3dDevice->SetPixelShaderConstantF(PS_DELTA_TIME, (float*)&D3DXVECTOR4(m_dt, 0.0f, 0.0f, 0.0f), 1);

      // Draw the wake into the simulation texture
      drawQuad(m_pd3dDevice, 
               D3DXVECTOR3(x - u[0] - v[0], y - u[1] - v[1], 0.0f), D3DCOLOR_RGBA(1, 0, 0, 0), D3DXVECTOR2(0.0f, 0.0f),
               D3DXVECTOR3(x + u[0] - v[0], y + u[1] - v[1], 0.0f), D3DCOLOR_RGBA(1, 0, 0, 0), D3DXVECTOR2(1.0f, 0.0f),
               D3DXVECTOR3(x + u[0] + v[0], y + u[1] + v[1], 0.0f), D3DCOLOR_RGBA(1, 0, 0, 0), D3DXVECTOR2(1.0f, 1.0f),
               D3DXVECTOR3(x - u[0] + v[0], y - u[1] + v[1], 0.0f), D3DCOLOR_RGBA(1, 0, 0, 0), D3DXVECTOR2(0.0f, 1.0f));


      m_pd3dDevice->SetTexture(0, NULL);
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
      m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

      m_pd3dDevice->SetPixelShader(NULL);


      // The water ghost is a goofy thing that bounces around the perturbing the water surface.
      // This is just done to make the water look more lively than a sheet of glass.
      if(m_enableWaterGhost)
        {
          float ghostTime;

          ghostTime = m_time * 1.5f;

          x = 0.5f * sin(ghostTime);
          y = -0.5f * cos(0.7f * ghostTime);
          v[0] = -0.05f * cos(ghostTime);
          v[1] = -0.05f * sin(ghostTime);
          u[0] = -0.1f * -sin(ghostTime);
          u[1] = -0.1f * cos(ghostTime);
          drawQuad(m_pd3dDevice, 
                   D3DXVECTOR3(x - u[0] - v[0], y - u[1] - v[1], 0.0f), D3DCOLOR_RGBA(128, 0, 255, 0), D3DXVECTOR2(0.0f, 0.0f),
                   D3DXVECTOR3(x + u[0] - v[0], y + u[1] - v[1], 0.0f), D3DCOLOR_RGBA(128, 0, 255, 0), D3DXVECTOR2(1.0f, 0.0f),
                   D3DXVECTOR3(x + u[0] + v[0], y + u[1] + v[1], 0.0f), D3DCOLOR_RGBA(128, 0, 255, 0), D3DXVECTOR2(1.0f, 1.0f),
                   D3DXVECTOR3(x - u[0] + v[0], y - u[1] + v[1], 0.0f), D3DCOLOR_RGBA(128, 0, 255, 0), D3DXVECTOR2(0.0f, 1.0f));
        }

      m_pd3dDevice->SetVertexShader(NULL);
    }

  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  hr = m_pd3dDevice->SetRenderTarget(0, m_backBufferRenderTarget);
}










////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//               9.0c sample framework methods
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//--------------------------------------------------------------------------------------
bool Water::IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed)
{
  // Need Shader Model 3.0 support. There isn't currently a fallback to software vertex processing
  // for lower shader models.
  if((pCaps->VertexShaderVersion < D3DVS_VERSION(3, 0)) || (pCaps->PixelShaderVersion < D3DPS_VERSION(3, 0)))
    {
      return(false);
    }

  return(true);
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool Water::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps)
{
    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT Water::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
  HRESULT     hr;
  HDC         hDC;
  int         fontHeight;
  D3DXMATRIX  sunkenBoatMatrix;
  float       scale;
  D3DXVECTOR3 viewerPosition;
  std::string fullpath;

  V_RETURN( m_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
  V_RETURN( m_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

  // Record the device's pointer
  m_pd3dDevice = pd3dDevice;
  gDirect3DDevice9 = m_pd3dDevice;  

  // Record the width and height
  m_deviceWidth = pBackBufferSurfaceDesc->Width;
  m_deviceHeight = pBackBufferSurfaceDesc->Height;

  // Load the heightmap
  m_landScale[0] = 50.0f;
  m_landScale[1] = 20.0f;
  m_landScale[2] = 50.0f;
  FindFile("ground.tga", fullpath);
  m_land = new HeightMap(fullpath.c_str(), D3DXVECTOR3(m_landScale[0], m_landScale[1], m_landScale[2]), D3DXVECTOR3(0.0f, 0.0f, 0.0f));

  // Create the vertex buffer and index buffer for the heightmap
  m_land->createVertexBuffer(m_pd3dDevice);
  m_land->createIndexBuffer(m_pd3dDevice);

  // Create a texture out of the heightmap
  m_land->createHeightTexture(m_pd3dDevice, m_landTexture);
  m_land->setTexture(LoadTexture("groundColor.tga"));

  // Initialize the viewer
  viewerPosition = D3DXVECTOR3(5.0f, 5.0f, 5.0f);
  m_viewer.set(viewerPosition, 1.3f * D3DX_PI, 0.0f);

  // Initialize the font
  hDC = GetDC(NULL);
  fontHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
  ReleaseDC(NULL, hDC);
  if(FAILED(hr = D3DXCreateFont(m_pd3dDevice, fontHeight, 0, FW_BOLD, 0, FALSE, 
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                                TEXT("Arial"), &m_font)))
    {
      return(DXTRACE_ERR(L"D3DXCreateFont", hr));
    }
  
  // Initialize all of the vertex and pixel shaders
  InitShaders();
                    
  // Set the light position
  m_lightPositionWorld = D3DXVECTOR3(25.0f, 25.0f, 25.0f);


  // Create the vertex buffer and index buffer for the wave simulation
  m_waveSimulationGrid.resize(m_waveSimulationWidth, m_waveSimulationHeight);
  m_waveSimulationGrid.clear(0.0f);

  assert(m_land != NULL);
  m_waveSimulationGrid.createVertexAndIndexBuffers(m_pd3dDevice, m_waveSimulationVertexBuffer, m_waveSimulationIndexBuffer, 
                                                   D3DXVECTOR3(25.0f, 10.0f, 25.0f), D3DXVECTOR3(25.0f, 0.0f, 25.0f), m_land);


  // Load texture to perturb the wave simulation
  m_wakeTexture = LoadTexture("brush.tga");
    
  // Load the skybox textures
  m_skyBoxTextures[0] = LoadTexture("CloudyHills_negz.tga");
  m_skyBoxTextures[1] = LoadTexture("CloudyHills_negx.tga");
  m_skyBoxTextures[2] = LoadTexture("CloudyHills_posz.tga");
  m_skyBoxTextures[3] = LoadTexture("CloudyHills_posx.tga");
  m_skyBoxTextures[4] = LoadTexture("CloudyHills_posy.tga");
  m_skyBoxTextures[5] = LoadTexture("groundColor.tga");
  
  // Load the texture used for fading
  m_fadeTexture = LoadTexture("fade.tga");

  // Load the boat mesh
  FindFile("boat4.x", fullpath);
  m_boatMesh.initialize(fullpath.c_str(), m_pd3dDevice);

  // Load the sunken boat mesh
  m_sunkenBoatMesh.initialize(fullpath.c_str(), m_pd3dDevice);
  scale = 1.0f / 3.5f;
  sunkenBoatMatrix._11 = 0.5f * scale; sunkenBoatMatrix._12 = 0.3f  * scale; sunkenBoatMatrix._13 = 0.0f * scale; sunkenBoatMatrix._14 = 0.0f;
  sunkenBoatMatrix._21 = 0.3f * scale; sunkenBoatMatrix._22 = -0.5f * scale; sunkenBoatMatrix._23 = 0.0f * scale; sunkenBoatMatrix._24 = 0.0f;
  sunkenBoatMatrix._31 = 0.0f * scale; sunkenBoatMatrix._32 = 0.0f  * scale; sunkenBoatMatrix._33 = 0.5f * scale; sunkenBoatMatrix._34 = 0.0f;
  sunkenBoatMatrix._41 = 25.0f;        sunkenBoatMatrix._42 =-5.5f;          sunkenBoatMatrix._43 = 25.0f; sunkenBoatMatrix._44 = 1.0f;
  m_sunkenBoatMesh.setWorldMatrix(sunkenBoatMatrix);

  // Load the boat texture
  m_boatTexture = LoadTexture("boat4.dds");

  // Create matrix stacks
  D3DXCreateMatrixStack(0, &m_worldMatrixStack);
  D3DXCreateMatrixStack(0, &m_viewMatrixStack);
  D3DXCreateMatrixStack(0, &m_projectionMatrixStack);

  // By default, we don't want to reflect the view transform
  m_reflectViewMatrix = false;

  return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT Water::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
  HRESULT    hr;
  D3DXMATRIX viewMatrix;

  V_RETURN( m_DialogResourceManager.OnD3D9ResetDevice() );
  V_RETURN( m_SettingsDlg.OnD3D9ResetDevice() );

  // Record the device's pointer
  m_pd3dDevice = pd3dDevice;
  gDirect3DDevice9 = m_pd3dDevice;

  // Record the width and height
  m_deviceWidth = pBackBufferSurfaceDesc->Width;
  m_deviceHeight = pBackBufferSurfaceDesc->Height;

  // Reset some basic state
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

  // Compute the aspect ratio
  m_aspectRatio = ((float)m_deviceWidth) / ((float)m_deviceHeight);

  // Load the world matrix with the identity matrix
  m_worldMatrixStack->LoadIdentity();

  // Set the view matrix
  m_viewer.computeViewMatrix((*m_viewMatrixStack->GetTop()));

  // Set the projection matrix
  D3DXMatrixPerspectiveFovRH(&m_projectionMatrix, m_fieldOfView, m_aspectRatio, 1.0f, 100.0f);
  D3DXMatrixPerspectiveFovRH(&m_reflectionProjectionMatrix, m_fieldOfView, m_aspectRatio, 1.0f, 100.0f);
  D3DXMatrixPerspectiveFovRH(&m_refractionProjectionMatrix, m_fieldOfView, m_aspectRatio, 1.0f, 100.0f);
  m_projectionMatrixStack->LoadMatrix(&m_projectionMatrix);
  
  // Set the fixed function transform and vertex shader constants
  SetMatrices();

  // Get a surface for the backbuffer
  hr = m_pd3dDevice->GetRenderTarget(0, &m_backBufferRenderTarget);
  assert(hr == D3D_OK);
  assert(m_backBufferRenderTarget != NULL);

  // Create a render target for rendering the world when underwater
  hr = D3DXCreateTexture(m_pd3dDevice, m_deviceWidth, m_deviceHeight, 1, 
                         D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_underwaterWorldRenderTargetTexture);
  assert(hr == D3D_OK);

  hr = m_underwaterWorldRenderTargetTexture->GetSurfaceLevel(0, &m_underwaterWorldRenderTargetSurface);
  assert(hr == D3D_OK);  

  // Reset the font
  if(m_font)
    {
      m_font->OnResetDevice();
    }

  // Create the vertex buffer for drawing quads
  hr = m_pd3dDevice->CreateVertexBuffer(4 * sizeof(Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
                                        Vertex::FVF, D3DPOOL_DEFAULT, &m_quadVertexBuffer, NULL);
  assert(hr == D3D_OK);

  // Resize and position the GUI appropriately
  m_HUD.SetLocation(m_deviceWidth - 170, 0);
  m_HUD.SetSize(170, 170);

  m_SampleUI.SetLocation(m_deviceWidth - 160, m_deviceHeight - 300);
  m_SampleUI.SetSize(170, 300);

  // Restore all of the stuff related to the water simulation and rendering
  RestoreWaterSimulationObjects();
  RestoreWaterRenderingObjects();

  return(S_OK);
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void Water::OnFrameMove(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime)
{
  D3DXVECTOR3 position;
  float       height;
  D3DXMATRIX  boatMatrix;
  float       x[3], y[3], z[3];
  float       positionX, positionY, positionZ;
  float       boatScale[3] = {-0.5f / 3.5f, 0.8f / 3.5f, 0.5f / 3.5f};


  // Compute the delta time
  if(m_tLastFrame >= 0.0f)
    {
      m_dt = (float)(time - (double)m_tLastFrame);
      m_time += m_dt;
    }
  m_tLastFrame = (float)time;


  // Handle viewer movement
  if(KeyIsDown('W') || KeyIsDown(VK_UP))
    {
      m_viewer.moveForward(0.1f);        
    }
  if(KeyIsDown('S') || KeyIsDown(VK_DOWN))
    {
      m_viewer.moveForward(-0.1f);        
    }
  if(KeyIsDown('A') || KeyIsDown(VK_LEFT))
    {
      m_viewer.moveRight(-0.1f);
    }
  if(KeyIsDown('D') || KeyIsDown(VK_RIGHT))
    {
      m_viewer.moveRight(0.1f);
    }
  if(KeyIsDown('E'))
    {
      m_height += 0.1f;
    }
  if(KeyIsDown('Q'))
    {
      m_height -= 0.1f;
    }

  // Stand the viewer on top of the terrain
  if(m_walkOnGround)
    {
      position = m_viewer.getPosition();
      height = position.y = m_land->height(position.x, position.z) + 2.0f;
    }
  else
    {
      position = m_viewer.getPosition();
      height = position.y = m_height;
    }

  // Force the view to stay in the world
  if(position.x < 2.5f)
    {
      position.x = 2.5f;
    }
  else if(position.x >= 47.5f)
    {
      position.x = 47.5f;
    }
  if(position.z < 2.5f)
    {
      position.z = 2.5f;
    }
  else if(position.z >= 47.5f)
    {
      position.z = 47.5f;
    }
  m_viewer.setPosition(position);


  // Compute and setup transforms
  m_viewer.computeViewMatrix((*m_viewMatrixStack->GetTop()));
  SetMatrices();

  // Check to see if the viewer is under water
  if(height < 0.0f)
    {
      m_underwater = true;
    }
  else
    {
      m_underwater = false;
    }


  // Move the boat
  x[0] = boatScale[0] * cos(m_time + 1.57f);
  x[1] = boatScale[0] * 0.0f;
  x[2] = boatScale[0] * sin(m_time + 1.57f);
  y[0] = boatScale[1] * 0.0f;
  y[1] = boatScale[1];
  y[2] = boatScale[1] * 0.0f;
  z[0] = boatScale[2] * -sin(m_time + 1.57f);
  z[1] = boatScale[2] * 0.0f;
  z[2] = boatScale[2] * cos(m_time + 1.57f);
  positionX = 25.0f + 12.5f * cos(m_time - 1.57f);
  positionY = 0.3f + 0.2f * cos(3.0f * m_time);
  positionZ = 25.0f + 12.5f * sin(m_time - 1.57f);

  boatMatrix = D3DXMATRIX(x[0],  x[1],  x[2],  0.0f,
                          y[0],  y[1],  y[2],  0.0f,
                          z[0],  z[1],  z[2],  0.0f,
                          positionX, positionY, positionZ, 1.0f);

  m_boatMesh.setWorldMatrix(boatMatrix);
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void Water::OnFrameRender(IDirect3DDevice9* pd3dDevice, double time, float elapsedTime)
{
  // If the settings dialog is being shown, then
  // render it instead of rendering the app's scene
  if( m_SettingsDlg.IsActive() )
  {
    m_SettingsDlg.OnRender( elapsedTime );
    return;
  }

  // Begin the scene
  if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
      // Do the wave equation simulation for the water
      SimulateWaveEquation();

      // Render the scene into a refraction texture
      RenderReflectionTexture();

      // Render the scene into a reflection texture
      RenderRefractionTexture();

      // Render the scene to the backbuffer
      RenderBackbuffer();

      // Render the surface of the water to the backbuffer
      RenderWater();

      // render informative junk
      RenderInformation();

      // Render the GUI
      if(m_renderGUI)
        {
          m_HUD.OnRender(elapsedTime);
          m_SampleUI.OnRender(elapsedTime);
        }
      
      // Now that we're done, go home and chug a 40oz.
      m_pd3dDevice->EndScene();

      // Do some miscellaneous book-keeping
      if(m_freezeSimulation == false)
        {
          m_waveSimulationIndex++;
        }
    }
}


//--------------------------------------------------------------------------------------
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//--------------------------------------------------------------------------------------
LRESULT Water::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing)
{
  // Always allow dialog resource manager calls to handle global messages
  // so GUI state is updated correctly
  m_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

  if( m_SettingsDlg.IsActive() )
  {
    m_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
    return 0;
  }

  if(m_renderGUI)
    {
      // Give the dialogs a chance to handle the message first
      *pbNoFurtherProcessing = m_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
      if(*pbNoFurtherProcessing)
        {
          return(0);
        }

      *pbNoFurtherProcessing = m_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
      if(*pbNoFurtherProcessing)
        {
          return(0);
        }
    }

  switch(uMsg)
    {
    case WM_PAINT:
      {
        if (m_loadingApp)
          {
            // Draw on the window tell the user that the app is loading
            HDC hDC = GetDC(hWnd);
            TCHAR strMsg[MAX_PATH];
            wsprintf(strMsg, TEXT("Loading... Please wait"));

            RECT rct;
            GetClientRect(hWnd, &rct);

            DrawText(hDC, strMsg, -1, &rct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            ReleaseDC(hWnd, hDC);
          }
        break;
      }
    case WM_MOUSEMOVE:
      {
        static POINT lastMouse;
        static bool  lastMouseValid = false;

        if(m_mouseIsClipped)
          {
            POINT mouse;
            POINT center;
            int deltaX, deltaY;

            mouse.x = LOWORD(lParam);
            mouse.y = HIWORD(lParam);
            ClientToScreen(hWnd, &mouse);

            center.x = (m_mouseClipRect.left + m_mouseClipRect.right) / 2;
            center.y = (m_mouseClipRect.top + m_mouseClipRect.bottom) / 2;

            deltaX = mouse.x - center.x;
            deltaY = mouse.y - center.y;

            if((deltaX != 0) || (deltaY != 0))
              {
                //dprintf("%d,%d\n", mouse.x - center.x, mouse.y - center.y);
                
                SetCursorPos(center.x, center.y);

                m_viewer.rotateLeftRight((float)-deltaX / 100.0f);
                m_viewer.rotateUpDown((float)deltaY / 100.0f);
              }
          }
        else if(wParam & MK_LBUTTON)
          {

            POINT mouse, delta;

            // Get the mouse coords
            mouse.x = LOWORD(lParam);
            mouse.y = HIWORD(lParam);

            if(lastMouseValid)
              {                
                // Compute the delta
                delta.x = mouse.x - lastMouse.x;
                delta.y = mouse.y - lastMouse.y;
                                
                m_viewer.rotateLeftRight((float)-delta.x / 100.0f);
                m_viewer.rotateUpDown((float)delta.y / 100.0f);
              }

            lastMouse = mouse;
            lastMouseValid = true;
          }
        else
          {
            lastMouseValid = false;
          }

        break;
      }
		case WM_KEYDOWN:
			{
        switch(wParam)
          {
          case VK_ESCAPE:
            {
              PostQuitMessage(0);
              break;
            }
          case VK_F1:
            {
              m_displayHelp = !m_displayHelp;
              break;
            }
          case 'T':
            {
              m_renderText = !m_renderText;
              break;
            }
          case 'G':
            {
              m_renderGUI = !m_renderGUI;
              break;
            }
          case 'B':
            {
              m_renderSunkenBoat = !m_renderSunkenBoat;
              break;
            }
          case 'M':
            {
              RECT  rect;
              POINT topLeft;
              POINT bottomRight;

              if(m_mouseIsClipped == false)
                {
                  GetClientRect(hWnd, &rect);
                  topLeft.x = rect.left;
                  topLeft.y = rect.top;
                  bottomRight.x = rect.right;
                  bottomRight.y = rect.bottom;

                  ClientToScreen(hWnd, &topLeft);
                  ClientToScreen(hWnd, &bottomRight);

                  m_mouseClipRect.left = topLeft.x;
                  m_mouseClipRect.top = topLeft.y;
                  m_mouseClipRect.right = bottomRight.x;
                  m_mouseClipRect.bottom = bottomRight.y;

                  SetCursorPos((m_mouseClipRect.left + m_mouseClipRect.right) / 2, (m_mouseClipRect.top + m_mouseClipRect.bottom) / 2);
            
                  // Clip the cursor
                  ClipCursor(&m_mouseClipRect);
                  ShowCursor(FALSE);
                  m_mouseIsClipped = true;
                }
              else
                {
                  ClipCursor(NULL);
                  ShowCursor(TRUE);
                  m_mouseIsClipped = false;
                }
              break;
            }
          default:
            {
              break;
            }
          }

				break;
			}
    }

  return(0);
}


//--------------------------------------------------------------------------------------
// As a convenience, the sample framework inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void Water::KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown)
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void Water::OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl)
{
  switch(nControlID)
    {
    case IDC_TOGGLE_FULLSCREEN:
      {
        DXUTToggleFullScreen();
        break;
      }
    case IDC_TOGGLE_REF:
      {
        DXUTToggleREF();
        break;
      }
    case IDC_CHANGE_DEVICE:
      {
        m_SettingsDlg.SetActive(!m_SettingsDlg.IsActive());
        break;
      }
    case IDC_ENABLE_SIMULATION:
      {
        m_freezeSimulation = !m_SampleUI.GetCheckBox(IDC_ENABLE_SIMULATION)->GetChecked();
        break;
      }
    case IDC_RENDER_MODE:
      {
        bool* variable;

        m_renderRefractionOnly = false;
        m_renderReflectionOnly = false;
        m_renderFresnelOnly = false;
        m_renderNormalsOnly = false;

        variable = (bool*)m_SampleUI.GetComboBox(IDC_RENDER_MODE)->GetSelectedData();

        if(variable != NULL)
          {
            *variable = true;
          }

        break;
      }
    case IDC_INTERMEDIATE_RENDER_TARGET:
      {
        m_textureOverlayIndex = *((int*)m_SampleUI.GetComboBox(IDC_INTERMEDIATE_RENDER_TARGET)->GetSelectedData());

        break;
      }
    case IDC_ENABLE_PERTURBATION:
      {
        m_stopPertubingWater = !m_SampleUI.GetCheckBox(IDC_ENABLE_PERTURBATION)->GetChecked();
        break;
      }
    case IDC_ENABLE_WATER_GHOST:
      {
        m_enableWaterGhost = m_SampleUI.GetCheckBox(IDC_ENABLE_WATER_GHOST)->GetChecked();
        break;
      }
    case IDC_WALK_ON_GROUND:
      {
        m_walkOnGround = m_SampleUI.GetCheckBox(IDC_WALK_ON_GROUND)->GetChecked();
        break;
      }
    case IDC_WIREFRAME_WATER:
      {
        m_renderWireframeWater = m_SampleUI.GetCheckBox(IDC_WIREFRAME_WATER)->GetChecked();
        break;
      }
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void Water::OnLostDevice(void)
{
  m_DialogResourceManager.OnD3D9LostDevice();
  m_SettingsDlg.OnD3D9LostDevice();

  if(m_font)
    {
      m_font->OnLostDevice();
    }

  SAFE_RELEASE(m_backBufferRenderTarget);
  SAFE_RELEASE(m_underwaterWorldRenderTargetTexture);
  SAFE_RELEASE(m_underwaterWorldRenderTargetSurface);
  SAFE_RELEASE(m_quadVertexBuffer);
  SAFE_RELEASE(m_waveSimulationTexture[0]);
  SAFE_RELEASE(m_waveSimulationTexture[1]);
  SAFE_RELEASE(m_waveSimulationTexture[2]);
  SAFE_RELEASE(m_waveSimulationSurface[0]);
  SAFE_RELEASE(m_waveSimulationSurface[1]);
  SAFE_RELEASE(m_waveSimulationSurface[2]);
  SAFE_RELEASE(m_waveSimulationVertexShaderTexture);
  SAFE_RELEASE(m_waveSimulationVertexShaderSurface);
  SAFE_RELEASE(m_waveSimulationDampeningTexture);
  SAFE_RELEASE(m_waveReflectionTexture);
  SAFE_RELEASE(m_waveRefractionTexture);
  SAFE_RELEASE(m_waveReflectionSurface);
  SAFE_RELEASE(m_waveRefractionSurface);
  SAFE_RELEASE(m_waveReflectionNearTexture);
  SAFE_RELEASE(m_waveRefractionNearTexture);
  SAFE_RELEASE(m_waveReflectionNearSurface);
  SAFE_RELEASE(m_waveRefractionNearSurface);
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void Water::OnDestroyDevice(void)
{
  m_DialogResourceManager.OnD3D9DestroyDevice();
  m_SettingsDlg.OnD3D9DestroyDevice();

  SAFE_RELEASE(m_font);

  delete(m_land);
  m_land = NULL;
  SAFE_RELEASE(m_landTexture);

  // destroy all of the vertex and pixel shaders
  DestroyShaders();

  SAFE_RELEASE(m_waveSimulationVertexBuffer);
  SAFE_RELEASE(m_waveSimulationIndexBuffer);
  SAFE_RELEASE(m_wakeTexture);
  SAFE_RELEASE(m_worldMatrixStack);
  SAFE_RELEASE(m_viewMatrixStack);
  SAFE_RELEASE(m_projectionMatrixStack);
  SAFE_RELEASE(m_skyBoxTextures[0]);
  SAFE_RELEASE(m_skyBoxTextures[1]);
  SAFE_RELEASE(m_skyBoxTextures[2]);
  SAFE_RELEASE(m_skyBoxTextures[3]);
  SAFE_RELEASE(m_skyBoxTextures[4]);
  SAFE_RELEASE(m_skyBoxTextures[5]);
  SAFE_RELEASE(m_fadeTexture);
  m_boatMesh.release();
  m_sunkenBoatMesh.release();
  SAFE_RELEASE(m_boatTexture);
}










////////////////////////////////////////////////////////////////////////////////
//
//                           Global functions
//
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
// 
// print to the debugger
//
//-----------------------------------------------------------------------------
int dprintf(const char* formatString, ...)
{
  va_list argumentList;
  int     returnValue;
  char    outputString[4096];

  va_start(argumentList, formatString);
  returnValue = vsprintf(outputString, formatString, argumentList);
  va_end(argumentList);

  OutputDebugString(toUnicode(outputString));

  return(returnValue);
}


//-----------------------------------------------------------------------------
// 
// print a matrix to the debugger
//
//-----------------------------------------------------------------------------
void dprintf(const D3DXMATRIX& matrix)
{
  dprintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._11, matrix._12, matrix._13, matrix._14);
  dprintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._21, matrix._22, matrix._23, matrix._24);
  dprintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._31, matrix._32, matrix._33, matrix._34);
  dprintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._41, matrix._42, matrix._43, matrix._44);  
}


//-----------------------------------------------------------------------------
// 
// print a matrix to the debugger
//
//-----------------------------------------------------------------------------
void dprintf(const char* description, const D3DXMATRIX& matrix)
{
  assert(description != NULL);

  dprintf("%s\n", description);
  dprintf(matrix);
  dprintf("\n");
}



//-----------------------------------------------------------------------------
// 
// Render a quad
//
//-----------------------------------------------------------------------------
HRESULT drawQuad(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3& position0, D3DCOLOR color0, const D3DXVECTOR2& texCoord0,
                 const D3DXVECTOR3& position1, D3DCOLOR color1, const D3DXVECTOR2& texCoord1,
                 const D3DXVECTOR3& position2, D3DCOLOR color2, const D3DXVECTOR2& texCoord2,
                 const D3DXVECTOR3& position3, D3DCOLOR color3, const D3DXVECTOR2& texCoord3)
{
  Vertex  vertices[4];
  HRESULT hr;
  void*   buffer;

  hr = device->SetFVF(Vertex::FVF);
  assert(hr == D3D_OK);
  
  // Load the vertex data
  vertices[0].position = position0;
  vertices[0].diffuse = color0;
  vertices[0].tex0[0] = texCoord0.x;
  vertices[0].tex0[1] = texCoord0.y;

  vertices[1].position = position1;
  vertices[1].diffuse = color1;
  vertices[1].tex0[0] = texCoord1.x;
  vertices[1].tex0[1] = texCoord1.y;

  vertices[2].position = position2;
  vertices[2].diffuse = color2;
  vertices[2].tex0[0] = texCoord2.x;
  vertices[2].tex0[1] = texCoord2.y;

  vertices[3].position = position3;
  vertices[3].diffuse = color3;
  vertices[3].tex0[0] = texCoord3.x;
  vertices[3].tex0[1] = texCoord3.y;

  // Copy the vertex data into the vertex buffer
  hr = Water::m_quadVertexBuffer->Lock(0, 0, &buffer, D3DLOCK_DISCARD);
  CHECK_RETURN_CODE(L"drawQuad  Water::m_quadVertexBuffer->Lock()", hr);
  
  memcpy(buffer, vertices, 4 * sizeof(Vertex));
  
  Water::m_quadVertexBuffer->Unlock();

  // Render the vertex buffer
  device->SetStreamSource(0, Water::m_quadVertexBuffer, 0, sizeof(Vertex));

  hr = device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
  CHECK_RETURN_CODE(L"drawQuad  DrawPrimitive()", hr);  

  return(hr);
}



//-----------------------------------------------------------------------------
// 
// Print an error message and quit
//
//-----------------------------------------------------------------------------
void fatalError(const char* formatString, ...)
{
  va_list argumentList;
  int     returnValue;
  char    outputString[4096];

  va_start(argumentList, formatString);
  returnValue = vsprintf(outputString, formatString, argumentList);
  va_end(argumentList);

  MessageBox(NULL, toUnicode(outputString), L"Fatal Error!", MB_OK | MB_ICONERROR);

  exit(-1);
}


//-----------------------------------------------------------------------------
// 
// Compute a random color
//
//-----------------------------------------------------------------------------
D3DCOLOR randomColor(void)
{
  return(D3DCOLOR_RGBA(rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, rand() & 0xFF));
}


//--------------------------------------------------------------------------------
//
// Convert a string to unicode in a static buffer
//
//--------------------------------------------------------------------------------
WCHAR* toUnicode(const char* string)
{
  assert(string != NULL);
  
  static WCHAR buffer[4096];
  assert(strlen(string) + 1 < (sizeof(buffer) / sizeof(buffer[0])));

  // Convert string to unicode
  MultiByteToWideChar(CP_ACP, 0, string, (int)strlen(string) + 1, buffer, sizeof(buffer) / sizeof(buffer[0]));

  return(buffer);
}


//--------------------------------------------------------------------------------
//
// Convert a unicode string to a c string in a static buffer
//
//--------------------------------------------------------------------------------
char* fromUnicode(const WCHAR* string)
{
  assert(string != NULL);
  
  BOOL unmappableChars;
  
  static char buffer[4096];
  assert(wcslen(string) + 1 < (sizeof(buffer) / sizeof(buffer[0])));

  // Convert unicode to normal c string
  WideCharToMultiByte(CP_ACP, 0, string, (int)wcslen(string) + 1, buffer, sizeof(buffer) / sizeof(buffer[0]), " ", &unmappableChars);

  return(buffer);
}
