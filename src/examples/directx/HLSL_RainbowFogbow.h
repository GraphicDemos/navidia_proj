#pragma once
/**
HLSL_RainbowFogbow NVIDIA SDK Demo

Description: -----------------
	This demo shows a practical technique for rendering rainbows, fogbows, 
corona and any other radially symetric optical scattering effects caused by directional lights

A quick overview of the technique

 1.  render any moisture in the scene to a texture.  This is a mask for where
     the optical effect should appear

 2.  render the scene normally

 3.  render a full screen quad using a lookup texture
     a. for each pixel on the screen calculate the angle between the view vector, and the light vector. 
	 b. use that angle to look up into the texture to find the proper color of light scattered at that point.


for a description of rainbows and fogbows see http://www.philiplaven.com/p2b.html
Phillip has also written a program to generate a Lee diagram of light scattered for various algorithms.
This is what I used to create my rainbow and corona textures.   http://www.philiplaven.com/p24.html


Once you have a good rainbow texture, the key to making the rainbow look real is having a good moisture texture.
In this demo, I use fog, a skybox generated with terragen, and another skybox with a scrolling noise texture to
give a hint of rain.   The better you can get a distribution of moisture in your scene for the rainbow to blend
with, the better the effect will be.


Some notes about rainbows:--------------------

-the lookup texture for the rainbow light should be blurred by the suns angular size 0.5 degrees.
   this should be baked into the texture

-rainbow light blends additively to existing light in the scene.
    aka current scene color + rainbow color
    aka alpha blend, one, one
	I use alpha blend, one, invsrccolor thought to keep from oversaturating. (real HDR would help here)

    
-horizontal thickness of moisture, 
	a thin sheet of rain will produce less bright rainbows than a thick sheet
	aka rainbow color  * water ammount, where water ammount ranges from 0 to 1

-rainbow light can be scattered and absorbed by other atmospheric particles.
	aka simplified..rainbow color * light color


For the Future:--------------------
- rainbows have a High Dynamic Range, especially going from a fogbow which is much darker to a full blown rainbow, which is relativley very bright.
  to implement this correctly we should be able to take this intensity into account.

- the rainbow could be baked into a cubemap texture then used on as a skybox.  This would be a good optimization when
  the rainbow isn't animating.

- this same type of rainbow lookup texture can be used to simulate other things like
  abalone shell which undergoes the same sorts of light refraction as that caused by water droplets

- this same technique of lookign up into a texture based on angle between view vector and light could maybe be used
  to simulate any sort of radially symetric optical effect. such as the color of scattered light in the sky.



Questions and Comments: cbrewer@nvidia.com

*/
#include <DXUT/DXUTcamera.h>
#include <DXUT/DXUTMesh.h>

// Struct to store the current input state
struct UserInput
{
    // TODO: change as needed
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
};
typedef std::basic_string<TCHAR> tstring; 
//-----------------------------------------------------------------------------
// Name: class RainbowFogbowSDKDemo
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. RainbowFogbowSDKDemo 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------

    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/*
    RainbowFogbowSDKDemo();
    virtual ~RainbowFogbowSDKDemo();

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

    inline bool KeyPressed(int vKey)
    {
        return (GetAsyncKeyState(vKey) & 0x8000) == 0x8000;
    }

    void UpdateInput(UserInput* pUserInput);
	*/
    BOOL                    g_bLoadingApp;          // TRUE, if the app is loading
    ID3DXFont*              g_pFont;                // D3DX font    
    UserInput               g_UserInput;            // Struct for storing user input 


	// NVIDIA Effect members/////////////////////////
	CFirstPersonCamera			g_Camera;
	D3DXVECTOR4					g_sunLightDirection;
	D3DXMATRIX					g_worldTransform;
	CDXUTXFileMesh*					g_pSkyBoxMesh;
	CDXUTXFileMesh*					g_pSkyBoxMoistureMesh;
	CDXUTXFileMesh*					g_pTerrainMesh;

	nv_RainbowEffect*			g_pNVRainbowEffect;
	float						g_rainbowDropletRadius;
	float						g_rainbowIntensity;
	LPD3DXEFFECT				g_pObjectEffects;
	D3DXHANDLE					g_hTechniqueRenderObjectsNormal;
	D3DXHANDLE					g_hTechniqueRenderObjectsBlack; 
	D3DXHANDLE					g_hTechniqueRenderSkyBoxMoisture; 

	LPD3DXEFFECT				g_pSkyBoxRainEffect;

	//helper to set proper directories and find paths and such
	HRESULT	LoadMeshHelperFunction(CDXUTXFileMesh* mesh, tstring meshFile, IDirect3DDevice9* pd3dDevice);

	//helper function to set up matrices for object effects
	void						SetUpMatriceeesForEffect(	D3DXMATRIX* view, 
										D3DXMATRIX* world, 
										D3DXMATRIX* proj);
	void						RenderMoisturePass(IDirect3DDevice9* pd3dDevice, double fTime);
	void						RenderSceneNormally(IDirect3DDevice9* pd3dDevice);

	bool						g_VisualizeRenderSteps;

	//sets the viewport to be at x, y and have width of w, and height of h
	void						SetViewPortHelper(DWORD x, DWORD y, DWORD w, DWORD h,IDirect3DDevice9* pd3dDevice);
