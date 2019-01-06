/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GPUFilterData.h#1 $

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

#ifndef _GPUFILTERDATA_
#define _GPUFILTERDATA_

#include <windows.h>
#include <vector>
#include <list>
#include <PIFilter.h>
#include <shared/GLProgram.h>
#include "PreviewManager.h"

using namespace std;

extern FilterRecord *gFilterRecord;

class GPUFilterData;

// Inherit this to set up your custom shader parameters
struct Pass
{
  GLProgram *program;   // The GLSlang program used for this pass
  float fWidth;         // The filter width of this pass
  float fHeight;        // The filter height of this pass

  // Override to set up custom parameters in the shader. This is called after
  // the GLSlang program has been enabled, and after all of the internal
  // parameters have been set. Use glUniform***() and GLProgram::GetUniformID()
  // to set your own shader parameters.
  virtual void SetParameters(GPUFilterData *filter) {}
};


// This retains all of the information necessary to perform any filter on the
// GPU. Inherit from it. Use the constructor to load up and validate shaders.
// If you use the vector structures provided, destruction of the shaders and
// programs will happen for you.
class GPUFilterData
{
  friend INT_PTR CALLBACK GlobalGUIProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);

public:
  static GPUFilterData *Get() {return me;}

  GPUFilterData();
  ~GPUFilterData();

  list<Pass *> mPasses;           // Passes, in the order they will be applied

  // These are optionally overridable. In fact, it's probably best
  // to just leave them alone, unless you've got something really
  // fancy planned (multiple GUIs, mayhap?)
  virtual int GetGUIResID();      // Returns the GUI resource ID
  virtual int GetPreviewResID();  // Returns the preview window's resource ID

protected:
  int mGUI;                      // Resource ID of the GUI dialog box
  int mPreview;                  // Resource ID of the preview window

  // Override this to handle Windows messages sent to the preview window
  // See the documentation for DialogProc in the MSDN library.
  virtual INT_PTR HandlePreviewMessages(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);

private:
  static GPUFilterData *me;
};

#endif