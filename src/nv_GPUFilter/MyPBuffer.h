/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/MyPBuffer.h#1 $

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

#ifndef _MYPBUFFER_
#define _MYPBUFFER_

#include <shared/pbuffer.h>

// Adds a necessary bit of functionality to some kind soul's PBuffer class
class MyPBuffer : public PBuffer
{
public:
  MyPBuffer(char *modeString, bool managed = false) : PBuffer(modeString, managed) {}

  void ActivateBlind()
  {
    if (!wglMakeCurrent(m_hDC, m_hGLRC))
        fprintf(stderr, "MyPBuffer::ActivateBlind() failed.\n");
  }

  HPBUFFERARB GetHandle() {return m_hPBuffer;}
protected:
private:
};

#endif