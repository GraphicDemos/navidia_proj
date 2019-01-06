/*********************************************************************NVMH1****
File:
nv_nvbdecl.h

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef _nv_nvb_loader_decl_h_
#define _nv_nvb_loader_decl_h_


#ifdef NV_NVB_LOADER_DLL

#ifdef NV_NVB_LOADER_EXPORTS
#define NVBCORE_API __declspec(dllexport)
#define MESHMERIZER_API __declspec(dllexport)

#else
#define NVBCORE_API __declspec(dllimport)
#define MESHMERIZER_API __declspec(dllimport)
#endif

#else
#define NVBCORE_API
#define  MESHMERIZER_API

#endif

#ifdef WIN32
#define inline_             __forceinline
#else
#define inline_             inline
#endif

#ifndef ASSERT
#define ASSERT              assert
#endif

#endif  // _nv_nvb_loader_decl_h_
