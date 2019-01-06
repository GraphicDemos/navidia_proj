#pragma once


#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"



// Struct to store the current input state
struct UserInput
{
    // TODO: change as needed
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
};

//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
public:
	ShaderManager *		m_pShaderManager;
	MeshVB *			m_pMeshVB;
	
	D3DXMATRIX			m_matWorldViewProjTrans;
	D3DXMATRIX			m_matView;
	D3DXMATRIX			m_matProj;
	SM_SHADER_INDEX		m_PSHI_FrontBackReg_PS30;
	SM_SHADER_INDEX		m_VSHI_FrontBackReg_VS30;

	void SetAllNull()
	{
		m_pShaderManager	= NULL;
		m_pMeshVB			= NULL;
	};

public:
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();

protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();
    virtual HRESULT ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat);

    HRESULT RenderText();

    void UpdateInput(UserInput* pUserInput);

private:
    BOOL                    m_bLoadingApp;          // TRUE, if the app is loading
    ID3DXFont*              m_pFont;                // D3DX font    
    UserInput               m_UserInput;            // Struct for storing user input 

    FLOAT                   m_fWorldRotX;           // World rotation state X-axis
    FLOAT                   m_fWorldRotY;           // World rotation state Y-axis
};

