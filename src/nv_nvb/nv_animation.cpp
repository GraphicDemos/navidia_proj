/*********************************************************************NVMH1****
File:
nv_animation.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvb/nv_nvb.h>
#include <nv_nvb/nv_animation.h>

nv_animation::nv_animation()
{
    num_keys = 0;      // number of animation keyframes
    freq = nv_zero;    // sampling frequency
                                    
    // animation tracks - null means not available
    rot = 0;            // rotation
    pos = 0;            // position
    scale = 0;          // scale
}

nv_animation::~nv_animation()
{
    if (pos)
        delete [] pos;
    pos = 0;
    if (rot)
        delete [] rot;
    rot = 0;
    if (scale)
        delete [] scale;
    scale = 0;

}

        /// Write an animation to an nv_output_stream.
        DECLSPEC_NV_NVB 
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_animation & rAnimation)
{
    rOutputStream << rAnimation.num_keys;
    rOutputStream << rAnimation.freq;

		bool bRotation = (rAnimation.rot ? true : false);
    bool bPosition = (rAnimation.pos ? true : false);
    bool bScale    = (rAnimation.scale ? true : false);

    rOutputStream << bRotation << bPosition << bScale;

    for (unsigned int iKey = 0; iKey < rAnimation.num_keys; ++iKey)
    {
        if (bRotation)
            rOutputStream << rAnimation.rot[iKey];
        if (bPosition)
            rOutputStream << rAnimation.pos[iKey];
        if (bScale)
            rOutputStream << rAnimation.scale[iKey];
    }

    return rOutputStream;
}

        /// Read an animation from an nv_input_stream.
        DECLSPEC_NV_NVB 
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_animation & rAnimation)
{
    rInputStream >> rAnimation.num_keys;
    rInputStream >> rAnimation.freq;

    bool bRotation;
    bool bPosition;
    bool bScale;

    rInputStream >> bRotation >> bPosition >> bScale;

    if (bRotation) 
        rAnimation.rot = new quat[rAnimation.num_keys];
    else
        rAnimation.rot = 0;

    if (bPosition)
        rAnimation.pos = new vec3[rAnimation.num_keys];
    else
        rAnimation.pos = 0;

    if (bScale)
        rAnimation.scale = new vec3[rAnimation.num_keys];
    else
        rAnimation.scale = 0;

    for (unsigned int iKey = 0; iKey < rAnimation.num_keys; ++iKey)
    {
        if (bRotation)
            rInputStream >> rAnimation.rot[iKey];
        if (bPosition)
            rInputStream >> rAnimation.pos[iKey];
        if (bScale)
            rInputStream >> rAnimation.scale[iKey];
    }

    return rInputStream;
}

