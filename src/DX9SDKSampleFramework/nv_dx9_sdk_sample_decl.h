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

#ifndef _nv_dx9_sdk_sample_decl_h_
#define _nv_dx9_sdk_sample_decl_h_


#ifdef NV_NVB_DX9SDKSampleFramework_DLL

#ifdef NV_NVB_DX9SDKSampleFramework_LOADER_EXPORTS
#define NVDX9SDKSampleFramework_API __declspec(dllexport)

#else
#define NVDX9SDKSampleFramework_API __declspec(dllimport)
#endif

#else
#define NVDX9SDKSampleFramework_API

#endif

#endif  // _nv_nvb_loader_decl_h_
