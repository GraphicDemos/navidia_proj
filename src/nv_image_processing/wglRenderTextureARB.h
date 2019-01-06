#ifndef WGL_RENDER_TEXTURE_ARB_H
#define WGL_RENDER_TEXTURE_ARB_H


//
// Includes
//

#include <GL/wglext.h>
#include "nv_image_processing_decl.h"


extern PFNWGLBINDTEXIMAGEARBPROC        wglBindTexImageARB;
extern PFNWGLRELEASETEXIMAGEARBPROC     wglReleaseTexImageARB;
extern PFNWGLSETPBUFFERATTRIBARBPROC    wglSetPbufferAttribARB;


DECLSPEC_NV_MAGE_PROCESSING_DECL_MANIP void initRenderTextureARB(HDC hDC);



#endif // WGL_RENDER_TEXTURE_ARB_H