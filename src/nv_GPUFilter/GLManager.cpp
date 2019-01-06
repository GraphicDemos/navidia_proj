/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GLManager.cpp#1 $

Copyright NVIDIA Corporation 2005
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Comments:

******************************************************************************/

#include <glh/glh_extensions.h>
#include <shared/ErrorHandling.h>

#include "GLManager.h"

GLManager *GLManager::me = 0;

GLManager::GLManager(HINSTANCE instance, PIXELFORMATDESCRIPTOR *pfd)
{
  if(me)
    throw "Anotehr GLManager object already exists";
  me = this;

  SetupPFD(pfd);

  mWnd = CreateWindow("STATIC", "", WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    0, 0, 10, 10, 0, 0, instance, 0);
  if(!mWnd)
    throw "Unable to create dummy window!";

  mDC = GetDC(mWnd);
  if(!mDC)
    throw "Unable to create dummy DC!";

  SetupDC();

  mRC = wglCreateContext(mDC);
  if(!mRC)
    throw "Unable to create RC!";

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = true;
  mManageDC = true;
  mManageRC = true;
}

GLManager::GLManager(HWND hwnd, bool manage, PIXELFORMATDESCRIPTOR *pfd)
{
  if(me)
    throw "Anotehr GLManager object already exists";
  me = this;

  SetupPFD(pfd);

  mWnd = hwnd;
  mDC = GetDC(mWnd);
  if(!mDC)
    throw "Unable to create DC!";

  SetupDC();

  mRC = wglCreateContext(mDC);
  if(!mRC)
    throw "Unable to create RC!";

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = manage;
  mManageDC = true;
  mManageRC = true;
}

GLManager::GLManager(HWND hwnd, HDC dc, bool manage)
{
  if(me)
    throw "Anotehr GLManager object already exists";
  me = this;

  mWnd = hwnd;
  mDC = dc;
  mRC = wglCreateContext(mDC);
  if(!mRC)
    throw "Unable to create RC!";

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = false;
  mManageDC = manage;
  mManageRC = true;
}

GLManager::GLManager(HWND hwnd, HDC dc, HGLRC rc, bool manage)
{
  if(me)
    throw "Anotehr GLManager object already exists";
  me = this;

  mWnd = hwnd;
  mDC = dc;
  mRC = rc;

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = false;
  mManageDC = manage;
  mManageRC = manage;
}

GLManager::~GLManager()
{
  if(mManageRC)
  {
    wglMakeCurrent(0,0);
    wglDeleteContext(mRC);
  }

  if(mManageDC)
    ReleaseDC(mWnd, mDC);

  if(mManageWND)
    DestroyWindow(mWnd);

  me = 0;
}

void GLManager::Update(HINSTANCE instance)
{
  ASSERT(wglGetCurrentContext() == mRC && wglGetCurrentDC() == mDC)

    // Destroy everything up to the window
  if(mManageDC)
    ReleaseDC(mWnd, mDC);

  if(mManageWND)
    DestroyWindow(mWnd);

  // recreate things
  mWnd = CreateWindow("STATIC", "", WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    0, 0, 10, 10, 0, 0, instance, 0);
  if(!mWnd)
    throw "Unable to create dummy window!";

  mDC = GetDC(mWnd);
  if(!mDC)
    throw "Unable to create dummy DC!";

  SetupDC();

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = true;
  mManageDC = true;
  mManageRC = true;
}

void GLManager::Update(HWND hwnd, bool manage)
{
  ASSERT(wglGetCurrentContext() == mRC && wglGetCurrentDC() == mDC)

  // Destroy everything up to the window (if managed)
  if(mManageDC)
    ReleaseDC(mWnd, mDC);

  if(mManageWND)
    DestroyWindow(mWnd);

  // Now set up the new window and DC
  mWnd = hwnd;

  mDC = GetDC(mWnd);
  if(!mDC)
    throw "Unable to create DC!";

  if(!SetPixelFormat(mDC, mPixelFormat, &mPFD))
    throw "Couldn't set pixel format on new DC";

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = manage;
  mManageDC = true;
  mManageRC = true;
}

void GLManager::Update(HWND hwnd, HDC dc, bool manage)
{
  // Destroy everything up to the window (if managed)
  if(mManageDC)
    ReleaseDC(mWnd, mDC);

  if(mManageWND)
    DestroyWindow(mWnd);

  mWnd = hwnd;
  mDC = dc;

  HGLRC trc = wglCreateContext(mDC);
  wglShareLists(trc, mRC);

  if(mManageRC)
    wglDeleteContext(mRC);

  mRC = trc;

  if(!wglMakeCurrent(mDC, mRC))
    throw "Unable to make the rendering context current";

  mManageWND = manage;
  mManageDC = true;
  mManageRC = true;
}

void GLManager::Update(HGLRC rc, bool manage)
{
  wglShareLists(rc, mRC);

  wglMakeCurrent(mDC, rc);

  if(mManageRC)
    wglDeleteContext(mRC);

  mRC = rc;
}

void GLManager::SetupPFD(PIXELFORMATDESCRIPTOR *pfd)
{
	PIXELFORMATDESCRIPTOR defaultPFD =
	{
    sizeof(PIXELFORMATDESCRIPTOR), 1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
    32, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, PFD_MAIN_PLANE, 0,0,0,0
  };

  if(pfd)
    memcpy(&mPFD, pfd, sizeof(PIXELFORMATDESCRIPTOR));
  else
    mPFD = defaultPFD;
}

void GLManager::SetupDC()
{
  mPixelFormat = ChoosePixelFormat(mDC, &mPFD);

  if(!mPixelFormat)
    throw "Couldn't find a fitting pixel format";

  if(!SetPixelFormat(mDC, mPixelFormat, &mPFD))
    throw "Couldn't set the pixel format";
}