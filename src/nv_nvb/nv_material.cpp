/*********************************************************************NVMH1****
File:
nv_material.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_material.h>

nv_material::nv_material()
{
    name = 0;
    id = NV_BAD_IDX;
    num_textures = 0;
    textures = 0;
    tex_channel = 0;

    diffuse = vec4_null;
    specular = vec4_null;
    shininess = nv_zero;
    ambient = vec4_null;
    emission = vec4_null;

    transparent = nv_one;

    fog = false;
    fog_color = vec4_one;

	// callbackfn = NULL;
	// userdata = 0;
	
}

nv_material::~nv_material()
{
    if (name)
    {
        delete [] name;
        name = 0;
    }

    if (num_textures)
    {
        delete [] textures;
        delete [] tex_channel;
        num_textures = 0;
        textures = 0;
        tex_channel = 0;
    }
	// if(callbackfn)
	//	callbackfn(this, NV_CBMATERIAL_DELETE, userdata);
}


        /// Write a material to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_material & rMaterial)
{
                                // Write the material's name, length first.
    rOutputStream << (unsigned int)strlen(rMaterial.name);
    rOutputStream.write(rMaterial.name, strlen(rMaterial.name));

    rOutputStream << rMaterial.id;

    rOutputStream << rMaterial.num_textures;
    for (unsigned int iTexture = 0; iTexture < rMaterial.num_textures; ++iTexture)
    {
        rOutputStream << rMaterial.textures[iTexture];
        rOutputStream << rMaterial.tex_channel[iTexture];
    }

    rOutputStream << rMaterial.diffuse;
    rOutputStream << rMaterial.specular;
    rOutputStream << rMaterial.shininess;
    rOutputStream << rMaterial.ambient;
    rOutputStream << rMaterial.emission;

    rOutputStream << rMaterial.transparent;

    rOutputStream << rMaterial.fog;
    rOutputStream << rMaterial.fog_color;
    rOutputStream << rMaterial.attr;

    return rOutputStream;
}

        /// Read a material from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_material & rMaterial)
{   
    int nNameLength;
    rInputStream >> nNameLength;
    rMaterial.name = new char[nNameLength+1];
    rInputStream.read(rMaterial.name, nNameLength);
    rMaterial.name[nNameLength] = '\0';

    rInputStream >> rMaterial.id;

    rInputStream >> rMaterial.num_textures;
    if (0 == rMaterial.num_textures)
    {
        rMaterial.textures    = 0;
        rMaterial.tex_channel = 0;
    }
    else
    {
        rMaterial.textures    = new nv_idx[rMaterial.num_textures];
        rMaterial.tex_channel = new nv_idx[rMaterial.num_textures];
    }
    for (unsigned int iTexture = 0; iTexture < rMaterial.num_textures; ++iTexture)
    {
        rInputStream >> rMaterial.textures[iTexture];
        rInputStream >> rMaterial.tex_channel[iTexture];
    }
    rInputStream >> rMaterial.diffuse;
    rInputStream >> rMaterial.specular;
    rInputStream >> rMaterial.shininess;
    rInputStream >> rMaterial.ambient;
    rInputStream >> rMaterial.emission;

    rInputStream >> rMaterial.transparent;

    rInputStream >> rMaterial.fog;
    rInputStream >> rMaterial.fog_color;
    rInputStream >> rMaterial.attr;

    return rInputStream;
}   
