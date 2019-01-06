#pragma once

#include "dxstdafx.h"
#include <shared/NVBScene9.h>
#include <shared/GetFilePath.h>
#include <vector>

typedef std::basic_string<TCHAR> tstring; 

#define NUM_MRT 3

#define USE_HWSHADOWMAP 1

struct QuadVertex
{
    QuadVertex(D3DXVECTOR3 const & vecPosition, 
                D3DXVECTOR2 const & vecUV)
        : Position(vecPosition)
        , Texture (vecUV)
    {};

    D3DXVECTOR3 Position;
    D3DXVECTOR2 Texture;
};

struct LightType
{
    D3DLIGHTTYPE Type;
    D3DXVECTOR3 Color;
    D3DXVECTOR3 Direction;
    D3DXVECTOR3 Position;
	float Range;
};

struct RTT_Type
{
	IDirect3DSurface9* surf;
	IDirect3DTexture9* tex;
	RTT_Type()
    {
        surf = NULL;
        tex  = NULL;
	}

	~RTT_Type()
	{
        if (surf)
        {
            surf->Release();
            surf = NULL;
        }

        if (tex)
        {
            tex->Release();
            tex = NULL;
        }
	}
};


enum RenderMode
{
    RM_NORMAL,
    RM_CHANNEL0,
    RM_CHANNEL1,
    RM_CHANNEL2,
};

//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class DeferredShading
{
public:

    DeferredShading();
    virtual ~DeferredShading();

    virtual HRESULT OneTimeSceneInit(HWND m_hWnd);
    virtual HRESULT RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice);
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT Render(IDirect3DDevice9* pd3dDevice);
    virtual HRESULT FinalCleanup();
	virtual HRESULT ConfirmDevice(D3DCAPS9*,DWORD,D3DFORMAT,D3DFORMAT);

	UINT m_iBBHeight;
	UINT m_iBBWidth;

	bool                    m_bCanTonemap;
	bool                    m_bDoScissor;
	bool                    m_bDebugScissor;

	bool                    m_bAnimateLights;
	bool                    m_bTonemap;

	RenderMode              m_currRenderMode;

	//These two are for hardware shadow maps
	float					m_fDepthBias;
	float					m_fBiasSlope;

	std::vector<LightType> m_Lights;
	//

	// Transforms
	D3DXMATRIX m_View;
	D3DXMATRIX m_Projection;
	D3DXMATRIX m_World;
	D3DXVECTOR3 m_LightPosition;
	D3DXVECTOR3 m_LightDirection;

	// GUI parameters
	bool m_UseDefaultDiffuseMap;
	bool m_UseDefaultNormalMap;
	bool m_Wireframe;
	bool m_DrawLight;
	bool m_DrawAxis;
	bool m_DrawNormals;
	bool m_DrawTangentBasis;
	bool m_PauseScene;
	bool m_PauseCamera;
	bool m_PauseLight;
	bool m_PauseFPS;

	NVBScene* m_Scene;

private:

	HRESULT InitializeScene(IDirect3DDevice9* pd3dDevice);

	// Textures
	HRESULT LoadTexture(IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DTEXTURE9&, D3DPOOL pool = D3DPOOL_DEFAULT, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTexture(IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DCUBETEXTURE9&, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTextures(IDirect3DDevice9* pd3dDevice);
	void ReleaseTextures();
	tstring m_DiffuseMapFilename;
	LPDIRECT3DTEXTURE9 m_DiffuseMap;
	tstring m_HeightMapFilename;
	LPDIRECT3DTEXTURE9 m_HeightMap;
	tstring m_NormalMapFilename;
	LPDIRECT3DTEXTURE9 m_NormalMap;
	tstring m_CubeMapFilename;
	LPDIRECT3DCUBETEXTURE9 m_CubeMap;

	// Geometry
	float m_DefaultSceneCenter[3];
	float m_DefaultSceneRadius;
	tstring m_SceneFilename;
	NVBScene* m_Light;

	// Timing
	float m_Time;
	float m_StartTime;
	float m_FPS;
	int   m_Frame;
	float m_LastTime;
	int   m_LastFrame;
	float m_Ambient;
	float m_AnimTime;
	static const float m_DefaultAmbient;

	//

	bool                    m_bLoadingApp;          // TRUE, if the app is loading
	bool					m_bSupportsHWShadowMaps;

	bool                    m_bFont;
	bool					m_bBlur;
	bool					m_bBlurFirst;			// If false, blur is added after tonemapping (wrong, but can look neat)
	bool					m_bShowHelp;
	float					m_fTonemapScale;
	
	D3DXMATRIX				m_matWorld;
	D3DXMATRIX				m_matView;
	D3DXMATRIX				m_matProj;
	D3DXMATRIX				m_matViewProj;
    D3DXMATRIX              m_matInvProj;
	D3DXMATRIX              m_matInvView;
	D3DXMATRIX				m_matInvViewProj;
	D3DXMATRIX				m_matDefaultView;
	D3DXMATRIX				m_matAlternateView;

    ID3DXEffect*            m_pEffect;

	IDirect3DSurface9*		m_pBackBuffer;
	IDirect3DSurface9*		m_pDepthBuffer;

    RTT_Type				m_LTSurfaces;
	RTT_Type				m_LTSurfacesFinal;
    RTT_Type				m_MRTs[NUM_MRT];
    RTT_Type                m_LuminanceSurfaces;
	RTT_Type				m_ShadowMap;
#if USE_HWSHADOWMAP
	RTT_Type				m_ShadowMapDummy;
#endif
	RTT_Type				m_BlurSurfaces1;
	RTT_Type				m_BlurSurfaces2;
	std::vector<RTT_Type>   m_TonemapSurfaces;

	RTT_Type				m_LastLuminance;
	RTT_Type				m_CurrLuminance;

	//VB for the fullscreen quad
	IDirect3DVertexBuffer9* m_pQuadVB;

	//VBs for all the mini-quads
	std::vector<IDirect3DVertexBuffer9*> m_QuadVec;

    //mesh for rendering light object
    ID3DXMesh*              m_pLightMesh;

	//matrix used when rendering fullscreen quad
	D3DXMATRIX m_quadMat;

	D3DXMATRIX m_lightMat;

    IDirect3DVertexDeclaration9* m_pDeclaration0;
	IDirect3DVertexDeclaration9* m_pDeclaration1;


	// Default filenames
    static const tstring m_DefaultSceneFilename;
    static const tstring m_DefaultNormalMapFilename;
    static const tstring m_DefaultDiffuseMapFilename;
	static const tstring m_DefaultEnvironmentMapFilename;

	HRESULT LoadShaders(IDirect3DDevice9* pd3dDevice);
	HRESULT DrawSceneToShadowMap(IDirect3DDevice9* pd3dDevice);
	HRESULT DrawSceneToMRT(IDirect3DDevice9* pd3dDevice);
    HRESULT DrawLightTransport(IDirect3DDevice9* pd3dDevice);
	HRESULT DrawFinalScene(IDirect3DDevice9* pd3dDevice);
	HRESULT BlurSurface(IDirect3DDevice9* pd3dDevice, IDirect3DTexture9* sourceTex);
    bool CanTonemap(LPDIRECT3D9 m_pD3D, D3DDEVTYPE devType);
    void AnimateLights();
   	void InitializeLights();
	void SetBumpDot3VSConstants(const D3DXMATRIX&);
	void SetBumpDot3VSConstantsSM(const D3DXMATRIX& world, const D3DXMATRIX& viewProj);
	RECT DetermineClipRect(const D3DXVECTOR3&, const float);
	bool IsShadowedMesh(const std::string meshName);
	HRESULT CreateQuad(IDirect3DDevice9* pd3dDevice, IDirect3DVertexBuffer9* &quadVB, unsigned int width, unsigned int height);
	void AddPointLight(float x, float y, float z);

	HRESULT CheckResourceFormatSupport(IDirect3DDevice9* pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);

};
