#ifndef HLSL_SOFTSHADOWS_H
#define HLSL_SOFTSHADOWS_H
#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include "shared/NVBScene9.h"
//ShadowMapVertex
struct SMVertex
{
    float x, y, z;
    float nx, ny, nz;
    DWORD diffuse;
    float tx, ty;
    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX0
    };
};

//ShadowMap Mesh
struct SMMeshInfo
{
    LPDIRECT3DVERTEXBUFFER9 pVB;
    LPDIRECT3DINDEXBUFFER9 pIB;
    DWORD dwNumVerts;
    DWORD dwNumFaces;
    D3DPRIMITIVETYPE primType;
    D3DXVECTOR3 scaleVec;
    D3DXVECTOR3 transVec;

    //constructor
    SMMeshInfo() :
        pVB(NULL),
        pIB(NULL),
        dwNumVerts(0),
        dwNumFaces(0),
        primType(D3DPT_TRIANGLELIST),
        scaleVec(1.0f, 1.0f, 1.0f),
        transVec(0.0f, 0.0f, 0.0f)
        {}
};
class SoftShadows
{
public:
	//various bias values
	float m_fDepthBias, m_fBiasSlope;
	D3DXMATRIX m_Projection;
	bool m_bWireframe;
	bool m_bPaused;
    float   m_softness, m_jitter;
	int m_width, m_height;
    int m_NumSamples;

	// data members
	float   m_time;
	float   m_startTime;
	float   m_fps;
	int     m_frame;
	LPD3DXEFFECT m_pEffect;
    CModelViewerCamera m_Light;

private:
	HMENU   m_main_menu;
	HMENU   m_context_menu;

	D3DXVECTOR4 m_lightDir;
	D3DXVECTOR3 m_lightPos;

	LPDIRECT3DSURFACE9 m_pBackBuffer, m_pZBuffer;
	LPDIRECT3DSURFACE9 m_pSMColorSurface, m_pSMZSurface;
	LPDIRECT3DTEXTURE9 m_pSMColorTexture, m_pSMZTexture, m_pSMDecalTexture,
                       m_pFloorTexture, m_pHeadTexture;
    LPDIRECT3DVOLUMETEXTURE9 m_pJitterTexture;

	IDirect3DVertexDeclaration9* m_pDeclaration, *m_pLSDeclaration;

	SMMeshInfo m_smBigship, m_smQuad;

	char *m_shader;

	D3DXATTRIBUTERANGE*  m_pAttributes;

	// Transforms
	D3DXMATRIX m_View;
	D3DXMATRIX m_World;
	D3DXMATRIX m_LightViewProj;

    NVBScene *m_pScene, *m_pLightSource;

	//bounding sphere attributes
	float m_fRadius;
	D3DXVECTOR3 m_vecCenter;

	//bit depth of shadow map
	int m_bitDepth;

	HRESULT SetVertexShaderMatrices(const D3DXMATRIX& worldMat, const D3DXMATRIX& viewMat, const D3DXMATRIX& projMat, const D3DXMATRIX& texMat);
	HRESULT RenderShadowMap(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX &worldMat);
    HRESULT DrawDirectionLight( IDirect3DDevice9* pd3dDevice );
	HRESULT CreateQuad(IDirect3DDevice9* pd3dDevice, SMMeshInfo* mesh);
	HRESULT CheckResourceFormatSupport(IDirect3DDevice9* pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);

public:
	HRESULT ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* m_pBackBufferSurfaceDesc );
	void LostDevice();
	HRESULT Render( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, const D3DXMATRIX* worldMat, const D3DXMATRIX* viewMat, const D3DXMATRIX* projMat);
	void SetShader(int shnum);

	SoftShadows();
};
#endif HLSL_SOFTSHADOWS_H