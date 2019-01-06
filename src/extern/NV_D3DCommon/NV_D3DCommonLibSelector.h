/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DCommon\
File:  NV_D3DCommonLibSelector.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NVD3DCOMMONLIBSELECTOR_H
#define H_NVD3DCOMMONLIBSELECTOR_H

// add the appropriate version of the library to the linker's list of inputs
#ifdef _UNICODE
	//#pragma comment( lib, "NV_D3DCommonDX9U.lib" )
	#pragma message("  Note: Including lib: NV_D3DCommonDX9U.lib\n")
#else
	//#pragma comment( lib, "NV_D3DCommonDX9.lib" )
	#pragma message("  Note: Including lib: NV_D3DCommonDX9.lib\n")
#endif

#endif			// H_NVD3DCOMMONLIBSELECTOR_H
