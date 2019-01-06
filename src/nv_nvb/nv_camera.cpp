/*********************************************************************NVMH1****
File:
nv_camera.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_camera.h>

nv_node::node_type nv_camera::type = CAMERA;

nv_node::node_type nv_camera::get_type() const // return the node type
{
    return type;
}

nv_camera::nv_camera()
{
    fov = nv_zero;
    focal_length = nv_zero;
}

nv_camera::~nv_camera()
{
}

        /// Write a camera to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_camera & rCamera)
{
                                // First write the node part of camera
    rOutputStream << dynamic_cast<const nv_node &>(rCamera);
                                
                                // Camera parameters
    rOutputStream << rCamera.fov << rCamera.focal_length;

    return rOutputStream;
}

        /// Read a camera from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_camera & rCamera)
{
                                // First write the node part of camera
    rInputStream >> dynamic_cast<nv_node &>(rCamera);
                                
                                // Camera parameters
    rInputStream >> rCamera.fov >> rCamera.focal_length;


    return rInputStream;
}

