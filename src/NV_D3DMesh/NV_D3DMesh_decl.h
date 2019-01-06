/*********************************************************************NVMH1****
 


******************************************************************************/

#ifndef _nv_d3d_mesh_dec_h_
#define _nv_d3d_mesh_dec_h_

#ifdef NV_D3D_MESH_DLL

#ifdef NV_D3D_MESH_EXPORTS
#define DECLSPEC_NV_D3D_MESH_API __declspec(dllexport)
#else
#define DECLSPEC_NV_D3D_MESH_API __declspec(dllimport)
#endif

#else
#define DECLSPEC_NV_D3D_MESH_API
#endif

#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "d3dx10.lib" )

#endif  // _nv_d3d_mesh_dec_h_
