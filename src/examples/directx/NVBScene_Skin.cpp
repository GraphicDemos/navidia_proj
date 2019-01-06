/******************************************************************************

  Copyright (C) 2002 NVIDIA Corporation
  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
  not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.
        
******************************************************************************/

#include "NVBScene_Skin.h"
#include <shared/nvtexture.h>

const int NVBScene_Skin::MAX_NUM_NODES = 200;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) { x->Release(); x=NULL; }; }
#endif

inline std::string TStringToString(const tstring &ts)
{
    char *result = new char[ts.size()+1];

    WideCharToMultiByte(CP_ACP, 0, ts.c_str(), ts.size(), result, ts.size() + 1, NULL, NULL);
    result[ts.size()] = '\0';

    std::string std_result(result);

    delete[] result;
    
    return std_result;
}

inline tstring StringToTString(const std::string &s)
{
    TCHAR *result = new TCHAR[s.size()+1];

    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, result, s.size() + 1);

    tstring t_result(result);

    delete[] result;
    
    return t_result;
}

NVBScene_Skin::NVBScene_Skin()
{
    Reset();
}

NVBScene_Skin::~NVBScene_Skin()
{
    Free();
}

void NVBScene_Skin::Reset()
{
    m_Device = 0;
    m_NumMeshes = 0;
    m_Meshes = 0;
    m_Radius = 0;
    m_NumKeys = 0;
    m_VertexHasNormal = false;
    m_VertexHasColor = false;
    m_VertexHasTexture = false;
    m_VertexHasS = false;
    m_VertexHasT = false;
    m_VertexHasSxT = false;
    m_VertexHasTangentBasis = false;
    m_HasACamera = false;

    m_IsSkinned = false;

    D3DXMatrixIdentity(&m_View);
}

void NVBScene_Skin::Free()
{
    ReleaseTextures();
    if (m_Meshes)
        delete [] m_Meshes;
    Reset();
}

HRESULT NVBScene_Skin::Load(const std::string& filename, LPDIRECT3DDEVICE9 device, GetFilePathCallback getFilePath)
{
    Free();
    m_Device = device;
    std::string fullname = (getFilePath ? TStringToString((*getFilePath)(StringToTString(filename))) : filename);
const char *blee = fullname.c_str();
    if (!NVBLoad(fullname.c_str(), &m_Scene, NVB_LHS)) {
        m_ErrorMessage = std::string("Could not load NVB scene ") + fullname;
        return S_FALSE;
    }
    m_Center.x = (m_Scene.aabb_min.x + m_Scene.aabb_max.x) / 2;
    m_Center.y = (m_Scene.aabb_min.y + m_Scene.aabb_max.y) / 2;
    m_Center.z = (m_Scene.aabb_min.z + m_Scene.aabb_max.z) / 2;
    D3DXVECTOR3 diagonal(m_Scene.aabb_max.x - m_Scene.aabb_min.x, m_Scene.aabb_max.y - m_Scene.aabb_min.y, m_Scene.aabb_max.z - m_Scene.aabb_min.z);
    m_Radius = D3DXVec3Length(&diagonal) / 2;
    HRESULT hr;
    if (FAILED(hr = LoadTextures(getFilePath)))
        return hr;
    if (FAILED(hr = LoadMeshes(getFilePath)))
        return hr;
    m_NumKeys = m_Scene.num_keys;
    Update(-1);
    if (FAILED(hr = CreateCube()))
        return hr;
    return S_OK;
}

void NVBScene_Skin::ReleaseTextures()
{
    for (unsigned int i = 0; i < m_Textures.size(); ++i)
        if (m_Textures[i])
            m_Textures[i]->Release();
    m_Textures.clear();
}

HRESULT NVBScene_Skin::LoadTexture(const std::string& name, GetFilePathCallback getFilePath)
{
    HRESULT hr;
    std::string fullname = (getFilePath ? TStringToString((*getFilePath)(StringToTString(name))) : name);
    LPDIRECT3DTEXTURE9 tex;
    if (FAILED(hr = D3DXCreateTextureFromFileExA(m_Device, fullname.c_str(),
            D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
            0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
            D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
            0, 0, 0, &tex))) {
        m_ErrorMessage = std::string("Could not load texture ") + fullname;
        return hr;
    }
    m_Textures.push_back(tex);
    m_TextureNames.push_back(name);
    return S_OK;
}

HRESULT NVBScene_Skin::LoadTextures(GetFilePathCallback getFilePath)
{
    HRESULT hr;
    ReleaseTextures();
    for (unsigned int i = 0; i < m_Scene.num_textures; ++i)
        if (FAILED(hr = LoadTexture(m_Scene.textures[i].name, getFilePath)))
            return hr;
    return S_OK;
}

HRESULT NVBScene_Skin::LoadMeshes(GetFilePathCallback getFilePath)
{
    HRESULT hr;

    //count meshes
    m_NumMeshes = 0;
    unsigned int i;
    for (i = 0; i < m_Scene.num_nodes; ++i) {
        const nv_node* node = m_Scene.nodes[i];
        if (node->get_type() == nv_node::GEOMETRY) {
            const nv_model* model = reinterpret_cast<const nv_model*>(node);
            for (unsigned int j = 0; j < model->num_meshes; ++j) {
                if (model->meshes[j].vertices == 0)
                    break;
                ++m_NumMeshes;
            }
        }
    }

    //allocate meshes
    if (m_Meshes)
        delete [] m_Meshes;
    m_Meshes = new Mesh [m_NumMeshes];
    m_VertexHasNormal = true;
    m_VertexHasColor = true;
    m_VertexHasTexture = true;
    m_VertexHasS = true;
    m_VertexHasT = true;
    m_VertexHasSxT = true;
    
    m_IsSkinned = false;

    //initialize bone remapper
    int* boneRemapper = new int[m_Scene.num_nodes];
    memset(boneRemapper, -1, m_Scene.num_nodes * sizeof(int));
    int numDistinctBones = 0;

    //fill in the meshes
    float normalLength = 0.1f * m_Radius;
    unsigned int n = 0;
    for (i = 0; i < m_Scene.num_nodes; ++i) {
        const nv_node* node = m_Scene.nodes[i];
        if (node->get_type() == nv_node::GEOMETRY) {
            const nv_model* model = reinterpret_cast<const nv_model*>(node);
            for (unsigned int j = 0; j < model->num_meshes; ++j) {
                const nv_mesh& mesh = model->meshes[j];
                if (mesh.vertices == 0)
                    break;
                //assert(mesh.num_texcoord_sets <= 1); //assume only one texture coordinate set
                assert((mesh.num_texcoord_sets == 0) || (mesh.texcoord_sets[0].dim == 2)); //assume texture coordinates of dimension 2
                Mesh& d3dMesh = m_Meshes[n];
                d3dMesh.m_Device = m_Device;
                d3dMesh.m_Name = node->name;
                d3dMesh.m_NumVertices = mesh.num_vertices;
                d3dMesh.m_NumTriangles = mesh.num_faces;
                d3dMesh.m_VertexHasNormal = (mesh.normals != 0);
                d3dMesh.m_VertexHasColor = (mesh.colors != 0);
                d3dMesh.m_VertexHasTexture = (mesh.num_texcoord_sets > 0) && (mesh.texcoord_sets[0].texcoords != 0);
                d3dMesh.m_VertexHasS = (mesh.num_texcoord_sets > 0) && (mesh.texcoord_sets[0].tangents != 0);
                d3dMesh.m_VertexHasT = (mesh.num_texcoord_sets > 0) && (mesh.texcoord_sets[0].binormals != 0);
                d3dMesh.m_VertexHasSxT = (mesh.normals != 0);
                d3dMesh.m_VertexHasTangentBasis = (d3dMesh.m_VertexHasS && d3dMesh.m_VertexHasT && d3dMesh.m_VertexHasSxT);
                d3dMesh.m_IsSkinned = mesh.skin;

                //vertex buffer
                Ptr<Mesh::Vertex> vertices(d3dMesh.m_NumVertices);
                Ptr<D3DXVECTOR3[2]> normals(mesh.normals ? d3dMesh.m_NumVertices : 0);
                Ptr<D3DXVECTOR3[2]> S(d3dMesh.m_VertexHasS ? d3dMesh.m_NumVertices : 0);
                Ptr<D3DXVECTOR3[2]> T(d3dMesh.m_VertexHasT ? d3dMesh.m_NumVertices : 0);
                Ptr<D3DXVECTOR3[2]> SxT(d3dMesh.m_VertexHasSxT ? d3dMesh.m_NumVertices : 0);
                for (unsigned int k = 0; k < d3dMesh.m_NumVertices; ++k) {
                    Mesh::Vertex& vert = vertices[k];
                    memset(&vert, 0, sizeof(Mesh::Vertex));
                    
                    vert.m_Position.x = mesh.vertices[k].x;
                    vert.m_Position.y = mesh.vertices[k].y;
                    vert.m_Position.z = mesh.vertices[k].z;
                    if (mesh.normals) {
                        vert.m_Normal.x = mesh.normals[k].x;
                        vert.m_Normal.y = mesh.normals[k].y;
                        vert.m_Normal.z = mesh.normals[k].z;
                        normals[k][0] = vert.m_Position;
                        normals[k][1] = vert.m_Position + normalLength * vert.m_Normal;
                    }
                    if (mesh.colors) {
                        vert.m_Color.x = mesh.colors[k].x;
                        vert.m_Color.y = mesh.colors[k].y;
                        vert.m_Color.z = mesh.colors[k].z;
                    }
                    if (d3dMesh.m_VertexHasTexture) {
                        vert.m_Texture.x = mesh.texcoord_sets[0].texcoords[2 * k];
                        vert.m_Texture.y = 1 - mesh.texcoord_sets[0].texcoords[2 * k + 1];
                    }
                    if (d3dMesh.m_VertexHasS) {
                        vert.m_S.x = mesh.texcoord_sets[0].tangents[k].x;
                        vert.m_S.y = mesh.texcoord_sets[0].tangents[k].y;
                        vert.m_S.z = mesh.texcoord_sets[0].tangents[k].z;
                        S[k][0] = vert.m_Position;
                        S[k][1] = vert.m_Position + normalLength * vert.m_S;
                    }
                    if (d3dMesh.m_VertexHasT) {
                        vert.m_T.x = - mesh.texcoord_sets[0].binormals[k].x;
                        vert.m_T.y = - mesh.texcoord_sets[0].binormals[k].y;
                        vert.m_T.z = - mesh.texcoord_sets[0].binormals[k].z;
                        T[k][0] = vert.m_Position;
                        T[k][1] = vert.m_Position + normalLength * vert.m_T;
                    }
                    if (mesh.normals) {
                        vert.m_SxT.x = mesh.normals[k].x;
                        vert.m_SxT.y = mesh.normals[k].y;
                        vert.m_SxT.z = mesh.normals[k].z;
                        SxT[k][0] = vert.m_Position;
                        SxT[k][1] = vert.m_Position + normalLength * vert.m_SxT;
                    }
                    if (d3dMesh.m_IsSkinned)
                    {
                        //grab indices and check for NV_BAD_IDX
                        //if index is bad, zero out weight
                        nv_idx index0 = mesh.bone_idxs[k * 4 + 0];
                        if (index0 == NV_BAD_IDX)
                        {
                            index0 = 0;
                            vert.m_Weights.x = 0.0f;
                        }
                        else
                            vert.m_Weights.x = mesh.weights[k][0];

                        nv_idx index1 = mesh.bone_idxs[k * 4 + 1];
                        if (index1 == NV_BAD_IDX)
                        {
                            index1 = 0;
                            vert.m_Weights.y = 0.0f;
                        }
                        else
                            vert.m_Weights.y = mesh.weights[k][1];

                        nv_idx index2 = mesh.bone_idxs[k * 4 + 2];
                        if (index2 == NV_BAD_IDX)
                        {
                            index2 = 0;
                            vert.m_Weights.z = 0.0f;
                        }
                        else
                            vert.m_Weights.z = mesh.weights[k][2];

                        nv_idx index3 = mesh.bone_idxs[k * 4 + 3];
                        if (index3 == NV_BAD_IDX)
                        {
                            index3 = 0;
                            vert.m_Weights.w = 0.0f;
                        }
                        else
                            vert.m_Weights.w = mesh.weights[k][3];

                        //do remapping
                        int remappedValue = boneRemapper[index0];
                        if (remappedValue == -1)
                        {
                            //we've never seen this bone index before
                            d3dMesh.m_Bones.push_back(index0);

                            vert.m_Indices.x = (float)numDistinctBones;
                            
                            boneRemapper[index0] = numDistinctBones++;
                        }
                        else 
                            vert.m_Indices.x = (float)remappedValue;   //use previous value

                        remappedValue = boneRemapper[index1];
                        if (remappedValue == -1)
                        {
                            //we've never seen this bone index before
                            d3dMesh.m_Bones.push_back(index1);

                            vert.m_Indices.y = (float)numDistinctBones;

                            boneRemapper[index1] = numDistinctBones++;

                        }
                        else 
                            vert.m_Indices.y = (float)remappedValue;   //use previous value
                        
                        remappedValue = boneRemapper[index2];
                        if (remappedValue == -1)
                        {
                            //we've never seen this bone index before
                            d3dMesh.m_Bones.push_back(index2);

                            vert.m_Indices.z = (float)numDistinctBones;

                            boneRemapper[index2] = numDistinctBones++;

                        }
                        else 
                            vert.m_Indices.z = (float)remappedValue;   //use previous value

                        remappedValue = boneRemapper[index3];
                        if (remappedValue == -1)
                        {
                            //we've never seen this bone index before
                            d3dMesh.m_Bones.push_back(index3);

                            vert.m_Indices.w = (float)numDistinctBones;

                            boneRemapper[index3] = numDistinctBones++;

                        }
                        else 
                            vert.m_Indices.w = (float)remappedValue;   //use previous value
                    }
                }
                if (d3dMesh.m_IsSkinned)
                {
                    assert(d3dMesh.m_BoneMatrices.size() == 0);
                    d3dMesh.m_BoneMatrices.resize(d3dMesh.m_Bones.size());
                    assert(d3dMesh.m_BoneMatrices.size() == d3dMesh.m_Bones.size());
                }

                unsigned int size = d3dMesh.m_NumVertices * sizeof(Mesh::Vertex);
                if (FAILED(hr = m_Device->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &d3dMesh.m_VertexBuffer, NULL))) {
                    m_ErrorMessage = "Could not create vertex buffer";
                    return hr;
                }
                unsigned char* buffer;
                if (FAILED(hr = d3dMesh.m_VertexBuffer->Lock(0, size, (void**)&buffer, 0))) {
                    m_ErrorMessage = "Could not lock vertex buffer";
                    return hr;
                }
                memcpy(buffer, &vertices[0], size);
                if (FAILED(hr = d3dMesh.m_VertexBuffer->Unlock())) {
                    m_ErrorMessage = "Could not unlock vertex buffer";
                    return hr;
                }

                //index buffer
                size = 3 * d3dMesh.m_NumTriangles * sizeof(unsigned int);
                if (FAILED(hr = m_Device->CreateIndexBuffer(size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &d3dMesh.m_IndexBuffer, NULL))) {
                    m_ErrorMessage = "Could not create index buffer";
                    return hr;
                }
                if (FAILED(hr = d3dMesh.m_IndexBuffer->Lock(0, size, (void**)&buffer, 0))) {
                    m_ErrorMessage = "Could not lock index buffer";
                    return hr;
                }
                WORD* idx_ptr = reinterpret_cast<WORD*>(buffer);
                for (unsigned int i = 0; i < d3dMesh.m_NumTriangles * 3; ++i)
                {
                    idx_ptr[i] = static_cast<WORD>(mesh.faces_idx[i]);
                }
                if (FAILED(hr = d3dMesh.m_IndexBuffer->Unlock())) {
                    m_ErrorMessage = "Could not unlock index buffer";
                    return hr;
                }

                size = d3dMesh.m_NumVertices * sizeof(D3DXVECTOR3[2]);
                //normal buffer
                if (mesh.normals) {
                    if (FAILED(hr = m_Device->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &d3dMesh.m_NormalBuffer, NULL))) {
                        m_ErrorMessage = "Could not create normal buffer";
                        return hr;
                    }
                    if (FAILED(hr = d3dMesh.m_NormalBuffer->Lock(0, size, (void**)&buffer, 0))) {
                        m_ErrorMessage = "Could not lock normal buffer";
                        return hr;
                    }
                    memcpy(buffer, &normals[0][0], size);
                    if (FAILED(hr = d3dMesh.m_NormalBuffer->Unlock())) {
                        m_ErrorMessage = "Could not unlock normal buffer";
                        return hr;
                    }
                }

                //S buffer
                if (d3dMesh.m_VertexHasS) {
                    if (FAILED(hr = m_Device->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &d3dMesh.m_SBuffer, NULL))) {
                        m_ErrorMessage = "Could not create S buffer";
                        return hr;
                    }
                    if (FAILED(hr = d3dMesh.m_SBuffer->Lock(0, size, (void**)&buffer, 0))) {
                        m_ErrorMessage = "Could not lock S buffer";
                        return hr;
                    }
                    memcpy(buffer, &S[0][0], size);
                    if (FAILED(hr = d3dMesh.m_SBuffer->Unlock())) {
                        m_ErrorMessage = "Could not unlock S buffer";
                        return hr;
                    }
                }

                //T buffer
                if (d3dMesh.m_VertexHasT) {
                    if (FAILED(hr = m_Device->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &d3dMesh.m_TBuffer, NULL))) {
                        m_ErrorMessage = "Could not create S buffer";
                        return hr;
                    }
                    if (FAILED(hr = d3dMesh.m_TBuffer->Lock(0, size, (void**)&buffer, 0))) {
                        m_ErrorMessage = "Could not lock T buffer";
                        return hr;
                    }
                    memcpy(buffer, &T[0][0], size);
                    if (FAILED(hr = d3dMesh.m_TBuffer->Unlock())) {
                        m_ErrorMessage = "Could not unlock T buffer";
                        return hr;
                    }
                }

                //SxT buffer
                if (mesh.normals) {
                    if (FAILED(hr = m_Device->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &d3dMesh.m_SxTBuffer, NULL))) {
                        m_ErrorMessage = "Could not create SxT buffer";
                        return hr;
                    }
                    if (FAILED(hr = d3dMesh.m_SxTBuffer->Lock(0, size, (void**)&buffer, 0))) {
                        m_ErrorMessage = "Could not lock SxT buffer";
                        return hr;
                    }
                    memcpy(buffer, &SxT[0][0], size);
                    if (FAILED(hr = d3dMesh.m_SxTBuffer->Unlock())) {
                        m_ErrorMessage = "Could not unlock SxT buffer";
                        return hr;
                    }
                }

                if (mesh.material_id != -1) {
                    //textures (assume at most one texture of each sort)
                    const nv_material& mat = m_Scene.materials[mesh.material_id];
                    unsigned int diffuse = m_Scene.num_textures;
                    unsigned int bump = m_Scene.num_textures;
                    unsigned int normal = m_Scene.num_textures;
                    unsigned int gloss = m_Scene.num_textures;
                    for (unsigned int t = 0; t < mat.num_textures; ++t)
                        switch (m_Scene.textures[mat.textures[t]].type) {
                        case nv_texture::DIFFUSE:
                            diffuse = mat.textures[t];
                            break;
                        case nv_texture::BUMP:
                            bump = mat.textures[t];
                            break;
                        case nv_texture::NORMAL:
                            normal = mat.textures[t];
                            break;
                        case nv_texture::GLOSS:
                            gloss = mat.textures[t];
                            break;
                        default:
                            break;
                        }
                    if (diffuse < m_Scene.num_textures) 
                        d3dMesh.m_DiffuseMap = m_Textures[diffuse];
                    if (bump < m_Scene.num_textures)
                        d3dMesh.m_HeightMap = m_Textures[bump];
                    if (normal < m_Scene.num_textures)
                        d3dMesh.m_NormalMap = m_Textures[normal];

                    //create a normal map if a height map is assigned, but no normal map
                    if (d3dMesh.m_HeightMap && !d3dMesh.m_NormalMap) {
                        int pos = m_TextureNames[bump].find_last_of('.');
                        std::string basename = m_TextureNames[bump].substr(0, pos);
                        std::string extension = m_TextureNames[bump].substr(pos + 1, m_TextureNames[bump].size() - pos);
                        if (FAILED(hr = LoadTexture(basename + "_normal." + extension, getFilePath))) {
                            d3dMesh.m_NormalMap = NVTexture2::CreateNormalMap(m_Device, d3dMesh.m_HeightMap, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);
                            if (d3dMesh.m_NormalMap)
                                m_Textures.push_back(d3dMesh.m_NormalMap);
                        }
                        else
                            d3dMesh.m_NormalMap = m_Textures.back();
                    }
                }

                if (!d3dMesh.m_VertexHasNormal)
                    m_VertexHasNormal = false;
                if (!d3dMesh.m_VertexHasColor)
                    m_VertexHasColor = false;
                if (!d3dMesh.m_VertexHasTexture)
                    m_VertexHasTexture = false;
                if (!d3dMesh.m_VertexHasS)
                    m_VertexHasS = false;
                if (!d3dMesh.m_VertexHasT)
                    m_VertexHasT = false;
                if (!d3dMesh.m_VertexHasSxT)
                    m_VertexHasSxT = false;
                
                //reverse logic for skinning, you can't expect the entire scene to be skinned
                if (d3dMesh.m_IsSkinned)
                    m_IsSkinned = true;

                ++n;
            }
        }
    }
    m_VertexHasTangentBasis = (m_VertexHasS && m_VertexHasT && m_VertexHasSxT);

    delete[] boneRemapper, boneRemapper = NULL;

    return S_OK;
}

void NVBScene_Skin::Update(int key, const D3DXMATRIX* transform)
{
    if (m_Scene.nodes == 0)
        return;
    if ((key >= 0) && (m_NumKeys == 0))
        return;
    unsigned int n = 0;
    for (unsigned int i = 0; i < m_Scene.num_nodes; ++i) {
        const nv_node* node = m_Scene.nodes[i];
        assert(node);
        if (node->get_type() == nv_node::GEOMETRY) {
            const nv_model& model = *reinterpret_cast<const nv_model*>(node);
            D3DXMATRIX mat;
            if ((0 <= key) && (key < (int)model.anim.num_keys)) {
                // if we have animation keys, we build the correct transform from the animation keys...
                mat4 xform(mat4_id);
                if (model.anim.rot)
                    xform.set_rot(model.anim.rot[key]);
                if (model.anim.pos)
                    xform.set_translation(model.anim.pos[key]);
                MatToD3DXMATRIX(xform, mat);
            }
            else
                // load the transform hierarchy...
                MatToD3DXMATRIX(model.xform, mat);
            for (unsigned int j = 0; j < model.num_meshes; ++j) {
                if (model.meshes[j].vertices == 0)
                    break;
                if (model.meshes[j].skin)
                {
                    for (int k = 0; k < (int)m_Meshes[n].m_Bones.size(); ++k)
                    {
                        mat4 xform(mat4_id);
                        mat4 refxform(mat4_id);
                        mat4 mattmp(mat4_id);
                       
                        int index = m_Meshes[n].m_Bones[k];
                        nv_node * bone_ref = m_Scene.nodes[index];
                        if (bone_ref->anim.num_keys && (key >= 0))
                        {
                            if (bone_ref->anim.rot)
                                mattmp.set_rot(bone_ref->anim.rot[key]);
                            if (bone_ref->anim.pos)
                                mattmp.set_translation(bone_ref->anim.pos[key]);
                            
                            invert(refxform, bone_ref->xform);
                            mult(xform, mattmp, refxform);
                        }

                        MatToD3DXMATRIX(xform, mat);
                        if (transform)
                            m_Meshes[n].m_BoneMatrices[k] = mat * (*transform);
                        else
                            m_Meshes[n].m_BoneMatrices[k] = mat;
                    }
                }
                else
                {
                    if (transform)
                        m_Meshes[n].m_Transform = mat * (*transform);
                    else
                        m_Meshes[n].m_Transform = mat;
                }
                ++n;
            }
        }
        else if (node->get_type() == nv_node::CAMERA) {
            const nv_camera* camera = reinterpret_cast<const nv_camera*>(node);
            float zFar = 15 * m_Radius;
            D3DXMatrixPerspectiveFovLH(&m_Projection, D3DXToRadian(camera->fov), 1.0f, 0.5f, zFar);
            vec3 lookAtPoint;
            if (camera->target == NV_BAD_IDX) {
                sub(lookAtPoint, m_Scene.aabb_max, m_Scene.aabb_min);
                scale(lookAtPoint, nv_zero_5);
                add(lookAtPoint, lookAtPoint, m_Scene.aabb_min);
            }
            else {
                const nv_node* target = m_Scene.nodes[camera->target];
                if (((0 <= key) && (key < (int)target->anim.num_keys)) && target->anim.pos)
                    lookAtPoint = target->anim.pos[key];
                else
                    target->xform.get_translation(lookAtPoint);
            }
            vec3 cameraPosition;
            if (((0 <= key) && (key < (int)camera->anim.num_keys)) && camera->anim.pos)
                cameraPosition = camera->anim.pos[key];
            else
                camera->xform.get_translation(cameraPosition);
            D3DXVECTOR3 eyePt(cameraPosition.x, cameraPosition.y, cameraPosition.z);
            D3DXVECTOR3 lookatPt(lookAtPoint.x, lookAtPoint.y, lookAtPoint.z);
            D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
            D3DXMatrixLookAtLH(&m_View, &eyePt, &lookatPt, &up);
            m_HasACamera = true;
        }
    }
}


HRESULT NVBScene_Skin::Draw()
{
    HRESULT hr;
    for (unsigned int i = 0; i < m_NumMeshes; ++i) {
        const Mesh& mesh = m_Meshes[i];
        m_Device->SetTexture(0, mesh.m_DiffuseMap);
        m_Device->SetTransform(D3DTS_WORLD, &mesh.m_Transform);
        if (FAILED(hr = mesh.Draw())) {
            m_ErrorMessage = "Could not draw mesh";
            return hr;
        }
    }
    return S_OK;
}

HRESULT NVBScene_Skin::DrawNormals(const D3DXMATRIX& world, const D3DXMATRIX& view, const D3DXMATRIX& projection)
{
    HRESULT hr;
    for (unsigned int i = 0; i < m_NumMeshes; ++i)
        if (FAILED(hr = m_Meshes[i].DrawNormals(m_Meshes[i].m_Transform * world, view, projection))) {
            m_ErrorMessage = "Could not draw mesh normals";
            return hr;
        }
    return S_OK;
}

HRESULT NVBScene_Skin::DrawTangentBasis(const D3DXMATRIX& world, const D3DXMATRIX& view, const D3DXMATRIX& projection)
{
    HRESULT hr;
    for (unsigned int i = 0; i < m_NumMeshes; ++i)
        if (FAILED(hr = m_Meshes[i].DrawTangentBasis(m_Meshes[i].m_Transform * world, view, projection))) {
            m_ErrorMessage = "Could not draw mesh tangent basis";
            return hr;
        }
    return S_OK;
}

HRESULT NVBScene_Skin::CreateCube()
{
    HRESULT hr;
    m_Cube.m_NumVertices = 4 * 6;
    m_Cube.m_NumTriangles = 2 * 6;
    if (FAILED(hr = m_Device->CreateVertexBuffer(m_Cube.m_NumVertices * sizeof(CubeVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_Cube.m_VertexBuffer, NULL))) {
        m_ErrorMessage = "Could not create cube vertex buffer!";
        return hr;
    }
    if (FAILED(hr = m_Device->CreateIndexBuffer(3 * m_Cube.m_NumTriangles * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_Cube.m_IndexBuffer, NULL))) {
        m_ErrorMessage = "Could not create cube index buffer!";
        return hr;
    }
    CubeVertex* vertices = 0;
    unsigned short* indices = 0;
    if (FAILED(hr = m_Cube.m_VertexBuffer->Lock(0, sizeof(CubeVertex) * m_Cube.m_NumVertices, reinterpret_cast<void**>(&vertices), 0))) {
        m_ErrorMessage = "Could not lock cube vertex buffer!";
        return hr;
    }
    // -Z face
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f,-1.0f));
    // +Z face
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
    // -Y face
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f,-1.0f));
    // +Y face
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f, 1.0f));
    // -X face
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(1.0f,-1.0f,-1.0f));
    // +X face
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f,-1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f,-1.0f, 1.0f));
    *vertices++ = CubeVertex(D3DXVECTOR3(-1.0f, 1.0f, 1.0f));
    if (FAILED(hr = m_Cube.m_VertexBuffer->Unlock())) {
        m_ErrorMessage = "Could not unlock cube vertex buffer!";
        return hr;
    }
    if (FAILED(hr = m_Cube.m_IndexBuffer->Lock(0, sizeof(unsigned short) * 3 * m_Cube.m_NumTriangles, reinterpret_cast<void**>(&indices), 0))) {
        m_ErrorMessage = "Could not lock cube index buffer!";
        return hr;
    }
    *indices++ =  0+0;   *indices++ =  0+1;   *indices++ =  0+2;
    *indices++ =  0+2;   *indices++ =  0+3;   *indices++ =  0+0;
    *indices++ =  4+0;   *indices++ =  4+1;   *indices++ =  4+2;
    *indices++ =  4+2;   *indices++ =  4+3;   *indices++ =  4+0;
    *indices++ =  8+0;   *indices++ =  8+1;   *indices++ =  8+2;
    *indices++ =  8+2;   *indices++ =  8+3;   *indices++ =  8+0;
    *indices++ = 12+0;   *indices++ = 12+1;   *indices++ = 12+2;
    *indices++ = 12+2;   *indices++ = 12+3;   *indices++ = 12+0;
    *indices++ = 16+0;   *indices++ = 16+1;   *indices++ = 16+2;
    *indices++ = 16+2;   *indices++ = 16+3;   *indices++ = 16+0;
    *indices++ = 20+0;   *indices++ = 20+1;   *indices++ = 20+2;
    *indices++ = 20+2;   *indices++ = 20+3;   *indices++ = 20+0;
    if (FAILED(hr = m_Cube.m_IndexBuffer->Unlock())) {
        m_ErrorMessage = "Could not unlock cube index buffer!";
        return hr;
    }
    return S_OK;
}

HRESULT NVBScene_Skin::DrawCube()
{
    HRESULT hr;
    m_Device->SetVertexShader(0);
    m_Device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0));
    m_Device->SetPixelShader(0);
    if (FAILED(hr = m_Device->SetStreamSource(0, m_Cube.m_VertexBuffer, 0, sizeof(CubeVertex))))
        return hr;
    if (FAILED(hr = m_Device->SetIndices(m_Cube.m_IndexBuffer)))
        return hr;
    if (FAILED(hr = m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_Cube.m_NumVertices, 0, m_Cube.m_NumTriangles)))
        return hr;
    return S_OK;
}

void NVBScene_Skin::MatToD3DXMATRIX(const mat4& mat, D3DXMATRIX& D3DMat)
{
    D3DMat._11 = mat._11; D3DMat._12 = mat._12; D3DMat._13 = mat._13; D3DMat._14 = mat._14;
    D3DMat._21 = mat._21; D3DMat._22 = mat._22; D3DMat._23 = mat._23; D3DMat._24 = mat._24;
    D3DMat._31 = mat._31; D3DMat._32 = mat._32; D3DMat._33 = mat._33; D3DMat._34 = mat._34;
    D3DMat._41 = mat._41; D3DMat._42 = mat._42; D3DMat._43 = mat._43; D3DMat._44 = mat._44;
}

NVBScene_Skin::Mesh::Mesh() :
    m_Device(0),
    m_DiffuseMap(0), m_HeightMap(0), m_NormalMap(0),
    m_NumVertices(0), m_VertexBuffer(0), m_NormalBuffer(0), m_SBuffer(0), m_TBuffer(0), m_SxTBuffer(0), m_NumTriangles(0), m_IndexBuffer(0),
    m_VertexHasNormal(false), m_VertexHasColor(false), m_VertexHasTexture(false), m_VertexHasS(false), m_VertexHasT(false), m_VertexHasSxT(false), m_VertexHasTangentBasis(false)
{
}

NVBScene_Skin::Mesh::~Mesh()
{
    Free();
}

HRESULT NVBScene_Skin::Mesh::Free()
{
    SAFE_RELEASE(m_IndexBuffer);
    SAFE_RELEASE(m_VertexBuffer);
    SAFE_RELEASE(m_NormalBuffer);
    SAFE_RELEASE(m_SBuffer);
    SAFE_RELEASE(m_TBuffer);
    SAFE_RELEASE(m_SxTBuffer);
    SAFE_RELEASE(m_DiffuseMap);
    SAFE_RELEASE(m_HeightMap);
    SAFE_RELEASE(m_NormalMap);

    return S_OK;
}

HRESULT NVBScene_Skin::Mesh::Draw() const
{
    HRESULT hr;
    if (FAILED(hr = m_Device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(Vertex))))
        return hr;
    if (FAILED(hr = m_Device->SetIndices(m_IndexBuffer)))
        return hr;
    if (FAILED(hr = m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumVertices, 0, m_NumTriangles)))
        return hr;
    return S_OK;
}

HRESULT NVBScene_Skin::Mesh::DrawNormals(const D3DXMATRIX& world, const D3DXMATRIX& view, const D3DXMATRIX& projection) const
{
    if (m_VertexHasNormal) {
        m_Device->SetTransform(D3DTS_WORLD, &world);
        m_Device->SetTransform(D3DTS_VIEW, &view);
        m_Device->SetTransform(D3DTS_PROJECTION, &projection);
        m_Device->SetTexture(0, 0);
        m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        m_Device->SetVertexShader(0);
        m_Device->SetFVF(D3DFVF_XYZ);
        m_Device->SetPixelShader(0);
        HRESULT hr;
        m_Device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffff00);
        if (FAILED(hr = m_Device->SetStreamSource(0, m_NormalBuffer, 0, sizeof(D3DXVECTOR3))))
            return hr;
        if (FAILED(hr = m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_NumVertices)))
            return hr;
    }
    return S_OK;
}

HRESULT NVBScene_Skin::Mesh::DrawTangentBasis(const D3DXMATRIX& world, const D3DXMATRIX& view, const D3DXMATRIX& projection) const
{
    if (m_VertexHasS || m_VertexHasT || m_VertexHasSxT) {
        m_Device->SetTransform(D3DTS_WORLD, &world);
        m_Device->SetTransform(D3DTS_VIEW, &view);
        m_Device->SetTransform(D3DTS_PROJECTION, &projection);
        m_Device->SetTexture(0, 0);
        m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        m_Device->SetVertexShader(0);
        m_Device->SetFVF(D3DFVF_XYZ);
        m_Device->SetPixelShader(0);
        HRESULT hr;
        if (m_VertexHasS) {
            m_Device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffff0000);
            if (FAILED(hr = m_Device->SetStreamSource(0, m_SBuffer, 0, sizeof(D3DXVECTOR3))))
                return hr;
            if (FAILED(hr = m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_NumVertices)))
                return hr;
        }
        if (m_VertexHasT) {
            m_Device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xff00ff00);
            if (FAILED(hr = m_Device->SetStreamSource(0, m_TBuffer, 0, sizeof(D3DXVECTOR3))))
                return hr;
            if (FAILED(hr = m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_NumVertices)))
                return hr;
        }
        if (m_VertexHasSxT) {
            m_Device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xff0000ff);
            if (FAILED(hr = m_Device->SetStreamSource(0, m_SxTBuffer, 0, sizeof(D3DXVECTOR3))))
                return hr;
            if (FAILED(hr = m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_NumVertices)))
                return hr;
        }
    }
    return S_OK;
}

