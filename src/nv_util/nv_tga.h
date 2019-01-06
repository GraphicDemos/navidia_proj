/*********************************************************************NVMH1****
File:
nv_tga.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef __nv_tga_h__
#define __nv_tga_h__


#include "nv_util.h"

namespace tga
{
	NVUTIL_API struct tgaImage
    {
        GLsizei  width;
        GLsizei  height;
        GLint    components;
        GLenum   format;

        GLsizei  cmapEntries;
        GLenum   cmapFormat;
        GLubyte *cmap;

        GLubyte *pixels;
	};

	NVUTIL_API  tgaImage * read(const char *filename);
} // namespace tga

#endif /* __nv_tga_h__ */
