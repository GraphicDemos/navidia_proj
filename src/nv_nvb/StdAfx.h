#ifndef __NVB_STD_AFX_H__
#define __NVB_STD_AFX_H__

#if defined(WIN32)

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#if _MSC_VER >= 1300
  //  #pragma message("Note: including lib: nv_nvbloader.lib") 
   // #pragma comment(lib,"nv_nvbloader.lib")
#endif

#endif

#include <nv_nvb/nv_nvb.h>

//#include <nv_nvbloader/NVBLoader.h>
#include "nv_nvbfactory.h"

#endif 
