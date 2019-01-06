#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr.h>
#include <tchar.h>
#include <DX9SDKSampleFramework/DX9SDKSampleFramework.h>

#include "FrontBackRegisterApp.h"

#include "NV_D3DCommon\NV_D3DCommonDX9.h"
#include "NV_D3DMesh\NV_D3DMeshDX9.h"

#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include "shared\GetFilePath.h"

#include <vector>

// Shader file names loaded by this module
#define PSHN_FRONTBACKREG_PS30		L"MEDIA\\Programs\\FrontBackReg_PS30.psh"
#define VSHN_FRONTBACKREG_VS30		L"MEDIA\\Programs\\FrontBackReg_VS30.vsh"

//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyD3DApplication* g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    CMyD3DApplication d3dApp;

    g_pApp  = &d3dApp;
    g_hInst = hInst;

    InitCommonControls();
    if (FAILED(d3dApp.Create(hInst)))
        return 0;

    return d3dApp.Run();
}

//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor.   Paired with ~CMyD3DApplication()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_dwCreationWidth           = 512;
    m_dwCreationHeight          = 512;
    m_strWindowTitle            = TEXT("Front-Back PS.3.0 Register");
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
    m_bStartFullscreen          = false;
    m_bShowCursorWhenFullscreen = false;

    m_pFont                     = NULL;
    m_bLoadingApp               = TRUE;

    ZeroMemory(&m_UserInput, sizeof(m_UserInput));
    m_fWorldRotX                = 0.0f;
    m_fWorldRotY                = 0.0f;

	SetAllNull();
}

//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: Application destructor.  Paired with CMyD3DApplication()
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}

//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // TODO: perform one time initialization

    // Drawing loading status message until app finishes loading
    SendMessage(m_hWnd, WM_PAINT, 0, 0);

    m_bLoadingApp = FALSE;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the display device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, 
                                         D3DFORMAT adapterFormat, 
                                         D3DFORMAT backBufferFormat)
{
    UNREFERENCED_PARAMETER(dwBehavior);
    UNREFERENCED_PARAMETER(adapterFormat);
    UNREFERENCED_PARAMETER(backBufferFormat);

/*
	// enable to select refrast
	if( pCaps->DeviceType != D3DDEVTYPE_REF )
	{
		return( E_FAIL );
	}
// */

	if( D3DSHADER_VERSION_MAJOR(pCaps->PixelShaderVersion ) < 3 )
	{
		FMsg("Device does not support pixel shaders 3.0!\n");
		return( E_FAIL );
	}

	return( S_OK );
}

//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Paired with DeleteDeviceObjects()
//       The device has been created.  Resources that are not lost on
//       Reset() can be created here -- resources in D3DPOOL_MANAGED,
//       D3DPOOL_SCRATCH, or D3DPOOL_SYSTEMMEM.  Image surfaces created via
//       CreateOffScreenPlainSurface are never lost and can be created here.  Vertex
//       shaders and pixel shaders can also be created here as they are not
//       lost on Reset().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // TODO: create device objects

    HRESULT hr;

    // Initialize the font
    HDC hDC = GetDC(NULL);
    int nHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(NULL, hDC);
    if (FAILED(hr = D3DXCreateFont(m_pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                   DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                                   TEXT("Arial"), &m_pFont)))
        return DXTRACE_ERR(L"D3DXCreateFont", hr);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	HRESULT hr = S_OK;

	SAFE_DELETE( m_pMeshVB );
	Mesh			mesh;
	MeshGeoCreator	gc;
	gc.InitSphereFromBox( &mesh, 1.3f,
							D3DXVECTOR3( 1.0f, 0.0f, 0.0f ), 10,
							D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 10,
							D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), 10 );

	std::vector< UINT > vVertsToRemove;
	vVertsToRemove.reserve( 30 );
	UINT i;
	float center = 0.0f;
	float thresh = 1.3f * 2.7f / 10.0f;
	for( i=0; i < mesh.GetNumVertices(); i++ )
	{
		if( fabs( mesh.GetVertexPosition(i).y - center ) < thresh )
			vVertsToRemove.push_back( i );
	}

	MeshProcessor mp;
	mp.RemoveVertices( &mesh, &vVertsToRemove[0], (UINT)(vVertsToRemove.size()) );
	
	m_pMeshVB = new MeshVB;
	FAIL_IF_NULL( m_pMeshVB );
	m_pMeshVB->CreateFromMesh( &mesh, m_pd3dDevice );

	SAFE_DELETE( m_pShaderManager );
	m_pShaderManager = new ShaderManager;
	FAIL_IF_NULL( m_pShaderManager );
	m_pShaderManager->Initialize( m_pd3dDevice, GetFilePath::GetFilePath );

	hr = m_pShaderManager->LoadAndAssembleShader( PSHN_FRONTBACKREG_PS30, SM_SHADERTYPE_PIXEL, &m_PSHI_FrontBackReg_PS30 );
	
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), "Couldn't load shader FrontBackReg_PS30" , hr );
		
	hr = m_pShaderManager->LoadAndAssembleShader( VSHN_FRONTBACKREG_VS30, SM_SHADERTYPE_VERTEX, &m_VSHI_FrontBackReg_VS30 );
	MSG_BREAK_AND_RET_VAL_IF( FAILED(hr), "Couldn't load shader FrontBackReg_VS30", hr );

	//-------------------------------------------------
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    // Set the world matrix
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity);

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    D3DXMATRIX matView;
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&m_matView, &vFromPt, &vLookatPt, &vUpVec);

    // Set the projection matrix
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f);

    if (m_pFont)
        m_pFont->OnResetDevice();

    return( hr );
}

//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // TODO: update world

    // Update user input state
    UpdateInput(&m_UserInput);

    // Update the world state according to user input
    D3DXMATRIX matWorld;
    D3DXMATRIX matRotY;
    D3DXMATRIX matRotX;

    if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
        m_fWorldRotY += m_fElapsedTime;
    else if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
        m_fWorldRotY -= m_fElapsedTime;

    if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
        m_fWorldRotX += m_fElapsedTime;
    else if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
        m_fWorldRotX -= m_fElapsedTime;

    D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
    D3DXMatrixRotationY(&matRotY, m_fWorldRotY);

    D3DXMatrixMultiply(&matWorld, &matRotX, &matRotY);

	D3DXMATRIX matWVP;
	D3DXMatrixMultiply( &matWVP, &matWorld, &m_matView );
	D3DXMatrixMultiply( &matWVP, &matWVP, &m_matProj );
	D3DXMatrixTranspose( &m_matWorldViewProjTrans, &matWVP );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: Update the user input.  Called once per frame 
//-----------------------------------------------------------------------------
void CMyD3DApplication::UpdateInput( UserInput* pUserInput )
{
    pUserInput->bRotateUp    = (m_bActive && (GetAsyncKeyState(VK_UP)    & 0x8000) == 0x8000);
    pUserInput->bRotateDown  = (m_bActive && (GetAsyncKeyState(VK_DOWN)  & 0x8000) == 0x8000);
    pUserInput->bRotateLeft  = (m_bActive && (GetAsyncKeyState(VK_LEFT)  & 0x8000) == 0x8000);
    pUserInput->bRotateRight = (m_bActive && (GetAsyncKeyState(VK_RIGHT) & 0x8000) == 0x8000);
}


//-----------------------------------------------------------------------------
// Name: RenderText()
// Desc: Renders stats and help text to the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor    = D3DCOLOR_ARGB(255, 255, 255, 0);
    TCHAR szMsg[MAX_PATH] = TEXT("");

    RECT rct;
    ZeroMemory(&rct, sizeof(rct));       

    rct.left   = 2;
    rct.right  = m_d3dsdBackBuffer.Width - 20;

    // Output display stats
    INT nNextLine = 5; 

    lstrcpy(szMsg, m_strFrameStats);
	rct.top = nNextLine; rct.bottom = rct.top + 20; nNextLine = rct.bottom;
    m_pFont->DrawText(NULL, szMsg, -1, &rct, DT_NOCLIP, fontColor);

    lstrcpy(szMsg, m_strDeviceStats);
	rct.top = nNextLine; rct.bottom = rct.top + 20; nNextLine = rct.bottom;
    m_pFont->DrawText(NULL, szMsg, -1, &rct, DT_NOCLIP, fontColor);

    lstrcpy(szMsg, TEXT("Front faces should appear green"));
	rct.top = nNextLine; rct.bottom = rct.top + 20; nNextLine = rct.bottom;
    m_pFont->DrawText(NULL, szMsg , -1, &rct, DT_NOCLIP, fontColor);

    lstrcpy(szMsg, TEXT("Back faces should appear red"));
	rct.top = nNextLine; rct.bottom = rct.top + 20; nNextLine = rct.bottom;
    m_pFont->DrawText(NULL, szMsg , -1, &rct, DT_NOCLIP, fontColor);

    // Output help
    nNextLine = m_d3dsdBackBuffer.Height; 

    lstrcpy(szMsg, TEXT("Use arrow keys to rotate object"));
    nNextLine -= 20; rct.top = nNextLine; rct.bottom = rct.top + 20;    
    m_pFont->DrawText(NULL, szMsg, -1, &rct, DT_NOCLIP, fontColor);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg, WPARAM wParam,
                                   LPARAM lParam)
{
    switch(msg)
    {
        case WM_PAINT:
        {
            if (m_bLoadingApp)
            {
                // Draw on the window tell the user that the app is loading
                // TODO: change as needed
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
    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
	SAFE_DELETE( m_pMeshVB );
	SAFE_DELETE( m_pShaderManager );

	if (m_pFont)
        m_pFont->OnLostDevice();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Paired with InitDeviceObjects()
//       Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.  
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    // TODO: Cleanup any objects created in InitDeviceObjects()
    SAFE_RELEASE(m_pFont);
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Paired with OneTimeSceneInit()
//       Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // TODO: Perform any final cleanup needed
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	FAIL_IF_NULL( m_pd3dDevice );
	FAIL_IF_NULL( m_pShaderManager );

    // Clear the viewport
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x000000ff, 1.0f, 0L);

    // Begin the scene
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {        
		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,	D3DFILL_WIREFRAME );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,	D3DCULL_NONE );

		m_pShaderManager->SetShader( m_VSHI_FrontBackReg_VS30 );
		m_pShaderManager->SetShader( m_PSHI_FrontBackReg_PS30 );
	
		m_pd3dDevice->SetVertexShaderConstantF( 0, (float*)&m_matWorldViewProjTrans, 4 );

		m_pMeshVB->Draw();

        // Render stats and help text  
        RenderText();

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}


