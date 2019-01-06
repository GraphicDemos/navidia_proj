///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains misc buffer related code.
 *  \file       NVBBuffer.h
 *  \author     Pierre Terdiman
 *  \date       April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __NVBBUFFER_H__
#define __NVBBUFFER_H__
#include "nv_nvb_loader_decl.h"
#include "NVBTypes.h"

     NVBCORE_API bool Delta(void* buffer, udword nbitems, udword itemsize);
     NVBCORE_API bool UnDelta(void* buffer, udword nbitems, udword itemsize);

#endif // __NVBBUFFER_H__
