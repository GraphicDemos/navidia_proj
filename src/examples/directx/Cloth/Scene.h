
#ifndef SCENE_H
#define SCENE_H

#include <list>

namespace Scene
{

// Initialization and cleanup
void Initialize();
void Cleanup();

// Device management
HRESULT OnCreateDevice(IDirect3DDevice9*);
void OnDestroyDevice();
HRESULT OnResetDevice(IDirect3DDevice9*);
void OnLostDevice();

// Animation
void Animate(float);

// Simulation
void Reset();
void SetSelectionFree(bool);
void Simulate(float, float, float);
void Interpolate(float);
void Cut(D3DXVECTOR3[3]);

// Rendering
void Render(IDirect3DDevice9*, const D3DXMATRIX&, const D3DXMATRIX&, bool, bool, bool, bool, bool, bool);
void RenderID(const D3DXMATRIX&);
void RenderWorldPosition(IDirect3DDevice9*, const D3DXMATRIX&, bool, bool);

// Environment
extern bool g_MoveEnvironmentOnly;
extern float g_Thinning;

// Object definition
class Object
{

public:

    // Initialization
    Object(bool = true, bool = true);
    virtual ~Object() { }

    // Device management
    virtual HRESULT OnCreateDevice(IDirect3DDevice9*) { return S_OK; }
    virtual void OnLostDevice() { }
    virtual HRESULT OnResetDevice(IDirect3DDevice9*) { return S_OK; }
    virtual void OnDestroyDevice() { }

    // ID
    unsigned short ID() const { return m_ID; }
    void SetID(unsigned short ID) { m_ID = ID; }

    // Size
    virtual void SetSize(int, int) { }
    virtual int Width() const { return 0; }
    virtual int Height() const { return 0; }

    // Dimension
    D3DXVECTOR3 Dimension() const;
    virtual void SetDimension(const D3DXVECTOR3&);

    // Position and orientation
    virtual const D3DXMATRIX& World() { return m_World; }
    virtual void SetWorld(const D3DXMATRIX&);

    // Simulation
    virtual void Reset() { }
    virtual void SetSelectionFree(bool) { }
    virtual void Simulate(float, float, float) { }
    virtual void Interpolate(float) { }
    virtual int RelaxationIterations() { return 0; }
    virtual void SetRelaxationIterations(int) { }
    virtual bool ShearConstraint(bool) { return false; }
    virtual void SetShearConstraint(bool) { }
    virtual float GravityStrength() { return 0; }
    virtual void SetGravityStrength(float) { }
    virtual float WindStrength() { return 0; }
    virtual void SetWindStrength(float) { }
    virtual float WindHeading() { return 0; }
    virtual void SetWindHeading(float) { }
    virtual void RemoveFromCollisionSet(int) { }
    virtual bool IsInCollisionSet(int) { return true; }
    virtual float* BeginPlaneList() { return 0; };
    virtual void EndPlaneList(int) { };
    virtual float* BeginSphereList() { return 0; };
    virtual void EndSphereList(int) { };
    virtual float* BeginBoxList() { return 0; };
    virtual void EndBoxList(int) { };
    virtual float* BeginEllipsoidList() { return 0; };
    virtual void EndEllipsoidList(int) { };

    // Rendering
    virtual void Render(ID3DXEffect&, const D3DXMATRIX&, bool = false);
    virtual void RenderID(const D3DXMATRIX&);
    virtual void RenderWorldPosition(const D3DXMATRIX&, bool) { }
    virtual void RenderNormals(const D3DXMATRIX&) { }
    virtual void RenderSelection(const D3DXMATRIX&) { }
    float DiffuseCoeff() const { return m_DiffuseCoeff; }
    float SpecularCoeff() const { return m_SpecularCoeff; }
    float SpecularPower() const { return m_SpecularPower; }
    LPDIRECT3DTEXTURE9 ColorTexture() const { return m_ColorTexture; }
    LPDIRECT3DTEXTURE9 NormalMap() const { return m_NormalMap; }
    void SetColorTexture(LPDIRECT3DTEXTURE9 colorTexture, float scaling = 1) { m_ColorTexture = colorTexture; m_TexCoordScaling = scaling; }
    void SetNormalMap(LPDIRECT3DTEXTURE9 normalMap) { m_NormalMap = normalMap; }
    void SetDiffuseCoeff(float coeff) { m_DiffuseCoeff = coeff; }
    void SetSpecularity(float coeff, float power = 1000) { m_SpecularCoeff = coeff; m_SpecularPower = power; }

    // Selection
    virtual void Target(float, float) { m_IsTargeted = !m_IsTargeted; }
    virtual void Select(float = -1, float = -1, bool = false) { m_IsSelected = !m_IsSelected; }
    bool IsSelectable() const { return m_IsSelectable; }
    bool IsSelected() const { return m_IsSelected; }
    bool IsTargeted() const { return m_IsTargeted; }
    virtual void SetAnchorPosition(int, const D3DXVECTOR3&) { }

    // Cutting
    virtual void Uncut() { }
    virtual void Cut(D3DXVECTOR3[3]) { }

    // Exporting
    virtual float* Export(float* data) { return data; }

protected:
    static D3DXVECTOR3 m_Color;
    IDirect3DDevice9* m_Device;
    unsigned short m_ID;
    D3DXMATRIX m_World;
    LPD3DXMESH m_Mesh;
    LPDIRECT3DTEXTURE9 m_ColorTexture;
    LPDIRECT3DTEXTURE9 m_NormalMap;
    float m_TexCoordScaling;
    float m_DiffuseCoeff;
    float m_SpecularCoeff;
    float m_SpecularPower;
    bool m_IsVisible;
    bool m_IsSelectable;
    bool m_IsSelected;
    bool m_IsTargeted;
    virtual void ComputeThinningMatrix(float, D3DXMATRIX&);
};

// Object categories
class Plane;
class Sphere;
class Box;
class Ellipsoid;

// Object list
extern std::list<Object *> g_ObjectList;
enum
{
    CLOTH_CURTAIN,
    CLOTH_FLAG,
    CLOTH_CAPE,
    CLOTH_SKIRT,
    CLOTH_NUM
};
extern Object* g_Cloth[CLOTH_NUM];

// Object positions
extern D3DXVECTOR3 g_SceneCenter[CLOTH_NUM];

// Object rendering
extern bool g_RenderScene[CLOTH_NUM];

// Object list edition
Plane* AddPlane(IDirect3DDevice9*, const D3DXVECTOR3&, float, bool = true, bool = true);
Sphere* AddSphere(IDirect3DDevice9*, const D3DXVECTOR3&, float, bool = true, bool = true);
Box* AddBox(IDirect3DDevice9*, const D3DXVECTOR3&, const D3DXVECTOR3&, bool = true, bool = true);
Ellipsoid* AddEllipsoid(IDirect3DDevice9*, const D3DXVECTOR3&, const D3DXVECTOR3&, bool = true, bool = true);
void RemoveSelection();
}
#endif
