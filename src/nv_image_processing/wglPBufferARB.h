#ifndef WGL_P_BUFFER_ARB_H
#define WGL_P_BUFFER_ARB_H


//
// Includes
//

#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>
#include "nv_image_processing_decl.h"

extern PFNWGLCREATEPBUFFERARBPROC      wglCreatePbufferARB;
extern PFNWGLGETPBUFFERDCARBPROC       wglGetPbufferDCARB;
extern PFNWGLRELEASEPBUFFERDCARBPROC   wglReleasePbufferDCARB;
extern PFNWGLDESTROYPBUFFERARBPROC     wglDestroyPbufferARB;
extern PFNWGLQUERYPBUFFERARBPROC       wglQueryPbufferARB;


DECLSPEC_NV_MAGE_PROCESSING_DECL_MANIP void initPBufferARB(HDC hDC);

#endif // WGL_P_BUFFER_ARB_H