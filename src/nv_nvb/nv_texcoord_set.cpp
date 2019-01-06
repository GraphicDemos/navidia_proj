/*********************************************************************NVMH1****
File:
nv_texcoord_set.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_texcoord_set.h>

nv_texcoord_set::nv_texcoord_set()
{
    dim = 0;
    texcoords = 0;
    binormals = 0;
    tangents = 0;
}

nv_texcoord_set::~nv_texcoord_set()
{
    if (texcoords)
    {
        delete [] texcoords;
        texcoords = 0;
    }

    if (binormals)
    {
        delete [] binormals;
        binormals = 0;
    }

    if (tangents)
    {
        delete [] tangents;
        tangents = 0;
    }
}

        /// Write a texcoord_set to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_texcoord_set & rTexCoordSet)
{
    rOutputStream << rTexCoordSet.num_coords;
    rOutputStream << rTexCoordSet.dim;

    unsigned int iCoord, iDim;
    for (iCoord = 0; iCoord < rTexCoordSet.num_coords; ++iCoord)
    {
        for (iDim = 0; iDim < rTexCoordSet.dim; ++iDim)
            rOutputStream << rTexCoordSet.texcoords[rTexCoordSet.dim * iCoord + iDim];
        rOutputStream << rTexCoordSet.binormals[iCoord];
        rOutputStream << rTexCoordSet.tangents[iCoord];
    }

    return rOutputStream;
}

        /// Read a texcoord_set from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_texcoord_set & rTexCoordSet)
{
    rInputStream >> rTexCoordSet.num_coords;
    rInputStream >> rTexCoordSet.dim;

    if (0 == rTexCoordSet.num_coords)
    {
        rTexCoordSet.texcoords = 0;
        rTexCoordSet.binormals = 0;
        rTexCoordSet.tangents  = 0;
    }
    else
    {
        rTexCoordSet.texcoords = new nv_scalar[rTexCoordSet.num_coords * rTexCoordSet.dim];
        rTexCoordSet.binormals = new vec3[rTexCoordSet.num_coords];
        rTexCoordSet.tangents  = new vec3[rTexCoordSet.num_coords];
    }

    unsigned int iCoord, iDim;

    for (iCoord = 0; iCoord < rTexCoordSet.num_coords; ++iCoord)
    {
        for (iDim = 0; iDim < rTexCoordSet.dim; ++iDim)
            rInputStream >> rTexCoordSet.texcoords[rTexCoordSet.dim * iCoord + iDim];
        rInputStream >> rTexCoordSet.binormals[iCoord];
        rInputStream >> rTexCoordSet.tangents[iCoord];
    }

    return rInputStream;
}
