/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DMesh\
File:  NV_D3DMeshDX9.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
This is the main header for the NV_D3DMesh library for DX9.  Include this in your own code that
uses the NV_D3DMesh library.  It will automaticaly add the NV_D3DMeshDX9.lib to your linker
inputs using #pragma comment( lib, "NV_D3DMeshDX9.lib" )

You can use NV_D3DMeshTypes.h in your headers so that only the types are defined.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NVD3DMESHDX9_LIB_H
#define H_NVD3DMESHDX9_LIB_H

// Include the header that includes sub-headers and selects the .lib to link with
#include "NV_D3DMesh\NV_D3DMesh.h"

#endif		// H_NVD3DMESHDX9_LIB_H
