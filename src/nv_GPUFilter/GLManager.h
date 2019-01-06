/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GLManager.h#1 $

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

#ifndef _GLMANAGER_
#define _GLMANAGER_

#include <windows.h>

// This class compartmentalizes the 
class GLManager
{
public:
  static GLManager *Get() {return me;}

  GLManager(HINSTANCE instance, PIXELFORMATDESCRIPTOR *pfd = 0);
  GLManager(HWND hwnd, bool manage, PIXELFORMATDESCRIPTOR *pfd = 0);
  GLManager(HWND hwnd, HDC dc, bool manage);
  GLManager(HWND hwnd, HDC dc, HGLRC rc, bool manage);
  ~GLManager();

  void Update(HINSTANCE instance);
  void Update(HWND hwnd, bool manage);
  void Update(HWND hwnd, HDC dc, bool manage);
  void Update(HGLRC rc, bool manage);

  HDC GetRootDC() {return mDC;}
  HWND GetRootWnd() {return mWnd;}
  HGLRC GetRootRC() {return mRC;}

protected:
  void SetupPFD(PIXELFORMATDESCRIPTOR *pfd);
  void SetupDC();

private:
  static GLManager *me;       // Singleton pointer

  HWND mWnd;                  // The managed window
  HDC mDC;                    // The managed device context
  HGLRC mRC;                  // The managed rendering context
  PIXELFORMATDESCRIPTOR mPFD; // A pixel format descriptor. Not used if mDC was passed manually
  int mPixelFormat;           // The current pixel format of mDC

  bool mManageDC;             // True if mDC shoudl be released when this is deleted
  bool mManageRC;             // True if mRC should be deleted when this is deleted
  bool mManageWND;            // True if mWnd should be closed when this is deleted
};

#endif