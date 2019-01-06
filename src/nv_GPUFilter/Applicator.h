/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/Applicator.h#1 $

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

#ifndef _APPLICATOR_
#define _APPLICATOR_

#include <PIFilter.h>
#include "GPUFilterData.h"
#include "MyPBuffer.h"



// Represents the big texture that is used for performing the filter operation.
// The "texture" can actually be many textures, thanks to Photoshop's ability to
// have lots of channels, and thanks to multiple render targets.

// This singleton requires a TileManager singleton to be initialized.

// All textures are RGBA8. They can represent anywhere from 1 to 4 color channels

class Applicator
{
  friend class PSTile;  // PSTile needs special access

public:
  static Applicator *Get() {return me;}

  Applicator(const VPoint &size);
  ~Applicator();

  VRect GetArea() {return mArea;}
  VRect GetWorkArea() {return mWorkArea;}
  VPoint GetWorkAreaSize();
  VPoint GetFilterSize() {return mFilterSize;}
  void Activate() {mPad.ActivateBlind();}
  void Deactivate() {wglMakeCurrent(mOldDC, mOldRC);}
  HPBUFFERARB GetPadHandle() {return mPad.GetHandle();}

  // This sets the chunk size to maximum and positions it at the point specified by target
  void Target(VPoint &target, bool top = true, bool left = true);
  VRect PeekTargetArea(VPoint &target, bool top = true, bool left = true);
  VRect PeekTargetWorkArea(VPoint &target, bool top = true, bool left = true);

  // Applies the filter. renderArea specifies the area you want rendered to.
  // If invertY is true, it'll draw the image upside down (you're gonna have to test
  // to see which is right). When the call returns, the current GL ReadBuffer will contain
  // the image results
  void FilterTo(
    const VRect &renderArea,
    bool invertY);

  // Call this if the filter size may have changed
  void ResetWorkArea();

protected:
private:
  static Applicator *me;

  HGLRC mOldRC;         // The applicator take control of the rendering context, so it remembers the old one
  HDC mOldDC;           // The applicator must remember the old DC, too
  VRect mArea;          // The area of the image the texture covers
  VRect mWorkArea;      // The area of the image that can be accurately filtered given mArea

  VPoint mAreaSize;     // The size of mArea
  VPoint mFilterSize;   // The cumulative size of the filter kernel
  VPoint mImageSize;    // The size of the overall image

  MyPBuffer mPad;      // The "pad" that all the drawing is done on
  GLuint mTexObject;    // The OpenGL texture object to be bound to mPad as a texture
};

#endif