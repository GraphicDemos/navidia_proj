
// Header files
#include "nvafx.h"
#include <shared/GetFilePath.h>
#include <fstream>
#include "Scene.h"
#include "ClothSim.h"

// Scene definition
namespace Scene
{

/*----------------------------------------------------------------------------------------------------------------------
    Objects
 ----------------------------------------------------------------------------------------------------------------------- */

// Collision object definition
class CollisionObject : public Object
{
 public:

    // Initialization
    CollisionObject(bool, bool);

    // Export
    virtual float* Export(float*) = 0;

    // Simulation
    void RemoveFromCollisionSet(int);
    bool IsInCollisionSet(int);
    void RenderWorldPosition(const D3DXMATRIX&, bool);

private:
    bool m_IsInCollisionSet[CLOTH_NUM];
};

// Plane definition
class Plane : public CollisionObject
{

public:

    // Initialization
    Plane(const D3DXVECTOR3&, float, bool, bool);

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    void OnDestroyDevice();

    // Dimension
    void SetDimension(const D3DXVECTOR3&) { }

    // Export
    float* Export(float*);

    // Instance number
    static int Num() { return m_Num; }

private:
    static LPD3DXMESH m_Mesh;
    static int m_Num;
    virtual void ComputeThinningMatrix(float, D3DXMATRIX& mat) { D3DXMatrixIdentity(&mat); }
};
LPD3DXMESH Plane::m_Mesh;
int Plane::m_Num;

class Sphere : public CollisionObject
{

public:

    // Initialization
    Sphere(const D3DXVECTOR3&, float, bool, bool);

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    void OnDestroyDevice();

    // Dimension
    void Sphere::SetDimension(const D3DXVECTOR3&);

    // Export
    float* Export(float*);

    // Instance number
    static int Num() { return m_Num; }

private:
    static LPD3DXMESH m_Mesh;
    static int m_Num;
};
LPD3DXMESH Sphere::m_Mesh;
int Sphere::m_Num;

class Box : public CollisionObject
{

public:

    // Initialization
    Box(const D3DXVECTOR3&, const D3DXVECTOR3&, bool, bool);

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    void OnDestroyDevice();

    // Export
    float* Export(float*);

    // Instance number
    static int Num() { return m_Num; }

private:
    static LPD3DXMESH m_Mesh;
    static int m_Num;
};
LPD3DXMESH Box::m_Mesh;
int Box::m_Num;

class Ellipsoid : public CollisionObject
{

public:

    // Initialization
    Ellipsoid(const D3DXVECTOR3&, const D3DXVECTOR3&, bool, bool);

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    void OnDestroyDevice();

    // Export
    float* Export(float*);

    // Instance number
    static int Num() { return m_Num; }

private:
    static LPD3DXMESH m_Mesh;
    static int m_Num;
};
LPD3DXMESH Ellipsoid::m_Mesh;
int Ellipsoid::m_Num;

class Cloth : public Object
{

public:

    // Initialization
    Cloth(const D3DXVECTOR3&, const D3DXVECTOR3&, const D3DXVECTOR3&, const wchar_t*, int, int);
    ~ Cloth();

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    HRESULT OnResetDevice(IDirect3DDevice9*);
    void OnLostDevice();
    void OnDestroyDevice();

    // Size
    void SetSize(int, int);
    int Width() const { return m_ClothSim->Width(); }
    int Height() const { return m_ClothSim->Height(); }

    // Position and orientation
    const D3DXMATRIX& World();
    void SetWorld(const D3DXMATRIX&);

    // Simulation
    void Reset();
    void SetSelectionFree(bool);
    void Simulate(float, float, float);
    void Interpolate(float interpolationFactor) { m_ClothSim->Interpolate(interpolationFactor); }
    void SetRelaxationIterations(int relaxationIterations) { m_ClothSim->SetRelaxationIterations(relaxationIterations); }
    int RelaxationIterations() { return m_ClothSim->RelaxationIterations(); }
    void SetShearConstraint(bool shearConstraint) { m_ClothSim->SetShearConstraint(shearConstraint); }
    bool ShearConstraint() { return m_ClothSim->ShearConstraint(); }
    void SetGravityStrength(float gravityStrength) { m_ClothSim->SetGravityStrength(gravityStrength); }
    float GravityStrength() { return m_ClothSim->GravityStrength(); }
    void SetWindStrength(float windStrength) { m_ClothSim->SetWindStrength(windStrength); }
    float WindStrength() { return m_ClothSim->WindStrength(); }
    void SetWindHeading(float windHeading) { m_ClothSim->SetWindHeading(windHeading); }
    float WindHeading() { return m_ClothSim->WindHeading(); }
    float* BeginPlaneList();
    void EndPlaneList(int);
    float* BeginSphereList();
    void EndSphereList(int);
    float* BeginBoxList();
    void EndBoxList(int);
    float* BeginEllipsoidList();
    void EndEllipsoidList(int);

    // Rendering
    void Render(ID3DXEffect&, const D3DXMATRIX&, bool);
    void RenderID(const D3DXMATRIX&);
    void RenderSelection(const D3DXMATRIX&);
    void RenderNormals(const D3DXMATRIX&);

    // Selection
    void Target(float, float);
    void Select(float, float, bool);
    void SetAnchorPosition(int, const D3DXVECTOR3&);
    void UpdateAnchorPositionBuffer();

    // Cutting
    void Cut(D3DXVECTOR3[3]);
    void Uncut();

private:

    // Device management
    HRESULT CreateBuffers();
    void DestroyBuffers();

    // Position and orientation
    bool m_WorldIsValid;

    // Simulation
    bool m_ResetSimulation;
    bool m_SelectionIsFree;
    ClothSim* m_ClothSim;
    LPDIRECT3DTEXTURE9 m_PlaneListTexture;
    int m_PlaneNum;
    LPDIRECT3DTEXTURE9 m_SphereListTexture;
    int m_SphereNum;
    LPDIRECT3DTEXTURE9 m_BoxListTexture;
    int m_BoxNum;
    LPDIRECT3DTEXTURE9 m_EllipsoidListTexture;
    int m_EllipsoidNum;

    // Rendering
    struct Vertex
    {
        static const DWORD FVF = (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
        D3DXVECTOR2 m_TexCoord;
    };
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;
    LPDIRECT3DINDEXBUFFER9 m_IndexBuffer;
    void FillIndexBuffer();
    int m_TriangleNum;
    int m_SwathWidth;

    // Normal rendering
    struct Normal
    {
        static const DWORD FVF = (D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE1(1));
        D3DXVECTOR2 m_TexCoord;
        float m_Length;
    };
    LPDIRECT3DVERTEXBUFFER9 m_NormalBuffer;

    // Selection
    struct AnchorPosition
    {
        unsigned int m_Count;
        int m_ID;
        D3DXVECTOR2 m_XY;
        D3DXVECTOR3 m_Color;
        D3DXVECTOR3 m_Position;
        bool m_PositionIsValid;
    };
    std::list<AnchorPosition> m_AnchorPositionList;
    bool m_AnchorPositionListIsValid;
    bool m_AnchorPositionListHasChanged;
    bool m_AnchorPositionListHasMoved;
    unsigned int m_AnchorPositionCount;
    D3DXMATRIX m_Transform;
    bool m_TransformIsValid;
    std::list<AnchorPosition>::iterator m_SelectedPositionListBegin;
    LPDIRECT3DVERTEXBUFFER9 m_AnchorPositionBuffer;
    LPDIRECT3DVERTEXBUFFER9 m_PositionBuffer;
    int m_TargetID;
    static LPDIRECT3DTEXTURE9 m_CenterTexture, m_CenterTextureSysMem;
    static LPDIRECT3DSURFACE9 m_CenterSurface, m_CenterSurfaceSysMem;
    void UpdateWorld();
};
LPDIRECT3DTEXTURE9 Cloth::m_CenterTexture;
LPDIRECT3DTEXTURE9 Cloth::m_CenterTextureSysMem;
LPDIRECT3DSURFACE9 Cloth::m_CenterSurface;
LPDIRECT3DSURFACE9 Cloth::m_CenterSurfaceSysMem;

// Object list
std::list<Object *> g_ObjectList;
void SortList();

// ID counter
static int g_ID = 1;

#define OBJECT_TEXTURE_SIZE 64

// Plane list and texture
#define PLANE_SIZE  4
#define MAX_PLANE   ((OBJECT_TEXTURE_SIZE * sizeof(D3DXVECTOR4)) / PLANE_SIZE)
static std::list<Object *>::iterator g_PlaneListBegin = g_ObjectList.end();
static void UpdatePlaneTexture();

// Sphere list and texture
#define SPHERE_SIZE 4
#define MAX_SPHERE  ((OBJECT_TEXTURE_SIZE * sizeof(D3DXVECTOR4)) / SPHERE_SIZE)
static std::list<Object *>::iterator g_SphereListBegin = g_ObjectList.end();
static void UpdateSphereTexture();

// Boxe list and texture
#define BOX_SIZE    (3 * 4)
#define MAX_BOX     ((OBJECT_TEXTURE_SIZE * sizeof(D3DXVECTOR4)) / BOX_SIZE)
static std::list<Object *>::iterator g_BoxListBegin = g_ObjectList.end();
static void UpdateBoxTexture();

// Ellipsoid list and texture
#define ELLIPSOID_SIZE  (3 * 4)
#define MAX_ELLIPSOID   ((OBJECT_TEXTURE_SIZE * sizeof(D3DXVECTOR4)) / ELLIPSOID_SIZE)
static std::list<Object *>::iterator g_EllipsoidListBegin = g_ObjectList.end();
static void UpdateEllipsoidTexture();

// Cloth list and texture
static std::list<Object *>::iterator g_ClothListBegin = g_ObjectList.end();
static Cloth* AddCloth(IDirect3DDevice9*, const D3DXVECTOR3&, const D3DXVECTOR3&, const D3DXVECTOR3&, const wchar_t* filename = 0,
                       int = 32, int = 32);

// Effects
static LPD3DXEFFECT g_SceneEffect;
static LPD3DXEFFECT g_GUIEffect;
static D3DXVECTOR3 g_ColorNotSelectable(0.6f, 0.6f, 1);
static D3DXVECTOR3 g_ColorNotSelected(1, 1, 1);
static D3DXVECTOR3 g_ColorSelected(1, 0, 0);
static D3DXVECTOR3 g_ColorSelectedFixed(0, 0, 1);
static D3DXVECTOR3 g_ColorSelectedMouseOver(1, 0.75f, 0);
static D3DXVECTOR3 g_ColorSelectedFixedMouseOver(0.8f, 0, 1);
static D3DXVECTOR3 g_ColorMouseOver(0.7f, 1, 0.5f);

// Cell
static Plane* g_Floor;
static LPDIRECT3DTEXTURE9 g_FloorColorTexture;
static LPDIRECT3DTEXTURE9 g_FloorNormalMap;
static Plane* g_Wall[4];
static LPDIRECT3DTEXTURE9 g_WallColorTexture;
static LPDIRECT3DTEXTURE9 g_WallNormalMap;
static Plane* g_Ceiling;
static LPDIRECT3DTEXTURE9 g_CeilingColorTexture;
static LPDIRECT3DTEXTURE9 g_CeilingNormalMap;

// Clothes
#define CELL_SIZE       7.0f
#define SCENE_RADIUS    (2 * (CELL_SIZE - 1) / 3)
#define SCENE_HEIGHT    (-2.0f)
Object* g_Cloth[CLOTH_NUM];
bool g_RenderScene[CLOTH_NUM];
D3DXVECTOR3 g_SceneCenter[CLOTH_NUM] = {
    D3DXVECTOR3(0, SCENE_HEIGHT, -SCENE_RADIUS), 
    D3DXVECTOR3(-sqrtf(3) / 2 * SCENE_RADIUS, SCENE_HEIGHT + 1, SCENE_RADIUS / 2), 
    D3DXVECTOR3(sqrtf(3) / 2 * SCENE_RADIUS, SCENE_HEIGHT + 0.5f, SCENE_RADIUS / 2), 
    D3DXVECTOR3(sqrtf(3) / 2 * SCENE_RADIUS, SCENE_HEIGHT + 0.5f, SCENE_RADIUS / 2)
};
static D3DXVECTOR3 g_Center[CLOTH_NUM] = {
    g_SceneCenter[CLOTH_CURTAIN] + D3DXVECTOR3(0, 1.5f, 0), 
    g_SceneCenter[CLOTH_FLAG], 
    g_SceneCenter[CLOTH_CAPE] + D3DXVECTOR3(1, 0, 0), 
    g_SceneCenter[CLOTH_SKIRT] + D3DXVECTOR3(1, 0, 0)
};
struct BoneAttachment
{
    char m_BoneName[256];
    D3DXVECTOR3 m_Position;
};
std::list<BoneAttachment> g_SkirtBoneAttachments;

// Cloth textures
static LPWSTR g_ColorTextureFilename[] = {
	TEXT("MEDIA/textures/2D/Cloth0.dds"),
	TEXT("MEDIA/textures/2D/nvlogo.tga"),
	TEXT("MEDIA/textures/2D/Cloth1.dds"),
	TEXT("MEDIA/textures/2D/Cloth2.dds")
};
static LPWSTR g_NormalMapFilename[] = {
    TEXT("MEDIA/textures/2D/Cloth0NormalMap.dds"),
    TEXT(" "),
	TEXT("MEDIA/textures/2D/Cloth1NormalMap.dds"),
	TEXT("MEDIA/textures/2D/Cloth2NormalMap.dds")
};
static float g_TexCoordScaling[] = { 8, 1, 8, 8 };
static float g_DiffuseCoeff[] = { 1.8f, 1, 1.8f, 2 };
static float g_SpecularCoeff[] = { 0.1f, 1, 0.1f, 0.1f };
static LPDIRECT3DTEXTURE9 g_ColorTexture[CLOTH_NUM];
static LPDIRECT3DTEXTURE9 g_NormalMap[CLOTH_NUM];

// Character
static D3DXFRAME* g_Character;
static D3DXMATRIXA16* g_CharacterBoneMatrices;
static ID3DXAnimationController* g_CharacterController;
static HRESULT CreateCharacter(IDirect3DDevice9*, D3DXFRAME *&, ID3DXAnimationController *&);
static void DestroyCharacter(D3DXFRAME *&, ID3DXAnimationController *&);
static void UpdateCharacter(float, D3DXFRAME*, ID3DXAnimationController*);
static void RenderCharacter(IDirect3DDevice9*, LPD3DXFRAME, ID3DXEffect&, bool);

// Rendering
float g_Thinning = -0.065f;

// Environment
bool g_MoveEnvironmentOnly;
static bool g_EnvironmentHasChanged;

/*----------------------------------------------------------------------------------------------------------------------
    Initialization and cleanup
 ----------------------------------------------------------------------------------------------------------------------- */

void Initialize()
{

    // Default environment
    g_Wall[0] = AddPlane(0, D3DXVECTOR3(-1, 0, 0), CELL_SIZE, true, false);
    g_Wall[1] = AddPlane(0, D3DXVECTOR3(1, 0, 0), CELL_SIZE, true, false);
    g_Ceiling = AddPlane(0, D3DXVECTOR3(0, -1, 0), CELL_SIZE / 2, true, false);
    g_Floor = AddPlane(0, D3DXVECTOR3(0, 1, 0), CELL_SIZE / 2, true, false);
    g_Wall[2] = AddPlane(0, D3DXVECTOR3(0, 0, -1), CELL_SIZE, true, false);
    g_Wall[3] = AddPlane(0, D3DXVECTOR3(0, 0, 1), CELL_SIZE, true, false);
    AddSphere(0, g_SceneCenter[CLOTH_CURTAIN] + D3DXVECTOR3(0, -0.5f, -0.75f), 1);
    AddEllipsoid(0, g_SceneCenter[CLOTH_CURTAIN], D3DXVECTOR3(2, 0.8f, 1.1f));
    AddBox(0, g_SceneCenter[CLOTH_CURTAIN] + D3DXVECTOR3(0, 1.25f, 1), D3DXVECTOR3(2, 0.5f, 0.5f));

    // Clothes
    g_Cloth[CLOTH_CURTAIN] = AddCloth(0, g_Center[CLOTH_CURTAIN], D3DXVECTOR3(1, 0, 0), D3DXVECTOR3(0, 0, 1));
    g_Cloth[CLOTH_CURTAIN]->SetGravityStrength(2.5f);
    g_Cloth[CLOTH_FLAG] = AddCloth(0, g_Center[CLOTH_FLAG], D3DXVECTOR3(1, 0, 0), D3DXVECTOR3(0, -1, 0));
    g_Cloth[CLOTH_FLAG]->SetWindStrength(5);
    g_Cloth[CLOTH_FLAG]->SetWindHeading(D3DXToRadian(40));
    tstring filename = GetFilePath::GetFilePath(TEXT("MEDIA\\textures\\GIm\\Skirt.pfm"), true);
    g_Center[CLOTH_SKIRT] = g_SceneCenter[CLOTH_SKIRT] + D3DXVECTOR3(1.02f, -0.35f, -0.03f);
    g_Cloth[CLOTH_SKIRT] = AddCloth(0, g_Center[CLOTH_SKIRT], D3DXVECTOR3(0, 0, 0.5f), D3DXVECTOR3(0, -1, 0), filename.c_str());
    g_Cloth[CLOTH_SKIRT]->SetGravityStrength(7.5f);
    g_Cloth[CLOTH_CAPE] = AddCloth(0, g_Center[CLOTH_CAPE], D3DXVECTOR3(0, 0, 0.5f), D3DXVECTOR3(0, -1, 0), 0, 26, 14);
    g_Cloth[CLOTH_CAPE]->SetGravityStrength(3.3f);
}

void Cleanup()
{
    for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o) {
        (*o)->OnLostDevice();
        (*o)->OnDestroyDevice();
        delete *o;
    }
    g_ObjectList.clear();
}

/*----------------------------------------------------------------------------------------------------------------------
    Device management
 ----------------------------------------------------------------------------------------------------------------------- */

HRESULT OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;

    // Cell textures
    if (g_Floor) {
        V_RETURN(D3DXCreateTextureFromFile(device,
                GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/cellfloor.jpg"), true).c_str(), &g_FloorColorTexture));
        g_Floor->SetColorTexture(g_FloorColorTexture);
    }
    V_RETURN(D3DXCreateTextureFromFile(device,
             GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/cellwall.jpg"), true).c_str(), &g_WallColorTexture));
    for (int i = 0; i < 4; i++)
        if (g_Wall[i])
            g_Wall[i]->SetColorTexture(g_WallColorTexture);
    if (g_Ceiling) {
        V_RETURN(D3DXCreateTextureFromFile(device,
                GetFilePath::GetFilePath(TEXT("MEDIA/textures/2D/cellceiling.jpg"), true).c_str(), &g_CeilingColorTexture));
        g_Ceiling->SetColorTexture(g_FloorColorTexture);
    }

    // Cloth render parameters
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c]) {
            V_RETURN(D3DXCreateTextureFromFile(device, GetFilePath::GetFilePath(g_ColorTextureFilename[c], true).c_str(),
                    &g_ColorTexture[c]));
            g_Cloth[c]->SetColorTexture(g_ColorTexture[c], g_TexCoordScaling[c]);
            if (c != 1) {
                V_RETURN(D3DXCreateTextureFromFile(device, GetFilePath::GetFilePath(g_NormalMapFilename[c], true).c_str(),
                        &g_NormalMap[c]));
                g_Cloth[c]->SetNormalMap(g_NormalMap[c]);
            }
            g_Cloth[c]->SetDiffuseCoeff(g_DiffuseCoeff[c]);
            g_Cloth[c]->SetSpecularity(g_SpecularCoeff[c]);
        }

    // Character
    if (g_Cloth[CLOTH_CAPE] || g_Cloth[CLOTH_SKIRT])
        V_RETURN(CreateCharacter(device, g_Character, g_CharacterController));

    // Effects
    static char g_GUIEffectStr[] =
        "int RTWidth;"
        "int RTHeight;"
        "int Dx;"
        "int Dy;"
        "float Epsilon = 0.1;"
        "float4x4 WorldViewProjection;"
        "float ObjectID;"
        "float4 RenderGlobalIDVS(float4 position : POSITION) : POSITION { return mul(position, WorldViewProjection); }"
        "float4 RenderGlobalIDPS() : COLOR { return float4(0, 0, 0, ObjectID); }"
        "technique RenderGlobalID {"
        "    pass {"
        "        VertexShader = compile vs_2_0 RenderGlobalIDVS();"
        "        PixelShader  = compile ps_2_0 RenderGlobalIDPS();"
        "    }"
        "}"
        "float4x4 ViewProjection;"
        "texture PositionTexture;"
        "sampler PositionSampler = sampler_state {"
        "    Texture = <PositionTexture>;"
        "    AddressU = CLAMP;"
        "    AddressV = CLAMP;"
        "    MinFilter = POINT;"
        "    MagFilter = POINT;"
        "};"
        "void RenderVertexIDVS(float2 texCoord : TEXCOORD0, out float4 position : POSITION, out float4 ID : TEXCOORD0)"
        "{"
        "    ID = float4((1 - Dx) * texCoord.x, (1 - Dy) * texCoord.y, 0, ObjectID);"
        "    float4 coord = float4(ID.x + 0.5 / RTWidth, ID.y + 0.5 / RTHeight, 0, 0);"
        "    position = mul(float4(tex2Dlod(PositionSampler, coord).xyz, 1), ViewProjection);"
        "}"
        "float4 RenderVertexIDPS(float4 ID : TEXCOORD0) : COLOR { return ID; }"
        "technique RenderVertexID {"
        "    pass {"
        "        CullMode = None;"
        "        VertexShader = compile vs_3_0 RenderVertexIDVS();"
        "        PixelShader  = compile ps_3_0 RenderVertexIDPS();"
        "    }"
        "}"
        "float2 PositionToTexCoord(float2 position)"
        "{"
        "    return float2(0.5 * (1 + position.x) + Epsilon / RTWidth, 0.5 * (1 - position.y) + Epsilon / RTHeight);"
        "}"
        "void RenderVS(float2 XY : TEXCOORD0, inout float4 color : TEXCOORD1, out float4 position : POSITION)"
        "{"
        "    float4 coord = float4(PositionToTexCoord(XY), 0, 0);"
        "    position = mul(float4(tex2Dlod(PositionSampler, coord).xyz, 1), ViewProjection);"
        "}"
        "float4 RenderPS(float4 color : TEXCOORD1) : COLOR { return color; }"
        "technique Render {"
        "    pass {"
        "        VertexShader = compile vs_3_0 RenderVS();"
        "        PixelShader = compile ps_3_0 RenderPS();"
        "    }"
        "}"
        "float3 Color;"
        "texture NormalTexture;"
        "sampler NormalSampler = sampler_state {"
        "    Texture = <NormalTexture>;"
        "    AddressU = CLAMP;"
        "    AddressV = CLAMP;"
        "    MinFilter = POINT;"
        "    MagFilter = POINT;"
        "};"
        "void RenderNormalVS(float2 texCoord : TEXCOORD0, float length : TEXCOORD1, out float4 position : POSITION)"
        "{"
        "    float4 coord = float4(texCoord.x + 0.5 * Dx, texCoord.y + 0.5 * Dy, 0, 0);"
        "    position = float4(tex2Dlod(PositionSampler, coord).xyz, 1);"
        "    float3 normal = tex2Dlod(NormalSampler, coord).xyz;"
        "    position.xyz += length * normal;"
        "    position = mul(position, ViewProjection);"
        "}"
        "float4 RenderNormalPS() : COLOR { return float4(Color, 1); }"
        "technique RenderNormal {"
        "    pass {"
        "        VertexShader = compile vs_3_0 RenderNormalVS();"
        "        PixelShader = compile ps_3_0 RenderNormalPS();"
        "    }"
        "}"
        "float4x4 World;"
        "float4 RenderWorldPositionVS(float4 position : POSITION, out float4 worldPosition : TEXCOORD0) : POSITION"
        "{"
        "    worldPosition = mul(position, World);"
        "    return mul(position, WorldViewProjection);"
        "}"
        "float4 RenderWorldPositionPS(float4 worldPosition : TEXCOORD0) : COLOR { return worldPosition; }"
        "technique RenderWorldPosition {"
        "    pass {"
        "        VertexShader = compile vs_3_0 RenderWorldPositionVS();"
        "        PixelShader  = compile ps_3_0 RenderWorldPositionPS();"
        "    }"
        "}"
        "static const int MAX_MATRICES = 26;"
        "float4x3 BoneWorld[MAX_MATRICES] : WORLDMATRIXARRAY;"
        "int NumBones;"
        "float4 RenderWorldPositionSkinnedVS(float4 position : POSITION, float4 blendWeights : BLENDWEIGHT, float4 blendIndices : BLENDINDICES, out float4 worldPosition : TEXCOORD0) : POSITION"
        "{"
        "    int4 indexVector = D3DCOLORtoUBYTE4(blendIndices);"
        "    float blendWeightArray[4] = (float[4])blendWeights;"
        "    int indexArray[4] = (int[4])indexVector;"
        "    float lastWeight = 0;"
        "    float3 skinPosition = 0;"
        "    for (int bone = 0; bone < NumBones - 1; ++bone) {"
        "        lastWeight += blendWeightArray[bone];"
        "        skinPosition += mul(position, BoneWorld[indexArray[bone]]) * blendWeightArray[bone];"
        "    }"
        "    lastWeight = 1 - lastWeight;"
        "    skinPosition += (mul(position, BoneWorld[indexArray[NumBones - 1]]) * lastWeight);"
        "    worldPosition = float4(skinPosition, 1);"
        "    return mul(float4(skinPosition.xyz, 1), ViewProjection);"
        "}"
        "technique RenderWorldPositionSkinned {"
        "    pass {"
        "        VertexShader = compile vs_3_0 RenderWorldPositionSkinnedVS();"
        "        PixelShader  = compile ps_3_0 RenderWorldPositionPS();"
        "    }"
        "}"
        "int N;"
        "void ComputeCenterVS(float2 XY : TEXCOORD0, float4 color : TEXCOORD1, out float4 position : POSITION, out float4 value : TEXCOORD0)"
        "{"
        "    position = float4(0, 0, 0, 1);"
        "    float4 coord = float4(PositionToTexCoord(XY), 0, 0);"
        "    value = float4(tex2Dlod(PositionSampler, coord).xyz, 1);"
        "}"
        "float4 ComputeCenterPS(float4 value : TEXCOORD0) : COLOR { return value / N; }"
        "technique ComputeCenter {"
        "    pass {"
        "        ZEnable = false;"
        "        ZWriteEnable = false;"
        "        AlphaBlendEnable = true;"
        "        BlendOp = Add;"
        "        SrcBlend = One;"
        "        DestBlend = One;"
        "        VertexShader = compile vs_3_0 ComputeCenterVS();"
        "        PixelShader = compile ps_3_0 ComputeCenterPS();"
        "    }"
        "}"
        ;
    V_RETURN(D3DXCreateEffect(device, g_GUIEffectStr, sizeof(g_GUIEffectStr), 0, 0, D3DXSHADER_DEBUG, 0, &g_GUIEffect, 0));
    V_RETURN(D3DXCreateEffectFromFile(device, GetFilePath::GetFilePath(TEXT("MEDIA/programs/Cloth/Scene.fx"), true).c_str(),
             0, 0, D3DXSHADER_DEBUG, 0, &g_SceneEffect, 0));

    // Objects
	for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o)
	{
		(*o)->OnCreateDevice(device);
	}

    SortList();

    // Cloth anchor points
    if (g_Cloth[CLOTH_CURTAIN]) {
        g_Cloth[CLOTH_CURTAIN]->Select(0, 0);
        g_Cloth[CLOTH_CURTAIN]->Select(0.5f, 0);
        g_Cloth[CLOTH_CURTAIN]->Select(1 - 1.0f / g_Cloth[CLOTH_CURTAIN]->Width(), 0);
    }
    if (g_Cloth[CLOTH_FLAG]) {
        g_Cloth[CLOTH_FLAG]->Select(0, 0, true);
        g_Cloth[CLOTH_FLAG]->Select(0, 1 / 3.0f, true);
        g_Cloth[CLOTH_FLAG]->Select(0, 2 / 3.0f, true);
        g_Cloth[CLOTH_FLAG]->Select(0, 1 - 1.0f / g_Cloth[CLOTH_FLAG]->Height(), true);
    }
    if (g_Cloth[CLOTH_SKIRT]) {
        BoneAttachment attach;
        int n = g_Cloth[CLOTH_SKIRT]->Width();
        for (int i = 0; i < n; ++i) {
            float u = i / static_cast<float>(n);
            sprintf(attach.m_BoneName, "Bip01_Spine");
            attach.m_Position[0] = 23;
            attach.m_Position[1] = 28 * cosf(2 * D3DX_PI * u) - 3;
            attach.m_Position[2] = 40 * sinf(2 * D3DX_PI * u);
            g_SkirtBoneAttachments.push_back(attach);
            g_Cloth[CLOTH_SKIRT]->Select(u, 1 - 1.0f / g_Cloth[CLOTH_SKIRT]->Height(), true);
        }
    }
    if (g_Cloth[CLOTH_CAPE]) {
        g_Cloth[CLOTH_CAPE]->Select(0, 0, true);
        g_Cloth[CLOTH_CAPE]->Select(0.25f, 0, true);
        g_Cloth[CLOTH_CAPE]->Select(0.5f, 0, true);
        g_Cloth[CLOTH_CAPE]->Select(0.75f, 0, true);
        g_Cloth[CLOTH_CAPE]->Select(1 - 1.0f / g_Cloth[CLOTH_CAPE]->Width(), 0, true);
    }

    return S_OK;
}

void OnDestroyDevice()
{
    for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o)
        (*o)->OnDestroyDevice();
    SAFE_RELEASE(g_SceneEffect);
    SAFE_RELEASE(g_GUIEffect);
    DestroyCharacter(g_Character, g_CharacterController);
    for (int c = 0; c < CLOTH_NUM; ++c) {
        SAFE_RELEASE(g_ColorTexture[c]);
        SAFE_RELEASE(g_NormalMap[c]);
    }
    SAFE_RELEASE(g_CeilingColorTexture);
    SAFE_RELEASE(g_WallColorTexture);
    SAFE_RELEASE(g_FloorColorTexture);
}

HRESULT OnResetDevice(IDirect3DDevice9* device)
{
    // Effects
    if (g_GUIEffect)
        g_GUIEffect->OnResetDevice();
    if (g_SceneEffect)
        g_SceneEffect->OnResetDevice();

    // Objects
    for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o)
        (*o)->OnResetDevice(device);

    // Environment collision textures
    UpdatePlaneTexture();
    UpdateSphereTexture();
    UpdateBoxTexture();
    UpdateEllipsoidTexture();

    return S_OK;
}

void OnLostDevice()
{
    if (g_GUIEffect)
        g_GUIEffect->OnLostDevice();
    if (g_SceneEffect)
        g_SceneEffect->OnLostDevice();
    for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o)
        (*o)->OnLostDevice();
}

/*----------------------------------------------------------------------------------------------------------------------
    Animation
 ----------------------------------------------------------------------------------------------------------------------- */

void Animate(float timeStep)
{
    // Character
    if (g_Character && (g_RenderScene[CLOTH_CAPE] || g_RenderScene[CLOTH_SKIRT]))
        UpdateCharacter(timeStep, g_Character, g_CharacterController);
}

/*----------------------------------------------------------------------------------------------------------------------
    Simulation
 ----------------------------------------------------------------------------------------------------------------------- */

void Reset()
{
    if (g_Character)
        UpdateCharacter(-1, g_Character, g_CharacterController);
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c] && g_RenderScene[c])
            g_Cloth[c]->Reset();
}

void SetSelectionFree(bool value)
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c])
            g_Cloth[c]->SetSelectionFree(value);
}

void Simulate(float time, float timeStep, float oldTimeStep)
{

    // Update collision objects
    if (g_EnvironmentHasChanged) {
        UpdatePlaneTexture();
        UpdateSphereTexture();
        UpdateBoxTexture();
        UpdateEllipsoidTexture();
        g_EnvironmentHasChanged = false;
    }

    // Simulate clothes
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c] && g_RenderScene[c])
            g_Cloth[c]->Simulate(time, timeStep, oldTimeStep);
}

void Interpolate(float interpolationFactor)
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c] && g_RenderScene[c])
            g_Cloth[c]->Interpolate(interpolationFactor);
}

void Cut(D3DXVECTOR3 cutter[3])
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c] && g_RenderScene[c])
            g_Cloth[c]->Cut(cutter);
}

/*----------------------------------------------------------------------------------------------------------------------
    Rendering
 ----------------------------------------------------------------------------------------------------------------------- */

void Render(IDirect3DDevice9* device, const D3DXMATRIX& view, const D3DXMATRIX& viewProjection, bool wireframeClothes,
            bool wireframeEnvironment, bool showNormals, bool showSelection, bool showModels, bool showInvisibleObjects)
{

    // Transform matrices
    D3DXMATRIX viewI;
    D3DXMatrixInverse(&viewI, 0, &view);
    g_SceneEffect->SetFloatArray("CameraPosition", D3DXVECTOR3(viewI(3, 0), viewI(3, 1), viewI(3, 2)), 3);
    g_SceneEffect->SetMatrix("ViewProjection", &viewProjection);

    // Drawing mode
    bool wireframe = wireframeEnvironment;

    // Character
    if (g_Character && (g_RenderScene[CLOTH_CAPE] || g_RenderScene[CLOTH_SKIRT]) && showModels) {
        g_SceneEffect->SetTechnique("Skinned");
        g_SceneEffect->SetBool("ColorOnly", false);
        g_SceneEffect->SetFloat("TexCoordScaling", 1);
        device->SetRenderState(D3DRS_FILLMODE, wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
        RenderCharacter(device, g_Character, *g_SceneEffect, true);
    }

    // Environment, then clothes
    bool renderingClothes = false;
    for (std::list < Object * >::const_iterator o = g_ObjectList.begin(); o != g_ObjectList.end(); ++o) {

        // Check if we start rendering the clothes
        if (o == g_ClothListBegin) {
            renderingClothes = true;
            wireframe = wireframeClothes;
        }

        // Skip disabled clothes
		int c = 0;
        for (c = 0; c < CLOTH_NUM; ++c)
            if (*o == g_Cloth[c] && !g_RenderScene[c])
                break;
        if (c < CLOTH_NUM)
            continue;

        // Render model
        g_SceneEffect->SetBool("ColorOnly", wireframe);
        D3DXVECTOR3 color;
        if (renderingClothes)
            color = D3DXVECTOR3(1, 1, 1);
        else if (!(*o)->IsSelectable())
            color = g_ColorNotSelectable;
        else if (showSelection) {
            if ((*o)->IsTargeted())
                if ((*o)->IsSelected())
                    color = g_ColorSelectedMouseOver;
                else
                    color = g_ColorMouseOver;
            else if ((*o)->IsSelected())
                color = g_ColorSelected;
            else
                color = g_ColorNotSelected;
        }
        else
            color = g_ColorNotSelected;
        g_SceneEffect->SetFloat("TexCoordScaling", 1);
        g_SceneEffect->SetFloat("DiffuseCoeff", (*o)->DiffuseCoeff());
        g_SceneEffect->SetFloat("SpecularCoeff", (*o)->SpecularCoeff());
        g_SceneEffect->SetFloat("SpecularPower", (*o)->SpecularPower());
        g_SceneEffect->SetFloatArray("Color", color, 3);
        g_SceneEffect->SetMatrix("World", &(*o)->World());
        D3DXMATRIX worldI;
        D3DXMatrixInverse(&worldI, 0, &(*o)->World());
        D3DXMATRIX worldIT;
        D3DXMatrixTranspose(&worldIT, &worldI);
        g_SceneEffect->SetMatrix("WorldIT", &worldIT);
        if ((*o)->ColorTexture()) {
            g_SceneEffect->SetTexture("ColorTexture", (*o)->ColorTexture());
            if ((*o)->NormalMap()) {
                g_SceneEffect->SetTexture("NormalMap", (*o)->NormalMap());
                g_SceneEffect->SetTechnique("TexturedBumpMapped");
            }
            else
                g_SceneEffect->SetTechnique("Textured");
        }
        else
            g_SceneEffect->SetTechnique("Shaded");
        device->SetRenderState(D3DRS_FILLMODE, wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
        (*o)->Render(*g_SceneEffect, viewProjection, showInvisibleObjects);

        // Render normals
        if (showNormals)
            (*o)->RenderNormals(viewProjection);

        // Render selection
        if (showSelection)
            (*o)->RenderSelection(viewProjection);
    }
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

void RenderID(const D3DXMATRIX& viewProjection)
{
    for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin(); o != Scene::g_ObjectList.end();
         ++o) {

        // Skip disabled clothes
        for (int c = 0; c < CLOTH_NUM; ++c)
            if (*o == g_Cloth[c] && !g_RenderScene[c])
                continue;
        (*o)->RenderID(viewProjection);
    }
}

void RenderWorldPosition(IDirect3DDevice9* device, const D3DXMATRIX& viewProjection, bool showModels, bool showInvisibleObjects)
{
    // Character
    if (g_Character && (g_RenderScene[CLOTH_CAPE] || g_RenderScene[CLOTH_SKIRT]) && showModels) {
        g_GUIEffect->SetTechnique("RenderWorldPositionSkinned");
        g_GUIEffect->SetMatrix("ViewProjection", &viewProjection);
        RenderCharacter(device, g_Character, *g_GUIEffect, false);
    }

    // Objects
    for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin(); o != Scene::g_ObjectList.end();
         ++o)
        (*o)->RenderWorldPosition(viewProjection, showInvisibleObjects);
}

/*----------------------------------------------------------------------------------------------------------------------
    Object list edition
 ----------------------------------------------------------------------------------------------------------------------- */

Plane* AddPlane(IDirect3DDevice9* device, const D3DXVECTOR3& center, float dist, bool isVisible, bool isSelectable)
{
    if (Plane::Num() == MAX_PLANE)
        return 0;
    Plane* plane = new Plane(center, dist, isVisible, isSelectable);
    plane->SetID(g_ID);
    ++g_ID;
    g_PlaneListBegin = g_ObjectList.insert(g_PlaneListBegin, plane);
    if (device)
        plane->OnCreateDevice(device);
    SortList();
    g_EnvironmentHasChanged = true;
    return plane;
}

void UpdatePlaneTexture()
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c]) {
            float* data = g_Cloth[c]->BeginPlaneList();
            int i, n;
            std::list<Object *>::const_iterator o;
            for (o = g_PlaneListBegin, i = 0, n = 0; i < Plane::Num(); ++i, ++o)
                if ((*o)->IsInCollisionSet(c)) {
                    data = (*o)->Export(data);
                    ++n;
                }
            g_Cloth[c]->EndPlaneList(n);
        }
}

Sphere* AddSphere(IDirect3DDevice9* device, const D3DXVECTOR3& center, float diameter, bool isVisible,
                  bool isSelectable)
{
    if (Sphere::Num() == MAX_SPHERE)
        return 0;
    Sphere* sphere = new Sphere(center, diameter, isVisible, isSelectable);
    sphere->SetID(g_ID);
    ++g_ID;
    g_SphereListBegin = g_ObjectList.insert(g_SphereListBegin, sphere);
    if (device)
        sphere->OnCreateDevice(device);
    SortList();
    g_EnvironmentHasChanged = true;
    return sphere;
}

void UpdateSphereTexture()
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c]) {
            float* data = g_Cloth[c]->BeginSphereList();
            int i, n;
            std::list<Object *>::const_iterator o;
            for (o = g_SphereListBegin, i = 0, n = 0; i < Sphere::Num(); ++i, ++o)
                if ((*o)->IsInCollisionSet(c)) {
                    data = (*o)->Export(data);
                    ++n;
                }
            g_Cloth[c]->EndSphereList(n);
        }
}

Box* AddBox(IDirect3DDevice9* device, const D3DXVECTOR3& center, const D3DXVECTOR3& dimension, bool isVisible,
            bool isSelectable)
{
    if (Box::Num() == MAX_BOX)
        return 0;
    Box* box = new Box(center, dimension, isVisible, isSelectable);
    box->SetID(g_ID);
    ++g_ID;
    g_BoxListBegin = g_ObjectList.insert(g_BoxListBegin, box);
    if (device)
        box->OnCreateDevice(device);
    SortList();
    g_EnvironmentHasChanged = true;
    return box;
}

void UpdateBoxTexture()
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c]) {
            float* data = g_Cloth[c]->BeginBoxList();
            int i, n;
            std::list<Object *>::const_iterator o;
            for (o = g_BoxListBegin, i = 0, n = 0; i < Box::Num(); ++i, ++o)
                if ((*o)->IsInCollisionSet(c)) {
                    data = (*o)->Export(data);
                    ++n;
                }
            g_Cloth[c]->EndBoxList(n);
        }
}

Ellipsoid* AddEllipsoid(IDirect3DDevice9* device, const D3DXVECTOR3& center, const D3DXVECTOR3& dimension,
                        bool isVisible, bool isSelectable)
{
    if (Ellipsoid::Num() == MAX_ELLIPSOID)
        return 0;
    Ellipsoid* ellipsoid = new Ellipsoid(center, dimension, isVisible, isSelectable);
    ellipsoid->SetID(g_ID);
    ++g_ID;
    g_EllipsoidListBegin = g_ObjectList.insert(g_EllipsoidListBegin, ellipsoid);
    if (device)
        ellipsoid->OnCreateDevice(device);
    SortList();
    g_EnvironmentHasChanged = true;
    return ellipsoid;
}

void UpdateEllipsoidTexture()
{
    for (int c = 0; c < CLOTH_NUM; ++c)
        if (g_Cloth[c]) {
            float* data = g_Cloth[c]->BeginEllipsoidList();
            int i, n;
            std::list<Object *>::const_iterator o;
            for (o = g_EllipsoidListBegin, i = 0, n = 0; i < Ellipsoid::Num(); ++i, ++o)
                if ((*o)->IsInCollisionSet(c)) {
                    data = (*o)->Export(data);
                    ++n;
                }
            g_Cloth[c]->EndEllipsoidList(n);
        }
}

Cloth* AddCloth(IDirect3DDevice9* device, const D3DXVECTOR3& center, const D3DXVECTOR3& edgeX,
                const D3DXVECTOR3& edgeY, const wchar_t* filename, int width, int height)
{
    Cloth* cloth = new Cloth(center, edgeX, edgeY, filename, width, height);
    cloth->SetID(g_ID);
    ++g_ID;
    g_ClothListBegin = g_ObjectList.insert(g_ClothListBegin, cloth);
    if (device)
        cloth->OnCreateDevice(device);
    SortList();
    g_EnvironmentHasChanged = true;
    return cloth;
}

// Rendering order
void SortList()
{

    // Render clothes last
    std::list<Object *>::iterator o = g_ClothListBegin;
    for (int i = 0; i < ClothSim::Num(); ++i) {
        g_ObjectList.push_back(*o);
        if (i == 0)
            g_ClothListBegin = --g_ObjectList.end();
        o = g_ObjectList.erase(o);
    }
}

void RemoveSelection()
{
    int i;
    std::list<Object *>::iterator o;
    g_ID = 1;
    for (o = g_PlaneListBegin, i = 0; i < Plane::Num();) {
        if ((*o)->IsSelected()) {
            (*o)->OnLostDevice();
            (*o)->OnDestroyDevice();
            delete *o;
            bool isBegin = o == g_PlaneListBegin;
            o = g_ObjectList.erase(o);
            if (Plane::Num() == 0)
                g_PlaneListBegin = g_ObjectList.end();
            else if (isBegin)
                g_PlaneListBegin = o;
        }
        else {
            (*o)->SetID(g_ID);
            ++g_ID;
            ++o;
            ++i;
        }
    }
    for (o = g_SphereListBegin, i = 0; i < Sphere::Num();) {
        if ((*o)->IsSelected()) {
            (*o)->OnLostDevice();
            (*o)->OnDestroyDevice();
            delete *o;
            bool isBegin = o == g_SphereListBegin;
            o = g_ObjectList.erase(o);
            if (Sphere::Num() == 0)
                g_SphereListBegin = g_ObjectList.end();
            else if (isBegin)
                g_SphereListBegin = o;
        }
        else {
            (*o)->SetID(g_ID);
            ++g_ID;
            ++o;
            ++i;
        }
    }
    for (o = g_BoxListBegin, i = 0; i < Box::Num();) {
        if ((*o)->IsSelected()) {
            (*o)->OnLostDevice();
            (*o)->OnDestroyDevice();
            delete *o;
            bool isBegin = o == g_BoxListBegin;
            o = g_ObjectList.erase(o);
            if (Box::Num() == 0)
                g_BoxListBegin = g_ObjectList.end();
            else if (isBegin)
                g_BoxListBegin = o;
        }
        else {
            (*o)->SetID(g_ID);
            ++g_ID;
            ++o;
            ++i;
        }
    }
    for (o = g_EllipsoidListBegin, i = 0; i < Ellipsoid::Num();) {
        if ((*o)->IsSelected()) {
            (*o)->OnLostDevice();
            (*o)->OnDestroyDevice();
            delete *o;
            bool isBegin = o == g_EllipsoidListBegin;
            o = g_ObjectList.erase(o);
            if (Ellipsoid::Num() == 0)
                g_EllipsoidListBegin = g_ObjectList.end();
            else if (isBegin)
                g_EllipsoidListBegin = o;
        }
        else {
            (*o)->SetID(g_ID);
            ++g_ID;
            ++o;
            ++i;
        }
    }
    for (o = g_ClothListBegin, i = 0; i < ClothSim::Num();) {
        (*o)->SetID(g_ID);
        ++g_ID;
        ++o;
        ++i;
    }
    g_EnvironmentHasChanged = true;
}

/*----------------------------------------------------------------------------------------------------------------------
    Object
 ----------------------------------------------------------------------------------------------------------------------- */

Object::Object(bool isVisible, bool isSelectable) :
    m_Device(0),
    m_Mesh(0),
    m_ColorTexture(0),
    m_NormalMap(0),
    m_TexCoordScaling(1),
    m_DiffuseCoeff(1),
    m_SpecularCoeff(0.4f),
    m_SpecularPower(1000),
    m_ID(0),
    m_IsVisible(isVisible),
    m_IsSelectable(isSelectable),
    m_IsSelected(false),
    m_IsTargeted(false)
{
    D3DXMatrixIdentity(&m_World);
}

void Object::SetWorld(const D3DXMATRIX& world)
{
    m_World = world;
    g_EnvironmentHasChanged = true;
}

D3DXVECTOR3 Object::Dimension() const
{
    D3DXVECTOR3 dimension;
    for (int i = 0; i < 3; ++i)
        dimension[i] = D3DXVec3Length(&D3DXVECTOR3(m_World(i, 0), m_World(i, 1), m_World(i, 2)));
    return dimension;
}

void Object::SetDimension(const D3DXVECTOR3& dimension)
{
    D3DXVECTOR3 currentDimension = Dimension();
    D3DXVECTOR3 scale;
    for (int i = 0; i < 3; ++i) {
        float newDimension = max(dimension[i], 0.1f);
        scale[i] = newDimension / currentDimension[i];
    }
    D3DXMATRIX mat;
    D3DXMatrixScaling(&mat, scale[0], scale[1], scale[2]);
    m_World = mat * m_World;
    g_EnvironmentHasChanged = true;
}

void Object::Render(ID3DXEffect& effect, const D3DXMATRIX& viewProjection, bool forceRender)
{
    if (!forceRender && !m_IsVisible)
        return;
    if (m_Mesh == 0)
        return;
    D3DXMATRIX thinning;
    ComputeThinningMatrix(g_Thinning, thinning);
    D3DXMATRIX worldViewProjection = thinning * m_World * viewProjection;
    effect.SetMatrix("WorldViewProjection", &worldViewProjection);
    unsigned int numPasses;
    effect.Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        effect.BeginPass(pass);
        m_Mesh->DrawSubset(0);
        effect.EndPass();
    }
    effect.End();
}

void Object::RenderID(const D3DXMATRIX& viewProjection)
{
    g_GUIEffect->SetTechnique("RenderGlobalID");
    g_GUIEffect->SetFloat("ObjectID", m_IsSelectable ? m_ID / 255.0f : 0);
    D3DXMATRIX worldViewProjection = m_World * viewProjection;
    g_GUIEffect->SetMatrix("WorldViewProjection", &worldViewProjection);
    Render(*g_GUIEffect, viewProjection);
}

void Object::ComputeThinningMatrix(float thinning, D3DXMATRIX& mat)
{
    D3DXVECTOR3 dim = Dimension();
    D3DXMatrixScaling(&mat, max(1 + thinning / dim[0], 0), max(1 + thinning / dim[1], 0), max(1 + thinning / dim[2], 0));
}

/*----------------------------------------------------------------------------------------------------------------------
    Collision object
 ----------------------------------------------------------------------------------------------------------------------- */

CollisionObject::CollisionObject(bool isVisible, bool isSelectable) : Object(isVisible, isSelectable)
{

    // By default, all clothes collide with the object
    for (int c = 0; c < CLOTH_NUM; ++c)
        m_IsInCollisionSet[c] = true;
}

void CollisionObject::RemoveFromCollisionSet(int n)
{
    if (0 <= n && n < CLOTH_NUM && m_IsInCollisionSet[n]) {
        m_IsInCollisionSet[n] = false;
        g_EnvironmentHasChanged = true;
    }
}

bool CollisionObject::IsInCollisionSet(int n)
{
    if (0 <= n && n < CLOTH_NUM)
        return m_IsInCollisionSet[n];
    else
        return true;
}

void CollisionObject::RenderWorldPosition(const D3DXMATRIX& viewProjection, bool showInvisibleObjects)
{
    g_GUIEffect->SetTechnique("RenderWorldPosition");
    D3DXMATRIX worldViewProjection = m_World * viewProjection;
    g_GUIEffect->SetMatrix("WorldViewProjection", &worldViewProjection);
    g_GUIEffect->SetMatrix("World", &m_World);
    Render(*g_GUIEffect, viewProjection, showInvisibleObjects);
}

/*----------------------------------------------------------------------------------------------------------------------
    Plane
 ----------------------------------------------------------------------------------------------------------------------- */

Plane::Plane(const D3DXVECTOR3& normal, float dist, bool isVisible, bool isSelectable) :
    CollisionObject(isVisible, isSelectable)
{
    D3DXVECTOR3 p = -dist * normal;
    D3DXQUATERNION quat;
    if (normal.x || normal.y || normal.z < 0) {
        if (normal.x || normal.y)
            D3DXQuaternionRotationAxis(&quat, &D3DXVECTOR3(-normal.y, normal.x, 0), acosf(normal.z));
        else
            quat = D3DXQUATERNION(1, 0, 0, 0);
        D3DXMatrixAffineTransformation(&m_World, 1, 0, &quat, &p);
    }
    else
        D3DXMatrixTranslation(&m_World, p.x, p.y, p.z);
}

HRESULT Plane::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;
    m_Device = device;
    if (m_Mesh == 0) {
        V_RETURN(D3DXCreateMeshFVF(2, 4, D3DXMESH_IB_MANAGED | D3DXMESH_VB_MANAGED,
                 D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0), m_Device, &m_Mesh));
        WORD* indices;
        V_RETURN(m_Mesh->LockIndexBuffer(0, (void**) &indices));
        indices[0] = 0;
        indices[1] = 2;
        indices[2] = 1;
        indices += 3;
        indices[0] = 0;
        indices[1] = 3;
        indices[2] = 2;
        V_RETURN(m_Mesh->UnlockIndexBuffer());
        float* vertices;
        V_RETURN(m_Mesh->LockVertexBuffer(0, (void**) &vertices));
        float size = CELL_SIZE;
        float tile = 2;
        vertices[0] = -size;
        vertices[1] = size;
        vertices[2] = 0;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = 0;
        vertices[2] = 1;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = 0;
        vertices += 2;
        vertices[0] = size;
        vertices[1] = size;
        vertices[2] = 0;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = 0;
        vertices[2] = 1;
        vertices += 3;
        vertices[0] = tile;
        vertices[1] = 0;
        vertices += 2;
        vertices[0] = size;
        vertices[1] = -size;
        vertices[2] = 0;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = 0;
        vertices[2] = 1;
        vertices += 3;
        vertices[0] = tile;
        vertices[1] = tile;
        vertices += 2;
        vertices[0] = -size;
        vertices[1] = -size;
        vertices[2] = 0;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = 0;
        vertices[2] = 1;
        vertices += 3;
        vertices[0] = 0;
        vertices[1] = tile;
        vertices += 2;
        V_RETURN(m_Mesh->UnlockVertexBuffer());
        LPD3DXMESH mesh;
        V_RETURN(D3DXTessellateNPatches(m_Mesh, 0, 64, false, &mesh, 0));
        m_Mesh->Release();
        m_Mesh = mesh;
    }
    Object::m_Mesh = m_Mesh;
    ++m_Num;

    return S_OK;
}

void Plane::OnDestroyDevice()
{
    --m_Num;
    if (m_Num == 0)
        SAFE_RELEASE(m_Mesh);
}

float* Plane::Export(float* data)
{
    *data++ = m_World(2, 0);
    *data++ = m_World(2, 1);
    *data++ = m_World(2, 2);
    *data++ = -(m_World(2, 0) * m_World(3, 0) + m_World(2, 1) * m_World(3, 1) + m_World(2, 2) * m_World(3, 2));
    return data;
}

/*----------------------------------------------------------------------------------------------------------------------
    Sphere
 ----------------------------------------------------------------------------------------------------------------------- */

Sphere::Sphere(const D3DXVECTOR3& center, float diameter, bool isVisible, bool isSelectable) :
    CollisionObject(isVisible, isSelectable)
{
    m_World(3, 0) = center.x;
    m_World(3, 1) = center.y;
    m_World(3, 2) = center.z;
    m_World(0, 0) = diameter;
    m_World(1, 1) = diameter;
    m_World(2, 2) = diameter;
}

HRESULT Sphere::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;
    m_Device = device;
    if (m_Mesh == 0)
        V_RETURN(D3DXCreateSphere(m_Device, 0.5f, 32, 32, &m_Mesh, 0));
    Object::m_Mesh = m_Mesh;
    ++m_Num;
    return S_OK;
}

void Sphere::SetDimension(const D3DXVECTOR3& dimension)
{
    Object::SetDimension(D3DXVECTOR3(dimension.x, dimension.x, dimension.x));
}

void Sphere::OnDestroyDevice()
{
    --m_Num;
    if (m_Num == 0)
        SAFE_RELEASE(m_Mesh);
}

float* Sphere::Export(float* data)
{
    *data++ = m_World(3, 0);
    *data++ = m_World(3, 1);
    *data++ = m_World(3, 2);
    *data++ = Dimension().x / 2;
    return data;
}

/*----------------------------------------------------------------------------------------------------------------------
    Box
 ----------------------------------------------------------------------------------------------------------------------- */

Box::Box(const D3DXVECTOR3& center, const D3DXVECTOR3& dimension, bool isVisible, bool isSelectable) :
    CollisionObject(isVisible, isSelectable)
{
    m_World(3, 0) = center.x;
    m_World(3, 1) = center.y;
    m_World(3, 2) = center.z;
    m_World(0, 0) = dimension.x;
    m_World(1, 1) = dimension.y;
    m_World(2, 2) = dimension.z;
}

HRESULT Box::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;
    m_Device = device;
    if (m_Mesh == 0)
        V_RETURN(D3DXCreateBox(m_Device, 1, 1, 1, &m_Mesh, 0));
    Object::m_Mesh = m_Mesh;
    ++m_Num;
    return S_OK;
}

void Box::OnDestroyDevice()
{
    --m_Num;
    if (m_Num == 0)
        SAFE_RELEASE(m_Mesh);
}

float* Box::Export(float* data)
{
    D3DXMATRIX worldInv;
    D3DXMatrixInverse(&worldInv, 0, &m_World);
    for (int x = 0; x < 3; ++x)
        for (int y = 0; y < 4; ++y)
            *data++ = worldInv(y, x);
    return data;
}

/*----------------------------------------------------------------------------------------------------------------------
    Ellipsoid
 ----------------------------------------------------------------------------------------------------------------------- */

Ellipsoid::Ellipsoid(const D3DXVECTOR3& center, const D3DXVECTOR3& dimension, bool isVisible, bool isSelectable) :
    CollisionObject(isVisible, isSelectable)
{
    m_World(3, 0) = center.x;
    m_World(3, 1) = center.y;
    m_World(3, 2) = center.z;
    m_World(0, 0) = dimension.x;
    m_World(1, 1) = dimension.y;
    m_World(2, 2) = dimension.z;
}

HRESULT Ellipsoid::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;
    m_Device = device;
    if (m_Mesh == 0)
        V_RETURN(D3DXCreateSphere(m_Device, 0.5f, 32, 32, &m_Mesh, 0));
    Object::m_Mesh = m_Mesh;
    ++m_Num;
    return S_OK;
}

void Ellipsoid::OnDestroyDevice()
{
    --m_Num;
    if (m_Num == 0)
        SAFE_RELEASE(m_Mesh);
}

float* Ellipsoid::Export(float* data)
{
    D3DXMATRIX worldInv;
    D3DXMatrixInverse(&worldInv, 0, &m_World);
    for (int x = 0; x < 3; ++x)
        for (int y = 0; y < 4; ++y)
            *data++ = worldInv(y, x);
    return data;
}

/*----------------------------------------------------------------------------------------------------------------------
    Cloth
 ----------------------------------------------------------------------------------------------------------------------- */

Cloth::Cloth(const D3DXVECTOR3& center, const D3DXVECTOR3& edgeX, const D3DXVECTOR3& edgeY, const wchar_t* filename, int width, int height) :
    m_TargetID(-1),
    m_WorldIsValid(false),
	m_AnchorPositionBuffer(0),
	m_PositionBuffer(0),
    m_AnchorPositionListIsValid(false),
    m_AnchorPositionListHasChanged(true),
    m_AnchorPositionListHasMoved(true),
    m_AnchorPositionCount(0),
    m_TransformIsValid(false),
    m_ResetSimulation(true),
    m_PlaneListTexture(0),
    m_SphereListTexture(0),
    m_BoxListTexture(0),
    m_EllipsoidListTexture(0),
    m_SelectionIsFree(false),
    m_VertexBuffer(0),
    m_IndexBuffer(0),
    m_TriangleNum(0),
    m_SwathWidth(10),
    m_NormalBuffer(0)
{
    D3DXMatrixIdentity(&m_Transform);
    m_SelectedPositionListBegin = m_AnchorPositionList.end();
    m_ClothSim = new ClothSim(center, edgeX, edgeY, filename, width, height);
    m_ClothSim->Initialize();
}

Cloth::~Cloth()
{
    delete m_ClothSim;
}

HRESULT Cloth::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;
    m_Device = device;
    if (m_CenterSurfaceSysMem == 0) {
        V_RETURN(D3DXCreateTexture(m_Device, 1, 1, 1, 0, D3DFMT_A16B16G16R16F, D3DPOOL_SYSTEMMEM, &m_CenterTextureSysMem));
        V_RETURN(m_CenterTextureSysMem->GetSurfaceLevel(0, &m_CenterSurfaceSysMem));
    }
    V_RETURN(m_ClothSim->OnCreateDevice(m_Device));
    V_RETURN(CreateBuffers());
    return S_OK;
}

void Cloth::OnDestroyDevice()
{
    m_ClothSim->OnDestroyDevice();
    DestroyBuffers();
    if (m_ClothSim->Num() == 0) {
        SAFE_RELEASE(m_CenterTextureSysMem);
        SAFE_RELEASE(m_CenterSurfaceSysMem);
    }
}

HRESULT Cloth::OnResetDevice(IDirect3DDevice9* device)
{
    HRESULT hr;

    // Render target to render selection center
    if (m_CenterSurface == 0) {
        V_RETURN(D3DXCreateTexture(m_Device, 1, 1, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_CenterTexture));
        V_RETURN(m_CenterTexture->GetSurfaceLevel(0, &m_CenterSurface));
    }

    // Cloth simulator
    V_RETURN(m_ClothSim->OnResetDevice());

    // Environment collision textures
    V_RETURN(D3DXCreateTexture(device, OBJECT_TEXTURE_SIZE, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A32B32G32R32F,
             D3DPOOL_DEFAULT, &m_PlaneListTexture));
    V_RETURN(D3DXCreateTexture(device, OBJECT_TEXTURE_SIZE, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A32B32G32R32F,
             D3DPOOL_DEFAULT, &m_SphereListTexture));
    V_RETURN(D3DXCreateTexture(device, OBJECT_TEXTURE_SIZE, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A32B32G32R32F,
             D3DPOOL_DEFAULT, &m_BoxListTexture));
    V_RETURN(D3DXCreateTexture(device, OBJECT_TEXTURE_SIZE, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A32B32G32R32F,
             D3DPOOL_DEFAULT, &m_EllipsoidListTexture));

    return S_OK;
}

void Cloth::OnLostDevice()
{
    SAFE_RELEASE(m_CenterTexture);
    SAFE_RELEASE(m_CenterSurface);
    SAFE_RELEASE(m_PlaneListTexture);
    SAFE_RELEASE(m_SphereListTexture);
    SAFE_RELEASE(m_BoxListTexture);
    SAFE_RELEASE(m_EllipsoidListTexture);
    m_ClothSim->OnLostDevice();
}

HRESULT Cloth::CreateBuffers()
{
    HRESULT hr;
    int width = m_ClothSim->Width();
    int height = m_ClothSim->Height();

    // Vertex buffer
    V_RETURN(m_Device->CreateVertexBuffer(m_ClothSim->Width() * m_ClothSim->Height() * sizeof(Vertex), 0, Vertex::FVF,
             D3DPOOL_MANAGED, &m_VertexBuffer, 0));
    Vertex* vertices;
    V_RETURN(m_VertexBuffer->Lock(0, 0, (void**) &vertices, 0));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            vertices[y * width + x].m_TexCoord[0] = x / static_cast<float>(width - 1);
            vertices[y * width + x].m_TexCoord[1] = y / static_cast<float>(height - 1);
        }
    V_RETURN(m_VertexBuffer->Unlock());

    // Index buffer
    V_RETURN(m_Device->CreateIndexBuffer((width - 1) * (height - 1) * 2 * 3 * sizeof(short), D3DUSAGE_WRITEONLY,
             D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_IndexBuffer, 0));
    FillIndexBuffer();

    // Normal buffer
    V_RETURN(m_Device->CreateVertexBuffer(2 * width * height * sizeof(Normal), 0, Normal::FVF, D3DPOOL_MANAGED,
             &m_NormalBuffer, 0));
    Normal* normals;
    V_RETURN(m_NormalBuffer->Lock(0, 0, (void**) &normals, 0));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            normals[y * 2 * width + 2 * x].m_TexCoord[0] = x / static_cast<float>(width);
            normals[y * 2 * width + 2 * x].m_TexCoord[1] = y / static_cast<float>(height);
            normals[y * 2 * width + 2 * x].m_Length = 0;
            normals[y * 2 * width + 2 * x + 1].m_TexCoord = normals[y * 2 * width + 2 * x].m_TexCoord;
            normals[y * 2 * width + 2 * x + 1].m_Length = 0.1f;
        }
    V_RETURN(m_NormalBuffer->Unlock());

    // Anchor position buffer
    V_RETURN(m_Device->CreateVertexBuffer(width * height * sizeof(ClothSim::Position), 0, ClothSim::Position::FVF, D3DPOOL_MANAGED,
             &m_AnchorPositionBuffer, 0));
    V_RETURN(m_Device->CreateVertexBuffer(width * height * sizeof(ClothSim::Position), 0, ClothSim::Position::FVF, D3DPOOL_MANAGED,
             &m_PositionBuffer, 0));
    m_AnchorPositionListIsValid = false;
    m_AnchorPositionListHasChanged = true;

    return S_OK;
}

void Cloth::DestroyBuffers()
{
    SAFE_RELEASE(m_VertexBuffer);
    SAFE_RELEASE(m_AnchorPositionBuffer);
    SAFE_RELEASE(m_PositionBuffer);
    SAFE_RELEASE(m_NormalBuffer);
    SAFE_RELEASE(m_IndexBuffer);
}

void Cloth::FillIndexBuffer()
{
    int width = m_ClothSim->Width();
    int height = m_ClothSim->Height();
    m_TriangleNum = 0;
    short (*indices)[3];
    m_IndexBuffer->Lock(0, 0, (void**) &indices, 0);
    for (short x0 = 0; x0 < width - 1; x0 += m_SwathWidth) {
        short x1 = x0 + m_SwathWidth;
        for (short y = 0; y < height - 1; ++y)
            for (short x = x0; (x < width - 1) && (x < x1); ++x) {
                int triangleIndex = 2 * (width - 1) * y + 2 * x;
                if (!m_ClothSim->TriangleIsCut(triangleIndex)) {
                    indices[m_TriangleNum][0] = (y + 0) * width + (x + 0);
                    indices[m_TriangleNum][1] = (y + 1) * width + (x + 1);
                    indices[m_TriangleNum][2] = (y + 1) * width + (x + 0);
                    ++m_TriangleNum;
                }
				++triangleIndex;
                if (!m_ClothSim->TriangleIsCut(triangleIndex)) {
                    indices[m_TriangleNum][0] = (y + 0) * width + (x + 0);
                    indices[m_TriangleNum][1] = (y + 0) * width + (x + 1);
                    indices[m_TriangleNum][2] = (y + 1) * width + (x + 1);
                    ++m_TriangleNum;
                }
            }
    }
    m_IndexBuffer->Unlock();
}

void Cloth::SetSize(int width, int height)
{
    if (m_ClothSim->Width() == width && m_ClothSim->Height() == height)
        return;
    Uncut();
    for (std::list<AnchorPosition>::iterator p = m_AnchorPositionList.begin(); p != m_AnchorPositionList.end();
         ++p) {
        int x = p->m_ID % m_ClothSim->Width();
        int y = p->m_ID / m_ClothSim->Width();
        x = static_cast<int>(x * width / static_cast<float>(m_ClothSim->Width()) + 0.5f);
        y = static_cast<int>(y * height / static_cast<float>(m_ClothSim->Height()) + 0.5f);
        p->m_ID = y * width + x;
        p->m_XY[0] = 2 * static_cast<float>(x) / width -1;
        p->m_XY[1] = -(2 * static_cast<float>(y) / height -1);
    }
    m_ClothSim->SetSize(width, height);
    DestroyBuffers();
    CreateBuffers();
    Reset();
}

float* Cloth::BeginPlaneList()
{
    D3DLOCKED_RECT rect;
    m_PlaneListTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
    return static_cast<float*>(rect.pBits);
}

void Cloth::EndPlaneList(int n)
{
    m_PlaneNum = n;
    m_PlaneListTexture->UnlockRect(0);
}

float* Cloth::BeginSphereList()
{
    D3DLOCKED_RECT rect;
    m_SphereListTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
    return static_cast<float*>(rect.pBits);
}

void Cloth::EndSphereList(int n)
{
    m_SphereNum = n;
    m_SphereListTexture->UnlockRect(0);
}

float* Cloth::BeginBoxList()
{
    D3DLOCKED_RECT rect;
    m_BoxListTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
    return static_cast<float*>(rect.pBits);
}

void Cloth::EndBoxList(int n)
{
    m_BoxNum = n;
    m_BoxListTexture->UnlockRect(0);
}

float* Cloth::BeginEllipsoidList()
{
    D3DLOCKED_RECT rect;
    m_EllipsoidListTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
    return static_cast<float*>(rect.pBits);
}

void Cloth::EndEllipsoidList(int n)
{
    m_EllipsoidNum = n;
    m_EllipsoidListTexture->UnlockRect(0);
}

void Cloth::Reset()
{
    m_ResetSimulation = true;
}

void Cloth::SetSelectionFree(bool isFree)
{
    if (m_SelectionIsFree != isFree) {
        m_AnchorPositionListHasChanged = true;
        if (isFree)
            m_WorldIsValid = false;
    }
    m_SelectionIsFree = isFree;
}

void Cloth::Simulate(float time, float timeStep, float oldTimeStep)
{

    // Updated environment texture for collision detection
    m_ClothSim->SetPlaneTexture(m_PlaneListTexture, OBJECT_TEXTURE_SIZE, m_PlaneNum);
    m_ClothSim->SetSphereTexture(m_SphereListTexture, OBJECT_TEXTURE_SIZE, m_SphereNum);
    m_ClothSim->SetBoxTexture(m_BoxListTexture, OBJECT_TEXTURE_SIZE, m_BoxNum);
    m_ClothSim->SetEllipsoidTexture(m_EllipsoidListTexture, OBJECT_TEXTURE_SIZE, m_EllipsoidNum);

    // Reset selected positions if simulation is reset
    bool freeSelectedPositions = m_SelectionIsFree;

    // Handle addition or deletion of anchor points
    int num = 0;
    if (m_AnchorPositionListHasChanged) {    
        if (m_AnchorPositionListIsValid)
            UpdateAnchorPositionBuffer();
        for (std::list<AnchorPosition>::const_iterator p = m_AnchorPositionList.begin();
            p != m_AnchorPositionList.end(); ++p) {
            if (p == m_SelectedPositionListBegin && freeSelectedPositions)
                break;
            ++num;
        }
        m_ClothSim->SetAnchorPositionsVertexBuffer(m_AnchorPositionBuffer, num);
    }

    // Handle moving anchor points
    num = 0;
    if (m_TransformIsValid) {
        ClothSim::Position* positions;
        m_PositionBuffer->Lock(0, 0, (void**) &positions, 0);
        for (std::list<AnchorPosition>::const_iterator p = m_SelectedPositionListBegin;
            p != m_AnchorPositionList.end(); ++p) {
            positions[num].m_XY = p->m_XY;
            ++num;
        }
        m_PositionBuffer->Unlock();
    }
    else if (m_AnchorPositionListHasMoved) {
        ClothSim::Position* positions;
        m_PositionBuffer->Lock(0, 0, (void**) &positions, 0);
        for (std::list<AnchorPosition>::const_iterator p = m_AnchorPositionList.begin();
            p != m_AnchorPositionList.end(); ++p)
            if (p->m_PositionIsValid) {
                positions[num].m_XY = p->m_XY;
                positions[num].m_Value = p->m_Position;
                ++num;
            }
        m_PositionBuffer->Unlock();
    }

    // Run simulation
    m_ClothSim->Simulate(time, timeStep, oldTimeStep, m_ResetSimulation, m_PositionBuffer, num, m_TransformIsValid ? &m_Transform : 0);

    // Reset state variables
    if (m_ResetSimulation) {
        m_ResetSimulation = false;
        m_WorldIsValid = false;
    }
    else {
        m_AnchorPositionListHasChanged = false;
        m_AnchorPositionListHasMoved = false;
        m_TransformIsValid = false;
        D3DXMatrixIdentity(&m_Transform);
    }
}

void Cloth::Render(ID3DXEffect& effect, const D3DXMATRIX& viewProjection, bool)
{
    if (m_NormalMap)
        effect.SetTechnique("SimulatedBumpMapped");
    else
        effect.SetTechnique("Simulated");
    m_Device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    m_Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
    m_Device->SetRenderState(D3DRS_STENCILREF, m_ID);
    m_Device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(Vertex));
    m_Device->SetIndices(m_IndexBuffer);
    m_Device->SetFVF(Vertex::FVF);
    effect.SetTexture("PositionTexture", m_ClothSim->PositionTexture());
    effect.SetTexture("NormalTexture", m_ClothSim->NormalTexture());
    effect.SetInt("RTWidth", m_ClothSim->Width());
    effect.SetInt("RTHeight", m_ClothSim->Height());
    effect.SetFloat("Dx", 1.0f / m_ClothSim->Width());
    effect.SetFloat("Dy", 1.0f / m_ClothSim->Height());
    effect.SetFloat("TexCoordScaling", m_TexCoordScaling);
    unsigned int numPasses;
    effect.Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        effect.BeginPass(pass);
        m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_ClothSim->Width() * m_ClothSim->Height(), 0, m_TriangleNum);
        effect.EndPass();
    }
    effect.End();
    m_Device->SetRenderState(D3DRS_STENCILREF, 0);
    m_Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    m_Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

void Cloth::RenderID(const D3DXMATRIX& viewProjection)
{
    g_GUIEffect->SetTechnique("RenderVertexID");
    g_GUIEffect->SetInt("RTWidth", m_ClothSim->Width());
    g_GUIEffect->SetInt("RTHeight", m_ClothSim->Height());
    g_GUIEffect->SetFloat("Dx", 1.0f / m_ClothSim->Width());
    g_GUIEffect->SetFloat("Dy", 1.0f / m_ClothSim->Height());
    g_GUIEffect->SetFloat("ObjectID", m_IsSelectable ? m_ID / 255.0f : 0);
    g_GUIEffect->SetMatrix("ViewProjection", &viewProjection);
    m_Device->SetFVF(Vertex::FVF);
    m_Device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(Vertex));
    m_Device->SetIndices(m_IndexBuffer);
    g_GUIEffect->SetTexture("PositionTexture", m_ClothSim->PositionTexture());
    unsigned int numPasses;
    g_GUIEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        g_GUIEffect->BeginPass(pass);
        m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_ClothSim->Width() * m_ClothSim->Height(), 0, m_TriangleNum);
        g_GUIEffect->EndPass();
    }
    g_GUIEffect->End();
}

void Cloth::RenderSelection(const D3DXMATRIX& viewProjection)
{
    if (m_AnchorPositionListIsValid)
        UpdateAnchorPositionBuffer();
    int n = 0;
    bool mouseOverFound = false;
    ClothSim::Position* anchorPositions;
    m_AnchorPositionBuffer->Lock(0, 0, (void**) &anchorPositions, 0);
    bool isNailed = true;
    for (std::list<AnchorPosition>::const_iterator p = m_AnchorPositionList.begin();
         p != m_AnchorPositionList.end(); ++p) {
        if (p == m_SelectedPositionListBegin)
            isNailed = false;
        if (p->m_ID == m_TargetID) {
            anchorPositions[n].m_Value = isNailed ? g_ColorSelectedFixedMouseOver : g_ColorSelectedMouseOver;
            mouseOverFound = true;
        }
        else
            anchorPositions[n].m_Value = p->m_Color;
        ++n;
    }
    if (m_TargetID >= 0 && !mouseOverFound) {
        anchorPositions[n].m_XY[0] = 2 * static_cast<float>(m_TargetID % m_ClothSim->Width ()) / m_ClothSim->Width () -1;
        anchorPositions[n].m_XY[1] = -(2 * static_cast<float>(m_TargetID / m_ClothSim->Width ()) / m_ClothSim->Height () -1);
        anchorPositions[n].m_Value = g_ColorMouseOver;
        ++n;
    }
    m_AnchorPositionBuffer->Unlock();
    m_TargetID = -1;
    if (n) {
        m_Device->SetFVF(ClothSim::Position::FVF);
        m_Device->SetStreamSource(0, m_AnchorPositionBuffer, 0, sizeof(ClothSim::Position));
        float pointSize = 10;
        m_Device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*) &pointSize));
        g_GUIEffect->SetTechnique("Render");
        g_GUIEffect->SetInt("RTWidth", m_ClothSim->Width());
        g_GUIEffect->SetInt("RTHeight", m_ClothSim->Height());
        g_GUIEffect->SetFloat("Dx", 1.0f / m_ClothSim->Width());
        g_GUIEffect->SetFloat("Dy", 1.0f / m_ClothSim->Height());
        g_GUIEffect->SetMatrix("ViewProjection", &viewProjection);
        g_GUIEffect->SetInt("RTWidth", m_ClothSim->Width ());
        g_GUIEffect->SetInt("RTHeight", m_ClothSim->Height ());
        g_GUIEffect->SetTexture("PositionTexture", m_ClothSim->PositionTexture());
        for (int i = 0; i < 2; ++i) {
            if (i) {
                m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
                m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
                m_Device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
                m_Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
                m_Device->SetRenderState(D3DRS_STENCILREF, m_ID);
            }
            unsigned int numPasses;
            g_GUIEffect->Begin(&numPasses, 0);
            for (unsigned int pass = 0; pass < numPasses; ++pass) {
                g_GUIEffect->BeginPass(pass);
                m_Device->DrawPrimitive(D3DPT_POINTLIST, 0, n);
                g_GUIEffect->EndPass();
            }
            g_GUIEffect->End();
            if (i) {
                m_Device->SetRenderState(D3DRS_STENCILREF, 0);
                m_Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
                m_Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
                m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);
                m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
            }
        }
    }
}

void Cloth::RenderNormals(const D3DXMATRIX& viewProjection)
{
    m_Device->SetFVF(Normal::FVF);
    m_Device->SetStreamSource(0, m_NormalBuffer, 0, sizeof(Normal));
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_GUIEffect->SetTechnique("RenderNormal");
    g_GUIEffect->SetMatrix("ViewProjection", &viewProjection);
    g_GUIEffect->SetTexture("PositionTexture", m_ClothSim->PositionTexture());
    g_GUIEffect->SetTexture("NormalTexture", m_ClothSim->NormalTexture());
    g_GUIEffect->SetFloatArray("Color", D3DXVECTOR3(1, 1, 0), 3);
    unsigned int numPasses;
    g_GUIEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        g_GUIEffect->BeginPass(pass);
        m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_ClothSim->Width() * m_ClothSim->Height());
        g_GUIEffect->EndPass();
    }
    g_GUIEffect->End();
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void Cloth::Target(float x, float y)
{
    m_TargetID = m_ClothSim->Width() * static_cast<int>(m_ClothSim->Height() * y + 0.5f) + static_cast<int>
        (m_ClothSim->Width() * x + 0.5f);
}

void Cloth::Select(float x, float y, bool isNailed)
{
    m_IsSelected = false;
    if (x < 0) {
        m_AnchorPositionList.erase(m_SelectedPositionListBegin, m_AnchorPositionList.end());
        m_SelectedPositionListBegin = m_AnchorPositionList.end();
        UpdateAnchorPositionBuffer();
        m_AnchorPositionListHasChanged = true;
        m_WorldIsValid = false;
        return;
    }
    int ID = m_ClothSim->Width() * static_cast<int>(m_ClothSim->Height() * y + 0.5f) + static_cast<int>
        (m_ClothSim->Width() * x + 0.5f);
    bool alreadyAnchored = false;
    bool alreadyNailed = true;
    std::list<AnchorPosition>::iterator p;
    int n = 0;
    for (p = m_AnchorPositionList.begin(); p != m_AnchorPositionList.end(); ++p, ++n) {
        if (p == m_SelectedPositionListBegin)
            alreadyNailed = false;
        if (p->m_ID == ID) {
            alreadyAnchored = true;
            break;
        }
    }
    if (alreadyAnchored) {
        if (isNailed && !alreadyNailed) {
            p->m_Color = g_ColorSelectedFixed;
            m_AnchorPositionList.push_front(*p);
        }
        else if (!isNailed && alreadyNailed) {
            p->m_Color = g_ColorSelected;
            m_AnchorPositionList.push_back(*p);
        }
        else
            m_WorldIsValid = false;
        std::list<AnchorPosition>::iterator next = m_AnchorPositionList.erase(p);
        if (p == m_SelectedPositionListBegin)
            m_SelectedPositionListBegin = next;
    }
    else {
        if (isNailed) {
            m_AnchorPositionList.push_front(AnchorPosition());
            p = m_AnchorPositionList.begin();
            p->m_Color = g_ColorSelectedFixed;
        }
        else {
            m_AnchorPositionList.push_back(AnchorPosition());
            p = --m_AnchorPositionList.end();
            if (m_SelectedPositionListBegin == m_AnchorPositionList.end())
                m_SelectedPositionListBegin = p;
            p->m_Color = g_ColorSelected;
        }
        p->m_Count = m_AnchorPositionCount;
        ++m_AnchorPositionCount;
        p->m_ID = ID;
        p->m_XY[0] = 2 * static_cast<float>(p->m_ID % m_ClothSim->Width ()) / m_ClothSim->Width () -1;
        p->m_XY[1] = -(2 * static_cast<float>(p->m_ID / m_ClothSim->Width ()) / m_ClothSim->Height () -1);
        p->m_Position = D3DXVECTOR3(0, 0, 0);
        p->m_PositionIsValid = false;
        m_WorldIsValid = false;
    }
    UpdateAnchorPositionBuffer();
    m_AnchorPositionListHasChanged = true;
    m_IsSelected = m_SelectedPositionListBegin != m_AnchorPositionList.end();
}

void Cloth::Cut(D3DXVECTOR3 cutter[3])
{
    if (m_ClothSim->ComputeCuts(cutter))
        FillIndexBuffer();
}

void Cloth::Uncut()
{
    m_ClothSim->Uncut();
    FillIndexBuffer();
}

const D3DXMATRIX& Cloth::World()
{
    UpdateWorld();
    return m_World;
}

void Cloth::UpdateWorld()
{
    if (m_WorldIsValid)
        return;
    if (m_CenterSurface == 0)
        return;
    int start = 0;
    std::list<AnchorPosition>::const_iterator p;
    for (p = m_AnchorPositionList.begin(); p != m_SelectedPositionListBegin; ++p)
         ++start;
    int n = 0;
    for (; p != m_AnchorPositionList.end(); ++p)
        ++n;
    if (n == 0)
        return;
    m_Device->SetRenderTarget(0, m_CenterSurface);
    m_Device->Clear(0L, 0, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L);
    m_Device->SetFVF(ClothSim::Position::FVF);
    m_Device->SetStreamSource(0, m_AnchorPositionBuffer, 0, sizeof(ClothSim::Position));
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
    float pointSize = 1;
    m_Device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*) &pointSize));
    g_GUIEffect->SetTechnique("ComputeCenter");
    g_GUIEffect->SetInt("RTWidth", m_ClothSim->Width ());
    g_GUIEffect->SetInt("RTHeight", m_ClothSim->Height ());
    g_GUIEffect->SetTexture("PositionTexture", m_ClothSim->PositionTexture());
    g_GUIEffect->SetInt("N", n);
    unsigned int numPasses;
    g_GUIEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        g_GUIEffect->BeginPass(pass);
        m_Device->DrawPrimitive(D3DPT_POINTLIST, start, n);
        g_GUIEffect->EndPass();
    }
    g_GUIEffect->End();
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);
    D3DLOCKED_RECT rect;
    m_Device->GetRenderTargetData(m_CenterSurface, m_CenterSurfaceSysMem);
    m_CenterTextureSysMem->LockRect(0, &rect, 0, D3DLOCK_READONLY);
    D3DXFLOAT16* pixel = static_cast<D3DXFLOAT16*>(rect.pBits);
    D3DXVECTOR3 center;
    D3DXFloat16To32Array(center, pixel, 3);
    m_CenterTextureSysMem->UnlockRect(0);
    D3DXMatrixTranslation(&m_World, center.x, center.y, center.z);
    m_WorldIsValid = true;
}

void Cloth::SetWorld(const D3DXMATRIX& world)
{
    if (g_MoveEnvironmentOnly)
        return;
    D3DXMATRIX worldInv;
    D3DXMatrixInverse(&worldInv, 0, &m_World);
    m_Transform = worldInv * world * m_Transform;
    m_TransformIsValid = true;
    m_World = world;
}

void Cloth::SetAnchorPosition(int n, const D3DXVECTOR3& position)
{
    for (std::list<AnchorPosition>::iterator p = m_AnchorPositionList.begin(); p != m_AnchorPositionList.end();
         ++p)
        if (p->m_Count == n) {
            p->m_Position = position;
            p->m_PositionIsValid = true;
            m_AnchorPositionListHasMoved = true;
            break;
        }
}

void Cloth::UpdateAnchorPositionBuffer()
{
    int num = 0;
    ClothSim::Position* anchorPositions;
    m_AnchorPositionBuffer->Lock(0, 0, (void**) &anchorPositions, 0);
    for (std::list<AnchorPosition>::const_iterator p = m_AnchorPositionList.begin();
        p != m_AnchorPositionList.end(); ++p) {
        anchorPositions[num].m_XY = p->m_XY;
        anchorPositions[num].m_Value = p->m_Color;
        ++num;
    }
    m_AnchorPositionBuffer->Unlock();
    m_AnchorPositionListIsValid = true;
}

/*----------------------------------------------------------------------------------------------------------------------
    Skinned character
 ----------------------------------------------------------------------------------------------------------------------- */

static HRESULT SetupBoneMatrixPointers(LPD3DXFRAME);
static HRESULT CreateBoneBoxes(LPD3DXFRAME);
static void UpdateFrameMatrices(LPD3DXFRAME, LPD3DXMATRIX);
static void UpdateBoneBoxes(LPD3DXFRAME);
static void UpdateAttachments(LPD3DXFRAME);

class CAllocateHierarchy : public ID3DXAllocateHierarchy
{

public:
    CAllocateHierarchy() { }
    STDMETHOD (CreateFrame) (THIS_ LPCSTR, LPD3DXFRAME*);
    STDMETHOD (CreateMeshContainer)
        (
            THIS_ LPCSTR,
            CONST D3DXMESHDATA*,
            CONST D3DXMATERIAL*,
            CONST D3DXEFFECTINSTANCE*,
            DWORD,
            CONST DWORD*,
            LPD3DXSKININFO,
            LPD3DXMESHCONTAINER*
        );
    STDMETHOD (DestroyFrame) (THIS_ LPD3DXFRAME);
    STDMETHOD (DestroyMeshContainer) (THIS_ LPD3DXMESHCONTAINER);
};

HRESULT CreateCharacter(IDirect3DDevice9* device, D3DXFRAME *& character,
                        ID3DXAnimationController *& characterController)
{
    HRESULT hr;
    tstring filename = GetFilePath::GetFilePath(TEXT("MEDIA/models/Tiny/tiny.x"), true);
    WCHAR strPath[MAX_PATH];
    StringCchCopy(strPath, MAX_PATH, filename.c_str());
    WCHAR* pLastSlash = wcsrchr(strPath, L'\\');
    if (pLastSlash)
        *pLastSlash = 0;
    WCHAR strCWD[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, strCWD);
    SetCurrentDirectory(strPath);
    CAllocateHierarchy Alloc;
    V_RETURN(D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, device, &Alloc, 0, &character,
             &characterController));
    V_RETURN(SetupBoneMatrixPointers(character));
    V_RETURN(CreateBoneBoxes(character));
    SetCurrentDirectory(strCWD);
    return S_OK;
}

void DestroyCharacter(D3DXFRAME *& character, ID3DXAnimationController *& characterController)
{
    if (g_Character) {
        CAllocateHierarchy Alloc;
        D3DXFrameDestroy(g_Character, &Alloc);
        SAFE_RELEASE(g_CharacterController);
        SAFE_DELETE_ARRAY(g_CharacterBoneMatrices);
    }
}

void UpdateCharacter(float timeStep, D3DXFRAME* character, ID3DXAnimationController* characterController)
{
    if (g_CharacterController) {
        if (timeStep < 0)
            g_CharacterController->SetTrackPosition(0, 0);
        else
            g_CharacterController->AdvanceTime(timeStep, 0);
    }
    D3DXMATRIX world;
    D3DXQUATERNION quat;
    D3DXQuaternionRotationYawPitchRoll(&quat, D3DX_PI / 2, 0, 0);
    D3DXMatrixAffineTransformation(&world, 0.008f, 0, &quat, &g_SceneCenter[CLOTH_CAPE]);
    UpdateFrameMatrices(g_Character, &world);
    UpdateBoneBoxes(g_Character);
    if (g_Cloth[CLOTH_CAPE] || g_Cloth[CLOTH_SKIRT])
        UpdateAttachments(g_Character);
}

struct BoneCollisionObject
{
    int m_Bone;
    Ellipsoid* m_CollisionObject;
    D3DXMATRIX m_Matrix;
};

// Structure derived from D3DXFRAME so we can add some app-specific
// info that will be stored with each frame
struct D3DXFRAME_DERIVED : public D3DXFRAME
{
    D3DXMATRIXA16 m_CombinedTransformationMatrix;
};

// Structure derived from D3DXMESHCONTAINER so we can add some
// app-specific info that will be stored with each mesh
struct D3DXMESHCONTAINER_DERIVED : public D3DXMESHCONTAINER
{
    LPDIRECT3DTEXTURE9* m_Textures;
    LPD3DXMESH m_Mesh;
    DWORD m_AttributeGroupNum;
    DWORD m_InfluenceNum;
    LPD3DXBUFFER m_BoneCombinationBuffer;
    D3DXMATRIX** m_BoneMatrices;
    D3DXMATRIX* m_BoneOffsetMatrices;
    DWORD m_PaletteEntryNum;
    BoneCollisionObject* m_BondCollisionObjectList;
    int m_BondCollisionObjectNum;
};

// Called to setup the pointers for a given bone to its transformation matrix
HRESULT SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER meshContainerBase)
{
    D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(meshContainerBase);
    meshContainer->m_BoneMatrices = new D3DXMATRIX *[meshContainer->pSkinInfo->GetNumBones()];
    for (unsigned int b = 0; b < meshContainer->pSkinInfo->GetNumBones(); ++b) {
        D3DXFRAME_DERIVED* frame = static_cast<D3DXFRAME_DERIVED*>
            (D3DXFrameFind(g_Character, meshContainer->pSkinInfo->GetBoneName(b)));
        if (frame == 0)
            return E_FAIL;

        meshContainer->m_BoneMatrices[b] = &frame->m_CombinedTransformationMatrix;
    }
    return S_OK;
}

// Called to setup the pointers for a given bone to its transformation matrix
HRESULT SetupBoneMatrixPointers(LPD3DXFRAME frame)
{
    HRESULT hr;
    if (frame->pMeshContainer)
        V_RETURN(SetupBoneMatrixPointersOnMesh(frame->pMeshContainer));
    if (frame->pFrameSibling)
        V_RETURN(SetupBoneMatrixPointers(frame->pFrameSibling));
    if (frame->pFrameFirstChild)
        V_RETURN(SetupBoneMatrixPointers(frame->pFrameFirstChild));
    return S_OK;
}

HRESULT CreateBoneBoxes(LPD3DXFRAME frameBase)
{
    HRESULT hr;
    if (frameBase->pMeshContainer) {
        D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(frameBase->pMeshContainer);
        meshContainer->m_BondCollisionObjectList = new BoneCollisionObject[2 * meshContainer->pSkinInfo->GetNumBones()];
        meshContainer->m_BondCollisionObjectNum = 0;
        char* vertices;
        if (SUCCEEDED(meshContainer->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**) &vertices))) {
            for (unsigned int b = 0; b < meshContainer->pSkinInfo->GetNumBones(); ++b) {
                const char* name = meshContainer->pSkinInfo->GetBoneName(b);
                BoneCollisionObject boneCollisionObject[2];
                int excludedCloth[2] = { -1, -1 };
                int boneCollisionObjectNum = 1;
                if (strcmp(name, "Bip01_R_Forearm") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(140, 45, 35), 0, 0,
                                             &D3DXVECTOR3(40, 0, 0));
                else if (strcmp(name, "Bip01_L_Forearm") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(140, 45, 35), 0, 0,
                                             &D3DXVECTOR3(40, 0, 0));
                else if (strcmp(name, "Bip01_R_UpperArm") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(140, 45, 45), 0, 0,
                                             &D3DXVECTOR3(40, 0, 0));
                else if (strcmp(name, "Bip01_L_UpperArm") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(140, 45, 45), 0, 0,
                                             &D3DXVECTOR3(40, 0, 0));
                else if (strcmp(name, "Bip01_Neck") == 0) {
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(50, 50, 124), 0, 0,
                                             &D3DXVECTOR3(-20, 0, 0));
                    excludedCloth[0] = CLOTH_SKIRT;
                }
                else if (strcmp(name, "Bip01_Spine3") == 0) {
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(150, 85, 120), 0, 0,
                                             &D3DXVECTOR3(-20, 0, 0));
                    excludedCloth[0] = CLOTH_SKIRT;
                }
                else if (strcmp(name, "Bip01_Spine") == 0) {
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(75, 85, 110), 0, 0,
                                             &D3DXVECTOR3(-15, -15, 0));
                    excludedCloth[0] = CLOTH_CAPE;
                    if (g_Cloth[CLOTH_CAPE]) {
                        D3DXMatrixTransformation(&boneCollisionObject[1].m_Matrix, 0, 0, &D3DXVECTOR3(190, 110, 110), 0, 0,
                                                &D3DXVECTOR3(-15, -20, 0));
                        excludedCloth[1] = CLOTH_SKIRT;
                        ++boneCollisionObjectNum;
                    }
                }
                else if (strcmp(name, "Bip01_L_Thigh") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(190, 80, 60), 0, 0,
                                             &D3DXVECTOR3(50, 0, 0));
                else if (strcmp(name, "Bip01_R_Thigh") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(190, 80, 60), 0, 0,
                                             &D3DXVECTOR3(50, 0, 0));
                else if (strcmp(name, "Bip01_L_Calf") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(160, 50, 50), 0, 0,
                                             &D3DXVECTOR3(40, -5, 5));
                else if (strcmp(name, "Bip01_R_Calf") == 0)
                    D3DXMatrixTransformation(&boneCollisionObject[0].m_Matrix, 0, 0, &D3DXVECTOR3(160, 50, 50), 0, 0,
                                             &D3DXVECTOR3(40, -5, -5));
                else
                    boneCollisionObjectNum = 0;
                for (int i = 0; i < boneCollisionObjectNum; ++i) {
                    boneCollisionObject[i].m_Bone = b;
                    boneCollisionObject[i].m_CollisionObject = AddEllipsoid(0, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 1, 1),
                                                                        false, false);
                    if (excludedCloth[i] >= 0)
                        boneCollisionObject[i].m_CollisionObject->RemoveFromCollisionSet(excludedCloth[i]);
                    meshContainer->m_BondCollisionObjectList[meshContainer->m_BondCollisionObjectNum] = boneCollisionObject[i];
                    ++meshContainer->m_BondCollisionObjectNum;
                }
            }
        }
        meshContainer->MeshData.pMesh->UnlockVertexBuffer();
    }
    if (frameBase->pFrameSibling)
        V_RETURN(CreateBoneBoxes(frameBase->pFrameSibling));
    if (frameBase->pFrameFirstChild)
        V_RETURN(CreateBoneBoxes(frameBase->pFrameFirstChild));
    return S_OK;
}

void UpdateFrameMatrices(LPD3DXFRAME frameBase, LPD3DXMATRIX parentMatrix)
{
    D3DXFRAME_DERIVED* frame = static_cast<D3DXFRAME_DERIVED*>(frameBase);
    if (parentMatrix)
        D3DXMatrixMultiply(&frame->m_CombinedTransformationMatrix, &frame->TransformationMatrix, parentMatrix);
    else
        frame->m_CombinedTransformationMatrix = frame->TransformationMatrix;
    if (frame->pFrameSibling)
        UpdateFrameMatrices(frame->pFrameSibling, parentMatrix);
    if (frame->pFrameFirstChild)
        UpdateFrameMatrices(frame->pFrameFirstChild, &frame->m_CombinedTransformationMatrix);
}

void UpdateBoneBoxes(LPD3DXFRAME frameBase)
{
    if (frameBase->pMeshContainer) {
        D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(frameBase->pMeshContainer);
        for (int c = 0; c < meshContainer->m_BondCollisionObjectNum; ++c)
            meshContainer->m_BondCollisionObjectList[c].m_CollisionObject->SetWorld(meshContainer->m_BondCollisionObjectList[c].m_Matrix **meshContainer->m_BoneMatrices[meshContainer->m_BondCollisionObjectList[c].m_Bone]);
    }
    if (frameBase->pFrameSibling)
        UpdateBoneBoxes(frameBase->pFrameSibling);
    if (frameBase->pFrameFirstChild)
        UpdateBoneBoxes(frameBase->pFrameFirstChild);
}

void UpdateAttachments(LPD3DXFRAME frameBase)
{
    if (frameBase->pMeshContainer) {
        D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(frameBase->pMeshContainer);
        if (g_Cloth[CLOTH_SKIRT]) {
            int i = 0;
            for (std::list<BoneAttachment>::iterator a = g_SkirtBoneAttachments.begin(); a != g_SkirtBoneAttachments.end(); ++a, ++i) {
                for (unsigned int b = 0; b < meshContainer->pSkinInfo->GetNumBones(); ++b) {
                    const char* name = meshContainer->pSkinInfo->GetBoneName(b);
                    if (strcmp(name, (*a).m_BoneName) == 0) {
                        D3DXVECTOR3 position;
                        D3DXVec3TransformCoord(&position, &(*a).m_Position, meshContainer->m_BoneMatrices[b]);
                        g_Cloth[CLOTH_SKIRT]->SetAnchorPosition(i, position);
                        break;
                    }
                }
            }
        }
        if (g_Cloth[CLOTH_CAPE])
            for (unsigned int b = 0; b < meshContainer->pSkinInfo->GetNumBones(); ++b) {
                const char* name = meshContainer->pSkinInfo->GetBoneName(b);
                D3DXVECTOR3 position;
                if (strcmp(name, "Bip01_R_Clavicle") == 0) {
                    D3DXVec3TransformCoord(&position, &D3DXVECTOR3(10, -30, -8), meshContainer->m_BoneMatrices[b]);
                    g_Cloth[CLOTH_CAPE]->SetAnchorPosition(0, position);
                }
                else if (strcmp(name, "Bip01_L_Clavicle") == 0) {
                    D3DXVec3TransformCoord(&position, &D3DXVECTOR3(15, -20, 10), meshContainer->m_BoneMatrices[b]);
                    g_Cloth[CLOTH_CAPE]->SetAnchorPosition(4, position);
                }
                else if (strcmp(name, "Bip01_Neck") == 0) {
                    D3DXVec3TransformCoord(&position, &D3DXVECTOR3(0, -10, -30), meshContainer->m_BoneMatrices[b]);
                    g_Cloth[CLOTH_CAPE]->SetAnchorPosition(1, position);
                    D3DXVec3TransformCoord(&position, &D3DXVECTOR3(0, -20, 0), meshContainer->m_BoneMatrices[b]);
                    g_Cloth[CLOTH_CAPE]->SetAnchorPosition(2, position);
                    D3DXVec3TransformCoord(&position, &D3DXVECTOR3(0, -10, 30), meshContainer->m_BoneMatrices[b]);
                    g_Cloth[CLOTH_CAPE]->SetAnchorPosition(3, position);
                }
            }
    }
    if (frameBase->pFrameSibling)
        UpdateAttachments(frameBase->pFrameSibling);
    if (frameBase->pFrameFirstChild)
        UpdateAttachments(frameBase->pFrameFirstChild);
}

static void DrawMeshContainer(IDirect3DDevice9*, LPD3DXMESHCONTAINER, LPD3DXFRAME, ID3DXEffect&, bool);

void RenderCharacter(IDirect3DDevice9* device, LPD3DXFRAME frame, ID3DXEffect& effect, bool shaded)
{
    LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
    while (meshContainer) {
        DrawMeshContainer(device, meshContainer, frame, effect, shaded);
        meshContainer = meshContainer->pNextMeshContainer;
    }
    if (frame->pFrameSibling)
        RenderCharacter(device, frame->pFrameSibling, effect, shaded);
    if (frame->pFrameFirstChild)
        RenderCharacter(device, frame->pFrameFirstChild, effect, shaded);
}

void DrawMeshContainer(IDirect3DDevice9* device, LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase,
                       ID3DXEffect& effect, bool shaded)
{
    D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(meshContainerBase);
    D3DXFRAME_DERIVED* frame = static_cast<D3DXFRAME_DERIVED*>(frameBase);
    LPD3DXBONECOMBINATION boneCombination = reinterpret_cast<LPD3DXBONECOMBINATION>
        (meshContainer->m_BoneCombinationBuffer->GetBufferPointer());
    for (unsigned int a = 0; a < meshContainer->m_AttributeGroupNum; ++a) {

        // first calculate all the world matrices
        for (unsigned int p = 0; p < meshContainer->m_PaletteEntryNum; ++p) {
            int i = boneCombination[a].BoneId[p];
            if (i != UINT_MAX)
                D3DXMatrixMultiply(&g_CharacterBoneMatrices[p], &meshContainer->m_BoneOffsetMatrices[i],
                                   meshContainer->m_BoneMatrices[i]);
        }
        // Number of bones
        effect.SetInt("NumBones", meshContainer->m_InfluenceNum - 1);
        effect.SetMatrixArray("BoneWorld", g_CharacterBoneMatrices, meshContainer->m_PaletteEntryNum);
        if (shaded) {
            effect.SetFloat("DiffuseCoeff", meshContainer->pMaterials[boneCombination[a].AttribId].MatD3D.Diffuse.r);
            effect.SetFloat("SpecularCoeff", 0);
            effect.SetFloatArray("Color", D3DXVECTOR3(1, 1, 1), 3);

            // setup the material of the mesh subset - REMEMBER to use the
            // original pre-skinning attribute id to get the correct material id
            effect.SetTexture("ColorTexture", meshContainer->m_Textures[boneCombination[a].AttribId]);
        }

        // Start the effect now that all parameters have been updated
        unsigned int numPasses;
        effect.Begin(&numPasses, 0);
        for (unsigned int pass = 0; pass < numPasses; ++pass) {
            effect.BeginPass(pass);
            meshContainer->MeshData.pMesh->DrawSubset(a);
            effect.EndPass();
        }
        effect.End();
    }
}

static HRESULT GenerateSkinnedMesh(IDirect3DDevice9*, D3DXMESHCONTAINER_DERIVED*);
static char* CopyName(const char*);

HRESULT CAllocateHierarchy::CreateFrame(LPCSTR name, LPD3DXFRAME* newFrame)
{
    D3DXFRAME_DERIVED* frame = new D3DXFRAME_DERIVED;
    frame->Name = CopyName(name);
    D3DXMatrixIdentity(&frame->TransformationMatrix);
    D3DXMatrixIdentity(&frame->m_CombinedTransformationMatrix);
    frame->pMeshContainer = 0;
    frame->pFrameSibling = 0;
    frame->pFrameFirstChild = 0;
    *newFrame = frame;
    return S_OK;
}

HRESULT CAllocateHierarchy::CreateMeshContainer(LPCSTR name, CONST D3DXMESHDATA* meshData, CONST D3DXMATERIAL* materials,
                                                CONST D3DXEFFECTINSTANCE* effectInstances, DWORD numMaterials,
                                                CONST DWORD* adjacency, LPD3DXSKININFO skinInfo,
                                                LPD3DXMESHCONTAINER* newMeshContainer)
{
    HRESULT hr;
    D3DXMESHCONTAINER_DERIVED* meshContainer = new D3DXMESHCONTAINER_DERIVED;
    memset(meshContainer, 0, sizeof(D3DXMESHCONTAINER_DERIVED));
    meshContainer->Name = CopyName(name);

    LPD3DXMESH mesh = meshData->pMesh;
    LPDIRECT3DDEVICE9 device;
    mesh->GetDevice(&device);

    // if no normals are in the mesh, add them
    if (!(mesh->GetFVF() & D3DFVF_NORMAL)) {
        meshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

        // clone the mesh to make room for the normals
        V_RETURN(mesh->CloneMeshFVF(mesh->GetOptions(), mesh->GetFVF() | D3DFVF_NORMAL, device,
                 &meshContainer->MeshData.pMesh));

        // get the new mesh pointer back out of the mesh container to use
        // NOTE: we do not release mesh because we do not have a reference to
        // it yet
        mesh = meshContainer->MeshData.pMesh;

        // now generate the normals for the pmesh
        D3DXComputeNormals(mesh, 0);
    }
    else {

        // if no normals, just add a reference to the mesh for the mesh
        // container
        meshContainer->MeshData.pMesh = mesh;
        meshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
        mesh->AddRef();
    }

    // allocate memory to contain the material information. This sample uses
    // the D3D9 materials and texture names instead of the EffectInstance
    // style materials
    meshContainer->NumMaterials = max(1, numMaterials);
    meshContainer->pMaterials = new D3DXMATERIAL[meshContainer->NumMaterials];
    meshContainer->m_Textures = new LPDIRECT3DTEXTURE9[meshContainer->NumMaterials];
    meshContainer->pAdjacency = new DWORD[mesh->GetNumFaces() * 3];
    memcpy(meshContainer->pAdjacency, adjacency, sizeof(DWORD) * mesh->GetNumFaces() * 3);
    memset(meshContainer->m_Textures, 0, sizeof(LPDIRECT3DTEXTURE9) * meshContainer->NumMaterials);

    // if materials provided, copy them
    if (numMaterials > 0) {
        memcpy(meshContainer->pMaterials, materials, sizeof(D3DXMATERIAL) * numMaterials);

        for (unsigned int m = 0; m < numMaterials; ++m)
            if (meshContainer->pMaterials[m].pTextureFilename) {
                WCHAR strTexturePath[MAX_PATH];
                WCHAR wszBuf[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, meshContainer->pMaterials[m].pTextureFilename, -1, wszBuf, MAX_PATH);
                wszBuf[MAX_PATH - 1] = L'\0';
                DXUTFindDXSDKMediaFileCch(strTexturePath, MAX_PATH, wszBuf);
                if (FAILED(D3DXCreateTextureFromFile(device, strTexturePath, &meshContainer->m_Textures[m])))
                    meshContainer->m_Textures[m] = 0;

                // don't remember a pointer into the dynamic memory, just
                // forget the name after loading
                meshContainer->pMaterials[m].pTextureFilename = 0;
            }
    }
    else {

        // if no materials provided, use a default one
        meshContainer->pMaterials[0].pTextureFilename = 0;
        memset(&meshContainer->pMaterials[0].MatD3D, 0, sizeof(D3DMATERIAL9));
        meshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
        meshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
        meshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
        meshContainer->pMaterials[0].MatD3D.Specular = meshContainer->pMaterials[0].MatD3D.Diffuse;
    }

    // if there is skinning information, save off the required data and then
    // setup for HW skinning
    // first save off the SkinInfo and original mesh data
    meshContainer->pSkinInfo = skinInfo;
    skinInfo->AddRef();

    meshContainer->m_Mesh = mesh;
    mesh->AddRef();

    // Will need an array of offset matrices to move the vertices from the
    // figure space to the bone's space
    meshContainer->m_BoneOffsetMatrices = new D3DXMATRIX[skinInfo->GetNumBones()];
    for (unsigned int b = 0; b < skinInfo->GetNumBones(); ++b)
        meshContainer->m_BoneOffsetMatrices[b] = *(meshContainer->pSkinInfo->GetBoneOffsetMatrix(b));

    // GenerateSkinnedMesh will take the general skinning information and
    // transform it to a HW friendly version
    V_RETURN(GenerateSkinnedMesh(device, meshContainer));

    *newMeshContainer = meshContainer;
    SAFE_RELEASE(device);
    return S_OK;
}

HRESULT CAllocateHierarchy::DestroyFrame(LPD3DXFRAME frame)
{
    SAFE_DELETE_ARRAY(frame->Name);
    SAFE_DELETE(frame);
    return S_OK;
}

HRESULT CAllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER meshContainerBase)
{
    D3DXMESHCONTAINER_DERIVED* meshContainer = static_cast<D3DXMESHCONTAINER_DERIVED*>(meshContainerBase);
    SAFE_DELETE_ARRAY(meshContainer->Name);
    SAFE_DELETE_ARRAY(meshContainer->pAdjacency);
    SAFE_DELETE_ARRAY(meshContainer->pMaterials);
    SAFE_DELETE_ARRAY(meshContainer->m_BoneOffsetMatrices);
    if (meshContainer->m_Textures)
        for (unsigned int m = 0; m < meshContainer->NumMaterials; ++m)
            SAFE_RELEASE(meshContainer->m_Textures[m]);
    SAFE_DELETE_ARRAY(meshContainer->m_Textures);
    SAFE_DELETE_ARRAY(meshContainer->m_BoneMatrices);
    SAFE_RELEASE(meshContainer->m_BoneCombinationBuffer);
    SAFE_RELEASE(meshContainer->MeshData.pMesh);
    SAFE_RELEASE(meshContainer->pSkinInfo);
    SAFE_RELEASE(meshContainer->m_Mesh);
    SAFE_DELETE(meshContainer);
    return S_OK;
}

// Called either by CreateMeshContainer when loading a skin mesh, or when
// changing methods. This function uses the skinInfo of the mesh
// container to generate the desired drawable mesh and bone combination
// table.
HRESULT GenerateSkinnedMesh(IDirect3DDevice9* device, D3DXMESHCONTAINER_DERIVED* meshContainer)
{
    HRESULT hr = S_OK;

    SAFE_RELEASE(meshContainer->MeshData.pMesh);
    SAFE_RELEASE(meshContainer->m_BoneCombinationBuffer);

    // Get palette size
    // First 9 constants are used for other data. Each 4x3 matrix takes
    // up 3 constants.
    // (96 - 9) /3 i.e. Maximum constant count - used constants
    unsigned int MaxMatrices = 26;
    meshContainer->m_PaletteEntryNum = min(MaxMatrices, meshContainer->pSkinInfo->GetNumBones());

    LPD3DXBUFFER buffer;
    V_RETURN(D3DXCreateBuffer(meshContainer->m_Mesh->GetNumFaces(), &buffer));
    V_RETURN(meshContainer->pSkinInfo->ConvertToIndexedBlendedMesh(meshContainer->m_Mesh,
             D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE, meshContainer->m_PaletteEntryNum, meshContainer->pAdjacency, 0,
             0, &buffer, &meshContainer->m_InfluenceNum, &meshContainer->m_AttributeGroupNum,
             &meshContainer->m_BoneCombinationBuffer, &meshContainer->MeshData.pMesh));
    V_RETURN(meshContainer->pSkinInfo->Remap(buffer->GetBufferSize() / sizeof(DWORD),
             static_cast<DWORD*>(buffer->GetBufferPointer())));
    buffer->Release();
    DWORD fvf = (meshContainer->MeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK) |
        D3DFVF_NORMAL |
        D3DFVF_TEX1 |
        D3DFVF_LASTBETA_UBYTE4;
    if (fvf != meshContainer->MeshData.pMesh->GetFVF()) {
        LPD3DXMESH mesh;
        V_RETURN(meshContainer->MeshData.pMesh->CloneMeshFVF(meshContainer->MeshData.pMesh->GetOptions(), fvf, device,
                 &mesh));
        meshContainer->MeshData.pMesh->Release();
        meshContainer->MeshData.pMesh = mesh;
    }
    D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
    V_RETURN(meshContainer->MeshData.pMesh->GetDeclaration(declaration));

    // the vertex shader is expecting to interpret the UBYTE4 as a
    // D3DCOLOR, so update the type
    // NOTE: this cannot be done with CloneMesh, that would convert the
    // UBYTE4 data to float and then to D3DCOLOR
    // this is more of a "cast" operation
    LPD3DVERTEXELEMENT9 declarationPtr = declaration;
    while (declarationPtr->Stream != 0xff) {
        if ((declarationPtr->Usage == D3DDECLUSAGE_BLENDINDICES) && (declarationPtr->UsageIndex == 0))
            declarationPtr->Type = D3DDECLTYPE_D3DCOLOR;
        ++declarationPtr;
    }
    V_RETURN(meshContainer->MeshData.pMesh->UpdateSemantics(declaration));

    // allocate a buffer for bone matrices, but only if another mesh has
    // not allocated one of the same size or larger
    // Allocate space for blend matrices
    SAFE_DELETE_ARRAY(g_CharacterBoneMatrices);

    g_CharacterBoneMatrices = new D3DXMATRIXA16[meshContainer->pSkinInfo->GetNumBones()];

    return S_OK;
}

char* CopyName(const char* str)
{
    int length = static_cast<int>(strlen(str) + 1);
    char* newStr = new char[length];
    memcpy(newStr, str, length * sizeof(char));
    return newStr;
}
}
