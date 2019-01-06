/*********************************************************************NVMH1****
File:
nv_jpeg.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef __nv_jpeg_h__
#define __nv_jpeg_h__

namespace jpeg
{
    extern int read(const char * filename, int * width, int * height, unsigned char ** pixels, int * components);
}

#endif // __nv_jpeg_h__
