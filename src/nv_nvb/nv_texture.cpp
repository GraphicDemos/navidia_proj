/*********************************************************************NVMH1****
File:
nv_texture.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_texture.h>

nv_texture::nv_texture()
{
    type = CUSTOM;
    name = 0;
    tex_mat = mat4_id;
}

nv_texture::nv_texture(const nv_texture & tex)
{
    copy_from(tex);
}

nv_texture::~nv_texture()
{
    if (name)
    {
        delete [] name;
        name = 0;
    }
}

const nv_texture & nv_texture::operator= (const nv_texture & tex)
{
    copy_from(tex);
    return *this;
}

void nv_texture::copy_from(const nv_texture & tex)
{
    type = tex.type;
    if (name)
    {
        delete [] name;
        name = 0;
    }
    
    if (tex.name)
    {
        name = new char[strlen(tex.name) + 1];
        strcpy(name,tex.name);
    }
    
    tex_mat = tex.tex_mat;
}


        /// Write a scene to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_texture & rTexture)
{
    rOutputStream << static_cast<unsigned int>(rTexture.type);

                                // Write the namestring, length first.
    rOutputStream << (unsigned int)strlen(rTexture.name);
    rOutputStream.write(rTexture.name, strlen(rTexture.name));

    rOutputStream << rTexture.tex_mat;

    return rOutputStream;
}

        /// Read a scene from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_texture & rTexture)
{
    unsigned int nType;
    rInputStream >> nType;
    rTexture.type = static_cast<nv_texture::_tex_type>(nType);

    int nStringLength;
    rInputStream >> nStringLength;
    rTexture.name = new char[nStringLength + 1] ;
    rTexture.name[nStringLength] = '\0';
    rInputStream.read(rTexture.name, nStringLength);

    rInputStream >> rTexture.tex_mat;

    return rInputStream;
}
