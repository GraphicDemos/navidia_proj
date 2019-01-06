/*********************************************************************NVMH1****
File:
nv_model.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    
******************************************************************************/

#include "stdafx.h"

nv_node::node_type nv_model::type = GEOMETRY;

nv_node::node_type nv_model::get_type() const // return the node type
{
    return type;
}

nv_model::nv_model()
{
    num_meshes = 0;
    meshes = 0;
    aabb_min = vec3( FLT_MAX, FLT_MAX, FLT_MAX);
    aabb_max = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX);
}

nv_model::~nv_model()
{
    if (num_meshes)
    {
        delete [] meshes;
        num_meshes = 0;
        meshes = 0;
    }
}

bool nv_model::accept(const nv_scene & scene, nv_visitor & visitor) const
{
    bool ret = nv_node::accept(scene,visitor);
    visitor.visit_model(this);
    return ret;
}

        /// Write a model to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_model & rModel)
{
                                // First write the node part of model
    rOutputStream << dynamic_cast<const nv_node &>(rModel);

                                // Meshes
    rOutputStream << rModel.num_meshes;
    for (unsigned int iMesh = 0; iMesh < rModel.num_meshes; ++iMesh)
        rOutputStream << rModel.meshes[iMesh];

                                // Bounding box 
    rOutputStream << rModel.aabb_min << rModel.aabb_max;

    return rOutputStream;
}

        /// Read a model from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_model & rModel)
{
                                // First write the node part of model
    rInputStream >> dynamic_cast<nv_node &>(rModel);

                                // Meshes
    rInputStream >> rModel.num_meshes;

    if (0 == rModel.num_meshes)
        rModel.meshes = 0;
    else
        rModel.meshes = new nv_mesh[rModel.num_meshes];

    for (unsigned int iMesh = 0; iMesh < rModel.num_meshes; ++iMesh)
        rInputStream >> rModel.meshes[iMesh];

                                // Bounding box 
    rInputStream >> rModel.aabb_min >> rModel.aabb_max;

    return rInputStream;
}

