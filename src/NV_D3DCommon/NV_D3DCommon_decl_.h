/*********************************************************************NVMH1****



******************************************************************************/

#ifndef _nv_d3d_common_dec_h_
#define _nv_d3d_common_dec_h_

#ifdef NV_D3D_COMMON_DLL

#ifdef NV_D3D_COMMON_EXPORTS
#define DECLSPEC_NV_D3D_COMMON_API __declspec(dllexport)
#else
#define DECLSPEC_NV_D3D_COMMON_API __declspec(dllimport)
#endif

#else
#define DECLSPEC_NV_D3D_COMMON_API
#endif

#endif  // _nv_d3d_mesh_dec_h_
