#ifndef WGL_PIXEL_FORMAT_ARB_H
#define WGL_PIXEL_FORMAT_ARB_H


//
// Includes
//

#include <GL/wglext.h>
#include <GL/glext.h>

#include "nv_image_processing_decl.h"

extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
extern PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB;


DECLSPEC_NV_MAGE_PROCESSING_DECL_MANIP void initPixelFormatARB(HDC hDC);

#endif // WGL_PIXEL_FORMAT_ARB_H