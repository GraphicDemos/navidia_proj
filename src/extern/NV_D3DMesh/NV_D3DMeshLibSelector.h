/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\inc\NV_D3DMesh\
File:  NV_D3DMeshLibSelector.h

Use this file to add the NV_D3DMesh library to the linker's list of inputs.  This is included as a
small header separate from the NV_D3DMesh headers so that a project that needs to link in a small
set of the library's functionality can do so without needing to include all of the library's headers.

-------------------------------------------------------------------------------|--------------------*/

#ifndef H_NVD3DMESHLIBSELECTOR_H
#define H_NVD3DMESHLIBSELECTOR_H


// When this library header is included, add the appropriate .lib to be used in linking.
#ifndef NVD3DMESH_NOLIB			// define this to supress the #pragma comment( lib ..)
	#ifdef UNICODE
		//#pragma comment( lib, "NV_D3DMeshDX9U.lib" )
		#pragma message("  Note: Including lib: NV_D3DMeshDX9U.lib\n")
	#else
		//#pragma comment( lib, "NV_D3DMeshDX9.lib" )
		#pragma message("  Note: Including lib: NV_D3DMeshDX9.lib\n")
	#endif
#endif

#endif			// H_NVD3DMESHLIBSELECTOR_H
