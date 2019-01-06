
// Header files
#include "nvafx.h"
#include <shared/GetFilePath.h>
#include "ClothSim.h"
#include <fstream>

// Static members
int ClothSim::m_Num;
LPDIRECT3DVERTEXBUFFER9 ClothSim::m_Quad;
LPD3DXEFFECT ClothSim::m_SimEffect;
bool ClothSim::m_SimEffectIsLost = true;
LPDIRECT3DTEXTURE9 ClothSim::m_NoiseTexture;
LPD3DXTEXTURESHADER ClothSim::m_NoiseTextureShader;

/*----------------------------------------------------------------------------------------------------------------------
    Initialization
 ----------------------------------------------------------------------------------------------------------------------- */

ClothSim::ClothSim(const D3DXVECTOR3& center, const D3DXVECTOR3& edgeX, const D3DXVECTOR3& edgeY, const wchar_t* filename,
                   int width, int height) :
    m_Device(0),
    m_PositionsAreValid(false),
    m_NormalsAreValid(false),
    m_SeamIsValid(false),
    m_ForceCoefficientsAreValid(false),
    m_ResponsivenessTextureIsValid(false),
    m_Width(width),
    m_Height(height),
    m_AnchorPositionBuffer(0),
    m_AnchorPositionNum(0),
    m_SeamBuffer(0),
    m_SeamNum(0),
    m_CutRTSurface(0),
    m_TriangleIsCut(0),
    m_PlaneListTexture(0),
    m_PlaneNum(0),
    m_SphereListTexture(0),
    m_SphereNum(0),
    m_BoxListTexture(0),
    m_BoxNum(0),
    m_EllipsoidListTexture(0),
    m_EllipsoidNum(0),
    m_InitialPositionTexture(0),
    m_PositionTextureSysMem(0),
    m_PositionSurfaceSysMem(0),
    m_NewPositions(0),
    m_CurrentPositions(0),
    m_OldPositions(0),
    m_InterpolatedPositions(0),
    m_RelaxationIterations(1),
    m_ShearConstraint(false),
    m_NormalTexture(0),
    m_NormalSurface(0),
    m_NormalIntermediateTexture(0),
    m_NormalIntermediateSurface(0),
    m_GravityStrength(0),
    m_WindStrength(0),
    m_WindHeading(0)
{
    for (int i = 0; i < SPRING_NUM; ++i) {
        m_ResponsivenessTexture[i] = 0;
        m_DistanceAtRestTexture[i] = 0;
    }
    for (int i = 0; i < 3; ++i) {
        m_PositionTexture[i] = 0;
        m_PositionSurface[i] = 0;
    }
    m_Center = center;
    m_EdgeX = edgeX;
    m_EdgeY = edgeY;
    if (filename) {
        m_Filename = new wchar_t[wcslen(filename) + 1];
        wcscpy(m_Filename, filename);
    }
    else
        m_Filename = 0;
}

ClothSim::~ClothSim()
{
    SAFE_DELETE_ARRAY(m_Filename);
}

void ClothSim::Initialize()
{
    if (m_NoiseTextureShader == 0) {
        LPD3DXBUFFER shader;
		HRESULT hr;

		 hr = D3DXCompileShaderFromFile(GetFilePath::GetFilePath(TEXT("MEDIA/programs/Cloth/ClothSim.fx"), true).c_str(), 0, 0,
			"GenerateNoise", "tx_1_0", 0, &shader, 0, 0) ;

        D3DXCreateTextureShader (static_cast<DWORD*>(shader->GetBufferPointer ()),&m_NoiseTextureShader);
        shader->Release();
    }
}

/*----------------------------------------------------------------------------------------------------------------------
    Device management
 ----------------------------------------------------------------------------------------------------------------------- */

HRESULT ClothSim::OnCreateDevice(IDirect3DDevice9* device)
{
    HRESULT hr;

    // Device
    m_Device = device;

    // Simulation quad
    if (m_Quad == 0) {
        V_RETURN(m_Device->CreateVertexBuffer(4 * sizeof(D3DXVECTOR2), D3DUSAGE_WRITEONLY, D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
                D3DPOOL_MANAGED, &m_Quad, 0));
        D3DXVECTOR2 simulationVertices[4];
        simulationVertices[0] = D3DXVECTOR2(-1.0f, 1.0f);
        simulationVertices[1] = D3DXVECTOR2(1.0f, 1.0f);
        simulationVertices[2] = D3DXVECTOR2(1.0f, -1.0f);
        simulationVertices[3] = D3DXVECTOR2(-1.0f, -1.0f);
        void* buffer;
        hr = m_Quad->Lock(0, 0, &buffer, 0);
        memcpy(buffer, simulationVertices, 4 * sizeof(D3DXVECTOR2));
        m_Quad->Unlock();
    }

    // Simulation effect
    if (m_SimEffect == 0)
        V_RETURN(D3DXCreateEffectFromFile(m_Device,
                 GetFilePath::GetFilePath(TEXT("MEDIA/programs/Cloth/ClothSim.cso"), true).c_str(), 0, 0, D3DXSHADER_DEBUG,
                 0, &m_SimEffect, 0));
    if (m_Filename) {
        D3DXIMAGE_INFO info;
        V_RETURN(D3DXCreateTextureFromFileEx(m_Device, m_Filename,
                                             D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
                                             1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED,
                                             D3DX_DEFAULT, D3DX_DEFAULT, 0,
                                             &info, 0, &m_InitialPositionTexture));
        m_Width = info.Width;
        m_Height = info.Height;
        
        // Seam
		size_t len = wcslen(m_Filename);
		char* seamFilename = new char[len + 2];
		wcstombs(seamFilename, m_Filename, len);
		seamFilename[len - 3] = 's';
		seamFilename[len - 2] = 'e';
		seamFilename[len - 1] = 'a';
		seamFilename[len] = 'm';
		seamFilename[len + 1] = 0;
		std::ifstream seamFile(seamFilename);
		delete [] seamFilename;
        int seamSizeMax = 2 * 2 * (m_Width + m_Height);
		int* coords = new int[seamSizeMax];
		int i = 0;
        for (i = 0; (i < seamSizeMax) && seamFile && !seamFile.eof(); ++i)
		    seamFile >> coords[i];
        m_SeamNum = i / 2;
        V_RETURN(m_Device->CreateVertexBuffer(m_SeamNum * sizeof(Position), 0, Position::FVF, D3DPOOL_MANAGED,
                &m_SeamBuffer, 0));
        Position* positions;
        V_RETURN(m_SeamBuffer->Lock(0, 0, (void**) &positions, 0));
        for (int v = 0; v < m_SeamNum; v += 4)
            SetSeam(&positions[v], &coords[2 * v]);
        V_RETURN(m_SeamBuffer->Unlock());
        delete [] coords;
    }

    // Position buffer to back up positions during device loss
    V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM,
             &m_PositionTextureSysMem));
    V_RETURN(m_PositionTextureSysMem->GetSurfaceLevel(0, &m_PositionSurfaceSysMem));

    // Noise
    if (m_NoiseTexture == 0) {
        V_RETURN(D3DXCreateTexture(device, 16, 16, D3DX_DEFAULT, 0, D3DFMT_R32F, D3DPOOL_MANAGED, &m_NoiseTexture));
        V_RETURN(D3DXFillTextureTX(m_NoiseTexture, m_NoiseTextureShader));
    }

    // Distance at rest
    D3DXVECTOR2 constraint(2 * D3DXVec3Length(&m_EdgeX) / (m_Width - 1), 2 * D3DXVec3Length(&m_EdgeY) / (m_Height - 1));
    m_DistanceAtRestXY = D3DXVec2Length(&constraint);
    m_DistanceAtRestX = constraint.x;
    m_DistanceAtRestY = constraint.y;
    if (m_InitialPositionTexture) {
        D3DLOCKED_RECT posRect;
        V_RETURN(m_InitialPositionTexture->LockRect(0, &posRect, 0, D3DLOCK_READONLY));
        D3DLOCKED_RECT distRect;

        // X-aligned springs
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[0]));
        V_RETURN(m_DistanceAtRestTexture[0]->LockRect(0, &distRect, 0, 0));
        for (int y = 0; y < m_Height; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            for (int i = 0; i < m_Width / 2; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i] - position[2 * i + 1];
                *dist = D3DXVec4Length(&D);
            }
        }
        V_RETURN(m_DistanceAtRestTexture[0]->UnlockRect(0));
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[1]));
        V_RETURN(m_DistanceAtRestTexture[1]->LockRect(0, &distRect, 0, 0));
        for (int y = 0; y < m_Height; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            for (int i = 0; i < m_Width / 2 - 1; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i + 1] - position[2 * i + 2];
                *dist = D3DXVec4Length(&D);
            }
            *dist = 0;
        }
        V_RETURN(m_DistanceAtRestTexture[1]->UnlockRect(0));

        // Y-aligned springs
        V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height / 2, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[2]));
        V_RETURN(m_DistanceAtRestTexture[2]->LockRect(0, &distRect, 0, 0));
        for (int i = 0; i < m_Height / 2; ++i) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + i * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + 2 * i * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (2 * i + 1) * posRect.Pitch);
            for (int x = 0; x < m_Width; ++x, ++dist) {
                D3DXVECTOR4 D = position[x] - position2[x];
                *dist = D3DXVec4Length(&D);
            }
        }
        V_RETURN(m_DistanceAtRestTexture[2]->UnlockRect(0));
        V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height / 2, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[3]));
        V_RETURN(m_DistanceAtRestTexture[3]->LockRect(0, &distRect, 0, 0));
        for (int i = 0; i < m_Height / 2 - 1; ++i) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + i * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (2 * i + 1) * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (2 * i + 2) * posRect.Pitch);
            for (int x = 0; x < m_Width; ++x, ++dist) {
                D3DXVECTOR4 D = position[x] - position2[x];
                *dist = D3DXVec4Length(&D);
            }
        }
        memset(static_cast<unsigned char*>(distRect.pBits) + (m_Height / 2 - 1) * distRect.Pitch, 0, distRect.Pitch);
        V_RETURN(m_DistanceAtRestTexture[3]->UnlockRect(0));

        // XY-aligned springs
        // XY down
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[4]));
        V_RETURN(m_DistanceAtRestTexture[4]->LockRect(0, &distRect, 0, 0));
        for (int y = 0; y < m_Height - 1; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (y + 1) * posRect.Pitch);
            for (int i = 0; i < m_Width / 2; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i] - position2[2 * i + 1];
                *dist = D3DXVec4Length(&D);
            }
        }
        memset(static_cast<unsigned char*>(distRect.pBits) + (m_Height - 1) * distRect.Pitch, 0, distRect.Pitch);
        V_RETURN(m_DistanceAtRestTexture[4]->UnlockRect(0));
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[5]));
        V_RETURN(m_DistanceAtRestTexture[5]->LockRect(0, &distRect, 0, 0));
        for (int y = 0; y < m_Height - 1; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (y + 1) * posRect.Pitch);
            for (int i = 0; i < m_Width / 2 - 1; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i + 1] - position2[2 * i + 2];
                *dist = D3DXVec4Length(&D);
            }
            *dist = 0;
        }
        memset(static_cast<unsigned char*>(distRect.pBits) + (m_Height - 1) * distRect.Pitch, 0, distRect.Pitch);
        V_RETURN(m_DistanceAtRestTexture[5]->UnlockRect(0));
        // XY up
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[6]));
        V_RETURN(m_DistanceAtRestTexture[6]->LockRect(0, &distRect, 0, 0));
        memset(static_cast<unsigned char*>(distRect.pBits), 0, distRect.Pitch);
        for (int y = 1; y < m_Height; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (y - 1) * posRect.Pitch);
            for (int i = 0; i < m_Width / 2; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i] - position2[2 * i + 1];
                *dist = D3DXVec4Length(&D);
            }
        }
        V_RETURN(m_DistanceAtRestTexture[6]->UnlockRect(0));
        V_RETURN(D3DXCreateTexture(m_Device, m_Width / 2, m_Height, 1, 0, D3DFMT_R32F, D3DPOOL_MANAGED,
                 &m_DistanceAtRestTexture[7]));
        V_RETURN(m_DistanceAtRestTexture[7]->LockRect(0, &distRect, 0, 0));
        memset(static_cast<unsigned char*>(distRect.pBits), 0, distRect.Pitch);
        for (int y = 1; y < m_Height; ++y) {
            float* dist = reinterpret_cast<float*>(static_cast<unsigned char*>(distRect.pBits) + y * distRect.Pitch);
            D3DXVECTOR4* position = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + y * posRect.Pitch);
            D3DXVECTOR4* position2 = reinterpret_cast<D3DXVECTOR4*>(static_cast<unsigned char*>(posRect.pBits) + (y - 1) * posRect.Pitch);
            for (int i = 0; i < m_Width / 2 - 1; ++i, ++dist) {
                D3DXVECTOR4 D = position[2 * i + 1] - position2[2 * i + 2];
                *dist = D3DXVec4Length(&D);
            }
            *dist = 0;
        }
        V_RETURN(m_DistanceAtRestTexture[7]->UnlockRect(0));

        V_RETURN(m_InitialPositionTexture->UnlockRect(0));
    }

    // Cut
    int numTriangles = 2 * (m_Width - 1) * (m_Height - 1);
    m_TriangleIsCut = new char[numTriangles];
    memset(m_TriangleIsCut, 0, numTriangles);

    ++m_Num;
    return S_OK;
}

void ClothSim::OnDestroyDevice()
{
    SAFE_RELEASE(m_InitialPositionTexture);
    SAFE_RELEASE(m_PositionTextureSysMem);
    SAFE_RELEASE(m_PositionSurfaceSysMem);
	SAFE_RELEASE(m_SeamBuffer);
    --m_Num;
    if (m_Num == 0) {
        SAFE_RELEASE(m_Quad);
        SAFE_RELEASE(m_SimEffect);
        SAFE_RELEASE(m_NoiseTexture);
        SAFE_RELEASE(m_NoiseTextureShader);
    }
    SAFE_DELETE_ARRAY(m_TriangleIsCut);
    for (int i = 0; i < SPRING_NUM; ++i)
        SAFE_RELEASE(m_DistanceAtRestTexture[i]);
}

HRESULT ClothSim::OnResetDevice()
{
    HRESULT hr;

    // Effect
    if (m_SimEffect && m_SimEffectIsLost) {
        m_SimEffect->OnResetDevice();
        m_SimEffectIsLost = false;
    }

    // Position texture
    for (int i = 0; i < 3; i++) {
        V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F,
                 D3DPOOL_DEFAULT, &m_PositionTexture[i]));
        V_RETURN(m_PositionTexture[i]->GetSurfaceLevel(0, &m_PositionSurface[i]));
    }

    // Normal texture
    V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F,
             D3DPOOL_DEFAULT, &m_NormalTexture));
    V_RETURN(m_NormalTexture->GetSurfaceLevel(0, &m_NormalSurface));
    if (m_SeamBuffer) {
        V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F,
                D3DPOOL_DEFAULT, &m_NormalIntermediateTexture));
        V_RETURN(m_NormalIntermediateTexture->GetSurfaceLevel(0, &m_NormalIntermediateSurface));
    }

    // Responsiveness textures
    for (int i = 0; i < SPRING_NUM; ++i)
        V_RETURN(D3DXCreateTexture(m_Device, m_Width, m_Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT,
                &m_ResponsivenessTexture[i]));
    m_ResponsivenessTextureIsValid = false;

    // Render target to compute cuts
    V_RETURN(m_Device->CreateRenderTarget(2 * m_Width, m_Height, D3DFMT_R5G6B5, D3DMULTISAMPLE_NONE, 0, true,
             &m_CutRTSurface, 0));

    // Update position textures
    for (int i = 0; i < 3; i++) {
        m_Device->UpdateTexture(m_PositionTextureSysMem, m_PositionTexture[i]);
        m_Device->GetRenderTargetData(m_PositionSurface[i], m_PositionSurfaceSysMem);
    }
    m_ForceCoefficientsAreValid = false;
    m_SeamIsValid = false;

    // Update position normals
    V_RETURN(m_Device->BeginScene());
    ComputeNormals();
    m_Device->EndScene();

    return S_OK;
}

void ClothSim::OnLostDevice()
{
    if (m_PositionSurfaceSysMem)
        // Save positions to restore them when device is reset
        m_Device->GetRenderTargetData(m_PositionSurface[m_InterpolatedPositions], m_PositionSurfaceSysMem);
    if (m_SimEffect) {
        m_SimEffect->OnLostDevice();
        m_SimEffectIsLost = true;
    }
    for (int i = 0; i < 3; ++i) {
        SAFE_RELEASE(m_PositionTexture[i]);
        SAFE_RELEASE(m_PositionSurface[i]);
    }
    SAFE_RELEASE(m_NormalTexture);
    SAFE_RELEASE(m_NormalSurface);
    SAFE_RELEASE(m_NormalIntermediateTexture);
    SAFE_RELEASE(m_NormalIntermediateSurface);
    for (int i = 0; i < SPRING_NUM; ++i)
        SAFE_RELEASE(m_ResponsivenessTexture[i]);
    SAFE_RELEASE(m_CutRTSurface);
}

/*----------------------------------------------------------------------------------------------------------------------
    Dimension
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::SetSize(int width, int height)
{
    if (m_Width == width && m_Height == height)
        return;
    m_Width = width;
    m_Height = height;
    OnLostDevice();
    OnDestroyDevice();
    Initialize();
    OnCreateDevice(m_Device);
    OnResetDevice();
}

/*----------------------------------------------------------------------------------------------------------------------
    Simulation
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::Simulate(float time, float timeStep, float oldTimeStep, bool reset,
                        LPDIRECT3DVERTEXBUFFER9 positionBuffer, int positionBufferNum,
                        const D3DXMATRIX* transform)
{
    // Simulation step
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_SimEffect->SetInt("RTWidth", m_Width);
    m_SimEffect->SetInt("RTHeight", m_Height);
    if (reset) {
        m_SimEffect->SetTechnique(m_InitialPositionTexture ? "ResetPositionsFromTexture" : "ResetPositions");
        m_SimEffect->SetTexture("CurrentPositionTexture", m_InitialPositionTexture);
        m_SimEffect->SetFloatArray("Origin", m_Center, 3);
        m_SimEffect->SetFloatArray("EdgeX", m_EdgeX, 3);
        m_SimEffect->SetFloatArray("EdgeY", m_EdgeY, 3);
        for (int i = 0; i < 3; i++) {
            m_Device->SetRenderTarget(0, m_PositionSurface[i]);
            unsigned int numPasses;
            m_SimEffect->Begin(&numPasses, 0);
            for (unsigned int pass = 0; pass < numPasses; ++pass) {
                m_SimEffect->BeginPass(pass);
                DrawFullscreenQuad();
                m_SimEffect->EndPass();
            }
            m_SimEffect->End();
        }
        m_OldPositions = 0;
        m_CurrentPositions = 1;
        m_NewPositions = 2;
        m_SeamIsValid = false;
        m_ForceCoefficientsAreValid = false;
    }
    else {

        // Seam
        UpdateSeam();

        // Force Coefficients
        UpdateForceCoefficients();

        // Responsiveness texture
        UpdateResponsivenessTextures();

        unsigned int numPasses;

        // Apply forces
        m_SimEffect->SetTechnique("ApplyForces_Tech");
        m_SimEffect->SetTexture("NoiseTexture", m_NoiseTexture);
        m_SimEffect->SetTexture("OldPositionTexture", m_PositionTexture[m_OldPositions]);
        m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_CurrentPositions]);
        m_SimEffect->SetFloat("GravityStrength", m_GravityStrength);
        m_SimEffect->SetFloat("WindStrength", m_WindStrength);
        m_SimEffect->SetFloat("WindHeading", m_WindHeading);
        m_SimEffect->SetFloat("TimeStep", timeStep);
        m_SimEffect->SetFloat("OldTimeStep", oldTimeStep);
        m_SimEffect->SetFloat("Time", time);
        m_Device->SetRenderTarget(0, m_PositionSurface[m_NewPositions]);
        m_SimEffect->Begin(&numPasses, 0);
        for (unsigned int pass = 0; pass < numPasses; ++pass) {
            m_SimEffect->BeginPass(pass);
            DrawFullscreenQuad();
            m_SimEffect->EndPass();
        }
        m_SimEffect->End();
        int discard = m_OldPositions;
        m_OldPositions = m_CurrentPositions;
        m_CurrentPositions = m_NewPositions;
        m_NewPositions = discard;

        if (positionBufferNum) {

            // Transform positions
            if (transform) {
                m_SimEffect->SetTechnique("TransformPositions");
                m_SimEffect->SetMatrix("Transform", transform);
                m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_OldPositions]);
            }

            // Set positions
            else if (positionBufferNum)
                m_SimEffect->SetTechnique("SetPositions");

            m_Device->SetRenderTarget(0, m_PositionSurface[m_CurrentPositions]);
            float pointSize = 1;
            m_Device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*) &pointSize));
            m_Device->SetFVF(Position::FVF);
            m_Device->SetStreamSource(0, positionBuffer, 0, sizeof(Position));
            m_SimEffect->Begin(&numPasses, 0);
            for (unsigned int pass = 0; pass < numPasses; ++pass) {
                m_SimEffect->BeginPass(pass);
                m_Device->DrawPrimitive(D3DPT_POINTLIST, 0, positionBufferNum);
                m_SimEffect->EndPass();
            }
            m_SimEffect->End();
        }

        // Satisfy constraints
        m_SimEffect->SetTechnique("SatisfyConstraints");
        D3DXHANDLE technique = m_SimEffect->GetTechniqueByName("SatisfyConstraints");
        m_SimEffect->SetFloat("Dx", 1.0f / m_Width);
        m_SimEffect->SetFloat("Dy", 1.0f / m_Height);
        m_SimEffect->SetFloat("Dp", 1.0f / m_PlaneListTextureSize);
        m_SimEffect->SetFloat("Ds", 1.0f / m_SphereListTextureSize);
        m_SimEffect->SetFloat("Db", 1.0f / m_BoxListTextureSize);
        m_SimEffect->SetFloat("De", 1.0f / m_EllipsoidListTextureSize);
        m_SimEffect->SetInt("PlaneListSize", m_PlaneNum);
        m_SimEffect->SetTexture("PlaneListTexture", m_PlaneListTexture);
        m_SimEffect->SetInt("SphereListSize", m_SphereNum);
        m_SimEffect->SetTexture("SphereListTexture", m_SphereListTexture);
        m_SimEffect->SetInt("BoxListSize", m_BoxNum);
        m_SimEffect->SetTexture("BoxListTexture", m_BoxListTexture);
        m_SimEffect->SetInt("EllipsoidListSize", m_EllipsoidNum);
        m_SimEffect->SetTexture("EllipsoidListTexture", m_EllipsoidListTexture);
        m_SimEffect->SetTexture("OldPositionTexture", m_PositionTexture[m_OldPositions]);
        for (int i = 0; i < m_RelaxationIterations; ++i) {
            m_SimEffect->Begin(&numPasses, 0);
            for (unsigned int pass = 0; pass < numPasses; ++pass) {
                D3DXPASS_DESC description;
                m_SimEffect->GetPassDesc(m_SimEffect->GetPass(technique, pass), &description);
                int n;
                if (strcmp(description.Name, "XSpringEven") == 0)
                    n = 0;
                else if (strcmp(description.Name, "XSpringOdd") == 0)
                    n = 1;
                else if (strcmp(description.Name, "YSpringEven") == 0)
                    n = 2;
                else if (strcmp(description.Name, "YSpringOdd") == 0)
                    n = 3;
                else if (strcmp(description.Name, "XYSpringDownEven") == 0)
                    n = 4;
                else if (strcmp(description.Name, "XYSpringDownOdd") == 0)
                    n = 5;
                else if (strcmp(description.Name, "XYSpringUpEven") == 0)
                    n = 6;
                else if (strcmp(description.Name, "XYSpringUpOdd") == 0)
                    n = 7;
                else
                    n = 8; // Seam and collision
                bool shearConstraint = (4 <= n && n <= 7);
                if (!m_ShearConstraint && shearConstraint)
                    continue;
                m_SimEffect->BeginPass(pass);
                m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_CurrentPositions]);
                if (n < SPRING_NUM) {
                    m_SimEffect->SetTexture("ResponsivenessTexture", m_ResponsivenessTexture[n]);
                    float distanceAtRest = 0;
                    if (m_DistanceAtRestTexture[n])
                        m_SimEffect->SetTexture("DistanceAtRestTexture", m_DistanceAtRestTexture[n]);
                    else {
                        if (shearConstraint)
                            distanceAtRest = m_DistanceAtRestXY;
                        else if (n < 2)
                            distanceAtRest = m_DistanceAtRestX;
                        else
                            distanceAtRest = m_DistanceAtRestY;
                    }
                    m_SimEffect->SetFloat("DistanceAtRest", distanceAtRest);
                }
                m_Device->SetRenderTarget(0, m_PositionSurface[m_NewPositions]);
                m_SimEffect->CommitChanges();
                DrawFullscreenQuad();
                m_SimEffect->EndPass();
                discard = m_CurrentPositions;
                m_CurrentPositions = m_NewPositions;
                m_NewPositions = discard;
            }
            m_SimEffect->End();
        }
    }
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_PositionsAreValid = true;
}

void ClothSim::Interpolate(float interpolationFactor)
{
	if (m_OldPositions == m_CurrentPositions) {
		m_InterpolatedPositions = m_CurrentPositions;
		return;
	}

    // Interpolated positions
    m_SimEffect->SetTechnique("Interpolate_Tech");
    m_SimEffect->SetTexture("OldPositionTexture", m_PositionTexture[m_OldPositions]);
    m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_CurrentPositions]);
    m_SimEffect->SetFloat("InterpolationFactor", interpolationFactor);
    m_SimEffect->SetInt("RTWidth", m_Width);
    m_SimEffect->SetInt("RTHeight", m_Height);
    m_InterpolatedPositions = m_NewPositions;
    m_Device->SetRenderTarget(0, m_PositionSurface[m_InterpolatedPositions]);
    unsigned int numPasses;
    m_SimEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        m_SimEffect->BeginPass(pass);
        DrawFullscreenQuad();
        m_SimEffect->EndPass();
    }
    m_SimEffect->End();

    // Normals
    ComputeNormals();
}

void ClothSim::ComputeNormals()
{
    RECT g_ScissorRect[] =
    {
        { 0, 0, 1, 1 },
        { 0, m_Height - 1, 1, m_Height },
        { 0, 1, 1, m_Height - 1 },
        { m_Width - 1, 0, m_Width, 1 },
        { m_Width - 1, m_Height - 1, m_Width, m_Height },
        { m_Width - 1, 1, m_Width, m_Height - 1 },
        { 1, 0, m_Width - 1, 1 },
        { 1, m_Height - 1, m_Width - 1, m_Height },
        { 1, 1, m_Width - 1, m_Height - 1 }
    };
    m_SimEffect->SetTechnique("ComputeNormals");
    m_SimEffect->SetInt("RTWidth", m_Width);
    m_SimEffect->SetInt("RTHeight", m_Height);
    m_SimEffect->SetFloat("Dx", 1.0f / m_Width);
    m_SimEffect->SetFloat("Dy", 1.0f / m_Height);
    m_Device->SetRenderTarget(0, m_SeamBuffer ? m_NormalIntermediateSurface : m_NormalSurface);
    m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_InterpolatedPositions]);
    m_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    unsigned int numPasses;
    m_SimEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        if (pass < numPasses - 1)
            m_Device->SetScissorRect(&g_ScissorRect[pass]);
        else if (m_SeamBuffer) {
            m_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            m_SimEffect->SetTexture("NormalTexture", m_NormalIntermediateTexture);
            m_Device->SetRenderTarget(0, m_NormalSurface);
        }
        else
            break;
        m_SimEffect->BeginPass(pass);
        m_SimEffect->CommitChanges();
        DrawFullscreenQuad();
        m_SimEffect->EndPass();
    }
    m_SimEffect->End();
    m_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    m_NormalsAreValid = true;
}

void ClothSim::DrawFullscreenQuad()
{
    m_Device->SetFVF(D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
    m_Device->SetStreamSource(0, m_Quad, 0, sizeof(D3DXVECTOR2));
    m_Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

/*----------------------------------------------------------------------------------------------------------------------
    Collision objects
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::SetPlaneTexture(LPDIRECT3DTEXTURE9 texture, int size, int num)
{
    m_PlaneListTexture = texture;
    m_PlaneListTextureSize = size;
    m_PlaneNum = num;
}

void ClothSim::SetSphereTexture(LPDIRECT3DTEXTURE9 texture, int size, int num)
{
    m_SphereListTexture = texture;
    m_SphereListTextureSize = size;
    m_SphereNum = num;
}

void ClothSim::SetBoxTexture(LPDIRECT3DTEXTURE9 texture, int size, int num)
{
    m_BoxListTexture = texture;
    m_BoxListTextureSize = size;
    m_BoxNum = num;
}

void ClothSim::SetEllipsoidTexture(LPDIRECT3DTEXTURE9 texture, int size, int num)
{
    m_EllipsoidListTexture = texture;
    m_EllipsoidListTextureSize = size;
    m_EllipsoidNum = num;
}

/*----------------------------------------------------------------------------------------------------------------------
    Anchor points
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::SetAnchorPositionsVertexBuffer(LPDIRECT3DVERTEXBUFFER9 vertexBuffer, int num)
{
    m_AnchorPositionBuffer = vertexBuffer;
    m_AnchorPositionNum = num;
    m_ResponsivenessTextureIsValid = false;
    m_ForceCoefficientsAreValid = false;
}

/*----------------------------------------------------------------------------------------------------------------------
    Cuts
 ----------------------------------------------------------------------------------------------------------------------- */

bool ClothSim::ComputeCuts(D3DXVECTOR3 cutter[3])
{
    // Compute cutter features
    D3DXVECTOR3 edge[3];
    edge[0] = cutter[1] - cutter[0];
    edge[1] = cutter[2] - cutter[1];
    edge[2] = cutter[0] - cutter[2];
    D3DXVECTOR3 normal;
    D3DXVec3Cross(&normal, &edge[0], &edge[1]);
    D3DXVec3Normalize(&normal, &normal);

    // Compute cuts
    m_Device->SetRenderTarget(0, m_CutRTSurface);
    m_Device->Clear(0L, 0, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L);
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_SimEffect->SetTechnique("Cut_Tech");
    m_SimEffect->SetInt("RTWidth", 2 * m_Width);
    m_SimEffect->SetInt("RTHeight", m_Height);
    m_SimEffect->SetFloat("Dx", 1.0f / m_Width);
    m_SimEffect->SetFloat("Dy", 1.0f / m_Height);
    m_SimEffect->SetFloatArray("CutterVertex0", cutter[0], 3);
    m_SimEffect->SetFloatArray("CutterVertex1", cutter[1], 3);
    m_SimEffect->SetFloatArray("CutterVertex2", cutter[2], 3);
    m_SimEffect->SetFloatArray("CutterEdge0", edge[0], 3);
    m_SimEffect->SetFloatArray("CutterEdge1", edge[1], 3);
    m_SimEffect->SetFloatArray("CutterEdge2", edge[2], 3);
    m_SimEffect->SetFloatArray("CutterNormal", normal, 3);
    m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_InterpolatedPositions]);
    unsigned int numPasses;
    m_SimEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        m_SimEffect->BeginPass(pass);
        DrawFullscreenQuad();
        m_SimEffect->EndPass();
    }
    m_SimEffect->End();
    m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);

    // Get back cut triangles
    bool isCut = false;
    D3DLOCKED_RECT rect;
    m_CutRTSurface->LockRect(&rect, 0, D3DLOCK_READONLY);
    for (int y = 0; y < m_Height - 1; ++y)
        for (int x = 0; x < 2 * (m_Width - 1); ++x)
            if (*(static_cast<unsigned char*>(rect.pBits) + y * rect.Pitch + 2 * x)) {
                int triangleIndex = 2 * (m_Width - 1) * y + x;
                if (!m_TriangleIsCut[triangleIndex]) {
                    m_TriangleIsCut[triangleIndex] = 1;
				    m_ResponsivenessTextureIsValid = false;
                    isCut = true;
                }
            }
    m_CutRTSurface->UnlockRect();

	return isCut;
}

void ClothSim::Uncut()
{
    int numTriangles = 2 * (m_Width - 1) * (m_Height - 1);
    memset(m_TriangleIsCut, 0, numTriangles);
    m_ResponsivenessTextureIsValid = false;
}

/*----------------------------------------------------------------------------------------------------------------------
    Force coefficients
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::UpdateForceCoefficients()
{
    if (m_ForceCoefficientsAreValid)
        return;
    m_Device->SetRenderTarget(0, m_PositionSurface[m_CurrentPositions]);
    m_SimEffect->SetTexture("CurrentPositionTexture", m_PositionTexture[m_OldPositions]);
    m_SimEffect->SetTechnique("SetFree");
    unsigned int numPasses;
    m_SimEffect->Begin(&numPasses, 0);
    for (unsigned int pass = 0; pass < numPasses; ++pass) {
        m_SimEffect->BeginPass(pass);
        DrawFullscreenQuad();
        m_SimEffect->EndPass();
    }
    m_SimEffect->End();
    if (m_AnchorPositionNum) {
        m_SimEffect->SetTechnique("SetUnfree");
        float pointSize = 1;
        m_Device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*) &pointSize));
        m_Device->SetFVF(Position::FVF);
        m_Device->SetStreamSource(0, m_AnchorPositionBuffer, 0, sizeof(Position));
        m_SimEffect->Begin(&numPasses, 0);
        for (unsigned int pass = 0; pass < numPasses; ++pass) {
            m_SimEffect->BeginPass(pass);
            m_Device->DrawPrimitive(D3DPT_POINTLIST, 0, m_AnchorPositionNum);
            m_SimEffect->EndPass();
        }
        m_SimEffect->End();
    }
    m_ForceCoefficientsAreValid = true;
}

/*----------------------------------------------------------------------------------------------------------------------
    Seam
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::UpdateSeam()
{
    if (m_SeamIsValid)
        return;
    if (m_SeamNum)
        for (int i = 0; i < 3; i++) {
            m_Device->SetRenderTarget(0, m_PositionSurface[i]);
            m_SimEffect->SetTechnique("SetSeam");
            m_Device->SetFVF(Position::FVF);
            m_Device->SetStreamSource(0, m_SeamBuffer, 0, sizeof(Position));
            unsigned int numPasses;
            m_SimEffect->Begin(&numPasses, 0);
            for (unsigned int pass = 0; pass < numPasses; ++pass) {
                m_SimEffect->BeginPass(pass);
                m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_SeamNum / 2);
                m_SimEffect->EndPass();
            }
            m_SimEffect->End();
        }
    m_SeamIsValid = true;
}

void ClothSim::SetSeam(Position positions[4], int coords[8])
{
    for (int i = 0; i < 4; ++i)
	    positions[i].m_XY = D3DXVECTOR2(2 * coords[2 * i] / static_cast<float>(m_Width) - 1, - (2 * coords[2 * i + 1] / static_cast<float>(m_Height) - 1));
    SetHalfSeam(&positions[0], &coords[0], &coords[6], &positions[1], &coords[2], &coords[4]);
    SetHalfSeam(&positions[2], &coords[4], &coords[2], &positions[3], &coords[6], &coords[0]);
}

void ClothSim::SetHalfSeam(Position* position0, int coord0[2], int coordAlterEgo0[2],
                           Position* position1, int coord1[2], int coordAlterEgo1[2])
{
    if (coordAlterEgo0[0] == coordAlterEgo1[0]) {
        int i = coordAlterEgo0[0] == 0 ? 2 : 3;
		position0->m_Value[0] = i + (coordAlterEgo0[1] + 0.5f) / m_Height;
		position1->m_Value[0] = i + (coordAlterEgo1[1] + 0.5f) / m_Height;
    }
    else {
        assert(coordAlterEgo0[1] == coordAlterEgo1[1]);
        int i = coordAlterEgo0[1] == 0 ? 4 : 5;
		position0->m_Value[0] = i + (coordAlterEgo0[0] + 0.5f) / m_Width;
		position1->m_Value[0] = i + (coordAlterEgo1[0] + 0.5f) / m_Width;
    }
}

/*----------------------------------------------------------------------------------------------------------------------
    Responsiveness textures
 ----------------------------------------------------------------------------------------------------------------------- */

void ClothSim::UpdateResponsivenessTextures()
{
    if (m_ResponsivenessTextureIsValid)
        return;
    // Lock responsiveness textures
    unsigned char* data[SPRING_NUM];
    int pitch[SPRING_NUM];
    for (int i = 0; i < SPRING_NUM; ++i) {
        D3DLOCKED_RECT rect;
        m_ResponsivenessTexture[i]->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
        data[i] = static_cast<unsigned char*>(rect.pBits);
        pitch[i] = rect.Pitch;
    }
    unsigned char minResponsiveness = 0;
    unsigned char midResponsiveness = 127;
    unsigned char maxResponsiveness = 255;

    // Set responsiveness so that the cloth boundaries aren't constrained

    // X-aligned springs
    for (int y = 0; y < m_Height; ++y)
        memset(data[0] + y * pitch[0], midResponsiveness, m_Width);
    for (int y = 0; y < m_Height; ++y) {
        *(data[1] + y * pitch[1]) = minResponsiveness;
        memset(data[1] + y * pitch[1] + 1, midResponsiveness, m_Width - 2);
        *(data[1] + y * pitch[1] + m_Width - 1) = minResponsiveness;
    }

    // Y-aligned springs
    for (int y = 0; y < m_Height; ++y)
        memset(data[2] + y * pitch[2], midResponsiveness, m_Width);
    memset(data[3], minResponsiveness, m_Width);
    for (int y = 1; y < m_Height - 1; ++y)
        memset(data[3] + y * pitch[3], midResponsiveness, m_Width);
    memset(data[3] + (m_Height - 1) * pitch[3], minResponsiveness, m_Width);

    // XY-aligned springs
    // XY down
    for (int x = 0; x < m_Width; x += 2)
        *(data[4] + x) = midResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[4] + x) = minResponsiveness;
    for (int y = 1; y < m_Height - 1; ++y)
        memset(data[4] + y * pitch[4], midResponsiveness, m_Width);
    for (int x = 0; x < m_Width; x += 2)
        *(data[4] + (m_Height - 1) * pitch[4] + x) = minResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[4] + (m_Height - 1) * pitch[4] + x) = midResponsiveness;
    for (int x = 0; x < m_Width; x += 2)
        *(data[5] + x) = minResponsiveness;
    for (int x = 1; x < m_Width - 1; x += 2)
        *(data[5] + x) = midResponsiveness;
    *(data[5] + m_Width - 1) = minResponsiveness;
    for (int y = 1; y < m_Height - 1; ++y) {
        *(data[5] + y * pitch[5]) = minResponsiveness;
        memset(data[5] + y * pitch[5] + 1, midResponsiveness, m_Width - 2);
        *(data[5] + y * pitch[5] + m_Width - 1) = minResponsiveness;
    }
    *(data[5] + (m_Height - 1) * pitch[5]) = minResponsiveness;
    for (int x = 2; x < m_Width; x += 2)
        *(data[5] + (m_Height - 1) * pitch[5] + x) = midResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[5] + (m_Height - 1) * pitch[5] + x) = minResponsiveness;

    // XY up
    for (int x = 0; x < m_Width; x += 2)
        *(data[6] + x) = minResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[6] + x) = midResponsiveness;
    for (int y = 1; y < m_Height - 1; ++y)
        memset(data[6] + y * pitch[6], midResponsiveness, m_Width);
    for (int x = 0; x < m_Width; x += 2)
        *(data[6] + (m_Height - 1) * pitch[6] + x) = midResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[6] + (m_Height - 1) * pitch[6] + x) = minResponsiveness;
    *data[7] = minResponsiveness;
    for (int x = 1; x < m_Width; x += 2)
        *(data[7] + x) = minResponsiveness;
    for (int x = 2; x < m_Width; x += 2)
        *(data[7] + x) = midResponsiveness;
    for (int y = 1; y < m_Height - 1; ++y) {
        *(data[7] + y * pitch[7]) = minResponsiveness;
        memset(data[7] + y * pitch[7] + 1, midResponsiveness, m_Width - 2);
        *(data[7] + y * pitch[7] + m_Width - 1) = minResponsiveness;
    }
    for (int x = 0; x < m_Width; x += 2)
        *(data[7] + (m_Height - 1) * pitch[7] + x) = minResponsiveness;
    for (int x = 1; x < m_Width - 1; x += 2)
        *(data[7] + (m_Height - 1) * pitch[7] + x) = midResponsiveness;
    *(data[7] + (m_Height - 1) * pitch[7] + m_Width - 1) = minResponsiveness;

    // Handle anchor points
    if (m_AnchorPositionNum) {
        Position* anchorPoints;
        m_AnchorPositionBuffer->Lock(0, 0, (void**) &anchorPoints, D3DLOCK_READONLY);

        // Zero responsiveness for all springs attached to an anchor point
        for (int a = 0; a < m_AnchorPositionNum; ++a) {
            int x = static_cast<int>(m_Width * (anchorPoints[a].m_XY[0] + 1) / 2 + 0.5f);
            int y = static_cast<int>(m_Height * (-anchorPoints[a].m_XY[1] + 1) / 2 + 0.5f);
            for (int i = 0; i < SPRING_NUM; ++i)
                *(data[i] + y * pitch[i] + x) = minResponsiveness;
        }

        // Set maximum responsiveness when attached to an anchor point
        for (int a = 0; a < m_AnchorPositionNum; ++a) {
            int x = static_cast<int>(m_Width * (anchorPoints[a].m_XY[0] + 1) / 2 + 0.5f);
            int y = static_cast<int>(m_Height * (-anchorPoints[a].m_XY[1] + 1) / 2 + 0.5f);
            unsigned char* s;

            // X-aligned springs
            if (x % 2) {
                s = data[0] + y * pitch[0] + x - 1;
                if (*s)
                    *s = maxResponsiveness;
                if (x < m_Width - 1) {
                    s = data[1] + y * pitch[1] + x + 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }
            else {
                s = data[0] + y * pitch[0] + x + 1;
                if (*s)
                    *s = maxResponsiveness;
                if (0 < x) {
                    s = data[1] + y * pitch[1] + x - 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }

            // Y-aligned springs
            if (y % 2) {
                s = data[1] + (y - 1) * pitch[1] + x;
                if (*s)
                    *s = maxResponsiveness;
                if (y < m_Height - 1) {
                    s = data[2] + (y + 1) * pitch[2] + x;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }
            else {
                s = data[1] + (y + 1) * pitch[1] + x;
                if (*s)
                    *s = maxResponsiveness;
                if (0 < y) {
                    s = data[2] + (y - 1) * pitch[2] + x;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }

            // XY-aligned springs
            // XY down
            if (x % 2) {
                if (0 < y) {
                    s = data[4] + (y - 1) * pitch[4] + x - 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
                if (x < m_Width - 1 && y < m_Height - 1) {
                    s = data[5] + (y + 1) * pitch[5] + x + 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }
            else {
                if (y < m_Height - 1) {
                    s = data[4] + (y + 1) * pitch[4] + x + 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
                if (0 < x && 0 < y) {
                    s = data[5] + (y - 1) * pitch[5] + x - 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }

            // XY up
            if (x % 2) {
                if (y < m_Height - 1) {
                    s = data[6] + (y + 1) * pitch[6] + x - 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
                if (x < m_Width - 1 && 0 < y) {
                    s = data[7] + (y - 1) * pitch[7] + x + 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }
            else {
                if (0 < y) {
                    s = data[6] + (y - 1) * pitch[6] + x + 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
                if (0 < x && y < m_Height - 1) {
                    s = data[7] + (y + 1) * pitch[7] + x - 1;
                    if (*s)
                        *s = maxResponsiveness;
                }
            }
        }
        m_AnchorPositionBuffer->Unlock();
    }

    // Handle cuts
	for (int y = 0; y < m_Height - 1; ++y)
		for (int x = 0; x < m_Width - 1; ++x) {
			if (m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x] || m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x + 1])
				Cut(data, pitch, x % 2 ? 7 : 6, x, y + 1, x + 1, y); // Cut XY-aligned up spring
			if (m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x + 1]) {
				if (m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x])
					Cut(data, pitch, x % 2 ? 5 : 4, x, y, x + 1, y + 1); // Cut XY-aligned down spring
				if (y == 0 || (0 < y && m_TriangleIsCut[2 * (m_Width - 1) * (y - 1) + 2 * x]))
					Cut(data, pitch, x % 2 ? 1 : 0, x, y, x + 1, y); // Cut X-aligned spring
				if (x == m_Width - 2 || (x < m_Width - 2 && m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x + 2]))
					Cut(data, pitch, y % 2 ? 3 : 2, x + 1, y, x + 1, y + 1); // Cut Y-aligned spring
			}
            if (m_TriangleIsCut[2 * (m_Width - 1) * y + 2 * x]) {
			    if (x == 0)
				    Cut(data, pitch, y % 2 ? 3 : 2, x, y, x, y + 1); // Cut Y-aligned spring
                if (y == m_Height - 2)
					Cut(data, pitch, x % 2 ? 1 : 0, x, y + 1, x + 1, y + 1); // Cut X-aligned spring
            }
		}

    // Unlock responsiveness textures
    for (int i = 0; i < SPRING_NUM; ++i)
        m_ResponsivenessTexture[i]->UnlockRect(0);

    m_ResponsivenessTextureIsValid = true;
}

void ClothSim::Cut(unsigned char* data[], int pitch[], int i, int x0, int y0, int x1, int y1)
{
    *(data[i] + y0 * pitch[i] + x0) = 0;
    *(data[i] + y1 * pitch[i] + x1) = 0;
}
