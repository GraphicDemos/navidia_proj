/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/PreviewManager.h#1 $

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

******************************************************************************/

#ifndef _PREVIEWMANAGER_
#define _PREVIEWMANAGER_

#include <PIFilter.h>
#include <list>
#include <GL/GL.h>
#include <shared/GLShader.h>
#include <shared/GLProgram.h>
#include "ShaderSrc.h"

using namespace std;

const VPoint MaxViewArea = {1536, 1536};    // The largest area of pixels to preview

// Manages all things having to do with the preview window
// Send it GUI messages, and the preview window will
// be taken care of. :)
// It's a singleton, and the GPUFilterData class you inherited is also a singleton,
// so to use this class all you have to do is declare an instance of each, and
// execute the filter. They can find each other from there.

class PreviewManager
{
  friend INT_PTR CALLBACK GlobalGUIProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);

public:
  static PreviewManager *Get() {return me;}    // Singleton getter

  PreviewManager();
  ~PreviewManager();

  int ShowPreview();          // Runs the preview window

  void Pan(VPoint &pan);      // Moves the view of the image by the number of pixels given
  void Resize(VPoint &size);  // Resizes the preview window itself
  //void Zoom(float factor);    // Zooms. The zoom factor is absolute, not relative
  void Draw();                // Draws the preview

  void ParamsChanging();
  void ParamsChanged();       // Tells the manager that the filter parameters changed,
                              // and everything must be re-filtered

protected:
  INT_PTR HandlePreviewMessages(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);           // Handles preview window messages

private:
  static PreviewManager *me;

  bool mPreviewInitialized;   // Is the preview initialized?

  VRect mArea;                // The area of pixels visible to the preview window
  VPoint mTexPos;             // The tile position of the pre-filtered tiles area
  VPoint mTileArea;           // Number of prefiltered tiles to keep around
  GLuint **mTextures;         // Pre-filtered tiles;
  bool **mDirty;              // Specifies dirty pre-filtered tiles
  bool *mColShifts;           // Column shift directions
  bool *mRowShifts;           // Row shift directions

  GLShader mBlitVS;           // Blitting vertex shader
  GLShader mBlitFS;           // Blitting fragment shader
  GLProgram mBlit;            // Blitting program

  GLuint mKTextures[8];       // Volume textures for varying values of K (in CMYK space)
  GLuint mGridTex;            // Grid texture

  void InitPreview();         // Initializes preview-specific stuff (not necessary for real filter op)
  void CleanUpPreview();      // cleans up preview-specific stuff

  void ApplyFilter(VPoint &tile);
};

#endif