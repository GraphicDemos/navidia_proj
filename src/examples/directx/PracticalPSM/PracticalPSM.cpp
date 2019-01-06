//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\PracticalPSM
// File:  PracticalPSM.cpp
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
// Link to: DX9SDKSampleFramework.lib d3dxof.lib dxguid.lib d3dx9dt.lib d3d9.lib winmm.lib comctl32.lib
//
//   Light-Space Perspective Shadow Map implementation generously provided by Oles Shishkovstov
//   Additional Reading:
//     Stamminger & Drettakis: Perspective Shadow Maps (SIGGRAPH 2002)
//     Kozlov: Perspective Shadow Maps: Care and Feeding (GPU Gems 2004)
//     Wimmer, Scherzer & Purgathofer: Light-Space Perspective Shadow Maps (Eurographics Rendering 2004)
//
//-----------------------------------------------------------------------------

#define STRICT

#include <Windows.h>
#include <commctrl.h>
#include <math.h>
#include <D3DX9.h>
#include <d3dx9math.h>
#include <d3dx9effect.h>
#include <d3dx9shader.h>
#include <DX9SDKSampleFramework/DX9SDKSampleFramework.h>
#include <shared/GetFilePath.h>
#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include <shared/NVFileDialog.h>

#include "PracticalPSM.h"
#include "Util.h"
#include "Bounding.h"
#include "common.h"
#include "FrameWork.h"
#include "Helpers.h"


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT PracticalPSM::Render(IDirect3DDevice9* m_pd3dDevice)
{
    HRESULT hr;
    D3DXHANDLE hTechnique = NULL;

    //  update animation
    Tick(m_pd3dDevice);

    //render into shadow map
    if(FAILED(RenderShadowMap(m_pd3dDevice)))
        return E_FAIL;

    if (FAILED(SetRenderStates(false,m_pd3dDevice)))
        return E_FAIL;

    if (m_bSupportsHWShadowMaps)
	{
		if (FAILED(m_pEffect->SetTechnique("UseHardwareShadowMap")))
		{
			MessageBox(NULL, _T("Failed to set 'UseHardwareShadowMap' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return E_FAIL;
		}
	}
	else
	{
		if (FAILED(m_pEffect->SetTechnique("UseR32FShadowMap")))
		{
			MessageBox(NULL, _T("Failed to set 'UseR32FShadowMap' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			return E_FAIL;
		}
	}

    //set depth map as texture
    if (FAILED(m_pEffect->SetTexture("ShadowColor", m_pSMColorTexture)))
        return E_FAIL;

	if(FAILED(m_pEffect->SetTexture("ShadowMap", (m_bSupportsHWShadowMaps)?m_pSMZTexture:m_pSMColorTexture)))
        return E_FAIL;
    
    //set special texture matrix for shadow mapping
    float fOffsetX = 0.5f + (0.5f / (float)((m_bSupportsPixelShaders20)?TEXDEPTH_WIDTH_20:TEXDEPTH_SIZE_11));
    float fOffsetY = 0.5f + (0.5f / (float)((m_bSupportsPixelShaders20)?TEXDEPTH_HEIGHT_20:TEXDEPTH_SIZE_11));
    unsigned int range = 1;
    float fBias    = 0.0f;
    D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
                                0.0f,    -0.5f,     0.0f,         0.0f,
                                0.0f,     0.0f,     (float)range, 0.0f,
                                fOffsetX, fOffsetY, 0.0f,         1.0f );


    //  draw the clawbot and the rockchunk
    {
        //  the shadow map is projective (m_bDisplayShadowMap unsets this flag)
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);        

        D3DXMATRIX textureMatrix;
        D3DXMatrixMultiply(&textureMatrix, &m_LightViewProj, &texScaleBiasMat);
        
        for (unsigned int i=0; i<m_ShadowReceiverObjects.size(); i++)
        {
            const ObjectInstance*& instance = m_ShadowReceiverObjects[i];

            D3DXMATRIX worldMatrix;
            D3DXMatrixTranslation(&worldMatrix, instance->translation.x, instance->translation.y, instance->translation.z);
            D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &m_World);
            if (FAILED(hr=DrawScene(instance->scene, &worldMatrix, &m_View, &m_Projection, &textureMatrix)))
                return hr;
        }
        //  set up the clawbot translation*world matrix, and the texture projection matrix   
        D3DXMATRIX tempScaleMatrix;
        D3DXMATRIX viewProjMatrix;
        D3DXMATRIX scaledTextureMatrix;
        D3DXMatrixScaling(&tempScaleMatrix, m_smQuad.scaleVec.x, m_smQuad.scaleVec.y, m_smQuad.scaleVec.z);
        D3DXMatrixMultiply(&viewProjMatrix, &m_View, &m_Projection);
        D3DXMatrixMultiply(&scaledTextureMatrix, &tempScaleMatrix, &textureMatrix);

        SetVertexShaderMatrices(tempScaleMatrix, viewProjMatrix, scaledTextureMatrix);


        if (FAILED(DrawQuad(m_pd3dDevice)))
            return E_FAIL;
    }

    if (m_bDisplayShadowMap)
    {
        // if we display the shadow map, do it here
        m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

        m_pEffect->SetTechnique("DrawHardwareShadowMap");
        if (FAILED(DrawQuad(m_pd3dDevice)))
            return E_FAIL;
    }
    m_frame++;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: BuildPSMProjectionMatrix
// Desc: Builds a perpsective shadow map transformation matrix
//-----------------------------------------------------------------------------
void PracticalPSM::BuildPSMProjectionMatrix()
{
    D3DXMATRIX lightView, lightProj, virtualCameraViewProj, virtualCameraView, virtualCameraProj;

    const D3DXVECTOR3 yAxis  ( 0.f, 1.f, 0.f);
    const D3DXVECTOR3 zAxis  ( 0.f, 0.f, 1.f);

    //  update the virutal scene camera's bounding parameters
    ComputeVirtualCameraParameters( );

    //  compute a slideback, that force some distance between the infinity plane and the view-box
    const float Z_EPSILON=0.0001f;
    float infinity = m_zFar/(m_zFar-m_zNear);
    float fInfinityZ = m_fMinInfinityZ;
    if ( (infinity<=fInfinityZ) && m_bSlideBack)
    {
        float additionalSlide = fInfinityZ*(m_zFar-m_zNear) - m_zFar + Z_EPSILON;
        m_fSlideBack = additionalSlide;
        m_zFar += additionalSlide;
        m_zNear += additionalSlide;
    }

    if (m_bSlideBack)
    {
        //  clamp the view-cube to the slid back objects...
        const D3DXVECTOR3 eyePt(0.f, 0.f, 0.f);
        const D3DXVECTOR3 eyeDir(0.f, 0.f, 1.f);
        D3DXMatrixTranslation(&virtualCameraView, 0.f, 0.f, m_fSlideBack);

        if ( m_bUnitCubeClip )
        {
            BoundingCone bc( &m_ShadowReceiverPoints, &virtualCameraView, &eyePt, &eyeDir );
            D3DXMatrixPerspectiveLH( &virtualCameraProj, 2.f*tanf(bc.fovx)*m_zNear, 2.f*tanf(bc.fovy)*m_zNear, m_zNear, m_zFar );
        }
        else
        {
            const float viewHeight = ZFAR_MAX * 0.57735026919f;  // tan(0.5f*VIEW_ANGLE)*ZFAR_MAX       
            float viewWidth  = viewHeight * m_fAspect;
            float halfFovy = atanf( viewHeight / (ZFAR_MAX+m_fSlideBack) );
            float halfFovx = atanf( viewWidth  / (ZFAR_MAX+m_fSlideBack) );

            D3DXMatrixPerspectiveLH( &virtualCameraProj, 2.f*tanf(halfFovx)*m_zNear, 2.f*tanf(halfFovy)*m_zNear, m_zNear, m_zFar );
        }
//        D3DXMatrixPerspectiveFovLH( &virtualCameraProj, 2.f*halfFovy, halfFovx/halfFovy, m_zNear, m_zFar);
    }
    else
    {
        D3DXMatrixIdentity( &virtualCameraView );
        D3DXMatrixPerspectiveFovLH( &virtualCameraProj, D3DXToRadian(60.f), m_fAspect, m_zNear, m_zFar);
    }

    D3DXMatrixMultiply(&virtualCameraViewProj, &m_View, &virtualCameraView);
    D3DXMatrixMultiply(&virtualCameraViewProj, &virtualCameraViewProj, &virtualCameraProj);
   
    D3DXMATRIX eyeToPostProjectiveVirtualCamera;
    D3DXMatrixMultiply(&eyeToPostProjectiveVirtualCamera, &virtualCameraView, &virtualCameraProj);

    D3DXVECTOR3 eyeLightDir;  D3DXVec3TransformNormal(&eyeLightDir, &m_lightDir, &m_View);

    //  directional light becomes a point on infinity plane in post-projective space
    D3DXVECTOR4 lightDirW (eyeLightDir.x, eyeLightDir.y, eyeLightDir.z, 0.f);   
    D3DXVECTOR4   ppLight;

    D3DXVec4Transform(&ppLight, &lightDirW, &virtualCameraProj);

    m_bShadowTestInverted = (ppLight.w < 0.f); // the light is coming from behind the eye
    
    //  compute the projection matrix...
    //  if the light is >= 1000 units away from the unit box, use an ortho matrix (standard shadow mapping)
    if ( (fabsf(ppLight.w) <= W_EPSILON) )  // orthographic matrix; uniform shadow mapping
    {
        D3DXVECTOR3 ppLightDirection(ppLight.x, ppLight.y, ppLight.z);
        D3DXVec3Normalize(&ppLightDirection, &ppLightDirection);

        BoundingBox ppUnitBox; ppUnitBox.maxPt = D3DXVECTOR3(1, 1, 1); ppUnitBox.minPt = D3DXVECTOR3(-1, -1, 0);
        D3DXVECTOR3 cubeCenter; ppUnitBox.Centroid( &cubeCenter );
        float t;

        ppUnitBox.Intersect( &t, &cubeCenter, &ppLightDirection );
        D3DXVECTOR3 lightPos = cubeCenter + 2.f*t*ppLightDirection;
        D3DXVECTOR3 axis = yAxis;
        
        //  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
        //  artifacts
        if ( fabsf(D3DXVec3Dot(&ppLightDirection, &yAxis))>0.99f )
            axis = zAxis;

        D3DXMatrixLookAtLH(&lightView, &lightPos, &cubeCenter, &axis);
        XFormBoundingBox(&ppUnitBox, &ppUnitBox, &lightView);
        D3DXMatrixOrthoOffCenterLH(&lightProj, ppUnitBox.minPt.x, ppUnitBox.maxPt.x, ppUnitBox.minPt.y, ppUnitBox.maxPt.y, ppUnitBox.minPt.z, ppUnitBox.maxPt.z);
        m_ppNear = ppUnitBox.minPt.z;
        m_ppFar  = ppUnitBox.maxPt.z;
        m_fSlideBack = 0.f;
    }
    else  // otherwise, use perspective shadow mapping
    {
        D3DXVECTOR3 ppLightPos;
        float wRecip = 1.0f / ppLight.w;
        ppLightPos.x = ppLight.x * wRecip;
        ppLightPos.y = ppLight.y * wRecip;
        ppLightPos.z = ppLight.z * wRecip;

        D3DXMATRIX eyeToPostProjectiveLightView;

        const float ppCubeRadius = 1.5f;  // the post-projective view box is [-1,-1,0]..[1,1,1] in DirectX, so its radius is 1.5
        const D3DXVECTOR3 ppCubeCenter(0.f, 0.f, 0.5f);

        if (m_bShadowTestInverted)  // use the inverse projection matrix
        {
            BoundingCone viewCone;
            if (!m_bUnitCubeClip)
            {
                //  project the entire unit cube into the shadow map  
                std::vector<BoundingBox> justOneBox;
                BoundingBox unitCube;
                unitCube.minPt = D3DXVECTOR3(-1.f, -1.f, 0.f);
                unitCube.maxPt = D3DXVECTOR3( 1.f, 1.f, 1.f );
                justOneBox.push_back(unitCube);
                D3DXMATRIX tmpIdentity;
                D3DXMatrixIdentity(&tmpIdentity);
                viewCone = BoundingCone(&justOneBox, &tmpIdentity, &ppLightPos);               
            }
            else
            {
                //  clip the shadow map to just the used portions of the unit box.
                viewCone = BoundingCone(&m_ShadowReceiverPoints, &eyeToPostProjectiveVirtualCamera, &ppLightPos);
            }
            
            //  construct the inverse projection matrix -- clamp the fNear value for sanity (clamping at too low
            //  a value causes significant underflow in a 24-bit depth buffer)
            //  the multiplication is necessary since I'm not checking shadow casters
            viewCone.fNear = max(0.001f, viewCone.fNear*0.3f);
            m_ppNear = -viewCone.fNear;
            m_ppFar  = viewCone.fNear;
            lightView = viewCone.m_LookAt;
            D3DXMatrixPerspectiveLH( &lightProj, 2.f*tanf(viewCone.fovx)*m_ppNear, 2.f*tanf(viewCone.fovy)*m_ppNear, m_ppNear, m_ppFar );
            //D3DXMatrixPerspectiveFovLH(&lightProj, 2.f*viewCone.fovy, viewCone.fovx/viewCone.fovy, m_ppNear, m_ppFar);
        }
        else  // regular projection matrix
        {
            float fFovy, fAspect, fFar, fNear;
            if (!m_bUnitCubeClip)
            {
                D3DXVECTOR3 lookAt = ppCubeCenter - ppLightPos;
                
                float distance = D3DXVec3Length(&lookAt);
                lookAt = lookAt / distance;
                
                D3DXVECTOR3 axis = yAxis;
                //  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
                //  artifacts
                if ( fabsf(D3DXVec3Dot(&yAxis, &lookAt))>0.99f )
                    axis = zAxis;

                //  this code is super-cheese.  treats the unit-box as a sphere
                //  lots of problems, looks like hell, and requires that MinInfinityZ >= 2
                D3DXMatrixLookAtLH(&lightView, &ppLightPos, &ppCubeCenter, &axis);
                fFovy = 2.f*atanf(ppCubeRadius/distance);
                fAspect = 1.f;
                fNear = max(0.001f, distance - 2.f*ppCubeRadius);
                fFar = distance + 2.f*ppCubeRadius;
                BoundingBox ppView;
                D3DXMatrixMultiply(&eyeToPostProjectiveLightView, &eyeToPostProjectiveVirtualCamera, &lightView);
            }
            else
            {
                //  unit cube clipping
                //  fit a cone to the bounding geometries of all shadow receivers (incl. terrain) in the scene
                BoundingCone bc(&m_ShadowReceiverPoints, &eyeToPostProjectiveVirtualCamera, &ppLightPos);
                lightView = bc.m_LookAt;
                D3DXMatrixMultiply(&eyeToPostProjectiveLightView, &eyeToPostProjectiveVirtualCamera, &lightView);
                float fDistance = D3DXVec3Length(&(ppLightPos-ppCubeCenter));
                fFovy = 2.f * bc.fovy;
                fAspect = bc.fovx / bc.fovy;
                fFar = bc.fFar;
                //  hack alert!  adjust the near-plane value a little bit, to avoid clamping problems
                fNear = bc.fNear * 0.6f;
            }
           
            fNear = max(0.001f, fNear);
            m_ppNear = fNear;
            m_ppFar = fFar;
            D3DXMatrixPerspectiveFovLH(&lightProj, fFovy, fAspect, m_ppNear, m_ppFar);
        }
    }

    //  build the composite matrix that transforms from world space into post-projective light space
    D3DXMatrixMultiply(&m_LightViewProj, &lightView, &lightProj);
    D3DXMatrixMultiply(&m_LightViewProj, &virtualCameraViewProj, &m_LightViewProj);
}

//-----------------------------------------------------------------------------
// Name: BuildLiSPSMProjectionMatrix
// Desc: Builds a light-space perspective shadow map projection matrix
//       Much thanks to Oles Shishkovstov, who provided the original implementation
//-----------------------------------------------------------------------------

void PracticalPSM::BuildLSPSMProjectionMatrix()
{
    if ( fabsf(m_fCosGamma) >= 0.999f )  // degenerates to uniform shadow map
    {
        BuildOrthoShadowProjectionMatrix();
    }
    else
    {
        //  compute shadow casters & receivers
        ComputeVirtualCameraParameters( );

        std::vector<D3DXVECTOR3> bodyB; bodyB.reserve( m_ShadowCasterPoints.size()*8 + 8 );
        Frustum eyeFrustum( &m_Projection );
        for ( int i=0; i<8; i++ ) bodyB.push_back( eyeFrustum.pntList[i] );

        //  build the convex body B by adding all the points for the shadow caster/receiver bounding boxes, plus
        //  the frustum extremities to a list of points
        std::vector<BoundingBox>::iterator boxIt = m_ShadowCasterPoints.begin();
        while ( boxIt != m_ShadowCasterPoints.end() )
        {
            const BoundingBox& box = *boxIt++;
            for ( int i=0; i<8; i++ ) bodyB.push_back( box.Point(i) );
        }

        //  compute the "light-space" basis, using the algorithm described in the paper
        //  note:  since bodyB is defined in eye space, all of these vectors should also be defined in eye space
        D3DXVECTOR3 leftVector, upVector, viewVector;

        const D3DXVECTOR3 eyeVector( 0.f, 0.f, -1.f );  // eye vector in eye space is always -Z
        D3DXVec3TransformNormal( &upVector, &m_lightDir, &m_View );  // lightDir is defined in eye space, so xform it
        //  note: lightDir points away from the scene, so it is already the "negative" up direction;
        //  no need to re-negate it.
        D3DXVec3Cross( &leftVector, &upVector, &eyeVector );
        D3DXVec3Normalize( &leftVector, &leftVector );
        D3DXVec3Cross( &viewVector, &upVector, &leftVector );

        D3DXMATRIX lightSpaceBasis;  
        lightSpaceBasis._11 = leftVector.x; lightSpaceBasis._12 = upVector.x; lightSpaceBasis._13 = viewVector.x; lightSpaceBasis._14 = 0.f;
        lightSpaceBasis._21 = leftVector.y; lightSpaceBasis._22 = upVector.y; lightSpaceBasis._23 = viewVector.y; lightSpaceBasis._24 = 0.f;
        lightSpaceBasis._31 = leftVector.z; lightSpaceBasis._32 = upVector.z; lightSpaceBasis._33 = viewVector.z; lightSpaceBasis._34 = 0.f;
        lightSpaceBasis._41 = 0.f;          lightSpaceBasis._42 = 0.f;        lightSpaceBasis._43 = 0.f;          lightSpaceBasis._44 = 1.f;

        //  rotate all points into this new basis
        D3DXVec3TransformCoordArray( &bodyB[0], sizeof(D3DXVECTOR3), &bodyB[0], sizeof(D3DXVECTOR3), &lightSpaceBasis, (UINT)bodyB.size() );

        BoundingBox lightSpaceBox( &bodyB );
        D3DXVECTOR3 lightSpaceOrigin;
        //  for some reason, the paper recommended using the x coordinate of the xformed viewpoint as
        //  the x-origin for lightspace, but that doesn't seem to make sense...  instead, we'll take
        //  the x-midpt of body B (like for the Y axis)
        lightSpaceBox.Centroid( &lightSpaceOrigin );
        float sinGamma = sqrtf( 1.f - m_fCosGamma*m_fCosGamma );
        
        //  use average of the "real" near/far distance and the optimized near/far distance to get a more pleasant result
        float Nopt0 = m_zNear + sqrtf(m_zNear*m_zFar);
        float Nopt1 = ZNEAR_MIN + sqrtf(ZNEAR_MIN*ZFAR_MAX);
        m_fLSPSM_Nopt  = (Nopt0 + Nopt1) / (2.f*sinGamma);
        //  add a constant bias, to guarantee some minimum distance between the projection point and the near plane
        m_fLSPSM_Nopt += 0.1f;
        //  now use the weighting to scale between 0.1 and the computed Nopt
        float Nopt = 0.1f + m_fLSPSM_NoptWeight * (m_fLSPSM_Nopt - 0.1f);

        lightSpaceOrigin.z = lightSpaceBox.minPt.z - Nopt;

        //  xlate all points in lsBodyB, to match the new lightspace origin, and compute the fov and aspect ratio
        float maxx=0.f, maxy=0.f, maxz=0.f;
        
        std::vector<D3DXVECTOR3>::iterator ptIt = bodyB.begin();

        while ( ptIt != bodyB.end() )
        {
            D3DXVECTOR3 tmp = *ptIt++ - lightSpaceOrigin;
            assert(tmp.z > 0.f);
            maxx = max(maxx, fabsf(tmp.x / tmp.z));
            maxy = max(maxy, fabsf(tmp.y / tmp.z));
            maxz = max(maxz, tmp.z);
        }

        float fovy = atanf(maxy);
        float fovx = atanf(maxx);

        D3DXMATRIX lsTranslate, lsPerspective;  
        
        D3DXMatrixTranslation(&lsTranslate, -lightSpaceOrigin.x, -lightSpaceOrigin.y, -lightSpaceOrigin.z);
        D3DXMatrixPerspectiveLH( &lsPerspective, 2.f*maxx*Nopt, 2.f*maxy*Nopt, Nopt, maxz );

        D3DXMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsTranslate );
        D3DXMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsPerspective );

        //  now rotate the entire post-projective cube, so that the shadow map is looking down the Y-axis
        D3DXMATRIX lsPermute, lsOrtho;

        lsPermute._11 = 1.f; lsPermute._12 = 0.f; lsPermute._13 = 0.f; lsPermute._14 = 0.f;
        lsPermute._21 = 0.f; lsPermute._22 = 0.f; lsPermute._23 =-1.f; lsPermute._24 = 0.f;
        lsPermute._31 = 0.f; lsPermute._32 = 1.f; lsPermute._33 = 0.f; lsPermute._34 = 0.f;
        lsPermute._41 = 0.f; lsPermute._42 = -0.5f; lsPermute._43 = 1.5f; lsPermute._44 = 1.f;

        D3DXMatrixOrthoLH( &lsOrtho, 2.f, 1.f, 0.5f, 2.5f );
        D3DXMatrixMultiply( &lsPermute, &lsPermute, &lsOrtho );
        D3DXMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsPermute );

        if ( m_bUnitCubeClip )
        {
            std::vector<D3DXVECTOR3> receiverPts;
            std::vector<BoundingBox>::iterator rcvrIt = m_ShadowReceiverPoints.begin();
            receiverPts.reserve(m_ShadowReceiverPoints.size() * 8);
            while ( rcvrIt++ != m_ShadowReceiverPoints.end() )
            {
                for ( int i=0; i<8; i++ )
                    receiverPts.push_back( rcvrIt->Point(i) );
            }

            D3DXVec3TransformCoordArray( &receiverPts[0], sizeof(D3DXVECTOR3), &receiverPts[0], sizeof(D3DXVECTOR3), &lightSpaceBasis, (UINT)receiverPts.size() );

            BoundingBox receiverBox( &receiverPts );
            receiverBox.maxPt.x = min( 1.f, receiverBox.maxPt.x );
            receiverBox.minPt.x = max(-1.f, receiverBox.minPt.x );
            receiverBox.maxPt.y = min( 1.f, receiverBox.maxPt.y );
            receiverBox.minPt.y = max(-1.f, receiverBox.minPt.y );
            float boxWidth = receiverBox.maxPt.x - receiverBox.minPt.x;
            float boxHeight = receiverBox.maxPt.y - receiverBox.minPt.y;

            if ( !ALMOST_ZERO(boxWidth) && !ALMOST_ZERO(boxHeight) )
            {
                float boxX = ( receiverBox.maxPt.x + receiverBox.minPt.x ) * 0.5f;
                float boxY = ( receiverBox.maxPt.y + receiverBox.minPt.y ) * 0.5f;

                D3DXMATRIX clipMatrix( 2.f/boxWidth,  0.f, 0.f, 0.f,
                                       0.f, 2.f/boxHeight, 0.f, 0.f,
                                       0.f,           0.f, 1.f, 0.f,
                                      -2.f*boxX/boxWidth, -2.f*boxY/boxHeight, 0.f, 1.f );
                D3DXMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &clipMatrix );
            }
        }

        D3DXMatrixMultiply( &m_LightViewProj, &m_View, &lightSpaceBasis );
    }
}

//-----------------------------------------------------------------------------
// Name: BuildTSMProjectionMatrix
// Desc: Builds a trapezoidal shadow transformation matrix
//-----------------------------------------------------------------------------

void PracticalPSM::BuildTSMProjectionMatrix()
{
    //  this isn't strictly necessary for TSMs; however, my 'light space' matrix has a
    //  degeneracy when view==light, so this avoids the problem.
    if ( fabsf(m_fCosGamma) >= 0.999f )
    {
        BuildOrthoShadowProjectionMatrix();
    }
    else
    {
        //  update list of shadow casters/receivers
        ComputeVirtualCameraParameters();

        //  get the near and the far plane (points) in eye space.
        D3DXVECTOR3 frustumPnts[8];

        Frustum eyeFrustum( &m_Projection );  // autocomputes all the extrema points

        for ( int i=0; i<4; i++ )
        {
            frustumPnts[i]   = eyeFrustum.pntList[(i<<1)];       // far plane
            frustumPnts[i+4] = eyeFrustum.pntList[(i<<1) | 0x1]; // near plane
        }

        //   we need to transform the eye into the light's post-projective space.
        //   however, the sun is a directional light, so we first need to find an appropriate
        //   rotate/translate matrix, before constructing an ortho projection.
        //   this matrix is a variant of "light space" from LSPSMs, with the Y and Z axes permuted

        D3DXVECTOR3 leftVector, upVector, viewVector;
        const D3DXVECTOR3 eyeVector( 0.f, 0.f, -1.f );  //  eye is always -Z in eye space

        //  code copied straight from BuildLSPSMProjectionMatrix
        D3DXVec3TransformNormal( &upVector, &m_lightDir, &m_View );  // lightDir is defined in eye space, so xform it
        D3DXVec3Cross( &leftVector, &upVector, &eyeVector );
        D3DXVec3Normalize( &leftVector, &leftVector );
        D3DXVec3Cross( &viewVector, &upVector, &leftVector );

        D3DXMATRIX lightSpaceBasis;  
        lightSpaceBasis._11 = leftVector.x; lightSpaceBasis._12 = viewVector.x; lightSpaceBasis._13 = -upVector.x; lightSpaceBasis._14 = 0.f;
        lightSpaceBasis._21 = leftVector.y; lightSpaceBasis._22 = viewVector.y; lightSpaceBasis._23 = -upVector.y; lightSpaceBasis._24 = 0.f;
        lightSpaceBasis._31 = leftVector.z; lightSpaceBasis._32 = viewVector.z; lightSpaceBasis._33 = -upVector.z; lightSpaceBasis._34 = 0.f;
        lightSpaceBasis._41 = 0.f;          lightSpaceBasis._42 = 0.f;          lightSpaceBasis._43 = 0.f;        lightSpaceBasis._44 = 1.f;

        //  rotate the view frustum into light space
        D3DXVec3TransformCoordArray( frustumPnts, sizeof(D3DXVECTOR3), frustumPnts, sizeof(D3DXVECTOR3), &lightSpaceBasis, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );

        //  build an off-center ortho projection that translates and scales the eye frustum's 3D AABB to the unit cube
        BoundingBox frustumBox( frustumPnts, sizeof(frustumPnts) / sizeof(D3DXVECTOR3) );

        //  also - transform the shadow caster bounding boxes into light projective space.  we want to translate along the Z axis so that
        //  all shadow casters are in front of the near plane.

        D3DXVECTOR3* shadowCasterPnts = NULL;
        shadowCasterPnts = new D3DXVECTOR3[8*m_ShadowCasterPoints.size()];
        for ( int i=0; i<m_ShadowCasterPoints.size(); i++ )
        {
            for ( int j=0; j<8; j++ ) shadowCasterPnts[i*8+j] = m_ShadowCasterPoints[i].Point(j);
        }

        D3DXVec3TransformCoordArray( shadowCasterPnts, sizeof(D3DXVECTOR3), shadowCasterPnts, sizeof(D3DXVECTOR3), &lightSpaceBasis, m_ShadowCasterPoints.size()*8 );
        BoundingBox casterBox( shadowCasterPnts, m_ShadowCasterPoints.size()*8 );
        delete [] shadowCasterPnts;

        float min_z = min( casterBox.minPt.z, frustumBox.minPt.z );
        float max_z = max( casterBox.maxPt.z, frustumBox.maxPt.z );

        if ( min_z <= 0.f )
        {
            D3DXMATRIX lightSpaceTranslate;
            D3DXMatrixTranslation( &lightSpaceTranslate, 0.f, 0.f, -min_z + 1.f );
            max_z = -min_z + max_z + 1.f;
            min_z = 1.f;
            D3DXMatrixMultiply ( &lightSpaceBasis, &lightSpaceBasis, &lightSpaceTranslate );
            D3DXVec3TransformCoordArray( frustumPnts, sizeof(D3DXVECTOR3), frustumPnts, sizeof(D3DXVECTOR3), &lightSpaceTranslate, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );
            frustumBox = BoundingBox( frustumPnts, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );
        }

        D3DXMATRIX lightSpaceOrtho;
        D3DXMatrixOrthoOffCenterLH( &lightSpaceOrtho, frustumBox.minPt.x, frustumBox.maxPt.x, frustumBox.minPt.y, frustumBox.maxPt.y, min_z, max_z );

        //  transform the view frustum by the new matrix
        D3DXVec3TransformCoordArray( frustumPnts, sizeof(D3DXVECTOR3), frustumPnts, sizeof(D3DXVECTOR3), &lightSpaceOrtho, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );


        D3DXVECTOR2 centerPts[2];
        //  near plane
        centerPts[0].x = 0.25f * (frustumPnts[4].x + frustumPnts[5].x + frustumPnts[6].x + frustumPnts[7].x);
        centerPts[0].y = 0.25f * (frustumPnts[4].y + frustumPnts[5].y + frustumPnts[6].y + frustumPnts[7].y);
        //  far plane
        centerPts[1].x = 0.25f * (frustumPnts[0].x + frustumPnts[1].x + frustumPnts[2].x + frustumPnts[3].x);
        centerPts[1].y = 0.25f * (frustumPnts[0].y + frustumPnts[1].y + frustumPnts[2].y + frustumPnts[3].y);

        D3DXVECTOR2 centerOrig = (centerPts[0] + centerPts[1])*0.5f;

        D3DXMATRIX trapezoid_space;

        D3DXMATRIX xlate_center(           1.f,           0.f, 0.f, 0.f,
                                           0.f,           1.f, 0.f, 0.f,
                                           0.f,           0.f, 1.f, 0.f,
                                 -centerOrig.x, -centerOrig.y, 0.f, 1.f );

        float half_center_len = D3DXVec2Length( &D3DXVECTOR2(centerPts[1] - centerOrig) );
        float x_len = centerPts[1].x - centerOrig.x;
        float y_len = centerPts[1].y - centerOrig.y;

        float cos_theta = x_len / half_center_len;
        float sin_theta = y_len / half_center_len;

        D3DXMATRIX rot_center( cos_theta, -sin_theta, 0.f, 0.f,
                               sin_theta,  cos_theta, 0.f, 0.f,
                                     0.f,        0.f, 1.f, 0.f,
                                     0.f,        0.f, 0.f, 1.f );

        //  this matrix transforms the center line to y=0.
        //  since Top and Base are orthogonal to Center, we can skip computing the convex hull, and instead
        //  just find the view frustum X-axis extrema.  The most negative is Top, the most positive is Base
        //  Point Q (trapezoid projection point) will be a point on the y=0 line.
        D3DXMatrixMultiply( &trapezoid_space, &xlate_center, &rot_center );
        D3DXVec3TransformCoordArray( frustumPnts, sizeof(D3DXVECTOR3), frustumPnts, sizeof(D3DXVECTOR3), &trapezoid_space, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );

        BoundingBox frustumAABB2D( frustumPnts, sizeof(frustumPnts)/sizeof(D3DXVECTOR3) );

        float x_scale = max( fabsf(frustumAABB2D.maxPt.x), fabsf(frustumAABB2D.minPt.x) );
        float y_scale = max( fabsf(frustumAABB2D.maxPt.y), fabsf(frustumAABB2D.minPt.y) );
        x_scale = 1.f/x_scale;
        y_scale = 1.f/y_scale;

        //  maximize the area occupied by the bounding box
        D3DXMATRIX scale_center( x_scale, 0.f, 0.f, 0.f,
                                 0.f, y_scale, 0.f, 0.f,
                                 0.f,     0.f, 1.f, 0.f,
                                 0.f,     0.f, 0.f, 1.f );

        D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &scale_center );

        //  scale the frustum AABB up by these amounts (keep all values in the same space)
        frustumAABB2D.minPt.x *= x_scale;
        frustumAABB2D.maxPt.x *= x_scale;
        frustumAABB2D.minPt.y *= y_scale;
        frustumAABB2D.maxPt.y *= y_scale;

        //  compute eta.
        float lambda = frustumAABB2D.maxPt.x - frustumAABB2D.minPt.x;
        float delta_proj = m_fTSM_Delta * lambda; //focusPt.x - frustumAABB2D.minPt.x;

        const float xi = -0.6f;  // 80% line

        float eta = (lambda*delta_proj*(1.f+xi)) / (lambda*(1.f-xi)-2.f*delta_proj);

        //  compute the projection point a distance eta from the top line.  this point is on the center line, y=0
        D3DXVECTOR2 projectionPtQ( frustumAABB2D.maxPt.x + eta, 0.f );

        //  find the maximum slope from the projection point to any point in the frustum.  this will be the
        //  projection field-of-view
        float max_slope = -1e32f;
        float min_slope =  1e32f;

        for ( int i=0; i < sizeof(frustumPnts)/sizeof(D3DXVECTOR3); i++ )
        {
            D3DXVECTOR2 tmp( frustumPnts[i].x*x_scale, frustumPnts[i].y*y_scale );
            float x_dist = tmp.x - projectionPtQ.x;
            if ( !(ALMOST_ZERO(tmp.y) || ALMOST_ZERO(x_dist)))
            {
                max_slope = max(max_slope, tmp.y/x_dist);
                min_slope = min(min_slope, tmp.y/x_dist);
            }
        }

        float xn = eta;
        float xf = lambda + eta;

        D3DXMATRIX ptQ_xlate(-1.f, 0.f, 0.f, 0.f,
                              0.f, 1.f, 0.f, 0.f,
                              0.f, 0.f, 1.f, 0.f,
                             projectionPtQ.x, 0.f, 0.f, 1.f );
        D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &ptQ_xlate );

        //  this shear balances the "trapezoid" around the y=0 axis (no change to the projection pt position)
        //  since we are redistributing the trapezoid, this affects the projection field of view (shear_amt)
        float shear_amt = (max_slope + fabsf(min_slope))*0.5f - max_slope;
        max_slope = max_slope + shear_amt;

        D3DXMATRIX trapezoid_shear( 1.f, shear_amt, 0.f, 0.f,
                                    0.f,       1.f, 0.f, 0.f,
                                    0.f,       0.f, 1.f, 0.f,
                                    0.f,       0.f, 0.f, 1.f );

        D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoid_shear );


        float z_aspect = (frustumBox.maxPt.z-frustumBox.minPt.z) / (frustumAABB2D.maxPt.y-frustumAABB2D.minPt.y);
        
        //  perform a 2DH projection to 'unsqueeze' the top line.
        D3DXMATRIX trapezoid_projection(  xf/(xf-xn),          0.f, 0.f, 1.f,
                                                0.f, 1.f/max_slope, 0.f, 0.f,
                                                0.f,           0.f, 1.f/(z_aspect*max_slope), 0.f,
                                     -xn*xf/(xf-xn),           0.f, 0.f, 0.f );

        D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoid_projection );

        //  the x axis is compressed to [0..1] as a result of the projection, so expand it to [-1,1]
        D3DXMATRIX biasedScaleX( 2.f, 0.f, 0.f, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 0.f, 0.f, 1.f, 0.f,
                                -1.f, 0.f, 0.f, 1.f );
        D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &biasedScaleX );

        D3DXMatrixMultiply( &trapezoid_space, &lightSpaceOrtho, &trapezoid_space );
        D3DXMatrixMultiply( &trapezoid_space, &lightSpaceBasis, &trapezoid_space );

        // now, focus on shadow receivers.
        if ( m_bUnitCubeClip )
        {
            D3DXVECTOR3* shadowReceiverPnts = NULL;
            shadowReceiverPnts = new D3DXVECTOR3[8*m_ShadowReceiverPoints.size()];
            for ( UINT i=0; i<m_ShadowReceiverPoints.size(); i++ )
            {
                for ( int j=0; j<8; j++ ) shadowReceiverPnts[i*8+j] = m_ShadowReceiverPoints[i].Point(j);
            }

            D3DXVec3TransformCoordArray( shadowReceiverPnts, sizeof(D3DXVECTOR3), shadowReceiverPnts, sizeof(D3DXVECTOR3), &trapezoid_space, m_ShadowReceiverPoints.size()*8 );
            BoundingBox rcvrBox( shadowReceiverPnts, m_ShadowReceiverPoints.size()*8 );
            delete [] shadowReceiverPnts;
            //  never shrink the box, only expand it.
            rcvrBox.maxPt.x = min( 1.f, rcvrBox.maxPt.x );
            rcvrBox.minPt.x = max(-1.f, rcvrBox.minPt.x );
            rcvrBox.maxPt.y = min( 1.f, rcvrBox.maxPt.y );
            rcvrBox.minPt.y = max(-1.f, rcvrBox.minPt.y );
            float boxWidth  = rcvrBox.maxPt.x - rcvrBox.minPt.x;
            float boxHeight = rcvrBox.maxPt.y - rcvrBox.minPt.y;
            
            //  the receiver box is degenerate, this will generate specials (and there shouldn't be any shadows, anyway).
            if ( !(ALMOST_ZERO(boxWidth) || ALMOST_ZERO(boxHeight)) )
            {
                //  the divide by two's cancel out in the translation, but included for clarity
                float boxX = (rcvrBox.maxPt.x+rcvrBox.minPt.x) / 2.f;
                float boxY = (rcvrBox.maxPt.y+rcvrBox.minPt.y) / 2.f;
                D3DXMATRIX trapezoidUnitCube( 2.f/boxWidth,                 0.f, 0.f, 0.f,
                                                    0.f,       2.f/boxHeight, 0.f, 0.f,
                                                    0.f,                 0.f, 1.f, 0.f,
                                        -2.f*boxX/boxWidth, -2.f*boxY/boxHeight, 0.f, 1.f );
                D3DXMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoidUnitCube );
            }
        }

        D3DXMatrixMultiply( &m_LightViewProj, &m_View, &trapezoid_space );

    }
}


//-----------------------------------------------------------------------------
// Name: BuildOrthoShadowProjectionMatrix
// Desc: Builds an orthographic shadow transformation matrix
//-----------------------------------------------------------------------------

void PracticalPSM::BuildOrthoShadowProjectionMatrix()
{
    //  update the list of shadow casters and receivers.
    ComputeVirtualCameraParameters();

    D3DXMATRIX lightView, lightProj;
    const D3DXVECTOR3 zAxis(0.f, 0.f, 1.f);
    const D3DXVECTOR3 yAxis(0.f, 1.f, 0.f);
    D3DXVECTOR3 eyeLightDir;

    D3DXVec3TransformNormal(&eyeLightDir, &m_lightDir, &m_View);

    float fHeight = D3DXToRadian(60.f);
    float fWidth = m_fAspect * fHeight;

    BoundingBox frustumAABB;
    if ( m_bUnitCubeClip )
    {
        frustumAABB = BoundingBox( &m_ShadowReceiverPoints );
    }
    else
    {
        frustumAABB.minPt = D3DXVECTOR3(-fWidth*ZFAR_MAX, -fHeight*ZFAR_MAX, ZNEAR_MIN);
        frustumAABB.maxPt = D3DXVECTOR3( fWidth*ZFAR_MAX,  fHeight*ZFAR_MAX, ZFAR_MAX);
    }
    
    //  light pt is "infinitely" far away from the view frustum.
    //  however, all that's really needed is to place it just outside of all shadow casters

    BoundingBox casterAABB( &m_ShadowCasterPoints );
    D3DXVECTOR3 frustumCenter; frustumAABB.Centroid( &frustumCenter );
    float t;
    casterAABB.Intersect( &t, &frustumCenter, &eyeLightDir );

    D3DXVECTOR3 lightPt = frustumCenter + 2.f*t*eyeLightDir;
    D3DXVECTOR3 axis;

    if ( fabsf(D3DXVec3Dot(&eyeLightDir, &yAxis))>0.99f )
        axis = zAxis;
    else
        axis = yAxis;

    D3DXMatrixLookAtLH( &lightView, &lightPt, &frustumCenter, &axis );
    
    XFormBoundingBox( &frustumAABB, &frustumAABB, &lightView );
    XFormBoundingBox( &casterAABB,  &casterAABB,  &lightView );

    //  use a small fudge factor for the near plane, to avoid some minor clipping artifacts
    D3DXMatrixOrthoOffCenterLH( &lightProj, frustumAABB.minPt.x, frustumAABB.maxPt.x,
                                            frustumAABB.minPt.y, frustumAABB.maxPt.y,
                                            casterAABB.minPt.z, frustumAABB.maxPt.z );

    D3DXMatrixMultiply( &lightView, &m_View, &lightView );
    D3DXMatrixMultiply( &m_LightViewProj, &lightView, &lightProj );
}

//-----------------------------------------------------------------------------
// Name: RenderShadowMap
// Desc: Called once per frame, the function is responsible for building the
//       perspective shadow map projection matrix and rendering the actual
//       shadow map
//-----------------------------------------------------------------------------

HRESULT PracticalPSM::RenderShadowMap(IDirect3DDevice9* m_pd3dDevice)
{  
    switch ( m_iShadowType )
    {
    case (int)SHADOWTYPE_PSM:
        BuildPSMProjectionMatrix();
        break;
    case (int)SHADOWTYPE_LSPSM:
        BuildLSPSMProjectionMatrix();
        break;
    case (int)SHADOWTYPE_TSM:
        BuildTSMProjectionMatrix();
        break;
    case (int)SHADOWTYPE_ORTHO:
        BuildOrthoShadowProjectionMatrix();
        break;
    }
    
    if (FAILED(SetRenderStates(true, m_pd3dDevice)))
        return E_FAIL;

    //  either draw black into the shadow map (ps 1.x), or a greyscale value representing Z (ps 2.0)
    if (m_bSupportsPixelShaders20)
    {
        if (FAILED(m_pEffect->SetTechnique("GenHardwareShadowMap20")))
        {
            MessageBox(NULL, _T("Failed to set 'GenHardwareShadowMap20' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }
    else
    {
        if (FAILED(m_pEffect->SetTechnique("GenHardwareShadowMap11")))
        {
            MessageBox(NULL, _T("Failed to set 'GenHardwareShadowMap11' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }

    D3DXMATRIX tempIdentity;
    D3DXMATRIX clawBotWorldMatrix;
    D3DXMatrixIdentity(&tempIdentity);

    m_pd3dDevice->SetVertexDeclaration(m_pDeclaration);

    //  blast through cached shadow caster objects, and render them all
    for (unsigned int i=0; i<m_ShadowCasterObjects.size(); i++)
    {
        const ObjectInstance*& instance = m_ShadowCasterObjects[i];
        D3DXMATRIX worldMatrix;
        D3DXMatrixTranslation(&worldMatrix, instance->translation.x, instance->translation.y, instance->translation.z);
        D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &m_World);
        if (FAILED(DrawScene(instance->scene, &worldMatrix, &m_LightViewProj, &tempIdentity, &tempIdentity)))
            return E_FAIL;
    }

    return S_OK;


}

//-----------------------------------------------------------------------------
//  PracticalPSM::ComputeVirtualCameraParameters( )
//    computes the near & far clip planes for the virtual camera based on the
//    scene.
//
//    bounds the field of view for the virtual camera based on a swept-sphere/frustum
//    intersection.  if the swept sphere representing the extrusion of an object's bounding
//    sphere along the light direction intersects the view frustum, the object is added to
//    a list of interesting shadow casters.  the field of view is the minimum cone containing
//    all eligible bounding spheres.
//-----------------------------------------------------------------------------

void PracticalPSM::ComputeVirtualCameraParameters( )
{
    bool hit = false;

    //  frustum is in world space, so that bounding boxes are minimal size (xforming an AABB
    //  generally increases its volume).
    D3DXMATRIX modelView;
    D3DXMATRIX modelViewProjection;
    D3DXMatrixMultiply(&modelView, &m_World, &m_View);
    D3DXMatrixMultiply(&modelViewProjection, &modelView, &m_Projection);
    D3DXVECTOR3 sweepDir = -m_lightDir;

    Frustum sceneFrustum( &modelViewProjection );

    m_ShadowCasterPoints.clear();
    m_ShadowCasterObjects.clear();
    m_ShadowReceiverObjects.clear();
    m_ShadowReceiverPoints.clear();

    for (unsigned int i=0; i<m_Scenes.size(); i++)
    {
        const ObjectInstance* instance = m_Scenes[i];
        BoundingBox instanceBox = *instance->aabb;
        instanceBox.minPt += instance->translation;
        instanceBox.maxPt += instance->translation;
        int inFrustum = sceneFrustum.TestBox(&instanceBox);  //  0 = outside.  1 = inside.   2 = intersection

        switch (inFrustum)
        {
        case 0:   // outside frustum -- test swept sphere for potential shadow caster
            {
                BoundingSphere instanceSphere(&instanceBox);
                if (sceneFrustum.TestSweptSphere(&instanceSphere, &sweepDir))
                {
                    hit = true;
                    XFormBoundingBox(&instanceBox, &instanceBox, &modelView);
                    m_ShadowCasterPoints.push_back(instanceBox);
                    m_ShadowCasterObjects.push_back(instance);
                }
                
                break;
            }
        case 1:  //  fully inside frustum.  so store large bounding box
            {
                hit = true;
                XFormBoundingBox(&instanceBox, &instanceBox, &modelView);
                m_ShadowCasterPoints.push_back(instanceBox);
                m_ShadowCasterObjects.push_back(instance);
                m_ShadowReceiverPoints.push_back(instanceBox);
                m_ShadowReceiverObjects.push_back(instance);
                break;
            }
        case 2:   //  big box intersects frustum.  test sub-boxen.  this improves shadow quality, since it allows
                  //  a tighter bounding cone to be used.
            {
                //  only include objects in list once
                m_ShadowCasterObjects.push_back(instance);
                m_ShadowReceiverObjects.push_back(instance);
                const std::vector<BoundingBox>& boxen = *instance->aaBoxen;
                for (int box=0; box<int(boxen.size()); box++)
                {
                    BoundingBox smallBox = boxen[box];
                    smallBox.minPt += instance->translation;
                    smallBox.maxPt += instance->translation;
                    if (sceneFrustum.TestBox(&smallBox)!=0)  // at least part of small box is in frustum
                    {
                        hit = true;
                        XFormBoundingBox(&smallBox, &smallBox, &modelView);
                        m_ShadowCasterPoints.push_back(smallBox);
                        m_ShadowReceiverPoints.push_back(smallBox);
                    }
                }
                break;
            }
        }
    }

    //  add the biggest shadow receiver -- the ground!
    GetTerrainBoundingBox(&m_ShadowReceiverPoints, &modelView, &sceneFrustum);

    //  these are the limits specified by the physical camera
    //  gamma is the "tilt angle" between the light and the view direction.
    m_fCosGamma = m_lightDir.x * m_View._13 +
                  m_lightDir.y * m_View._23 +
                  m_lightDir.z * m_View._33;

    if (!hit)
    {
        m_zNear = ZNEAR_MIN;
        m_zFar = ZFAR_MAX;
        m_fSlideBack = 0.f;
    }
    else
    {
        float min_z = 1e32f;
        float max_z = 0.f;
        for (unsigned int i=0;i < m_ShadowReceiverPoints.size(); i++) 
        {
            min_z = min(min_z, m_ShadowReceiverPoints[i].minPt.z);
            max_z = max(max_z, m_ShadowReceiverPoints[i].maxPt.z);
        }
        m_zNear = max(ZNEAR_MIN, min_z);  
        m_zFar = min( ZFAR_MAX, max_z );
        m_fSlideBack = 0.f;
    }
}

//-----------------------------------------------------------------------------
// Name: CPracticalPSM()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
PracticalPSM::PracticalPSM()
{
    m_time = ::timeGetTime()*0.001f;
    m_startTime = m_time;
    m_frame = 0;
    m_fps = 30;
    m_pEffect = NULL;

    m_pBackBuffer = NULL;
    m_pZBuffer = NULL;
    m_pSMColorTexture = NULL;
    m_pSMZTexture = NULL;
    m_pSMColorSurface = NULL;
    m_pSMZSurface = NULL;
    m_bUnitCubeClip = true;
    m_bSlideBack = true;
    m_lightDir = D3DXVECTOR3(0.f, 0.f, 0.f);
    m_bLightAnimation = true;
    m_bitDepth = 24;
    m_Paused = false;
    m_bSupportsPixelShaders20 = false;
    m_bDisplayShadowMap = false;
    m_pDeclaration = NULL;
    m_iDepthBias = 4;
    m_fLSPSM_NoptWeight = 1.f;
    m_fBiasSlope = 1.0f;
    m_fSlideBack = 0.f;
    m_pRockChunk = NULL;
    m_pClawBot = NULL;
    m_fMinInfinityZ = 1.5f;
    m_fTSM_Delta = 0.52f;
    m_iShadowType = (int)SHADOWTYPE_PSM;

    D3DXMatrixIdentity(&m_World);
    D3DXMatrixIdentity(&m_View);
    D3DXMatrixIdentity(&m_Projection);
}