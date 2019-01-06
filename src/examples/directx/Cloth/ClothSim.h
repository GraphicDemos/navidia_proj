
#ifndef CLOTH_SIM_H
#define CLOTH_SIM_H

#include <list>

// Cloth simulator
class ClothSim
{

public:

    // Initialization
    ClothSim(const D3DXVECTOR3&, const D3DXVECTOR3&, const D3DXVECTOR3&, const wchar_t* = 0, int = 32, int = 32);
    ~ClothSim();
    void Initialize();

    // Device management
    HRESULT OnCreateDevice(IDirect3DDevice9*);
    void OnDestroyDevice();
    HRESULT OnResetDevice();
    void OnLostDevice();

    // Dimension
    void SetSize(int, int);
    int Width() { return m_Width; }
    int Height() { return m_Height; }

    // Simulation
    void Simulate(float, float, float, bool, LPDIRECT3DVERTEXBUFFER9, int, const D3DXMATRIX*);
    void Interpolate(float);
    void SetRelaxationIterations(int relaxationIterations) { m_RelaxationIterations = relaxationIterations; }
    int RelaxationIterations() { return m_RelaxationIterations; }

    // Positions
    const LPDIRECT3DTEXTURE9 PositionTexture() { return m_PositionTexture[m_InterpolatedPositions]; }

    // Normals
    const LPDIRECT3DTEXTURE9 NormalTexture() { return m_NormalTexture; }

    // Forces
    void SetShearConstraint(bool shearConstraint) { m_ShearConstraint = shearConstraint; }
    bool ShearConstraint() { return m_ShearConstraint; }
    void SetGravityStrength(float gravityStrength) { m_GravityStrength = gravityStrength; }
    float GravityStrength() { return m_GravityStrength; }
    void SetWindStrength(float windStrength) { m_WindStrength = windStrength; }
    float WindStrength() { return m_WindStrength; }
    void SetWindHeading(float windHeading) { m_WindHeading = windHeading; }
    float WindHeading() { return m_WindHeading; }

    // Collision objects
    void SetPlaneTexture(LPDIRECT3DTEXTURE9, int, int);
    void SetSphereTexture(LPDIRECT3DTEXTURE9, int, int);
    void SetBoxTexture(LPDIRECT3DTEXTURE9, int, int);
    void SetEllipsoidTexture(LPDIRECT3DTEXTURE9, int, int);

    // Anchor points
    struct Position
    {
        static const DWORD FVF = (D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE3(1));
        D3DXVECTOR2 m_XY;
        D3DXVECTOR3 m_Value;
    };
    void SetAnchorPositionsVertexBuffer(LPDIRECT3DVERTEXBUFFER9, int);

    // Seam
    void SetSeam(Position[4], int[8]);
    void SetHalfSeam(Position*, int[2], int[2], Position*, int[2], int[2]);

    // Cut
    bool ComputeCuts(D3DXVECTOR3[3]);
    char TriangleIsCut(int index) const { return m_TriangleIsCut[index]; }
    void Uncut();

    // Instance number
    static int Num() { return m_Num; }

private:

    // Instance number
    static int m_Num;

    // Device
    IDirect3DDevice9* m_Device;

    // Size (number of vertices in both dimensions)
    int m_Width;
    int m_Height;

    // Simulation
    void ComputeNormals();
    void DrawFullscreenQuad();
    void UpdateForceCoefficients();
    void UpdateSeam();
    void UpdateResponsivenessTextures();

    // Simulation quad
    static LPDIRECT3DVERTEXBUFFER9 m_Quad;

    // Simulation effect
    static LPD3DXEFFECT m_SimEffect;
    static bool m_SimEffectIsLost;

    // Positions and normals
    wchar_t* m_Filename;
    LPDIRECT3DTEXTURE9 m_PositionTexture[3], m_PositionTextureSysMem, m_InitialPositionTexture;
    LPDIRECT3DSURFACE9 m_PositionSurface[3], m_PositionSurfaceSysMem;
    LPDIRECT3DTEXTURE9 m_NormalTexture, m_NormalIntermediateTexture;
    LPDIRECT3DSURFACE9 m_NormalSurface, m_NormalIntermediateSurface;
    int m_NewPositions, m_CurrentPositions, m_OldPositions, m_InterpolatedPositions;
    bool m_PositionsAreValid;
    bool m_NormalsAreValid;
    D3DXVECTOR3 m_Center;
    D3DXVECTOR3 m_EdgeX;
    D3DXVECTOR3 m_EdgeY;

    // Forces
    float m_GravityStrength;
    float m_WindStrength;
    float m_WindHeading;
    bool m_ForceCoefficientsAreValid;

    // Constraints
    int m_RelaxationIterations;
    bool m_ShearConstraint;
    float m_DistanceAtRestXY;
    float m_DistanceAtRestX;
    float m_DistanceAtRestY;
    enum
    {
        SPRING_NUM  = 8
    };  // Number of springs attached to a single vertex
    LPDIRECT3DTEXTURE9 m_ResponsivenessTexture[SPRING_NUM], m_DistanceAtRestTexture[SPRING_NUM];
    bool m_ResponsivenessTextureIsValid;

    // Collision objects
    LPDIRECT3DTEXTURE9 m_PlaneListTexture;
    int m_PlaneListTextureSize;
    int m_PlaneNum;
    LPDIRECT3DTEXTURE9 m_SphereListTexture;
    int m_SphereListTextureSize;
    int m_SphereNum;
    LPDIRECT3DTEXTURE9 m_BoxListTexture;
    int m_BoxListTextureSize;
    int m_BoxNum;
    LPDIRECT3DTEXTURE9 m_EllipsoidListTexture;
    int m_EllipsoidListTextureSize;
    int m_EllipsoidNum;

    // Anchor points
    LPDIRECT3DVERTEXBUFFER9 m_AnchorPositionBuffer;
    int m_AnchorPositionNum;

    // Seam
    LPDIRECT3DVERTEXBUFFER9 m_SeamBuffer;
    int m_SeamNum;
    bool m_SeamIsValid;

    // Cutting
    LPDIRECT3DSURFACE9 m_CutRTSurface;
    char* m_TriangleIsCut;
	void Cut(unsigned char*[], int[], int, int, int, int, int);

    // Noise
    static LPDIRECT3DTEXTURE9 m_NoiseTexture;
    static LPD3DXTEXTURESHADER m_NoiseTextureShader;
};
#endif
