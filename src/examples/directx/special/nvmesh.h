/******************************************************************************

  Copyright (C) 1999, 2000 NVIDIA Corporation
  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
  not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.
  
    Comments:
    
      
        
******************************************************************************/
#ifndef __NVMESH_H
#define __NVMESH_H

//#include <shared/NVDX8Macros.h>
//#include <shared/nvdevice.h>
#include <string>
#include <tchar.h>
#include <D3DX9.h>
typedef std::basic_string<TCHAR> tstring; 

class NVBounds
{
public:
	void Transform(const D3DXMATRIX* pMatrix);
	D3DXVECTOR3 m_vecCenter;
	D3DXVECTOR3 m_vecMinExtents;
	D3DXVECTOR3 m_vecMaxExtents;
	float m_fRadius;
};

// Base frame
class NVMesh
{
public:
	NVMesh(TCHAR* strName = _T("NVMesh") );
	~NVMesh();

	virtual HRESULT InitMaterials( IDirect3DDevice9* pd3dDevice, LPD3DXBUFFER pMtrlBuffer );

	// Rendering
    virtual HRESULT Render( IDirect3DDevice9* pd3dDevice, 
		            BOOL bDrawOpaqueSubsets = TRUE,
		            BOOL bDrawAlphaSubsets = TRUE );

	// Mesh access
    virtual LPD3DXMESH GetSysMemMesh() { return m_pSysMemMesh; }
    virtual LPD3DXMESH GetLocalMesh()  { return m_pLocalMesh; }
	virtual const NVBounds* GetBounds() const { return &m_Bounds; }

	// Rendering options
	virtual HRESULT SetFVF( IDirect3DDevice9* pd3dDevice, DWORD dwFVF );
	virtual HRESULT RenderPart(IDirect3DDevice9* pDevice, DWORD dwPart);
	/*virtual HRESULT SetVertexShader(DWORD dwVertexShader) { m_dwVertexShader = dwVertexShader; return S_OK;}*/
	virtual HRESULT SetTexture(LPDIRECT3DBASETEXTURE9 pTexture) { m_pTexture = pTexture; return S_OK; }
	virtual HRESULT GetRenderInfo();
	virtual HRESULT ComputeBounds();
	
	// Initializing
    virtual HRESULT RestoreDeviceObjects( IDirect3DDevice9* pd3dDevice );
    virtual HRESULT InvalidateDeviceObjects();

	// Creation/destruction
	virtual HRESULT Create( IDirect3DDevice9* pd3dDevice, const tstring& strFilename );
	virtual HRESULT Create( IDirect3DDevice9* pd3dDevice, LPD3DXFILEDATA pFileData );
	virtual HRESULT Destroy();

	TCHAR               m_strName[512];
    LPD3DXMESH          m_pSysMemMesh;    // SysSem mesh, lives through resize
    LPD3DXMESH          m_pLocalMesh;     // Local mesh, rebuilt on resize
    
    DWORD               m_dwNumMaterials; // Materials for the mesh
    D3DMATERIAL9*       m_pMaterials;
    LPDIRECT3DTEXTURE9* m_pTextures;

	/*DWORD					m_dwVertexShader;*/
	LPDIRECT3DBASETEXTURE9	m_pTexture;

	// Attributes for rendering parts of the model
	LPD3DXBUFFER		m_pMeshAdjacency;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;

	DWORD				m_dwAttributes;
	DWORD				m_dwStride;
	D3DXATTRIBUTERANGE*	m_pAttributeTable;	// Attributes

	tstring				m_strMeshPath;

	NVBounds m_Bounds;

	virtual void SetRenderPart(DWORD dwRenderPart) { m_dwRenderPart = dwRenderPart; }
	virtual DWORD GetRenderPart() const { return m_dwRenderPart; }
	DWORD m_dwRenderPart;
};

#endif // __NVDEVICE_H
	