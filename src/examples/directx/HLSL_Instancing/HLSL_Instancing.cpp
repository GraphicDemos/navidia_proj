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
// File: HLSL_Instancing.cpp
// Desc: This sample shows the use of D3D's Instancing API.  You should see the whitepaper for
//			the full details.  The gist is that we have two instanced meshes.  Space ships and 
//			asteroids.  The big motherships are just garnish.  The instanced mesh data
//			is in on VB stream, and world matrices are stored in the second stream.  See OnRender()
//			for the guts of this.
//-----------------------------------------------------------------------------
#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/SDKmisc.h>
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

const D3DVERTEXELEMENT9 g_aCloneMeshVertDecl[] =
{
	{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
	{0, 36, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
	{0, 48, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

struct MeshVertex
{
	MeshVertex(D3DXVECTOR3 p,D3DXVECTOR3 n,D3DXVECTOR2 u): pos(p),normal(n),uv(u)
	{}
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 uv;
};

const D3DVERTEXELEMENT9 g_aMeshVertDecl[] =
{
	{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
	{0, 36, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
	{0, 48, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{1, 0,  D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	{1, 16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
	{1, 32, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
	{1, 48, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	D3DDECL_END()
};

struct MatrixVertex
{
public:
	MatrixVertex(D3DXVECTOR4& i1,D3DXVECTOR4& i2,D3DXVECTOR4&i3,D3DXVECTOR4&i4,D3DXCOLOR&c)
		: r1(i1),r2(i2),r3(i3),c1(c) {}
	D3DXVECTOR4 r1;     // row 1
	D3DXVECTOR4 r2;     // row 2
	D3DXVECTOR4 r3;		// row 3
	D3DXCOLOR c1;
};

#include "../nv_skybox.h"
#include "AsteroidManager.h"
#include "MothershipManager.h"
#include "SpaceShipManager.h"
#include "nv_d3d9FullScreenQuad.h"

D3DXCOLOR vWhite = D3DXCOLOR(1,1,1,1);
const static int INSTANCE_INIT_SRAND=1425236;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pSprite = NULL;       // Sprite for batching draw text calls
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
CFirstPersonCamera      g_Camera;               // A model viewing camera
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
bool                    g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader

// Configs
bool	g_bUseInstancing = true;
bool	g_bShowStats = true;
int		g_numShipInstances = 500;
int		g_numRockInstances = 4000;
int		g_numDrawCalls;


// dirty bits
bool g_bReAllocateMatrixVB = true;		// if true the VBs need to be re-created due to size changes

LPDIRECT3DVERTEXDECLARATION9	g_pMeshVertexDecl;				// Full 2 stream decl
LPDIRECT3DVERTEXDECLARATION9	g_pCloneMeshVertexDecl;			// 1st stream used to conform VB

// ------------------- Space ships instancing data ---------------
int		g_numShipPolys;
int		g_numShipVerts;
int		g_numShipIndices;
D3DXMATRIX *					g_shipInstanceMatrices=0;
D3DXCOLOR *						g_shipInstanceColors=0;
LPDIRECT3DVERTEXBUFFER9			g_shipVB=0;						// VB of a single model verts
LPDIRECT3DINDEXBUFFER9			g_shipIB=0;						// IB will contain indices
LPDIRECT3DVERTEXBUFFER9			g_shipMatrixVB=0;				// VB of model world matrices
LPDIRECT3DTEXTURE9				g_pShipTexture = NULL;			// Mesh texture
LPDIRECT3DTEXTURE9				g_pShipBumpTexture = NULL;		// Mesh texture

// ------------------- Asteroid Rock instancing data ---------------
int		g_numRockPolys;
int		g_numRockVerts;
int		g_numRockIndices;
D3DXMATRIX *					g_rockInstanceMatrices=0;
LPDIRECT3DVERTEXBUFFER9			g_rockVB=0;						// VB of a single model verts
LPDIRECT3DINDEXBUFFER9			g_rockIB=0;						// IB will contain indices
LPDIRECT3DVERTEXBUFFER9			g_rockMatrixVB=0;				// VB of model world matrices
LPDIRECT3DTEXTURE9				g_pRockTexture = NULL;			// Mesh texture
LPDIRECT3DTEXTURE9				g_pRockBumpTexture = NULL;		// Mesh texture


LPDIRECT3DSURFACE9				g_pSharedDepth = NULL;
LPDIRECT3DTEXTURE9				g_pRenderTargetTexture = NULL;  // For scene texture
LPDIRECT3DSURFACE9				g_pRenderTargetSurface = NULL;

LPDIRECT3DTEXTURE9				g_pDownsampleTargetTexture = NULL;  // downsampled scen
LPDIRECT3DSURFACE9				g_pDownsampleTargetSurface = NULL;

LPDIRECT3DTEXTURE9				g_pBlurHTargetTexture = NULL;  // downsampled and h blurred
LPDIRECT3DSURFACE9				g_pBlurHTargetSurface = NULL;

LPDIRECT3DTEXTURE9				g_pBlurVTargetTexture = NULL;  // downsampled and h & v blurred
LPDIRECT3DSURFACE9				g_pBlurVTargetSurface = NULL;

LPDIRECT3DSURFACE9				g_pBBSurface = NULL;	// original swap chain surface
LPDIRECT3DSURFACE9				g_pBBDepthSurface = NULL;	// original swap chain depth surface

// Misc classes to control "AI" of ships and placements, and non-instancing stuff
nv_SkyBox *				g_skybox = new nv_SkyBox();
SpaceShipManager*		g_shipManager = new SpaceShipManager();
AsteroidManager*		g_asteroidManager = new AsteroidManager();
MothershipManager *		g_mothershipManager = new MothershipManager();
nv_D3D9FullScreenQuad*  g_fullScreenQuad = new nv_D3D9FullScreenQuad();

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_VIEWWHITEPAPER      2
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_INSTANCECOUNT		5
#define IDC_INSTANCECOUNT_STATIC 6
#define IDC_ROCKCOUNT			7
#define IDC_ROCKCOUNT_STATIC	8
#define IDC_USEINSTANCING		9

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, const WCHAR* strFileName, 
				 const D3DVERTEXELEMENT9* aMeshDecl,
				 LPDIRECT3DVERTEXBUFFER9* ppVB,LPDIRECT3DINDEXBUFFER9* ppIB, 
				 int *pNumPolys, int *pNumVerts, int *pNumIndices );
void    RenderText( double fTime );
HRESULT RenderLightArrow( D3DXVECTOR3 lightDir, D3DXCOLOR arrowColor );

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Set the callback functions. These functions allow the sample framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, NULL,true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"MeshInstancing" );
    DXUTCreateDevice(  true, 1024, 768 );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	WCHAR sz[100];

    // Initialize dialogs
    g_SampleUI.Init( &g_DialogResourceManager );
	g_SampleUI.SetCallback( OnGUIEvent );  

	int iY = 0;
    g_SampleUI.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY += 24, 125, 28 );

	g_SampleUI.AddSlider( IDC_INSTANCECOUNT,0,iY += 45,250,22,1,6000,g_numShipInstances);
	_snwprintf( sz, 100, L"# Ships: %d", g_numShipInstances ); sz[99] = 0;
	g_SampleUI.AddStatic( IDC_INSTANCECOUNT_STATIC, sz, 00, iY-16, 125, 22 );

	g_SampleUI.AddSlider( IDC_ROCKCOUNT,0,iY += 30,250,22,1,16000,g_numRockInstances);
	_snwprintf( sz, 100, L"# Rocks: %d", g_numRockInstances ); sz[99] = 0;
	g_SampleUI.AddStatic( IDC_ROCKCOUNT_STATIC, sz, 0,iY-16 , 125, 22 );

	g_SampleUI.AddCheckBox( IDC_USEINSTANCING,L"Use Instancing?",35, iY +=30, 225,20,g_bUseInstancing );

}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	if(pCaps->VertexShaderVersion < D3DVS_VERSION(2,0) ||
		pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    else
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }

    // This application is designed to work on a pure device by not using 
    // IDirect3D9::Get*() methods, so create a pure device if supported and using HWVP.
    if ((pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 && 
        (pDeviceSettings->d3d9.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 )
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_PUREDEVICE;

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif

	pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D16;

	// Disable instancing for cards that can't handle it.
	if(pCaps->VertexShaderVersion < D3DVS_VERSION(3,0))
	{
		g_bUseInstancing = false;
		CDXUTCheckBox *checkBox = NULL;
		checkBox = g_SampleUI.GetCheckBox(IDC_USEINSTANCING);
		checkBox->SetChecked(false);
		checkBox->SetEnabled(false);
		checkBox->SetText(L"Use Instancing? (Not Supported)");
	}

    return true;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                              L"Arial", &g_pFont ) );

	V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );

	// -------------------------------------------------------------------------------------------
	// Create our VBs and IBs and stuff
	V(pd3dDevice->CreateVertexDeclaration(g_aMeshVertDecl, &g_pMeshVertexDecl));
	V(pd3dDevice->CreateVertexDeclaration(g_aCloneMeshVertDecl, &g_pCloneMeshVertexDecl));

    return S_OK;
}

//--------------------------------------------------------------------------------------
// This function loads a model from a .X file and optimizes it in place, and marshals it
//	into a VB following the vertex format passed in.
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, const WCHAR * strFileName, 
				 const D3DVERTEXELEMENT9* aMeshDecl,
				 LPDIRECT3DVERTEXBUFFER9* ppVB,LPDIRECT3DINDEXBUFFER9* ppIB, 
				 int *pNumPolys, int *pNumVerts, int *pNumIndices )
{
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    // Load the mesh with D3DX and get back a ID3DXMesh*.  For this
    // sample we'll ignore the X file's embedded materials since we know 
    // exactly the model we're loading.  See the mesh samples such as
    // "OptimizedMesh" for a more generic mesh loading example.
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
    V_RETURN( D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh) );

    DWORD *rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        ID3DXMesh* pTempMesh;
        V( pMesh->CloneMeshFVF( pMesh->GetOptions(), 
                                  pMesh->GetFVF() | D3DFVF_NORMAL, 
                                  pd3dDevice, &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );
        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }

	rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
	if( rgdwAdjacency == NULL )
		return E_OUTOFMEMORY;

    // Optimize the mesh for this graphics card's vertex cache 
    // so when rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader 
    // on those vertices so it will improve perf.     
    V( pMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency) );
    V( pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL) );
    delete []rgdwAdjacency;

	// Clone the mesh into one that uses our decl
	ID3DXMesh* pConformedMesh = NULL;
	V(pMesh->CloneMesh(pMesh->GetOptions(),aMeshDecl,pd3dDevice,&pConformedMesh));
	pMesh->Release();
	pMesh = pConformedMesh;

	// Update tangent and binorm info
	rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
	if( rgdwAdjacency == NULL )
		return E_OUTOFMEMORY;
	V( pMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency) );
	V(D3DXComputeTangent( pMesh, 0,0,0,0,rgdwAdjacency));
	delete []rgdwAdjacency;

	// Create our model VB
	int numMeshVerts = pMesh->GetNumVertices();
	int bufferSize = numMeshVerts*D3DXGetDeclVertexSize( aMeshDecl, 0 );
	V(pd3dDevice->CreateVertexBuffer(bufferSize,D3DUSAGE_WRITEONLY, 0,D3DPOOL_DEFAULT, ppVB, NULL));

	// Fill the vertex buffer with data from the mesh object
	LPVOID pMeshVertices;
	V((*ppVB)->Lock( 0, 0, &pMeshVertices, 0));

	// Pull straight from mesh object VB, they have the same decl so it should be good
	LPVOID pMeshData;
	pMesh->LockVertexBuffer(0,&pMeshData);
	memcpy(pMeshVertices,pMeshData,bufferSize);

	// Release our locks
	V((*ppVB)->Unlock());
	V(pMesh->UnlockVertexBuffer());

	// Now for the IB.  This is silly. Gotta basically memcpy out the IB to replicate it.  
	V(pMesh->GetIndexBuffer(ppIB));
	D3DINDEXBUFFER_DESC ibDesc;
	V((*ppIB)->GetDesc(&ibDesc));
	(*ppIB)->Release();
	
	int numMeshIndices = ibDesc.Format==D3DFMT_INDEX16?(ibDesc.Size/2):(ibDesc.Size/4);
	int numPolysPerModel = numMeshIndices/3;

	int indexBufferSize = ibDesc.Format==D3DFMT_INDEX16?(numMeshIndices*2):(numMeshIndices*4);
	V(pd3dDevice->CreateIndexBuffer(indexBufferSize,D3DUSAGE_WRITEONLY,ibDesc.Format,D3DPOOL_DEFAULT,ppIB,NULL));

	LPVOID pIBDataSrc;
	LPVOID pIBDataDst;
	V(pMesh->LockIndexBuffer(D3DLOCK_READONLY,&pIBDataSrc));
	V((*ppIB)->Lock(0,indexBufferSize,&pIBDataDst,0));
	memcpy(pIBDataDst,pIBDataSrc,indexBufferSize);
	V(pMesh->UnlockIndexBuffer());
	V((*ppIB)->Unlock());

	SAFE_RELEASE(pMesh);

	// Propagate back to caller
	*pNumIndices = numMeshIndices;
	*pNumPolys = numPolysPerModel;
	*pNumVerts = numMeshVerts;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 1.0f, 5000.0f );
	g_Camera.SetRotateButtons(true,false,false);

    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-300, 0 );
    g_SampleUI.SetSize( 170, 300 );

	// -------------------------------------------------------------------------------------------
	// Load ship model and create VB/IB and instance matrix VB
	V_RETURN( LoadMesh ( pd3dDevice, L"MEDIA\\models\\SpaceShip\\missile1.x", g_aCloneMeshVertDecl, 
		&g_shipVB, &g_shipIB, &g_numShipPolys,&g_numShipVerts, &g_numShipIndices ));

	// Create our matrix VB
	V(pd3dDevice->CreateVertexBuffer(g_numShipInstances*sizeof(MatrixVertex),
		D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0,D3DPOOL_DEFAULT, &g_shipMatrixVB, NULL));

	// -------------------------------------------------------------------------------------------
	// Load rock model and create VB/IB and instance matrix VB
	V_RETURN( LoadMesh ( pd3dDevice, L"MEDIA\\models\\Asteroid\\aster1.x", g_aCloneMeshVertDecl, 
		&g_rockVB, &g_rockIB, &g_numRockPolys, &g_numRockVerts, &g_numRockIndices ));

	// Create our matrix VB
	V(pd3dDevice->CreateVertexBuffer(g_numRockInstances*sizeof(MatrixVertex),
		D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0,D3DPOOL_DEFAULT, &g_rockMatrixVB, NULL));

	// Preshaders are parts of the shader that the effect system pulls out of the 
	// shader and runs on the host CPU. They should be used if you are GPU limited. 
	// The D3DXSHADER_NO_PRESHADER flag disables preshaders.
	DWORD dwShaderFlags = 0;
	dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

	//---------------------------------------------------------------------------
	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\programs\\HLSL_Instancing.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

	// Create the mesh texture from a file
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SpaceShip\\missile1-Color.dds" ) );

	V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
		D3DX_DEFAULT, D3DX_DEFAULT, 0, 
		NULL, NULL, &g_pShipTexture ) );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\SpaceShip\\missile1-normal.dds" ) );

	V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
		D3DX_DEFAULT, D3DX_DEFAULT, 0, 
		NULL, NULL, &g_pShipBumpTexture ) );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\Asteroid\\aster1.dds" ) );

	V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
		D3DX_DEFAULT, D3DX_DEFAULT, 0, 
		NULL, NULL, &g_pRockTexture ) );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\models\\Asteroid\\aster1-normal.dds" ) );

	V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, 
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
		D3DX_DEFAULT, D3DX_DEFAULT, 0, 
		NULL, NULL, &g_pRockBumpTexture ) );

	// Set effect variables as needed
	D3DXCOLOR colorMtrlAmbient(0.15f, 0.15f, 0.15f, 0);
	V_RETURN( g_pEffect->SetValue("g_MaterialAmbientColor", &colorMtrlAmbient, sizeof(D3DXCOLOR) ) );

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye(0.0f, 30.0f, 30.0f);
	D3DXVECTOR3 vecAt (0.0f, 20.0f, 0.0f);
	g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetScalers(0.01f,65.f);

	//---------------------------------------------------------------------------
	// initilize our sky box and it's cube map
	LPDIRECT3DCUBETEXTURE9 cubeTex;
	g_skybox->Init(pd3dDevice);
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MEDIA\\textures\\cubemaps\\biggerStars.dds" ) );

	V(D3DXCreateCubeTextureFromFileEx(pd3dDevice, str,D3DX_DEFAULT,0,0,D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,D3DX_FILTER_LINEAR,
		D3DX_FILTER_LINEAR,0,NULL,NULL,&cubeTex));
	g_skybox->SetCubeMap(cubeTex);

	//---------------------------------------------------------------------------
	D3DXVECTOR3 v = D3DXVECTOR3(300.f,300.f,300.0f);
	srand(INSTANCE_INIT_SRAND);
	g_shipManager->SetWorldExtents(-v,v);
	g_asteroidManager->SetWorldExtents(-v,v);
	g_mothershipManager->SetWorldExtents(-v,v);
	g_shipManager->Init(g_numShipInstances);	
	g_asteroidManager->Init(g_numRockInstances);	
	g_mothershipManager->Init(10);

	g_mothershipManager->Create(pd3dDevice);

	// Create our scene RT texs and surfaces
	V(D3DXCreateTexture(pd3dDevice,
		pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height,
		0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
		&g_pRenderTargetTexture));

	g_pRenderTargetTexture->GetSurfaceLevel(0,&g_pRenderTargetSurface);

	V(D3DXCreateTexture(pd3dDevice,
		pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height,
		0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
		&g_pDownsampleTargetTexture));

	g_pDownsampleTargetTexture->GetSurfaceLevel(0,&g_pDownsampleTargetSurface);

	V(D3DXCreateTexture(pd3dDevice,
		pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height,
		0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
		&g_pBlurHTargetTexture));

	g_pBlurHTargetTexture->GetSurfaceLevel(0,&g_pBlurHTargetSurface);

	V(D3DXCreateTexture(pd3dDevice,
		pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height,
		0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
		&g_pBlurVTargetTexture));

	g_pBlurVTargetTexture->GetSurfaceLevel(0,&g_pBlurVTargetSurface);

	// Need to pull out and use the created RT sizes, since create texture might bump them
	//		 up to pow2 textures without telling us.
	D3DSURFACE_DESC desc;
	g_pRenderTargetSurface->GetDesc(&desc);
	V(pd3dDevice->CreateDepthStencilSurface(desc.Width,desc.Height,		
		D3DFMT_D24X8,D3DMULTISAMPLE_NONE,0,true,&g_pSharedDepth,NULL));

	// Setup full screen rendering
	g_fullScreenQuad->RestoreDeviceObjects(pd3dDevice);
	pd3dDevice->GetRenderTarget(0,&g_pBBSurface);
	pd3dDevice->GetDepthStencilSurface(&g_pBBDepthSurface);
	g_fullScreenQuad->SetUpForRenderTargetSurface(g_pBBSurface);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	HRESULT hr;
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

	// number of instances have been changes, so update our matrix vb to be the right size
	if(g_bReAllocateMatrixVB)
	{
		SAFE_RELEASE(g_shipMatrixVB);
		V(pd3dDevice->CreateVertexBuffer(g_numShipInstances*sizeof(MatrixVertex),
			D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0,D3DPOOL_DEFAULT, &g_shipMatrixVB, NULL));

		if(g_shipInstanceMatrices) delete[] g_shipInstanceMatrices;
		g_shipInstanceMatrices = new D3DXMATRIX[g_numShipInstances];

		if(g_shipInstanceColors) delete[] g_shipInstanceColors;
		g_shipInstanceColors = new D3DXCOLOR[g_numShipInstances];

		SAFE_RELEASE(g_rockMatrixVB);
		V(pd3dDevice->CreateVertexBuffer(g_numRockInstances*sizeof(MatrixVertex),
			D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0,D3DPOOL_DEFAULT, &g_rockMatrixVB, NULL));

		if(g_rockInstanceMatrices) delete[] g_rockInstanceMatrices;
		g_rockInstanceMatrices = new D3DXMATRIX[g_numRockInstances];

		// reinit our managers to contain the right amount of simulated objects
		g_shipManager->Init(g_numShipInstances);
		g_asteroidManager->Init(g_numRockInstances);

		g_bReAllocateMatrixVB = false;
	}

	// update our little ships, rocks and motherships
	g_asteroidManager->Update(fElapsedTime);
	g_shipManager->Update(fElapsedTime,g_asteroidManager,g_mothershipManager);
	g_mothershipManager->Update(fElapsedTime);

	// Update our per instance matrices
	for(int i=0;i<g_numShipInstances;i++)
	{	
		D3DMATRIX matrix = g_shipManager->MakeMatrixFor(i);
		g_shipInstanceMatrices[i] = matrix;
		g_shipInstanceColors[i] = g_shipManager->GetColorFor(i);
	}
	for(int i=0;i<g_numRockInstances;i++)
	{	
		D3DMATRIX matrix = g_asteroidManager->MakeMatrixFor(i);
		g_rockInstanceMatrices[i] = matrix;
	}

}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mViewProjection;
    D3DXVECTOR3 vLightDir;
    UINT iPass, cPasses;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mViewInv;
	D3DXMATRIXA16 mIdentity;

	D3DXMatrixIdentity(&mIdentity);

	// reset our draw count
	g_numDrawCalls = 0;
    // Clear the back buffer
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER , D3DXCOLOR(0.0f,0.25f,0.25f,0.55f), 1.0f, 0) );

	// Get the projection & view matrix from the camera class
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();
	D3DXMatrixInverse(&mViewInv,0,&mView);
	mViewProjection = mView * mProj;

	// Calc light 
	
	D3DXVec3TransformNormal(&vLightDir,&D3DXVECTOR3( -0.81675f,0.1867f,0.5456f ),&mViewProjection);
	D3DXVec3Normalize(&vLightDir,&vLightDir);

	V( g_pEffect->SetValue( "g_LightDir", &vLightDir, sizeof(D3DXVECTOR3) ) );
	V( g_pEffect->SetValue( "g_LightColor", &D3DXCOLOR(1.0f,1.0f,1.0f,1.0f), sizeof(D3DXCOLOR) ) );
	V( g_pEffect->SetMatrix( "g_mViewProj", &mViewProjection ) );
	V( g_pEffect->SetMatrix( "g_mViewInverse", &mViewInv ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		// Draw a simple skybox
		g_skybox->Render(pd3dDevice,mView);
		g_numDrawCalls++;

		if(g_bUseInstancing)
		{
			// Update the instance streams with the per instance data.
			MatrixVertex* pVertices;
			V(g_shipMatrixVB->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_DISCARD));
			for(int i=0;i<g_numShipInstances;i++)
			{	
				D3DMATRIX matrix = g_shipInstanceMatrices[i];
				// since no projection, encode the 4x3 part in a 3x4(shader will decode)
				pVertices[i].r1 = D3DXVECTOR4(matrix._11,matrix._21,matrix._31,matrix._41);
				pVertices[i].r2 = D3DXVECTOR4(matrix._12,matrix._22,matrix._32,matrix._42);
				pVertices[i].r3 = D3DXVECTOR4(matrix._13,matrix._23,matrix._33,matrix._43);
				pVertices[i].c1 = g_shipInstanceColors[i];
			}
			V(g_shipMatrixVB->Unlock());

			V(g_rockMatrixVB->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_DISCARD));
			for(int i=0;i<g_numRockInstances;i++)
			{	
				D3DMATRIX matrix = g_rockInstanceMatrices[i];
				// since no projection, encode the 4x3 part in a 3x4(shader will decode)
				pVertices[i].r1 = D3DXVECTOR4(matrix._11,matrix._21,matrix._31,matrix._41);
				pVertices[i].r2 = D3DXVECTOR4(matrix._12,matrix._22,matrix._32,matrix._42);
				pVertices[i].r3 = D3DXVECTOR4(matrix._13,matrix._23,matrix._33,matrix._43);
				pVertices[i].c1 = vWhite;
			}
			V(g_rockMatrixVB->Unlock());

			V(pd3dDevice->SetVertexDeclaration(g_pMeshVertexDecl));

			// Set the first stream to be the indexed data and render N instances
			V(pd3dDevice->SetStreamSourceFreq(0,(D3DSTREAMSOURCE_INDEXEDDATA | g_numShipInstances)));

			// Set the scond stream to be per instance data and iterate once per instance
			V(pd3dDevice->SetStreamSourceFreq(1,(D3DSTREAMSOURCE_INSTANCEDATA | 1)));

			// -------------------------------------------------------------------------------
			// SPACESHIPS
			V( g_pEffect->SetTechnique( "RenderSceneInstance" ) );
			V(pd3dDevice->SetStreamSource(0, g_shipVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 0 )));
			V(pd3dDevice->SetIndices(g_shipIB));
			V(pd3dDevice->SetStreamSource(1, g_shipMatrixVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 1 )));

			// Configure the lighting constants
			V( g_pEffect->SetFloat("SpecExpon",120.0f));
			V( g_pEffect->SetFloat("Ks",0.3f));
			V( g_pEffect->SetFloat("Bumpy",1.5f));
			V( g_pEffect->SetFloat("Kd",1.0f));
			V( g_pEffect->SetTexture( "g_MeshTexture", g_pShipTexture) );
			V( g_pEffect->SetTexture( "g_MeshBumpTexture", g_pShipBumpTexture) );

			// Draw with our effect 
			V( g_pEffect->Begin(&cPasses, 0) );
			for (iPass = 0; iPass < cPasses; iPass++)
			{
				V( g_pEffect->BeginPass(iPass) );
				g_numDrawCalls++;
				V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,g_numShipVerts,0,g_numShipIndices/3));
				V( g_pEffect->EndPass() );
			}
			V( g_pEffect->End() );

			// -------------------------------------------------------------------------------
			// ROCKS
			V( g_pEffect->SetTechnique( "RenderRockInstance" ) );
			V(pd3dDevice->SetStreamSourceFreq(0,(D3DSTREAMSOURCE_INDEXEDDATA | g_numRockInstances)));
			V(pd3dDevice->SetStreamSourceFreq(1,(D3DSTREAMSOURCE_INSTANCEDATA | 1)));
			V(pd3dDevice->SetIndices(g_rockIB));
			V(pd3dDevice->SetStreamSource(0, g_rockVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 0 )));
			V(pd3dDevice->SetStreamSource(1, g_rockMatrixVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 1 )));

			// Configure the lighting constants
			V( g_pEffect->SetFloat("SpecExpon",12.0f));
			V( g_pEffect->SetFloat("Ks",0.2f));
			V( g_pEffect->SetFloat("Bumpy",0.9f));
			V( g_pEffect->SetFloat("Kd",1.0f));
			V( g_pEffect->SetTexture( "g_MeshTexture", g_pRockTexture) );
			V( g_pEffect->SetTexture( "g_MeshBumpTexture", g_pRockBumpTexture) );

			// Draw with our effect 
			V( g_pEffect->Begin(&cPasses, 0) );
			for (iPass = 0; iPass < cPasses; iPass++)
			{
				V( g_pEffect->BeginPass(iPass) );
				g_numDrawCalls++;
				V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,g_numRockVerts,0,g_numRockIndices/3));
				V( g_pEffect->EndPass() );
			}
			V( g_pEffect->End() );

			// -------------------------------------------------------------------------------
			// Reset the freq for streams or D3D will complain
			V(pd3dDevice->SetStreamSourceFreq(0,1));
			V(pd3dDevice->SetStreamSourceFreq(1,1));
		}
		else
		{
			V( g_pEffect->SetTechnique( "RenderSceneNormal" ) );
			V(pd3dDevice->SetVertexDeclaration(g_pCloneMeshVertexDecl));
			V(pd3dDevice->SetIndices(g_shipIB));
			V(pd3dDevice->SetStreamSource(0, g_shipVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 0 )));

			for(int i=0;i<g_numShipInstances;i++)
			{
				D3DXMATRIXA16 matrix = g_shipInstanceMatrices[i];
				D3DXVECTOR4 diffuseC(g_shipInstanceColors[i].r,g_shipInstanceColors[i].g,g_shipInstanceColors[i].b,1);

				V( g_pEffect->SetMatrix( "g_mWorld", &(matrix) ));
				V( g_pEffect->SetFloat("SpecExpon",120.0f));
				V( g_pEffect->SetFloat("Ks",0.3f));
				V( g_pEffect->SetFloat("Bumpy",1.5f));
				V( g_pEffect->SetFloat("Kd",1.0f));
				g_pEffect->SetVector("g_MaterialDiffuseColor",&diffuseC);

				V( g_pEffect->SetTexture( "g_MeshTexture", g_pShipTexture) );
				V( g_pEffect->SetTexture( "g_MeshBumpTexture", g_pShipBumpTexture) );

				V( g_pEffect->CommitChanges() );
				V( g_pEffect->Begin(&cPasses, 0) );
				for (iPass = 0; iPass < cPasses; iPass++)
				{
					V( g_pEffect->BeginPass(iPass) );
					g_numDrawCalls++;
					V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,g_numShipVerts,0,g_numShipIndices/3));
					V( g_pEffect->EndPass() );
				}
				V( g_pEffect->End() );
			}

			V( g_pEffect->SetTechnique( "RenderRockNormal" ) );
			V(pd3dDevice->SetIndices(g_rockIB));
			V(pd3dDevice->SetStreamSource(0, g_rockVB, 0, D3DXGetDeclVertexSize( g_aMeshVertDecl, 0 )));

			for(int i=0;i<g_numRockInstances;i++)
			{
				D3DXMATRIXA16 matrix = g_rockInstanceMatrices[i];

				V( g_pEffect->SetMatrix( "g_mWorld", &(matrix) ));
				V( g_pEffect->SetValue("g_MaterialDiffuseColor", &vWhite, sizeof(D3DXCOLOR) ) );
				V( g_pEffect->SetFloat("SpecExpon",12.0f));
				V( g_pEffect->SetFloat("Ks",0.2f));
				V( g_pEffect->SetFloat("Bumpy",0.9f));
				V( g_pEffect->SetFloat("Kd",1.0f));

				V( g_pEffect->SetTexture( "g_MeshTexture", g_pRockTexture) );
				V( g_pEffect->SetTexture( "g_MeshBumpTexture", g_pRockBumpTexture) );

				V( g_pEffect->CommitChanges() );
				V( g_pEffect->Begin(&cPasses, 0) );
				for (iPass = 0; iPass < cPasses; iPass++)
				{
					V( g_pEffect->BeginPass(iPass) );
					g_numDrawCalls++;
					V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,g_numRockVerts,0,g_numRockIndices/3));
					V( g_pEffect->EndPass() );
				}
				V( g_pEffect->End() );
			}
		}
       
        V( pd3dDevice->EndScene() );
    }

	// some mother ships for garnish
	g_numDrawCalls += g_mothershipManager->Render(pd3dDevice,g_pEffect);

	// Draw HUD
	V(pd3dDevice->SetRenderTarget(0,g_pBBSurface));
	V(pd3dDevice->SetDepthStencilSurface(g_pBBDepthSurface));
	V( pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DXCOLOR(0.0f,0.25f,0.25f,0.55f), 1.0f, 0) );
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		g_SampleUI.OnRender( fElapsedTime );

		RenderText( fTime );
		V( pd3dDevice->EndScene() );
	}
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText( double fTime )
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	if(!g_bShowStats)
	{
		txtHelper.DrawTextLine( L"F4 To enable stats.\n" );
		txtHelper.End();
		return;
	}
	else
	{
		txtHelper.DrawTextLine( DXUTGetFrameStats() );
		txtHelper.DrawTextLine( DXUTGetDeviceStats() );

		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
		txtHelper.DrawFormattedTextLine( L"Polys: %d DawCalls: %d",
			g_mothershipManager->GetNumPolysRendered()+ 
			g_numShipPolys*g_numShipInstances + 
			g_numRockPolys*g_numRockInstances, 
			g_numDrawCalls );
	}
    
    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls:" );

        txtHelper.SetInsertionPos( 250, pd3dsdBackBuffer->Height-15*5 );
        txtHelper.DrawTextLine( L"Hide help: F1.  PAUSE pauses.\n"
								L"Arrow Keys Move Camera\n"
								L"LMB Rotates Camera.\n"
                                L"Quit: ESC\n" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );

    // No toggling to REF, it's too slow.
	if(uMsg == WM_KEYDOWN && wParam == VK_F3) 
	{
		*pbNoFurtherProcessing = 0;
		return 0;
	}

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}

void UpdateMatrixVB()
{
	g_numShipInstances = max(1,g_numShipInstances);
	g_bReAllocateMatrixVB = true;
}

//--------------------------------------------------------------------------------------
// As a convenience, the sample framework inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: g_bShowHelp = !g_bShowHelp; break;
			case VK_F4: g_bShowStats = !g_bShowStats; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
       // case IDC_VIEWWHITEPAPER:   DXUTLaunchReadme( DXUTGetHWND() ); break;
		case IDC_INSTANCECOUNT:
			{
				g_numShipInstances = g_SampleUI.GetSlider( IDC_INSTANCECOUNT )->GetValue();
				WCHAR sz[100];
				_snwprintf( sz, 100, L"# Ships: %d", g_numShipInstances ); sz[99] = 0;
				g_SampleUI.GetStatic( IDC_INSTANCECOUNT_STATIC )->SetText( sz );
				UpdateMatrixVB();
			}
			break;
		case IDC_ROCKCOUNT:
			{
				g_numRockInstances = g_SampleUI.GetSlider( IDC_ROCKCOUNT )->GetValue();
				WCHAR sz[100];
				_snwprintf( sz, 100, L"# Rocks: %d", g_numRockInstances ); sz[99] = 0;
				g_SampleUI.GetStatic( IDC_ROCKCOUNT_STATIC )->SetText( sz );
				UpdateMatrixVB();
			}
			break;
		case IDC_USEINSTANCING: 
			{
				g_bUseInstancing = !g_bUseInstancing; 
			}
			break;
    }
    
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    CDXUTDirectionWidget::StaticOnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE(g_pSprite);

	SAFE_RELEASE(g_pEffect);

	SAFE_RELEASE(g_pShipTexture);
	SAFE_RELEASE(g_pShipBumpTexture);

	SAFE_RELEASE(g_pRockTexture);
	SAFE_RELEASE(g_pRockBumpTexture);

	SAFE_RELEASE(g_shipVB);
	SAFE_RELEASE(g_shipIB);
	SAFE_RELEASE(g_shipMatrixVB);

	SAFE_RELEASE(g_rockVB);
	SAFE_RELEASE(g_rockIB);
	SAFE_RELEASE(g_rockMatrixVB);

	SAFE_RELEASE(g_pSharedDepth);
	SAFE_RELEASE(g_pRenderTargetSurface);
	SAFE_RELEASE(g_pRenderTargetTexture);

	SAFE_RELEASE(g_pDownsampleTargetTexture);
	SAFE_RELEASE(g_pDownsampleTargetSurface);

	SAFE_RELEASE(g_pBlurHTargetTexture);
	SAFE_RELEASE(g_pBlurHTargetSurface);
	SAFE_RELEASE(g_pBlurVTargetTexture);
	SAFE_RELEASE(g_pBlurVTargetSurface);

	SAFE_RELEASE(g_pBBSurface);
	SAFE_RELEASE(g_pBBDepthSurface);

	g_fullScreenQuad->InvalidateDeviceObjects();
	g_mothershipManager->Destroy();
	g_skybox->Destroy();

}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();
	SAFE_RELEASE(g_pMeshVertexDecl);
	SAFE_RELEASE(g_pCloneMeshVertexDecl);
    SAFE_RELEASE(g_pFont);
}