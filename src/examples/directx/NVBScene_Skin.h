/******************************************************************************

  Copyright (C) 2002 NVIDIA Corporation
  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
  not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.
        
******************************************************************************/

//hacky specialized skinned version of NVBScene

#ifndef NVBSCENE_SKIN_H
#define NVBSCENE_SKIN_H

#pragma warning(disable : 4786)
#include <TCHAR.H>
#include <string>
#include <vector>
#include <d3dx9.h>
#include <nv_nvb/nv_nvb.h>

typedef std::basic_string<TCHAR> tstring;

class NVBScene_Skin {
public:
    static const int MAX_NUM_NODES;

    typedef tstring (*GetFilePathCallback)(const tstring&);
    class Mesh {
    public:
        class Vertex {
        public:
            D3DXVECTOR3 m_Position;
            D3DXVECTOR3 m_Normal;
            D3DXVECTOR3 m_Color;
            D3DXVECTOR2 m_Texture;
            D3DXVECTOR3 m_S;
            D3DXVECTOR3 m_T;
            D3DXVECTOR3 m_SxT;
            D3DXVECTOR4 m_Weights;  //four bones
            D3DXVECTOR4 m_Indices;
        };
        Mesh();
        ~Mesh();
        HRESULT Draw() const;
        HRESULT DrawNormals(const D3DXMATRIX&, const D3DXMATRIX&, const D3DXMATRIX&) const;
        HRESULT DrawTangentBasis(const D3DXMATRIX&, const D3DXMATRIX&, const D3DXMATRIX&) const;
        LPDIRECT3DDEVICE9 m_Device;
        std::string m_Name;
        D3DXMATRIX m_Transform;
        LPDIRECT3DTEXTURE9 m_DiffuseMap;
        LPDIRECT3DTEXTURE9 m_HeightMap;
        LPDIRECT3DTEXTURE9 m_NormalMap;
        bool m_VertexHasNormal;
        bool m_VertexHasColor;
        bool m_VertexHasTexture;
        bool m_VertexHasS;
        bool m_VertexHasT;
        bool m_VertexHasSxT;
        bool m_VertexHasTangentBasis;

        //skinning bits
        bool m_IsSkinned;
        std::vector<int> m_Bones;
        std::vector<D3DXMATRIX> m_BoneMatrices;

        unsigned int m_NumVertices;
        LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;
        LPDIRECT3DVERTEXBUFFER9 m_NormalBuffer;
        LPDIRECT3DVERTEXBUFFER9 m_SBuffer;
        LPDIRECT3DVERTEXBUFFER9 m_TBuffer;
        LPDIRECT3DVERTEXBUFFER9 m_SxTBuffer;
        unsigned int m_NumTriangles;
        LPDIRECT3DINDEXBUFFER9 m_IndexBuffer;
    private:
        HRESULT Free();
    };
    NVBScene_Skin();
    ~NVBScene_Skin();
    void Free();
    HRESULT Load(const std::string&, LPDIRECT3DDEVICE9, GetFilePathCallback getFilePath = 0);
    void Update(int key = -1, const D3DXMATRIX* transform = 0);
    HRESULT Draw();
    HRESULT DrawNormals(const D3DXMATRIX&, const D3DXMATRIX&, const D3DXMATRIX&);
    HRESULT DrawTangentBasis(const D3DXMATRIX&, const D3DXMATRIX&, const D3DXMATRIX&);
    HRESULT DrawCube();
    LPDIRECT3DDEVICE9 m_Device;
    unsigned int m_NumMeshes;
    Mesh* m_Meshes;
    D3DXVECTOR3 m_Center;
    float m_Radius;
    unsigned int m_NumKeys;
    bool m_HasACamera;
    D3DXMATRIX m_Projection;
    D3DXMATRIX m_View;
    std::string m_ErrorMessage;
    bool m_VertexHasNormal;
    bool m_VertexHasColor;
    bool m_VertexHasTexture;
    bool m_VertexHasS;
    bool m_VertexHasT;
    bool m_VertexHasSxT;
    bool m_VertexHasTangentBasis;
    bool m_IsSkinned;

private:
    nv_scene m_Scene;

    //vertex for the environment cube
    class CubeVertex {
    public:
        CubeVertex(const D3DXVECTOR3& vecPosition)
            : m_vecPosition(vecPosition),
                m_vecTexture(vecPosition) // Not an error! Copies position into texture coords to draw cubemap as box
        {};
        D3DXVECTOR3 m_vecPosition;
        D3DXVECTOR3 m_vecTexture;
    };
    //smart pointer to simplify freeing of temporary heap allocation
    template<class T> class Ptr {
    public:
        Ptr(unsigned int n) { m_Ptr = (n ? new T[n] : 0); }
        ~Ptr() { if (m_Ptr) delete [] m_Ptr; }
        T& operator[](unsigned int i) { return m_Ptr[i]; }
    private:
        T* m_Ptr;
    };
    void Reset();
    HRESULT LoadTextures(GetFilePathCallback);
    HRESULT LoadTexture(const std::string&, GetFilePathCallback);
    void ReleaseTextures();
    HRESULT LoadMeshes(GetFilePathCallback);
    HRESULT CreateCube();
    static void MatToD3DXMATRIX(const mat4&, D3DXMATRIX&);
    std::vector<LPDIRECT3DTEXTURE9> m_Textures;
    std::vector<std::string> m_TextureNames; //will remove once we have a better way to deal with normal maps
    Mesh m_Cube;
};

#endif
