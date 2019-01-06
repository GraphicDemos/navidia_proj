/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GPUFilter.h#1 $

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

#ifndef _GPUFILTER_
#define _GPUFILTER_

#include <PIFilter.h>
#include <windows.h>

#include <shared/GLShader.h>
#include <shared/GLProgram.h>
#include <shared/TileManager.h>
#include "Applicator.h"
#include "GPUFilterData.h"

// We all agreed on this
const int MaxFilterSize = 255;
const VPoint ApplicatorSize = {2048, 2048};

// This class organizes all of the basic data and operations
// of a GPU-based filter into one class.
class GPUFilter
{
public:
  static GPUFilter *Get() {return me;}  // singleton getter

  GPUFilter();
  ~GPUFilter();

  void ApplyFilter();       // Do it!

protected:
  bool NextChunkPos(VPoint &chunk);
  bool ShouldRetireTileSource(VPoint &tilepos);

private:

  static GPUFilter *me;     // Singleton

  VPoint mImageSize;        // The size of the image to be filtered
  VRect mImageRect;         // The area of the image to be filtered

  // Intermediate values in all the rectangle math that goes on
  VPoint mChunkPos;         // The position of the chunk being filtered
  VPoint mNextChunk;        // The next position of the chunk
  bool mGoingRight;         // Are we moving right or left across the image?
  bool mNextGoingRight;     // The next going-right state
  VRect mRelevantTiles;     // The tiles relevant to the current chunk

  void DoChunkFilter();     // Do the filter side of Go()
  void DoChunkDataRead();   // Do the data-read side of Go()
};

#endif