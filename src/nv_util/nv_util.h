/*********************************************************************NVMH1****
File:
nv_util.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef __nv_util_h__
#define __nv_util_h__

#ifdef _WIN32
#  include <windows.h>
#  pragma warning (disable:4786) // Disable the STL debug information warnings
#endif

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include <assert.h>
#include <vector>
#include <string>
#include <map>

#include <limits.h>

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

// Function linkage
#if NV_UTIL_DLL
#ifdef NV_UTIL_EXPORTS
#define NVUTIL_API __declspec(dllexport)
#define NVUTIL_CLASS __declspec(dllexport)
#else
#define NVUTIL_API __declspec(dllimport)
#define NVUTIL_CLASS __declspec(dllimport)
#endif
#else  
#define NVUTIL_API
#define NVUTIL_CLASS
#endif  

namespace nv_util
{
	NVUTIL_API const char *   set_search_path(const char * path);
	NVUTIL_API const char *   get_search_path();
	NVUTIL_API bool           findfile(const char * filename, int size, char * pathname);
} // nv_util


#endif // __nv_util_h__  
