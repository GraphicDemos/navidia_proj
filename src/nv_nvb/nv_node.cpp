/*********************************************************************NVMH1****
File:
nv_node.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#include "stdafx.h"

nv_node::node_type nv_node::type = ANONYMOUS;

nv_node::node_type nv_node::get_type() const // return the node type
{
    return type;
}

nv_node::nv_node()
{
    name = 0;
    xform = mat4_id;
    num_children = 0;
    children = 0;
    parent = NV_BAD_IDX;
    target = NV_BAD_IDX;
}

nv_node::~nv_node()
{
    if (children)
    {
        delete [] children;
        num_children = 0;
        children = 0;
    }

    if (name)
    {
        delete [] name;
        name = 0;
    }   
}

//
// Public methods
//

        /// Convert to camera
        nv_camera *
nv_node::asCamera()
{
    return static_cast<nv_camera *>(this);
}

        /// Convert to model
        nv_model *
nv_node::asModel()
{
    return static_cast<nv_model *>(this);
}

        /// Convert to light
        nv_light *
nv_node::asLight()
{
    return static_cast<nv_light *>(this);
}


        bool 
nv_node::accept(const nv_scene & scene, nv_visitor & visitor) const
{
    // depth first traversal
    for (unsigned int i = 0; i < num_children; ++i)
        scene.nodes[children[i]]->accept(scene,visitor);

    visitor.visit_node(this);      
    return true;
}

        /// Write a node to an nv_output_stream.
        DECLSPEC_NV_NVB 
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_node & rNode)
{
                                // Write the node's name
    unsigned int nNameLength = 0;
    if (rNode.name) nNameLength = strlen(rNode.name);
    rOutputStream << nNameLength;
    rOutputStream.write(rNode.name, nNameLength);
                                
                                // Write the
    rOutputStream << rNode.xform;

                                // Write the children (number and indices)
    rOutputStream << rNode.num_children;
    for (unsigned int iChild = 0; iChild < rNode.num_children; ++iChild)
        rOutputStream << rNode.children[iChild];

                                // Write the parent and target indices
    rOutputStream << rNode.parent << rNode.target;

                                // Write the node's animation
    rOutputStream << rNode.anim;

                                // Write the node's attributes
    rOutputStream << rNode.attr;

    return rOutputStream;
}

        /// Read a node from an nv_input_stream.
        DECLSPEC_NV_NVB 
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_node & rNode)
{
                                // Write the node's name
    int nNameLength;
    rInputStream >> nNameLength;
    rNode.name = new char[nNameLength+1];
    rInputStream.read(rNode.name, nNameLength);
    rNode.name[nNameLength] = '\0';
                                
    rInputStream >> rNode.xform;
                                // Write the children (number and indices)
    rInputStream >> rNode.num_children;

    if (0 == rNode.num_children)
        rNode.children = 0;
    else
        rNode.children = new nv_idx[rNode.num_children];

    for (unsigned int iChild = 0; iChild < rNode.num_children; ++iChild)
        rInputStream >> rNode.children[iChild];

                                // Write the parent and target indices
    rInputStream >> rNode.parent >> rNode.target;
                                // Write the node's animation
    rInputStream >> rNode.anim;
                                // Write the node's attributes
    rInputStream >> rNode.attr;

    return rInputStream;
}

