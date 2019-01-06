/*********************************************************************NVMH1****
File:
nv_scene.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

    
******************************************************************************/

#include "stdafx.h"

nv_scene::nv_scene()
{
    name = 0;
    num_nodes = 0;
    nodes = 0;
    num_textures = 0;
    textures = 0;
    num_materials = 0;
    materials = 0;
    ambient = vec4_null;

    // scene aabb bounding box initialization...
    aabb_min = vec3( FLT_MAX, FLT_MAX, FLT_MAX);
    aabb_max = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX);
    
    // models aabb bounding box initialization...
    models_aabb_min = vec3( FLT_MAX, FLT_MAX, FLT_MAX);
    models_aabb_max = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX);

    num_keys = 0;

    ambient = vec4(nv_scalar(.1),nv_scalar(.1),nv_scalar(.1),nv_one);
}

nv_scene::~nv_scene()
{
    unsigned int i;

    if (name)
    {
        delete [] name;
        name = 0;
    }

    if (num_nodes)
    {
        for (i = 0; i < num_nodes; ++i)
            nodes[i]->release();
        num_nodes = 0;
        delete [] nodes;
        nodes = 0;
    }

    if (num_textures)
    {
        num_textures = 0;
        delete [] textures;
        textures = 0;
    }

    if (num_materials)
    {
        num_materials = 0;
        delete [] materials;
        materials = 0;
    }
}

nv_idx nv_scene::find_node_idx(const nv_node * node)
{
    unsigned int i;
    for ( i = 0; i < num_nodes; ++i)
    {
        if (nodes[i] == node)
            return nv_idx(i);
    }
    
    return NV_BAD_IDX;
}

bool nv_scene::accept(nv_visitor & visitor) const
{
    for (unsigned int i = 0; i < num_nodes; ++i)
    {
        // find root nodes
        if (nodes[i]->parent == NV_BAD_IDX)
            if (nodes[i]->accept(*this,visitor) == false)
                return false;
    }
    return true;
}


        /// Write a scene to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_scene & rScene)
{
                                // Write the scene's name
    unsigned int nNameLength = 0;
    if (rScene.name) 
        nNameLength = strlen(rScene.name);

    rOutputStream << nNameLength;
    rOutputStream.write(rScene.name, nNameLength);

                                // Write out all the nodes
    rOutputStream << rScene.num_nodes;
    for (unsigned int iNode = 0; iNode < rScene.num_nodes; ++iNode)
    {
        rOutputStream << rScene.nodes[iNode]->get_type();
        switch (rScene.nodes[iNode]->get_type())
        {
            case nv_node::CAMERA:
            {
                rOutputStream << *(rScene.nodes[iNode]->asCamera());
            }
            break;
            case nv_node::GEOMETRY:
            {
                rOutputStream << *(rScene.nodes[iNode]->asModel());
            }
            break;
            case nv_node::LIGHT:
            {
                rOutputStream << *(rScene.nodes[iNode]->asLight());
            }   
            break;
            case nv_node::ANONYMOUS:
            {
                rOutputStream << *(rScene.nodes[iNode]);
            }
            break;
            default:
                assert(false); // We came across a node we don't know how to serialize
        }
    }

    rOutputStream << rScene.num_textures;
    for (unsigned int iTexture = 0; iTexture < rScene.num_textures; ++iTexture)
        rOutputStream << rScene.textures[iTexture];

    rOutputStream << rScene.num_materials;
    for (unsigned int iMaterial = 0; iMaterial < rScene.num_materials; ++iMaterial)
        rOutputStream << rScene.materials[iMaterial];

    rOutputStream << rScene.ambient;

                                // Bounding boxes.
    rOutputStream << rScene.aabb_min        << rScene.aabb_max;
    rOutputStream << rScene.models_aabb_min << rScene.models_aabb_max;

    rOutputStream << rScene.num_keys;

    return rOutputStream;
}

        /// Read a scene from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_scene & rScene)
{
                                // Write the scene's name
    unsigned int nNameLength;
    rInputStream >> nNameLength;
    rScene.name = new char[nNameLength+1];
    rInputStream.read(rScene.name, nNameLength);
    rScene.name[nNameLength] = '\0';

                                // Write out all the nodes
    rInputStream >> rScene.num_nodes;
    if (0 == rScene.num_nodes)
        rScene.nodes = 0;
    else
        rScene.nodes = new nv_node*[rScene.num_nodes];

    for (unsigned int iNode = 0; iNode < rScene.num_nodes; ++iNode)
    {
        nv_node::node_type eNodeType;
        rInputStream >> reinterpret_cast<unsigned int &>(eNodeType);
        switch (eNodeType)
        {
            case nv_node::CAMERA:
            {
                nv_camera * pCamera = new nv_camera;
                rInputStream >> *pCamera;
                rScene.nodes[iNode] = pCamera;
            }
            break;
            case nv_node::GEOMETRY:
            {
                nv_model * pModel = new nv_model;
                rInputStream >> *pModel;
                rScene.nodes[iNode] = pModel;
            }
            break;
            case nv_node::LIGHT:
            {
                nv_light * pLight = new nv_light;
                rInputStream >> *pLight;
                rScene.nodes[iNode] = pLight;
            }   
            break;
            case nv_node::ANONYMOUS:
            {
                nv_node * pNode = new nv_node;
                rInputStream >> *pNode;
                rScene.nodes[iNode] = pNode;
            }
            break;
            default:
                assert(false); // We came across a node we don't know how to serialize
        }
    }

    rInputStream >> rScene.num_textures;
    if (0 == rScene.num_textures)
        rScene.textures = 0;
    else
        rScene.textures = new nv_texture[rScene.num_textures];
    for (unsigned int iTexture = 0; iTexture < rScene.num_textures; ++iTexture)
        rInputStream >> rScene.textures[iTexture];

    rInputStream >> rScene.num_materials;
    if (0 == rScene.num_materials)
        rScene.materials = 0;
    else
        rScene.materials = new nv_material[rScene.num_materials];
    for (unsigned int iMaterial = 0; iMaterial < rScene.num_materials; ++iMaterial)
        rInputStream >> rScene.materials[iMaterial];

    rInputStream >> rScene.ambient;

                                // Bounding boxes.
    rInputStream >> rScene.aabb_min        >> rScene.aabb_max;
    rInputStream >> rScene.models_aabb_min >> rScene.models_aabb_max;

    rInputStream >> rScene.num_keys;

    return rInputStream;
}

