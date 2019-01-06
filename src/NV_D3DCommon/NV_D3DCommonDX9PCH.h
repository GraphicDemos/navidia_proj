/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  NV_D3DCommonDX9PCH.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Pre-compiled headers added 7/23/2004
Use this header to include standard sytem include files, or project specific include files that
are used frequently but changed infrequently

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NV_D3DCOMMONDX9PCH_H
#define H_NV_D3DCOMMONDX9PCH_H

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include "shared\NV_StringFuncs.h"

#define NV_USING_D3D9
#include "NV_D3DCommon\NV_D3DCommon.h"

#endif				// H_NV_D3DCOMMONDX9PCH_H