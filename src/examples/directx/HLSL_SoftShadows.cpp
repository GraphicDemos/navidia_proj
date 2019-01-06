#include "HLSL_SoftShadows.h"
#include <stdlib.h>
#include <shared/NVFileDialog.h>
#include <shared/GetFilePath.h>
#pragma warning(disable : 4786)
#include <fstream>
#include <vector>
#include <string>
#pragma warning(disable : 4786)
#include <assert.h>

static const int TEXDEPTH_HEIGHT = 1024;
static const int TEXDEPTH_WIDTH = 1024;
static const int JITTER_SIZE = 32;
static const int JITTER_SAMPLES = 8;

SoftShadows::SoftShadows()
{
    m_time = ::timeGetTime()*0.001f;
    m_startTime = m_time;
    m_frame = 0;
    m_fps = 30;
    m_main_menu = 0;
    m_context_menu = 0;
    m_pEffect = NULL;

    m_NumSamples = 64;
    m_softness = 8.0f; 
    m_jitter = 1.0f;

    m_pAttributes = NULL;
    m_bWireframe = false;
    m_fRadius = 0;
    m_vecCenter = D3DXVECTOR3(0.f, 0.f, 0.f);
    m_pBackBuffer = NULL;
    m_pZBuffer = NULL;
    m_pSMDecalTexture = NULL;
    m_pSMZTexture = NULL;
    m_pSMColorSurface = NULL;
    m_pSMZSurface = NULL;
    m_lightDir = D3DXVECTOR4(0.f, 0.f, 0.f, 0.f);
    m_bitDepth = 24;
    m_bPaused = true;
    m_pDeclaration = NULL;
    m_fDepthBias = 0.0004f;
    m_fBiasSlope = 5.0f;
	m_pScene = m_pLightSource = NULL;
	m_shader = NULL;

    D3DXMatrixIdentity(&m_World);
    D3DXMatrixIdentity(&m_View);
    D3DXMatrixIdentity(&m_Projection);
}

HRESULT SoftShadows::ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* m_pBackBufferSurfaceDesc )
{
    HRESULT hr;

    assert(pd3dDevice);

    D3DFORMAT zFormat = D3DFMT_D24S8;
    m_bitDepth = 24;

    if(FAILED(CheckResourceFormatSupport(pd3dDevice, zFormat, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL)))
    {
        MessageBox(NULL, _T("Device/driver does not support hardware shadow maps!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    //setup buffers
    if(FAILED(pd3dDevice->GetRenderTarget(0, &m_pBackBuffer)))
        return E_FAIL;
    if(FAILED(pd3dDevice->GetDepthStencilSurface(&m_pZBuffer)))
        return E_FAIL;

    if(!m_pBackBuffer || !m_pZBuffer)
        return E_FAIL;

    if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, 
                                          GetFilePath::GetMediaFilePath(_T("nvlogo_spot.png")).c_str(), 
                                          TEXDEPTH_WIDTH,
                                          TEXDEPTH_HEIGHT,
                                          1,
                                          D3DUSAGE_RENDERTARGET,
                                          D3DFMT_A8R8G8B8,
                                          D3DPOOL_DEFAULT,
                                          D3DX_DEFAULT,
                                          D3DX_DEFAULT,
                                          0,
                                          NULL,
                                          NULL,
                                          &m_pSMDecalTexture)))
        return E_FAIL;

    if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, 
                                          GetFilePath::GetMediaFilePath(_T("decal_image.png")).c_str(),
                                          TEXDEPTH_WIDTH,
                                          TEXDEPTH_HEIGHT,
                                          D3DX_DEFAULT,
                                          D3DUSAGE_RENDERTARGET,
                                          D3DFMT_A8R8G8B8,
                                          D3DPOOL_DEFAULT,
                                          D3DX_DEFAULT,
                                          D3DX_DEFAULT,
                                          0,
                                          NULL,
                                          NULL,
                                          &m_pFloorTexture)))
        return E_FAIL;

    if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, 
                                          GetFilePath::GetMediaFilePath(_T("Walker_Head-1024DF-7.png")).c_str(),
                                          TEXDEPTH_WIDTH,
                                          TEXDEPTH_HEIGHT,
                                          D3DX_DEFAULT,
                                          D3DUSAGE_RENDERTARGET,
                                          D3DFMT_A8R8G8B8,
                                          D3DPOOL_DEFAULT,
                                          D3DX_DEFAULT,
                                          D3DX_DEFAULT,
                                          0,
                                          NULL,
                                          NULL,
                                          &m_pHeadTexture)))
        return E_FAIL;

    D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
    if( (zFormat == D3DFMT_D16) ||
        (zFormat == D3DFMT_D15S1) )
        colorFormat = D3DFMT_R5G6B5;

    if(FAILED(pd3dDevice->CreateTexture(TEXDEPTH_WIDTH, TEXDEPTH_HEIGHT, 1, D3DUSAGE_RENDERTARGET, colorFormat, 
        D3DPOOL_DEFAULT, &m_pSMColorTexture, NULL)))
        return E_FAIL;
    if(FAILED(pd3dDevice->CreateTexture(TEXDEPTH_WIDTH, TEXDEPTH_HEIGHT, 1, D3DUSAGE_DEPTHSTENCIL, zFormat, 
        D3DPOOL_DEFAULT, &m_pSMZTexture, NULL)))
        return E_FAIL;
    if(FAILED(pd3dDevice->CreateVolumeTexture(JITTER_SIZE, JITTER_SIZE, JITTER_SAMPLES*JITTER_SAMPLES/2, 1, 
        D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pJitterTexture, NULL)))
        return E_FAIL;
    if(!m_pSMColorTexture || !m_pSMZTexture || !m_pSMDecalTexture || !m_pFloorTexture || !m_pHeadTexture || !m_pJitterTexture)
        return E_FAIL;

    // Retrieve top-level surfaces of our shadow buffer (need these for use with SetRenderTarget)
    if(FAILED(m_pSMColorTexture->GetSurfaceLevel(0, &m_pSMColorSurface)))
        return E_FAIL;
    if(FAILED(m_pSMZTexture->GetSurfaceLevel(0, &m_pSMZSurface)))
        return E_FAIL;
    if(!m_pSMColorSurface || !m_pSMZSurface)
        return E_FAIL;

    // Build the jitter texture
    D3DLOCKED_BOX lb;
    m_pJitterTexture->LockBox(0, &lb, NULL, 0);

    unsigned char *data = (unsigned char *)lb.pBits;

    for (int i = 0; i<JITTER_SIZE; i++) {
        for (int j = 0; j<JITTER_SIZE; j++) {
            float rot_offset = ((float)rand() / RAND_MAX - 1) * 2 * 3.1415926f;

            for (int k = 0; k<JITTER_SAMPLES*JITTER_SAMPLES/2; k++) {

                int x, y;
                float v[4];

                x = k % (JITTER_SAMPLES / 2);
                y = (JITTER_SAMPLES - 1) - k / (JITTER_SAMPLES / 2);

                v[0] = (float)(x * 2 + 0.5f) / JITTER_SAMPLES;
                v[1] = (float)(y + 0.5f) / JITTER_SAMPLES;
                v[2] = (float)(x * 2 + 1 + 0.5f) / JITTER_SAMPLES;
                v[3] = v[1];
                
                // jitter
                v[0] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                v[1] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                v[2] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                v[3] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;

                // warp to disk
                float d[4];
                d[0] = sqrtf(v[1]) * cosf(2 * 3.1415926f * v[0] + rot_offset);
                d[1] = sqrtf(v[1]) * sinf(2 * 3.1415926f * v[0] + rot_offset);
                d[2] = sqrtf(v[3]) * cosf(2 * 3.1415926f * v[2] + rot_offset);
                d[3] = sqrtf(v[3]) * sinf(2 * 3.1415926f * v[2] + rot_offset);

                d[0] = (d[0] + 1.0) / 2.0;
                data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 0] = (unsigned char)(d[0] * 255);
                d[1] = (d[1] + 1.0) / 2.0;
                data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 1] = (unsigned char)(d[1] * 255);
                d[2] = (d[2] + 1.0) / 2.0;
                data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 2] = (unsigned char)(d[2] * 255);
                d[3] = (d[3] + 1.0) / 2.0;
                data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 3] = (unsigned char)(d[3] * 255);
            }
        }
    }
    m_pJitterTexture->UnlockBox(0);

    // Assign registers to the relevant vertex attributes
    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  
        { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };
    pd3dDevice->CreateVertexDeclaration(declaration, &m_pDeclaration);

    const char* profileOpts[] = 
    {
        "-profileopts", "dcls", NULL,
    };

    DWORD tempFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0;

    hr = CreateQuad(pd3dDevice,&m_smQuad);
    tstring fileName(_T("WalkerHead.nvb"));

    m_pScene = new NVBScene;
    hr = m_pScene->Load(fileName, pd3dDevice, GetFilePath::GetMediaFilePath);
    if(FAILED(hr))
        return hr;

    m_lightPos.x = 6.0f; 
    m_lightPos.y = 6.0f; 
    m_lightPos.z = 1.0f;

    m_lightDir.x = m_lightPos.x;
    m_lightDir.y = m_lightPos.y;
    m_lightDir.z = m_lightPos.z;
    m_lightDir.w = 0.0f;
    D3DXVec4Normalize(&m_lightDir, &m_lightDir);
   
    // Load our Effect file
    hr = D3DXCreateEffectFromFile(pd3dDevice, 
                        GetFilePath::GetFilePath(_T("MEDIA\\programs\\HLSL_SoftShadows.cso")).c_str(), 
                        NULL, NULL, 0, NULL, &m_pEffect, NULL);

    return S_OK;
}

HRESULT SoftShadows::SetVertexShaderMatrices(const D3DXMATRIX& worldMat, const D3DXMATRIX& viewMat, const D3DXMATRIX& projMat, const D3DXMATRIX& texMat)
{
    D3DXMATRIX worldViewProjMat = worldMat * viewMat * projMat;

    D3DXMATRIX worldITMat;    
    D3DXMatrixInverse(&worldITMat, NULL, &worldMat);
    D3DXMatrixTranspose(&worldITMat, &worldITMat);
        
    m_pEffect->SetMatrix("World", &worldMat);
    m_pEffect->SetMatrix("WorldViewProj", &worldViewProjMat);
    m_pEffect->SetMatrix("WorldIT", &worldITMat);
    m_pEffect->SetMatrix("TexTransform", &texMat);
    
    return S_OK;
}
HRESULT SoftShadows::CreateQuad(IDirect3DDevice9* pd3dDevice, SMMeshInfo* mesh)
{    
    HRESULT hr = S_OK;

    hr = pd3dDevice->CreateVertexBuffer(4 * sizeof(SMVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &(mesh->pVB), NULL);
    if(FAILED(hr))
        return hr;

    SMVertex* pVData;
    hr = mesh->pVB->Lock(0, 0, (void**)&pVData, 0);
    if(FAILED(hr))
        return hr;
    float value = 10.0f;
    pVData[0].x  = -value; pVData[0].y  = -0.65f; pVData[0].z  = value;
    pVData[0].nx = 0.0f;  pVData[0].ny = 1.0f; pVData[0].nz = 0.0f;
    pVData[0].diffuse = 0xffffffff;
    pVData[0].tx  = -value/2; pVData[0].ty  = value/2;

    pVData[1].x  = value;  pVData[1].y  = -0.65f; pVData[1].z  = value;
    pVData[1].nx = 0.0f;  pVData[1].ny = 1.0f; pVData[1].nz = 0.0f;
    pVData[1].diffuse = 0xffffffff;
    pVData[1].tx  = value/2; pVData[1].ty  = value/2;

    pVData[2].x  = -value; pVData[2].y  = -0.65f; pVData[2].z  = -value;
    pVData[2].nx = 0.0f;  pVData[2].ny = 1.0f; pVData[2].nz = 0.0f;
    pVData[2].diffuse = 0xffffffff;
    pVData[2].tx  = -value/2; pVData[2].ty  = -value/2;

    pVData[3].x  = value;  pVData[3].y  = -0.65f; pVData[3].z  = -value;
    pVData[3].nx = 0.0f;  pVData[3].ny = 1.0f; pVData[3].nz = 0.0f;
    pVData[3].diffuse = 0xffffffff;
    pVData[3].tx  = value/2; pVData[3].ty  = -value/2;
    hr = mesh->pVB->Unlock();
    if(FAILED(hr))
        return hr;

    hr = pd3dDevice->CreateIndexBuffer(4 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &(mesh->pIB), NULL);
    if(FAILED(hr))
        return hr;

    WORD* pIData;
    hr = mesh->pIB->Lock(0, 0, (void**)&pIData, 0);
    if(FAILED(hr))
        return hr;
    //it's a strip
    pIData[0] = 0;
    pIData[1] = 2;
    pIData[2] = 1;
    pIData[3] = 3;
    hr = mesh->pIB->Unlock();

    mesh->dwNumFaces = 2;
    mesh->dwNumVerts = 4;
    mesh->primType = D3DPT_TRIANGLESTRIP;

    //quad doesn't get scaled / translated
    mesh->scaleVec = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    mesh->transVec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    return S_OK;
}
HRESULT SoftShadows::CheckResourceFormatSupport(IDirect3DDevice9* pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
{
    HRESULT hr = S_OK;
    IDirect3D9* tempD3D = NULL;
    pd3dDevice->GetDirect3D(&tempD3D);
    D3DCAPS9 devCaps;
    pd3dDevice->GetDeviceCaps(&devCaps);

	D3DDISPLAYMODE displayMode;
    tempD3D->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);
    
    hr = tempD3D->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);
    
    tempD3D->Release(), tempD3D = NULL;
    
    return hr;
}

HRESULT SoftShadows::RenderShadowMap(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX &worldMat)
{
    //setup matrices for shadowmap
    D3DXVECTOR3 eye, lookAt, up;
    
    eye = *m_Light.GetEyePt();
    lookAt = *m_Light.GetLookAtPt();

    // Constrain the Light's eye point.
    if (eye.y < 2.0f) eye.y = 2.0f;
    m_Light.SetViewParams(&eye, &lookAt);

    lookAt.x = 0;             lookAt.y = 0;             lookAt.z = 0;
	up.x     = 0.0f;          up.y     = 1.0f;          up.z     = 0.0f;
    
    D3DXMATRIX lightView, lightProj;
    D3DXMatrixLookAtLH(&lightView, &eye, &lookAt, &up);

    m_pEffect->SetValue("LightPos", (float*)&eye, sizeof(float)*3);
    
    D3DXMatrixPerspectiveFovLH(&lightProj, D3DXToRadian(60.0f), 1.0f, 1.0f, 25.0f);

    m_LightViewProj = lightView * lightProj;
    
    //set render target to shadow map surfaces
    if(FAILED(pd3dDevice->SetRenderTarget(0, m_pSMColorSurface)))
        return E_FAIL;

    //set depth stencil
    if(FAILED(pd3dDevice->SetDepthStencilSurface(m_pSMZSurface)))
        return E_FAIL;


    //save old viewport
    D3DVIEWPORT9 oldViewport;
    pd3dDevice->GetViewport(&oldViewport);

    //set new, funky viewport
    D3DVIEWPORT9 newViewport;
    newViewport.X = 0;
    newViewport.Y = 0;
    newViewport.Width  = TEXDEPTH_WIDTH;
    newViewport.Height = TEXDEPTH_HEIGHT;
    newViewport.MinZ = 0.0f;
    newViewport.MaxZ = 1.0f;
    pd3dDevice->SetViewport(&newViewport);

    //use technique that will draw plain black pixels
    if (FAILED(m_pEffect->SetTechnique("GenHardwareShadowMap")))
    {
        MessageBox(NULL, _T("Failed to set 'GenHardwareShadowMap' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    //depth bias
    pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&m_fDepthBias);
    pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);

    pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00FFFFFF, 1.0f, 0L);

    D3DXMATRIX tempIdentity;
    D3DXMatrixIdentity(&tempIdentity);

    SetVertexShaderMatrices(m_World, lightView, lightProj, tempIdentity);
    pd3dDevice->SetVertexDeclaration(m_pDeclaration);

    // render mesh using GenHardwareShadowMap technique
    for (unsigned int i = 0; i < m_pScene->m_NumMeshes; ++i) 
    {
        const NVBScene::Mesh& mesh = m_pScene->m_Meshes[i];

        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, 0.0125, 0.0125, 0.0125);
        D3DXMATRIX worldMat = mesh.m_Transform * m_World * scale;
        SetVertexShaderMatrices(worldMat, lightView, lightProj, tempIdentity);
        pd3dDevice->SetVertexDeclaration(m_pDeclaration);

        // render mesh using GenHardwareShadowMap technique
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                // Set the state for a particular pass in a technique.
                m_pEffect->BeginPass(uPass);

                // Draw it 
                if (FAILED(mesh.Draw()))
                    return E_FAIL;
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }

    D3DXMATRIX tempScaleMat;
    D3DXMatrixScaling(&tempScaleMat, m_smQuad.scaleVec.x, m_smQuad.scaleVec.y, m_smQuad.scaleVec.z);
    
    SetVertexShaderMatrices(tempScaleMat, lightView, lightProj, tempIdentity);

    //set vb
    HRESULT hr = pd3dDevice->SetStreamSource(0, m_smQuad.pVB, 0, sizeof(SMVertex));
    if (FAILED(hr))
        return hr;
    
    //set index buffer
    hr = pd3dDevice->SetIndices(m_smQuad.pIB);
    if(FAILED(hr))
        return hr;

    //render quad
    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.
            //pd3dDevice->DrawIndexedPrimitive(m_smQuad.primType, 0, 0, m_smQuad.dwNumVerts, 0, m_smQuad.dwNumFaces);
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }
    
    pd3dDevice->SetViewport(&oldViewport);

    //depth bias
    float fTemp = 0.0f;
    pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fTemp);
    pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fTemp);

    return S_OK;
}
void SoftShadows::LostDevice()
{
    SAFE_DELETE(m_pScene);
    SAFE_DELETE(m_pLightSource);
    SAFE_RELEASE(m_smBigship.pVB);
    SAFE_RELEASE(m_smBigship.pIB);
    SAFE_RELEASE(m_smQuad.pVB);
    SAFE_RELEASE(m_smQuad.pIB);

    SAFE_RELEASE(m_pSMColorTexture);
    SAFE_RELEASE(m_pSMDecalTexture);
    SAFE_RELEASE(m_pFloorTexture);
    SAFE_RELEASE(m_pHeadTexture);
    SAFE_RELEASE(m_pSMZTexture);
    SAFE_RELEASE(m_pSMColorSurface);
    SAFE_RELEASE(m_pSMZSurface);
	SAFE_RELEASE(m_pJitterTexture);

    SAFE_RELEASE(m_pBackBuffer);
    SAFE_RELEASE(m_pZBuffer);

    SAFE_RELEASE(m_pDeclaration);

    SAFE_DELETE_ARRAY(m_pAttributes);

    SAFE_RELEASE(m_pEffect);
}

void SoftShadows::SetShader(int shnum)
{
    static int currentShader = shnum;
    static char effectName[80] = "UseHardwareShadowMap";

    if(shnum < 0) shnum = currentShader;
    switch(shnum) {
        case 0:
            m_shader = "UseHardwareShadowMap";
            break;
        case 1:
            sprintf(effectName, "UseHardwareSoftShadowMap2_%d", m_NumSamples);
            m_shader = effectName;
            break;
        case 2:
            sprintf(effectName, "UseHardwareSoftShadowMap3_%d", m_NumSamples);
            m_shader = effectName;
            break;
        case 3:
            m_shader = "ShowPenumbra";
            break;
    }
}

HRESULT SoftShadows::Render( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, 
								  const D3DXMATRIX* cworldMat, const D3DXMATRIX* cviewMat, const D3DXMATRIX* cprojMat)
{
    HRESULT hr;
    D3DXHANDLE hTechnique = NULL;
    static bool firstTime = true;
    static D3DXMATRIX rotate;
    D3DXMATRIX rotateInc;

    // update time
    m_time = ::timeGetTime()*0.001f;
    if (m_frame == 0)
        m_startTime = m_time;
    else if (m_time > m_startTime)
        m_fps = (float)m_frame / (m_time - m_startTime);

    if(firstTime) {
        D3DXMatrixIdentity(&rotate);
        firstTime = false;
    }
    if(!m_bPaused) {
        D3DXMatrixRotationY(&rotateInc, -0.0005 * 620.0 / m_fps);
        rotate = rotate * rotateInc;
    }

    // Update view matrix
    m_View = *cviewMat;
    // Update scene position
    m_World = rotate;
    m_pScene->Update(0, &m_World);

    pd3dDevice->SetRenderState(D3DRS_FILLMODE, (m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

    //render into shadow map
    if(FAILED(RenderShadowMap(pd3dDevice, *cworldMat)))
        return E_FAIL;

    if (FAILED(m_pEffect->SetTechnique(m_shader)))
    {
        MessageBox(NULL, _T("Failed to set 'UseHardwareShadowMap' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    //set render target back to normal back buffer / depth buffer
    if(FAILED(pd3dDevice->SetRenderTarget(0, m_pBackBuffer)))
        return E_FAIL;

    if(FAILED(pd3dDevice->SetDepthStencilSurface(m_pZBuffer)))
        return E_FAIL;

    pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.6, 0.6, 0.6, 1.0), 1.0f, 0L);

    //set soft shadow params
    float softness[] = {0, 0, 0, m_softness / TEXDEPTH_WIDTH},
            jitter[] = {m_jitter / JITTER_SIZE, m_jitter / JITTER_SIZE, 0, 0},
			resolution[] = {m_width, m_height, 1, 1},
            Shade = 0.0;
    m_pEffect->SetValue("FilterSize", softness, sizeof(float)*4);
    m_pEffect->SetValue("JitterScale", jitter, sizeof(float)*4);
	m_pEffect->SetValue("Resolution", resolution, sizeof(float)*4);
    m_pEffect->SetValue("Shade", &Shade, sizeof(float));

    //set jitter map as texture
    if(FAILED(m_pEffect->SetTexture("Jitter", m_pJitterTexture)))
        return E_FAIL;

    //set depth map as texture
    if(FAILED(m_pEffect->SetTexture("ShadowMap", m_pSMZTexture)))
        return E_FAIL;
    
    //set spotlight texture
    if(FAILED(m_pEffect->SetTexture("SpotLight", m_pSMDecalTexture)))
        return E_FAIL;

    //set head texture
    if(FAILED(m_pEffect->SetTexture("BaseTexture", m_pHeadTexture)))
        return E_FAIL;

    //set special texture matrix for shadow mapping
    float fOffsetX = 0.5f + (0.5f / (float)TEXDEPTH_WIDTH);
    float fOffsetY = 0.5f + (0.5f / (float)TEXDEPTH_HEIGHT);
    unsigned int range = 1;            //note different scale in DX9!
    float fBias    = 0.0f;
    D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
                                0.0f,    -0.5f,     0.0f,         0.0f,
                                0.0f,     0.0f,     (float)range, 0.0f,
                                fOffsetX, fOffsetY, fBias,        1.0f );
    
    SetVertexShaderMatrices(m_World, m_View, m_Projection, m_World * m_LightViewProj * texScaleBiasMat);
    pd3dDevice->SetVertexDeclaration(m_pDeclaration);

    // render mesh using HardwareShadowMapTechnique
    for (unsigned int i = 0; i < m_pScene->m_NumMeshes; ++i) 
    {
        const NVBScene::Mesh& mesh = m_pScene->m_Meshes[i];

        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, 0.0125, 0.0125, 0.0125);
        D3DXMATRIX worldMat = mesh.m_Transform * m_World * scale;
        SetVertexShaderMatrices(worldMat, m_View, m_Projection, worldMat * m_LightViewProj * texScaleBiasMat);

        // render mesh using GenHardwareShadowMap technique
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                // Set the state for a particular pass in a technique.
                m_pEffect->BeginPass(uPass);

                // Draw it 
                if (FAILED(mesh.Draw()))
                    return E_FAIL;
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }

    //set floor texture
    if(FAILED(m_pEffect->SetTexture("BaseTexture", m_pFloorTexture)))
        return E_FAIL;

    D3DXMATRIX tempScaleMat;
    D3DXMatrixScaling(&tempScaleMat, m_smQuad.scaleVec.x, m_smQuad.scaleVec.y, m_smQuad.scaleVec.z);
    SetVertexShaderMatrices(tempScaleMat, m_View, m_Projection, tempScaleMat * m_LightViewProj * texScaleBiasMat);

    //set vb
    hr = pd3dDevice->SetStreamSource(0, m_smQuad.pVB, 0, sizeof(SMVertex));
    if (FAILED(hr))
        return hr;

    //set index buffer
    hr = pd3dDevice->SetIndices(m_smQuad.pIB);
    if(FAILED(hr))
        return hr;

    // Set Shade to 1.0 to effectively kill shading
    Shade = 1.0;
    m_pEffect->SetValue("Shade", &Shade, sizeof(float));

    //render quad using HardwareShadowMapTechnique
    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            // Set the state for a particular pass in a technique.
            m_pEffect->BeginPass(uPass);

            // Draw it 
            if(FAILED(pd3dDevice->DrawIndexedPrimitive(m_smQuad.primType, 0, 0, m_smQuad.dwNumVerts, 0, m_smQuad.dwNumFaces)))
                return E_FAIL;
			m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    // Draw the lightsource widget
    DrawDirectionLight(pd3dDevice);

    m_frame++;

    return S_OK;
}

HRESULT SoftShadows::DrawDirectionLight( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;

	if (m_pLightSource == NULL) {
		// Load light model if not done yet
		m_pLightSource = new NVBScene;
		if (FAILED(hr = m_pLightSource->Load(_T("directionLight.nvb"), pd3dDevice, GetFilePath::GetMediaFilePath))) {
			MessageBox(NULL, _T("Failed to load the direction light model"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return hr;
		}
	}

	// Arrow direction in object space
	D3DXVECTOR3 dirArrow = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	// Rotate to make the arrow parallel to the light direction
	D3DXVECTOR3 rotationAxis, 
                LightSourceDir = (*m_Light.GetLookAtPt()) - (*m_Light.GetEyePt());
    D3DXVec3Normalize(&LightSourceDir, &LightSourceDir);
	D3DXVec3Cross(&rotationAxis, &dirArrow, &LightSourceDir);
	D3DXVec3Normalize(&rotationAxis, &rotationAxis);
	D3DXMATRIX world;
	D3DXMatrixRotationAxis(&world, &rotationAxis, acosf(D3DXVec3Dot(&LightSourceDir, &dirArrow)));

	// Scale
	float scale = 0.05f;
	D3DXMATRIX scaling;
	D3DXMatrixScaling(&scaling, scale, scale, scale);
	world *= scaling;

	// Translate to offset from the object
	D3DXMATRIX translation;// = m_World;
	D3DXMatrixIdentity(&translation);
	D3DXMatrixTranslation(&translation, (*m_Light.GetEyePt()).x + translation._41, (*m_Light.GetEyePt()).y + translation._42, (*m_Light.GetEyePt()).z + translation._43);
	world *= translation;

	// Set shader parameters
    SetVertexShaderMatrices(world, m_View, m_Projection, world);
	pd3dDevice->SetVertexDeclaration(m_pDeclaration);

	// Activate shaders
	if (FAILED(m_pEffect->SetTechnique("DrawLightSource")))
	{
		MessageBox(NULL, _T("Unable to set DrawLightSource technique"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		return E_FAIL;
	}

	UINT uPasses;
	if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
		for (UINT uPass = 0; uPass < uPasses; uPass++) {
			// Set the state for a particular pass in a technique.
			m_pEffect->BeginPass(uPass);

			// Draw the light
			if (FAILED(hr = m_pLightSource->m_Meshes[0].Draw()))
				return hr;
			m_pEffect->EndPass();
		}
		m_pEffect->End();
	}

	return S_OK;
}