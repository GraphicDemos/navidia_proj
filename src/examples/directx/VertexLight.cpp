//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VertexLight
// File:  VertexLight.cpp
// 
// Copyright NVIDIA Corporation 2002-2003
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Note: This code uses the D3D Framework helper library.
// Link to: DX9SDKSampleFramework.lib d3dxof.lib dxguid.lib d3dx9dt.lib d3d9.lib winmm.lib comctl32.lib DXUT.lib
//
//-----------------------------------------------------------------------------

#define STRICT

#include <Windows.h>
#include <commctrl.h>
#include <math.h>
#include <shared/MouseUI9.h>
#include <shared/GetFilePath.h>
#include <assert.h>
#include "resource.h"

#include "VertexLight.h"

HINSTANCE g_hInstance = NULL;

// Old-style D3D vertices
class Vertex
{
public:
    Vertex(const D3DXVECTOR3& Pos, const D3DXVECTOR3& Norm, const D3DXVECTOR2& Tex)
        : Position(Pos), Normal(Norm), Texture(Tex)
    {}

    Vertex()
    {}

    D3DXVECTOR3 Position;
    D3DXVECTOR3 Normal;
    D3DXVECTOR2 Texture;
};

class TLVertex
{
public:
    D3DXVECTOR4 Position;
    DWORD Diffuse;
    DWORD Specular;
    D3DXVECTOR2 Texture;
};

#define D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

//-----------------------------------------------------------------------------
// Name: VertexLight()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
VertexLight::VertexLight()
{
    m_pEffect = NULL;
    m_szCurrentTechnique[0] = 0;
    
    m_time = 0;
    m_startTime = ::timeGetTime()*0.001f;
    m_animTime = 0;
    m_frame = 0;
    
    m_LightType = LIGHTTYPE_POINT;
    m_dwCurrentLightDraw = -1;
    m_fAngle = 0;
    m_pSphereBuffer = NULL;
    m_pShapeMesh = NULL;
    m_pLightMesh = NULL;
    m_bWireframe = false;
    m_bPause = false;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT VertexLight::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;
    
    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//  The device exists, but was just lost or reset, and is now being restored.  
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here.  Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT VertexLight::RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice)
{
    assert(pd3dDevice);

    HRESULT hr;
    DWORD dwVBFlags = D3DUSAGE_WRITEONLY;

    // Load our Effect file from disk
    // note: path is relative to MEDIA\ dir
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\VertexLight.fx")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    OnChangeLightType(pd3dDevice);

    // Setup constants

    // material power 
    D3DXVECTOR4 MaterialPower(0.0f, 0.0f, 0.0f, 5.0f);
    m_pEffect->SetVector("MatPower", &MaterialPower);

    // Attenuation for lights
    D3DXVECTOR4 Attenuation(1.0f, 0.0f, 0.0f, 0.0f);
    m_pEffect->SetVector("LightAttenuation[0]", &Attenuation);
    m_pEffect->SetVector("LightAttenuation[1]", &Attenuation);
    m_pEffect->SetVector("LightAttenuation[2]", &Attenuation);

    return S_OK;
}

HRESULT VertexLight::OnChangeLightType(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;
    SAFE_DELETE(m_pShapeMesh);
    SAFE_DELETE(m_pLightMesh);
    SAFE_RELEASE(m_pSphereBuffer);

    // update light params
    switch(m_LightType)
    {
        case LIGHTTYPE_MANYPOINT:
            m_dwNumLights = 17;
            strcpy(m_szCurrentTechnique, "ManyPoint");
            break;
        case LIGHTTYPE_POINT:
            m_dwNumLights = 3;
            strcpy(m_szCurrentTechnique, "PointLight");
            break;
        case LIGHTTYPE_DIRECTIONAL:
            m_dwNumLights = 2;
            strcpy(m_szCurrentTechnique, "DirectionalLight");
            break;
        case LIGHTTYPE_TWOSIDE:
            {
                // rotate the object to a better default viewing angle:
                D3DXVECTOR4 axis = D3DXVECTOR4(-0.28f, -0.17f, -0.45f, 0.17f);
                D3DXVec4Normalize(&axis, &axis);
                D3DXQUATERNION q = D3DXQUATERNION(axis.x, axis.y, axis.z, axis.w);
                D3DXMATRIX rotMat;
                D3DXMatrixRotationQuaternion(&rotMat, &q);
            }
            m_dwNumLights = 1;
            strcpy(m_szCurrentTechnique, "TwoSide");
            break;
    }

    m_LightColorDiffuse.resize(m_dwNumLights);
    m_LightColorSpecular.resize(m_dwNumLights);
    m_LightColorAmbient.resize(m_dwNumLights);

    // Setup the light colors
    srand(1);
    for (DWORD dwCurrentLight = 0; dwCurrentLight < m_dwNumLights; dwCurrentLight++)
    {
        switch (dwCurrentLight)
        {
            case 0:
                m_LightColorDiffuse[dwCurrentLight] = D3DXCOLOR(0.8f, 0.0f, 0.0f, 1.0f);
                m_LightColorSpecular[dwCurrentLight] = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
                m_LightColorAmbient[dwCurrentLight] = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
                break;
            case 1:
                m_LightColorDiffuse[dwCurrentLight] = D3DXCOLOR(0.0f, 0.8f, 0.0f, 1.0f);
                m_LightColorSpecular[dwCurrentLight] = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
                m_LightColorAmbient[dwCurrentLight] = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
                break;
            case 2:
                m_LightColorDiffuse[dwCurrentLight] = D3DXCOLOR(0.0f, 0.0f, 0.8f, 1.0f);
                m_LightColorSpecular[dwCurrentLight] = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
                m_LightColorAmbient[dwCurrentLight] = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
                break;
            default:
                float fLuminance = (float)(dwCurrentLight / (float)(m_dwNumLights + 1));
                D3DXCOLOR Luminance(fLuminance, fLuminance, fLuminance, 1.0f);
                D3DXCOLOR Color((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, 1.0f);
                D3DXColorModulate(&m_LightColorDiffuse[dwCurrentLight], &Luminance, &Color);
                break;
        }
    }

    // We want a white light in the twosided case
    if (m_LightType == LIGHTTYPE_TWOSIDE)
    {
        m_LightColorDiffuse[0] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Create a shape to light
    if (m_LightType == LIGHTTYPE_TWOSIDE)
    {
        hr = GenerateSphere(D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // Center
                    0.8f,                // Radius
                    10,                    // Rings
                    20,                    // Sections
                    1.0f, 1.0f, 1.0f,    // Scaling
					pd3dDevice
                    );    
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Failed to generate sphere"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }
    }
    else
    {
        m_pShapeMesh = new NVMesh();
        hr = m_pShapeMesh->Create(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\models\\teapot.x")).c_str());
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Failed to create teapot!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }

        m_pShapeMesh->SetFVF(pd3dDevice, D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1);
    }
    
    m_pLightMesh = new NVMesh();
    // Create a shape to display the position of the light
    switch(m_LightType)
    {
        case LIGHTTYPE_MANYPOINT:
        case LIGHTTYPE_POINT:
            hr = m_pLightMesh->Create(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\models\\sphere.x")).c_str());
            if (FAILED(hr))
            {
                MessageBox(NULL, _T("Could not load sphere.x"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                return hr;
            }
            break;
        case LIGHTTYPE_DIRECTIONAL:
        case LIGHTTYPE_TWOSIDE:
            hr = m_pLightMesh->Create(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\models\\arrow.x")).c_str());
            if (FAILED(hr))
            {
                MessageBox(NULL, _T("Could not load arrow.x"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                return hr;
            }
            break;
    }
    m_pLightMesh->SetFVF(pd3dDevice, D3DFVF_NORMAL | D3DFVF_XYZ);

    if (m_pShapeMesh)
        m_pShapeMesh->RestoreDeviceObjects(pd3dDevice);

    if (m_pLightMesh)
        m_pLightMesh->RestoreDeviceObjects(pd3dDevice);

    return hr;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//  The device exists, but is about to be Reset(), so we should release some things. 
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here. Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT VertexLight::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_pEffect);

    SAFE_DELETE(m_pShapeMesh);
    SAFE_DELETE(m_pLightMesh);
    SAFE_RELEASE(m_pSphereBuffer);

   return S_OK;
}

HRESULT VertexLight::SetTransform(D3DXMATRIX& world)
{
    D3DXMATRIX matWorldView;
    D3DXMATRIX matWorldViewIT;
    D3DXMATRIX matWorldViewProj;

    D3DXMatrixMultiply(&matWorldView, &world, &m_view);
    D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &m_proj);
    D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldView);
    D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);
    
    m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProj);
    m_pEffect->SetMatrix("WorldView", &matWorldView);
    m_pEffect->SetMatrix("WorldViewIT", &matWorldViewIT);

    return S_OK;
}

// This setmaterial call is made by the mesh renderer when it needs to change the material for the shape
// it is about to draw.  We use the opportunity to calculate the light properties for the lights shining 
// on the material
HRESULT VertexLight::OnSetMaterial(D3DMATERIAL9* pMat)
{
    DWORD dwCurrentLight;

    D3DXCOLOR LightAmbient;
    D3DXCOLOR LightDiffuse;
    D3DXCOLOR LightSpecular;
    
    D3DXCOLOR matAmbient(0.0f, 0.6f, 0.6f, 1.0f);
    D3DXCOLOR matDiffuse(pMat->Diffuse);
    D3DXCOLOR matSpecular(pMat->Specular);
            
    // Lighting colors - Light 1
    for (dwCurrentLight = 0; dwCurrentLight < m_dwNumLights; dwCurrentLight++)
    {
        D3DXColorModulate(&LightAmbient, &m_LightColorAmbient[dwCurrentLight], &matAmbient);
        D3DXColorModulate(&LightDiffuse, &m_LightColorDiffuse[dwCurrentLight], &matDiffuse);
        D3DXColorModulate(&LightSpecular, &m_LightColorSpecular[dwCurrentLight], &matSpecular);

        // We only do ambient/specular for the directional/point light case.
        // The many point lights sample doesn't do ambient and specular
        char str[64];
        if (m_LightType != LIGHTTYPE_MANYPOINT)
        {
            sprintf(str, "LightAmbient[%d]", dwCurrentLight);
            m_pEffect->SetVector(str, (D3DXVECTOR4*)&LightAmbient);
            sprintf(str, "LightSpecular[%d]", dwCurrentLight);
            m_pEffect->SetVector(str, (D3DXVECTOR4*)&LightSpecular);
        }
        sprintf(str, "LightDiffuse[%d]", dwCurrentLight);
        m_pEffect->SetVector(str, (D3DXVECTOR4*)&LightDiffuse);

        // Are we currently drawing the lights themselves?
        if (m_dwCurrentLightDraw != -1)
        {
            // By default the fixed color is loaded with the light color, but when we are rendering 
            // the directional arrows, we want to use the default material
            if (((m_LightType == LIGHTTYPE_DIRECTIONAL) || (m_LightType == LIGHTTYPE_TWOSIDE)))
                m_pEffect->SetVector("FixedColor", (D3DXVECTOR4*)&pMat->Diffuse);
            else
                m_pEffect->SetVector("FixedColor", (D3DXVECTOR4*)&m_LightColorDiffuse[(m_dwCurrentLightDraw+1)%m_dwNumLights]);
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT VertexLight::Render(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;

    // update time
    float prev_time = m_time;
    m_time = ::timeGetTime()*0.001f;
    if (m_frame == 0)
        m_startTime = m_time;
    m_time -= m_startTime;
    if (!m_bPause)
        m_animTime += m_time - prev_time;

    DWORD i;
    D3DXMATRIX matTemp;

    // Clear to grey
    hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xAA, 0xAA, 0xAA ), 1.0, 0);

    pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    // Increase rotation
    m_fAngle = m_animTime * 100.0f;

    if (m_LightType != LIGHTTYPE_TWOSIDE)
    {
        const NVBounds* pBounds = m_pShapeMesh->GetBounds();

        // Move the loaded object to the middle and scale
        D3DXMatrixScaling(&matTemp, 1.0f / pBounds->m_fRadius, 1.0f / pBounds->m_fRadius, 1.0f / pBounds->m_fRadius);
        D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);

        D3DXMatrixTranslation(&matTemp, -pBounds->m_vecCenter.x, -pBounds->m_vecCenter.y, -pBounds->m_vecCenter.z);
        D3DXMatrixMultiply(&m_matWorld, &matTemp, &m_matWorld);
    }
    
        
    // Load transform data.
    SetTransform(m_matWorld);

    // Update the light positions
    std::vector<D3DXVECTOR3> vLight;
    vLight.resize(m_dwNumLights);

    float fAngleIncr = 360.0f / m_dwNumLights;
    for (i = 0; i < m_dwNumLights; i++)
    {
        float x, y, z;
        float fLightDistance = 0.3f + 1.0f;
        D3DXVECTOR3 vecLightAngles;
        fLightDistance = 1.0f;
        //fLightDistance = 0.5f;
        vecLightAngles.x = D3DXToRadian(-m_fAngle + fAngleIncr * i);
        vecLightAngles.y = D3DXToRadian(m_fAngle + fAngleIncr * i);
        vecLightAngles.z = D3DXToRadian(m_fAngle + 45.0f + fAngleIncr * i);

        x = fLightDistance * cos(vecLightAngles.x);
        y = fLightDistance * sin(vecLightAngles.y);
        z = fLightDistance * sin(vecLightAngles.z);

        // For directional, create a light vector that points towards the origin
        switch(m_LightType)
        {
            case LIGHTTYPE_TWOSIDE:
                z = 1.1f;
                x = -x;
                y = -y;
                break;
            case LIGHTTYPE_POINT:
            case LIGHTTYPE_MANYPOINT:
                break;
            case LIGHTTYPE_DIRECTIONAL:
                x = -x;
                y = -y;
                z = -z;
                break;
        }

        vLight[i] = D3DXVECTOR3(x, y, z);

        D3DXVECTOR4 vLightEye;
        if (m_LightType == LIGHTTYPE_POINT)
        {
            vLight[i] += D3DXVECTOR3(m_matWorld._41, m_matWorld._42, m_matWorld._43);
            D3DXVec3Transform(&vLightEye, &vLight[i], &m_view);
        }
        else if (m_LightType == LIGHTTYPE_MANYPOINT)
        {
            D3DXMATRIX matWorldInverse;

            vLight[i] += D3DXVECTOR3(m_matWorld._41, m_matWorld._42, m_matWorld._43);
            // Transform light back to model space where the lighting will be done
            D3DXMatrixInverse(&matWorldInverse, NULL, &m_matWorld);
            D3DXVec3Transform(&vLightEye, &vLight[i], &matWorldInverse);
        }
        else
        {
            D3DXVECTOR3 vLightToEye;

            // Transform direction vector into eye space
            D3DXVec3Normalize(&vLightToEye, &vLight[i]);
            D3DXVec3TransformNormal(&vLightToEye, &vLightToEye, &m_view);
            D3DXVec3Normalize(&vLightToEye, &vLightToEye);
            
            char str[64];
            sprintf(str, "LightDirectionEyeSpace[%d]", i);
            m_pEffect->SetValue(str, (float*)&vLightToEye, 3*sizeof(float));

            // Shader math requires that the vector is to the light
            vLightEye.x = -vLightToEye.x;
            vLightEye.y = -vLightToEye.y;
            vLightEye.z = -vLightToEye.z;
            vLightEye.w = 1.0f;
        }
        
        char str[64];
        sprintf(str, "LightPosition[%d]", i);
        m_pEffect->SetVector(str, &vLightEye);
    }


    // Draw the object
    switch(m_LightType)
    {
        case LIGHTTYPE_TWOSIDE:
            pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            break;
        default:
            pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            break;
    }

    if (m_LightType != LIGHTTYPE_TWOSIDE)
    {
        m_pEffect->SetTechnique(m_szCurrentTechnique); //added by RG..

        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.
                
                OnSetMaterial(&m_pShapeMesh->m_pMaterials[m_pShapeMesh->m_pAttributeTable[0].AttribId] );

                m_pShapeMesh->Render(pd3dDevice);

                m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }
    else
    {
        pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        // Point at the vertex data
        hr = pd3dDevice->SetStreamSource(0, m_pSphereBuffer, 0, (sizeof(D3DXVECTOR3) * 3));
        if (FAILED(hr))
            return hr;

        pd3dDevice->SetFVF(D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0));

        D3DXCOLOR LightFront;
        D3DXCOLOR LightBack;
        
        D3DXCOLOR matFront(1.0f, 0.0f, 0.0f, 1.0f);
        D3DXCOLOR matBack(1.0f, 1.0f, 1.0f, 1.0f);

        // Lighting colors - Light 1
        D3DXColorModulate(&LightFront, &m_LightColorDiffuse[0], &matFront);
        D3DXColorModulate(&LightBack, &m_LightColorDiffuse[0], &matBack);

        m_pEffect->SetVector("FrontColor", (D3DXVECTOR4*)&LightFront);
        m_pEffect->SetVector("BackColor",  (D3DXVECTOR4*)&LightBack);

        m_pEffect->SetTechnique(m_szCurrentTechnique);

        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.
                pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_dwSphereFaces / 2);
                m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }
    
    // Draw the lights
    pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    hr = m_pEffect->SetTechnique("FixedColor");
    if (FAILED(hr))
        return hr;

    // Don't want textures
    pd3dDevice->SetTexture(0, NULL);

    D3DXMATRIX TeapotWorld = m_matWorld;

    // Walk the list of lights, calculate the correct info needed in the vertex shader to apply them, 
    // and draw them
    for (i = 0; i < m_dwNumLights; i++)
    {
        const NVBounds* pBounds = m_pLightMesh->GetBounds();
        D3DXMatrixIdentity(&m_matWorld);

        // Translate to the middle
        D3DXMatrixTranslation(&matTemp, -pBounds->m_vecCenter.x, -pBounds->m_vecCenter.y, -pBounds->m_vecCenter.z);
        D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);

        float fLightScale = 0.1f; 
        D3DXMatrixScaling(&matTemp, fLightScale / pBounds->m_fRadius, fLightScale / pBounds->m_fRadius, fLightScale / pBounds->m_fRadius);
        D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);

        if ((m_LightType != LIGHTTYPE_POINT) && (m_LightType != LIGHTTYPE_MANYPOINT))
        {
            // Orientation of the light object (the arrow)
            D3DXVECTOR3 vLightObject = D3DXVECTOR3(0.0f, -1.0f, 0.0f);

            // Create the rotation axis
            D3DXVECTOR3 vOrthoNormal;
            D3DXVec3Cross(&vOrthoNormal, &vLight[i], &vLightObject);
            D3DXVec3Normalize(&vOrthoNormal, &vOrthoNormal);

            D3DXVECTOR3 vLightDirection;
            D3DXVec3Normalize(&vLightDirection, &vLight[i]);

            // Calculate the angle between the two vectors.
            float fAngle = acos(D3DXVec3Dot(&vLightDirection, &vLightObject));

            // Rotate the object about our rotation axis to map one vector onto another
            D3DXMatrixRotationAxis(&matTemp, &vOrthoNormal, -fAngle);
            D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);

            // vLight is a direction vector, pointing at the origin, so we negate it to find the position
            D3DXMatrixTranslation(&matTemp, -vLight[i].x, -vLight[i].y, -vLight[i].z);

            matTemp._41 += TeapotWorld._41;
            matTemp._42 += TeapotWorld._42;
            matTemp._43 += TeapotWorld._43;

            D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);
        }
        else
        {
            // Put the light at it's location
            D3DXMatrixTranslation(&matTemp,vLight[i].x, vLight[i].y, vLight[i].z);
            D3DXMatrixMultiply(&m_matWorld, &m_matWorld, &matTemp);
        }

        SetTransform(m_matWorld);

        // Set the fixed color for the light
        m_dwCurrentLightDraw = i;
        
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.

                OnSetMaterial(&m_pLightMesh->m_pMaterials[m_pLightMesh->m_pAttributeTable[0].AttribId] );

                m_pLightMesh->Render(pd3dDevice);  

                m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }
    m_dwCurrentLightDraw = -1;

    m_frame++;

    return S_OK;
}

HRESULT VertexLight::GenerateSphere(D3DXVECTOR3& vCenter, FLOAT fRadius, WORD wNumRings, WORD wNumSections, FLOAT sx, FLOAT sy, FLOAT sz, IDirect3DDevice9* pd3dDevice)
{
    FLOAT x, y, z, v, rsintheta; // Temporary variables
    WORD  i, j, n, m;            // counters
    D3DXVECTOR3 vPoint;
    HRESULT hr;

    SAFE_RELEASE(m_pSphereBuffer);

    //Generate space for the required triangles and vertices.
    WORD       wNumTriangles = (wNumRings + 1) * wNumSections * 2;
    DWORD      dwNumIndices   = wNumTriangles*3;
    DWORD      dwNumVertices  = (wNumRings + 1) * wNumSections + 2;

    Vertex* pvVertices     = new Vertex[dwNumVertices];
    WORD*      pwIndices      = new WORD[3*wNumTriangles];

    // Generate vertices at the top and bottom points.
    D3DXVECTOR3 vTopPoint  = vCenter + D3DXVECTOR3( 0.0f, +sy*fRadius, 0.0f);
    D3DXVECTOR3 vBotPoint  = vCenter + D3DXVECTOR3( 0.0f, -sy*fRadius, 0.0f);
    D3DXVECTOR3 vNormal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    pvVertices[0]               = Vertex( D3DXVECTOR3(vTopPoint.x, vTopPoint.y, vTopPoint.z),  D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z), D3DXVECTOR2(0.0f, 0.0f) );
    pvVertices[dwNumVertices-1] = Vertex( D3DXVECTOR3(vBotPoint.x, vTopPoint.y, vTopPoint.z), D3DXVECTOR3(-vNormal.x, -vNormal.y, -vNormal.z), D3DXVECTOR2(0.0f, 0.0f) );

    // Generate vertex points for rings
    FLOAT dtheta = (float)(D3DX_PI / (wNumRings + 2));     //Angle between each ring
    FLOAT dphi   = (float)(2*D3DX_PI / wNumSections); //Angle between each section
    FLOAT theta  = dtheta;
    n = 1; //vertex being generated, begins at 1 to skip top point

    for( i = 0; i < (wNumRings+1); i++ )
    {
        y = fRadius * (float)cos(theta); // y is the same for each ring
        v = theta / D3DX_PI;     // v is the same for each ring
        rsintheta = fRadius * (float)sin(theta);
        FLOAT phi = 0.0f;

        for( j = 0; j < wNumSections; j++ )
        {
            x = rsintheta * (float)sin(phi);
            z = rsintheta * (float)cos(phi);
        
            FLOAT u = (FLOAT)(1.0 - phi / (2*D3DX_PI) );
            
            vPoint        = vCenter + D3DXVECTOR3( sx*x, sy*y, sz*z );
            vNormal       = D3DXVECTOR3( x/fRadius, y/fRadius, z/fRadius );
            D3DXVec3Normalize(&vNormal, &vNormal);
            pvVertices[n] = Vertex( D3DXVECTOR3(vPoint.x, vPoint.y, vPoint.z), D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z), D3DXVECTOR2(u, v) );

            phi += dphi;
            ++n;
        }
        theta += dtheta;
    }

    // Generate triangles for top and bottom caps.
    for( i = 0; i < wNumSections; i++ )
    {
        pwIndices[3*i+0] = 0;
        pwIndices[3*i+1] = i + 1;
        pwIndices[3*i+2] = 1 + ((i + 1) % wNumSections);

        pwIndices[3*(wNumTriangles - wNumSections + i)+0] = (WORD)( dwNumVertices - 1 );
        pwIndices[3*(wNumTriangles - wNumSections + i)+1] = (WORD)( dwNumVertices - 2 - i );
        pwIndices[3*(wNumTriangles - wNumSections + i)+2] = (WORD)( dwNumVertices - 2 - 
                ((1 + i) % wNumSections) );
    }

    // Generate triangles for the rings
    m = 1;            // first vertex in current ring,begins at 1 to skip top point
    n = wNumSections; // triangle being generated, skip the top cap 
        
    for( i = 0; i < wNumRings; i++ )
    {
        for( j = 0; j < wNumSections; j++ )
        {
            pwIndices[3*n+0] = m + j;
            pwIndices[3*n+1] = m + wNumSections + j;
            pwIndices[3*n+2] = m + wNumSections + ((j + 1) % wNumSections);
            
            pwIndices[3*(n+1)+0] = pwIndices[3*n+0];
            pwIndices[3*(n+1)+1] = pwIndices[3*n+2];
            pwIndices[3*(n+1)+2] = m + ((j + 1) % wNumSections);
            
            n += 2;
        }
        m += wNumSections;
    }

    // Put the sphere in a VB.
    hr = pd3dDevice->CreateVertexBuffer(dwNumIndices * (sizeof(D3DXVECTOR3) * 3), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pSphereBuffer, NULL);
    if (FAILED(hr))
    {
        delete [] pvVertices;
        delete [] pwIndices;

        return hr;
    }

    SphereVertex* pSphereVertexBase;
    SphereVertex* pSphereVertex;
    DWORD dwVertices;

    hr = m_pSphereBuffer->Lock(0, dwNumIndices * (sizeof(D3DXVECTOR3) * 3), (void**)&pSphereVertexBase, 0);
    if (FAILED(hr))
        return hr;

    pSphereVertex = pSphereVertexBase;
    for (dwVertices = 0; dwVertices < dwNumIndices; dwVertices++)
    {
        pSphereVertex->Position = pvVertices[pwIndices[dwVertices]].Position;
        pSphereVertex->Normal = pvVertices[pwIndices[dwVertices]].Normal;
        pSphereVertex++;
    }

    pSphereVertex = pSphereVertexBase;
    for (dwVertices = 0; dwVertices < dwNumIndices; dwVertices += 3)
    {
        D3DXVECTOR3 vecV0,vecV1;
        D3DXVECTOR3 vecFaceNormal;
        vecV0 = pvVertices[pwIndices[dwVertices]].Position - pvVertices[pwIndices[dwVertices + 1]].Position;
        vecV1 = pvVertices[pwIndices[dwVertices]].Position - pvVertices[pwIndices[dwVertices + 2]].Position;

        D3DXVec3Cross(&vecFaceNormal, &vecV0, &vecV1);
        D3DXVec3Normalize(&vecFaceNormal, &vecFaceNormal);
        
        // Copy the face normal into all 3 buffers
        pSphereVertex->FaceNormal = vecFaceNormal;
        pSphereVertex++;

        pSphereVertex->FaceNormal = vecFaceNormal;
        pSphereVertex++;
        
        pSphereVertex->FaceNormal = vecFaceNormal;
        pSphereVertex++;
    }

    m_dwSphereFaces = (dwNumIndices / 3);

    m_pSphereBuffer->Unlock();

    delete [] pvVertices;
    delete [] pwIndices;
    return S_OK;
}

//-----------------------------------------------------------------------------
// Handlers for context menu actions:
//-----------------------------------------------------------------------------

void VertexLight::TogglePause()
{
    m_bPause = !m_bPause;
}

void VertexLight::ToggleWireframe()
{
    m_bWireframe = !m_bWireframe;
}
