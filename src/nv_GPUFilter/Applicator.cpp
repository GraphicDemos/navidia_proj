/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/Applicator.cpp#2 $

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
	Implementation of Applicator class - actual filter graphics functionality

******************************************************************************/

#include <glh/glh_extensions.h>
#include <shared/ErrorHandling.h>
#include <math.h>
#include "Applicator.h"
#include "GPUFilter.h"
#include "GPUFilterData.h"
#include <FilterBigDocument.h>

extern FilterRecord *gFilterRecord;

Applicator *Applicator::me = 0;

Applicator::Applicator(const VPoint &size) : 
  mPad("rgba double textureRECT", true)
{
  if(me)
    throw "You already have an applicator! Throw it away! Throw it away!";
  me = this;

  VPoint zero = {0,0};
  VPoint bigimage = GetImageSize();
  GPUFilterData &data = *GPUFilterData::Get();

  mAreaSize.h = min(size.h, bigimage.h);
  mAreaSize.v = min(size.v, bigimage.v);

  mImageSize = GetImageSize();

  // Set the filter size so everyone can access it
  mFilterSize.h = 0;
  mFilterSize.v = 0;
  mArea.left = mArea.right = mArea.top = mArea.bottom = 0;

  Target(zero, true, true);
  ResetWorkArea();

  GLASSERT()

  glGenTextures(1, &mTexObject);
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, mTexObject);
  GLASSERT()

  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GLASSERT()

  // Create the pbuffers
  if(!mPad.Initialize(mAreaSize.h, mAreaSize.v, false, true))
    throw "Failed to create the applicator!";

  mOldRC = wglGetCurrentContext();
  mOldDC = wglGetCurrentDC();
}

Applicator::~Applicator()
{
  GLASSERT()

  Deactivate();

  GLASSERT()

  mPad.Destroy();

  GLASSERT()

  if(mTexObject)
  {
    glDeleteTextures(1, &mTexObject);
    mTexObject = 0;
  }

  GLASSERT()

  me = 0;
}

void Applicator::ResetWorkArea()
{
  ASSERT(GPUFilterData::Get())
  GPUFilterData &data = *GPUFilterData::Get();

  // Set the filter size so everyone can access it
  mFilterSize.h = 0;
  mFilterSize.v = 0;
  for(list<Pass *>::iterator i = data.mPasses.begin();
      i != data.mPasses.end();
      i++)
  {
    // Accumulate all the filter sizes in the pass group
    mFilterSize.h += (int)ceil((*i)->fWidth);
    mFilterSize.v += (int)ceil((*i)->fHeight);
  }

  // Now you need to correct your working area
  mWorkArea.left = mArea.left + ((mArea.left == 0) ? 0 : mFilterSize.h);
  mWorkArea.right = mArea.right - ((mArea.right == mImageSize.h) ? 0 : mFilterSize.h);
  mWorkArea.top = mArea.top + ((mArea.top == 0) ? 0 : mFilterSize.v);
  mWorkArea.bottom = mArea.bottom - ((mArea.bottom == mImageSize.v) ? 0 : mFilterSize.v);
}

void Applicator::FilterTo(
  const VRect &renderArea,
  bool invertY)
{
  ASSERT(GPUFilterData::Get())
  GPUFilterData &data = *GPUFilterData::Get();
  VRect passarea;
  int param;

  // Intersect renderArea with bufferArea, so we don't render more than we need to
  // Also fluff the area by the filter size, so the filter doesn't get crappy data
  passarea.left = max(renderArea.left, mArea.left) - mFilterSize.h;
  passarea.right = min(renderArea.right, mArea.right) + mFilterSize.h;
  passarea.top = max(renderArea.top, mArea.top) - mFilterSize.v;
  passarea.bottom = min(renderArea.bottom, mArea.bottom) + mFilterSize.v;

	  /// DEBUG - KB
//	  static char myMsg[128];
//	  sprintf(myMsg,"passarea %d/%d %d/%d",
//		  passarea.left,passarea.right,passarea.bottom,passarea.top);
//	  MessageBox(0, myMsg, "FilterTo", MB_OK);
  // Filter through all the passes
  for(list<Pass *>::iterator i = data.mPasses.begin();
      i != data.mPasses.end();
      i++)
  {
    Pass &pass = *(*i);
    bool passinvert;

    SwapBuffers(wglGetCurrentDC());

    // If it's the last pass, do the Y invert (if necessary)
    if(*i == data.mPasses.back())
      passinvert = invertY;
    else
      passinvert = false;

    // Reduce drawing area for next pass
    passarea.left += (int)ceil(pass.fWidth);
    passarea.right -= (int)ceil(pass.fWidth);
    passarea.top += (int)ceil(pass.fHeight);
    passarea.bottom -= (int)ceil(pass.fHeight);

    GLASSERT()

    if(!pass.program->Use())
      throw pass.program->GetInfo();

    GLASSERT()

    // Bind the read buffer as a texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, mTexObject);

    GLASSERT()

    mPad.Bind(WGL_FRONT_LEFT_ARB);

    GLASSERT()

    param = pass.program->GetUniformID("chunk");
    if(param != -1)
      glUniform1iARB(param, 0);
    GLASSERT()
    param = pass.program->GetUniformID("chunkPos");
    if(param != -1)
      glUniform2fARB(
        param,
        (float)mArea.left,
        (float)mArea.top);
    GLASSERT()
    param = pass.program->GetUniformID("chunkSize");
    if(param != -1)
      glUniform2fARB(
        param,
        (float)mAreaSize.h,
        (float)mAreaSize.v);
    GLASSERT()
    param = pass.program->GetUniformID("imageSize");
    if(param != -1)
      glUniform2fARB(
        param,
        (float)mImageSize.h,
        (float)mImageSize.v);
    GLASSERT()
    param = pass.program->GetUniformID("filterSize");
    if(param != -1)
      glUniform2fARB(
        param,
        (float)pass.fWidth,
        (float)pass.fHeight);
    GLASSERT()

    glMatrixMode(GL_PROJECTION);
    GLASSERT()

    glLoadIdentity();

    if(passinvert)
      swap(mArea.top, mArea.bottom);

    glOrtho(
      mArea.left,
      mArea.right,
      mArea.top,
      mArea.bottom,
      -1, 1);

    if(passinvert)
      swap(mArea.top, mArea.bottom);

    GLASSERT()

    // Allow the shader to set it's own parameters and stuff for the pass
    pass.SetParameters(&data);

    GLASSERT()

    // Filter!
    glRecti(passarea.left, passarea.top, passarea.right, passarea.bottom);

    GLASSERT()

    // Unbind the read buffer
    mPad.Release(WGL_FRONT_LEFT_ARB);

    GLASSERT()
  }

  // SwapBuffers(wglGetCurrentDC()); // let the caller decide if they need to do anything befo0re swapping -- KB
}

void Applicator::Target(VPoint &target, bool top, bool left)
{
  // Get the area
  mArea = PeekTargetArea(target, top, left);

  // Calculate the work area
  mWorkArea.left = mArea.left + ((mArea.left == 0) ? 0 : mFilterSize.h);
  mWorkArea.right = mArea.right - ((mArea.right == mImageSize.h) ? 0 : mFilterSize.h);
  mWorkArea.top = mArea.top + ((mArea.top == 0) ? 0 : mFilterSize.v);
  mWorkArea.bottom = mArea.bottom - ((mArea.bottom == mImageSize.v) ? 0 : mFilterSize.v);
}

VRect Applicator::PeekTargetArea(VPoint &target, bool top, bool left)
{
  VRect area;

  // Calculate the filter area.
  if(top)   // Targeting top
  {
    area.top = target.v - mFilterSize.v;
    area.bottom = area.top + mAreaSize.v;
  }
  else      // Targeting bottom
  {
    area.bottom = target.v + mFilterSize.v;
    area.top = area.bottom - mAreaSize.v;
  }

  if(left)  // targeting left side
  {
    area.left = target.h - mFilterSize.h;
    area.right = area.left + mAreaSize.h;
  }
  else      // targeting right side
  {
    area.right = target.h + mFilterSize.h;
    area.left = area.right - mAreaSize.h;
  }

  // Shift area so that it is within the image
  if(area.right > mImageSize.h)
  {
    area.left -= area.right - mImageSize.h;
    area.right = mImageSize.h;
  }
  if(area.bottom > mImageSize.v)
  {
    area.top -= area.bottom - mImageSize.v;
    area.bottom = mImageSize.v;
  }
  if(area.left < 0)
  {
    area.right -= area.left;
    area.left = 0;
  }
  if(area.top < 0)
  {
    area.bottom -= area.top;
    area.top = 0;
  }

  return area;
}

VRect Applicator::PeekTargetWorkArea(VPoint &target, bool top, bool left)
{
  VRect workarea;
  VRect area = PeekTargetArea(target, top, left);

  // Calculate the work area
  workarea.left = area.left + ((area.left == 0) ? 0 : mFilterSize.h);
  workarea.right = area.right - ((area.right == mImageSize.h) ? 0 : mFilterSize.h);
  workarea.top = area.top + ((area.top == 0) ? 0 : mFilterSize.v);
  workarea.bottom = area.bottom - ((area.bottom == mImageSize.v) ? 0 : mFilterSize.v);

  return workarea;
}

VPoint Applicator::GetWorkAreaSize()
{
  VPoint size;

  size.h = mWorkArea.right - mWorkArea.left;
  size.v = mWorkArea.bottom - mWorkArea.top;

  return size;
}