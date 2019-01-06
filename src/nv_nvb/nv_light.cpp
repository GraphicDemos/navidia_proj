/*********************************************************************NVMH1****
File:
nv_light.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    Defines the entry point for the DLL application.
    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_light.h>

nv_node::node_type nv_light::type = LIGHT;

nv_node::node_type nv_light::get_type() const // return the node type
{
    return type;
}

nv_light::nv_light()
{
    color = vec4_one;
    specular = vec4_one;
    ambient = vec4_null;
    light = ANONYMOUS;          
    direction = vec3_null;
    range = nv_zero;
    Kc = nv_zero;
    Kl = nv_zero;
    Kq = nv_zero;
    falloff = nv_zero;
    theta = nv_zero;
    phi = nv_zero;
    specular_exp = nv_zero;
}

nv_light::~nv_light()
{
    
}

        /// Write a light to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_light & rLight)
{
                                // First write the node part of light
    rOutputStream << dynamic_cast<const nv_node &>(rLight);

                                // Light colors
    rOutputStream << rLight.color << rLight.specular << rLight.ambient;

                                // Light type
    rOutputStream << static_cast<const unsigned int>(rLight.light);

                                // Specular exponent
    rOutputStream << rLight.specular_exp;
    
                                // Direction and range
    rOutputStream << rLight.direction;
    rOutputStream << rLight.range;

                                // Attenuation parameters
    rOutputStream << rLight.Kc << rLight.Kl << rLight.Kq;

                                // Spotlight parameters
    rOutputStream << rLight.falloff << rLight.theta << rLight.phi;
 
    return rOutputStream;
}

        /// Read a light from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_light & rLight)
{
                                // First write the node part of light
    rInputStream >> dynamic_cast<nv_node &>(rLight);

                                // Light colors
    rInputStream >> rLight.color >> rLight.specular >> rLight.ambient;

                                // Light type
    rInputStream >> reinterpret_cast<unsigned int&>(rLight.light);

                                // Specular exponent
    rInputStream >> rLight.specular_exp;
    
                                // Direction and range
    rInputStream >> rLight.direction;
    rInputStream >> rLight.range;

                                // Attenuation parameters
    rInputStream >> rLight.Kc >> rLight.Kl >> rLight.Kq;

                                // Spotlight parameters
    rInputStream >> rLight.falloff >> rLight.theta >> rLight.phi;
 

    return rInputStream;
}
