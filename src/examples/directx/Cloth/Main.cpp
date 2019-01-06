
// Header files
#include "nvafx.h"
#include "GUI.h"

// Initial window size
#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

// Static functions
static bool CALLBACK IsDeviceAcceptable(D3DCAPS9 *, D3DFORMAT, D3DFORMAT, bool, void*);
static bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings*, const D3DCAPS9*, void*);
static BOOL IsDepthFormatOk(D3DFORMAT, D3DFORMAT, D3DFORMAT);

// Entry point
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    // Setup callbacks DXUTSetCallbackD3D9DeviceCreated
    DXUTSetCallbackD3D9DeviceCreated(GUI::OnCreateDevice);
    DXUTSetCallbackD3D9DeviceDestroyed(GUI::OnDestroyDevice);
    DXUTSetCallbackD3D9DeviceReset(GUI::OnResetDevice);
    DXUTSetCallbackD3D9DeviceLost(GUI::OnLostDevice);
    DXUTSetCallbackMsgProc(GUI::MsgProc);
    DXUTSetCallbackMouse(GUI::Mouse, true);
    DXUTSetCallbackKeyboard(GUI::KeyboardProc);
    DXUTSetCallbackD3D9FrameRender(GUI::OnFrameRender);
	DXUTSetCallbackD3D9DeviceFrameMove(GUI::OnFrameMove);

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings(true, true);

    // GUI initialization
    GUI::Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize the sample framework and create the window and Direct3D device
    DXUTInit(true, true,NULL, true);
    DXUTCreateWindow(L"Cloth");
    DXUTCreateDevice(true, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Enter the main loop; FrameMove and FrameRender are called when there
    // is idle time between handling window messages.
    DXUTMainLoop();

    // GUI cleanup
    GUI::Cleanup();

    return DXUTGetExitCode();
}

// Called during device initialization, this code checks the device for some
// minimum set of capabilities, and rejects those that don't pass by
// returning false.
bool CALLBACK IsDeviceAcceptable(D3DCAPS9* caps, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, bool IsWindowed, void* pUserContext)
{
    IDirect3D9* D3D = DXUTGetD3D9Object();

    // Support for Shader Model 3.0
    if (caps->VertexShaderVersion < D3DVS_VERSION(3, 0))
        return false;
    if (caps->PixelShaderVersion < D3DPS_VERSION(3, 0))
        return false;

    // Support for D3DFMT_A32B32G32R32F render target
    if (FAILED(D3D->CheckDeviceFormat(caps->AdapterOrdinal, caps->DeviceType, adapterFormat, D3DUSAGE_RENDERTARGET,
        D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F))) {
        return false;
    }

    // Support for stencil buffer (this is used to display cloth selected points)
    if (!IsDepthFormatOk(D3DFMT_D24S8, adapterFormat, backBufferFormat) &&
        !IsDepthFormatOk(D3DFMT_D24X4S4, adapterFormat, backBufferFormat) &&
        !IsDepthFormatOk(D3DFMT_D15S1, adapterFormat, backBufferFormat) &&
        !IsDepthFormatOk(D3DFMT_D24FS8, adapterFormat, backBufferFormat))
        return false;

    return true;
}

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* caps, void* pUserContext)
{
    pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	pDeviceSettings->d3d9.pp.BackBufferCount = 1;

    // This sample requires a stencil buffer (only to display cloth selected points)
    if (IsDepthFormatOk(D3DFMT_D24S8, pDeviceSettings->d3d9.AdapterFormat, pDeviceSettings->d3d9.pp.BackBufferFormat))
        pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    else if (IsDepthFormatOk(D3DFMT_D24X4S4, pDeviceSettings->d3d9.AdapterFormat, pDeviceSettings->d3d9.pp.BackBufferFormat))
        pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D24X4S4;
    else if (IsDepthFormatOk(D3DFMT_D24FS8, pDeviceSettings->d3d9.AdapterFormat, pDeviceSettings->d3d9.pp.BackBufferFormat))
        pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D24FS8;
    else if (IsDepthFormatOk(D3DFMT_D15S1, pDeviceSettings->d3d9.AdapterFormat, pDeviceSettings->d3d9.pp.BackBufferFormat))
        pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D15S1;

    return true;
}

// Returns true if a particular depth-stencil format can be created and used
// with an adapter format and backbuffer format combination.
BOOL IsDepthFormatOk(D3DFORMAT DepthFormat, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat)
{

    // Verify that the depth format exists 
    HRESULT hr = DXUTGetD3D9Object()->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AdapterFormat,
                                                       D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, DepthFormat);
    if (FAILED(hr))
        return FALSE;

    // Verify that the backbuffer format is valid
    hr = DXUTGetD3D9Object()->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AdapterFormat, D3DUSAGE_RENDERTARGET,
                                               D3DRTYPE_SURFACE, BackBufferFormat);
    if (FAILED(hr))
        return FALSE;

    // Verify that the depth format is compatible
    hr = DXUTGetD3D9Object()->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AdapterFormat, BackBufferFormat,
                                                    DepthFormat);

    return SUCCEEDED(hr);
}
