/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/GPUFilter.cpp#2 $

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
	Implementation of filter class - calls Applicator to do actual work

******************************************************************************/

#include <glh/glh_extensions.h>
#include <shared/pbuffer.h>
#include <shared/ErrorHandling.h>
#include "GPUFilter.h"
#include "PreviewManager.h"
#include "Resource.h"
#include <FilterBigDocument.h>

GPUFilter *GPUFilter::me = 0;

GPUFilter::GPUFilter()
  : mGoingRight(true)
{
  if(me)
    throw "You already have a filter made! Buffoon!";
  me = this;

  mImageRect = GetFilterRect();
	mImageSize.v = mImageRect.bottom - mImageRect.top;
	mImageSize.h = mImageRect.right - mImageRect.left;

  mChunkPos.h = mChunkPos.v = -1;
}

GPUFilter::~GPUFilter()
{
  me = 0;
}

void GPUFilter::ApplyFilter()
{
  ASSERT(TileManager::Get())

  Applicator applicator(ApplicatorSize);

  mChunkPos.h = -1;
  mChunkPos.v = -1;
  mGoingRight = NextChunkPos(mChunkPos);

  applicator.Activate();

  while(mChunkPos.h != -1 && mChunkPos.v != -1)
  {
//	  /// DEBUG - KB
//	  static char myMsg[64];
//	  sprintf(myMsg,"ChunkPos %d/%d",mChunkPos.h,mChunkPos.v);
//	  MessageBox(0, myMsg, "ApplyFilter", MB_OK);
    applicator.Target(mChunkPos, true, mGoingRight);

    mNextGoingRight = NextChunkPos(mNextChunk);
    DoChunkFilter();
    DoChunkDataRead();

    mChunkPos = mNextChunk;
    mGoingRight = mNextGoingRight;
  }

  applicator.Deactivate();
}

void GPUFilter::DoChunkFilter()
{
  VPoint tile;          // Just an iterator
  VRect workarea;       // stores the working area so we don't have to keep asking
  VRect relevantTiles;  // Tracks relevant tiles as needed
  VRect relevantArea;   // Tracks relevant pixels as needed

  Applicator &applicator = *Applicator::Get();
  TileManager &tileMgr = *TileManager::Get();

  workarea = applicator.GetWorkArea();

  // Find all the relevant source tiles
  relevantArea = applicator.GetArea();
  if(mChunkPos.v != workarea.top)
    relevantArea.top = mChunkPos.v - applicator.GetFilterSize().v;
  if(mGoingRight) {
    if(mChunkPos.h != workarea.left)
      relevantArea.left = mChunkPos.h - applicator.GetFilterSize().h;
  } else {
    if(mChunkPos.h != workarea.right)
      relevantArea.right = mChunkPos.h + applicator.GetFilterSize().h;
  }

  relevantTiles = tileMgr.GetTouchingTiles(relevantArea);

  // Render each relevant tile onto the application texture
  for(tile.h = relevantTiles.left;tile.h < relevantTiles.right;tile.h++) {
    for(tile.v = relevantTiles.top;tile.v < relevantTiles.bottom;tile.v++) {
      // Render the source to the application texture
      tileMgr.RenderSource(tile, applicator.GetArea());
      // Check to see if we can retire the source
	  if(ShouldRetireTileSource(tile)) {
        tileMgr.RetireSource(tile);
	  }
    }
  }

  // Find all the relevant destination tiles
  relevantArea = applicator.GetWorkArea(); // KB - why are we calling GetWorkArea() again??
#ifdef _DEBUG
	  VRect workArea = relevantArea; // KB - DEBUG
#endif

  relevantArea.top = mChunkPos.v;
  if(mGoingRight)
    relevantArea.left = mChunkPos.h;
  else
    relevantArea.right = mChunkPos.h;

  mRelevantTiles = tileMgr.GetContainedTiles(relevantArea);

  relevantArea.left = mRelevantTiles.left * tileMgr.GetStandardTileSize().h;
  relevantArea.right = mRelevantTiles.right * tileMgr.GetStandardTileSize().h;
  relevantArea.top = mRelevantTiles.top * tileMgr.GetStandardTileSize().v;
  relevantArea.bottom = mRelevantTiles.bottom * tileMgr.GetStandardTileSize().v;

  // Intersect it with the filter region
  VRect filterarea = GetFilterRect();
#ifdef _DEBUG
	  VRect oldRel = relevantArea; // KB - DEBUG
	  VRect appArea = applicator.GetArea(); // KB - DEBUG
#endif

  relevantArea.left = max(relevantArea.left, filterarea.left);
  relevantArea.right = min(relevantArea.right, filterarea.right);
  relevantArea.top = max(relevantArea.top, filterarea.top);
  relevantArea.bottom = min(relevantArea.bottom, filterarea.bottom);

  if(relevantArea.left == relevantArea.right)
    throw "Applicator not big enough to render a single tile!";

//	  /// DEBUG - KB
#ifdef _DEBUG
	  static char myMsg[128];
	  sprintf(myMsg,"filterarea (%d,%d)-(%d,%d)\nworkArea (%d,%d)-(%d,%d)\nmRelevant Tiles (%d,%d)-(%d,%d) (x%d*%d)\nRelevant Tiles (%d,%d)-(%d,%d)\nRelevant Area (%d,%d)-(%d,%d)\n(was (%d,%d)-(%d,%d))\nApplicator Area (%d,%d)-(%d,%d)",
		  filterarea.left,		filterarea.top,		filterarea.right,		filterarea.bottom,
		  workArea.left,		workArea.top,		workArea.right,			workArea.bottom,
          mRelevantTiles.left,	mRelevantTiles.top,	mRelevantTiles.right,	mRelevantTiles.bottom,
		  tileMgr.GetStandardTileSize().h,	tileMgr.GetStandardTileSize().v,
		  relevantTiles.left,	relevantTiles.top,	relevantTiles.right,	relevantTiles.bottom,
		  relevantArea.left,	relevantArea.top,	relevantArea.right,		relevantArea.bottom,
		  oldRel.left,			oldRel.top,			oldRel.right,			oldRel.bottom,
		  appArea.left,			appArea.top,		appArea.right,			appArea.bottom);
	  MessageBox(0, myMsg, "DoChunkFilter", MB_OK);
#endif

  applicator.FilterTo(relevantArea, false);
#ifdef _DEBUG
  MessageBox(0,"FilterTo() complete","DoChunkFilter", false);
#endif
  //SwapBuffers(wglGetCurrentDC()); // redundant -- KB
  //SwapBuffers(wglGetCurrentDC());
}

void GPUFilter::DoChunkDataRead()
{
  int channels;
  VRect beft;
#ifdef _DEBUG
  MessageBox(0,"DoChunkReadData starting","DoChunkDataRead", false);
#endif
  VPoint bigimage = GetImageSize();
  VPoint appoffset = {Applicator::Get()->GetArea().top, Applicator::Get()->GetArea().left};

  GLenum format;

	channels = gFilterRecord->planes;

	GLenum type = (gFilterRecord->depth == 8) ? GL_UNSIGNED_BYTE : ((gFilterRecord->depth == 16) ? GL_SHORT : GL_FLOAT);
	GLint nPixelBytes = (gFilterRecord->depth == 8) ? 1 : ((gFilterRecord->depth == 16) ? 2 : 4);

  gFilterRecord->outLoPlane = 0;
#ifdef _DEBUG
  char cMsg[64];
  sprintf(cMsg,"image has %d channels",channels);
  MessageBox(0,cMsg,"DoChunkDataRead", false);
#endif

  switch(channels)
  {
  case 1:
    format = GL_RED;
    gFilterRecord->outHiPlane = 0;
    break;
  case 2:
    format = GL_LUMINANCE_ALPHA;
    gFilterRecord->outHiPlane = 1;
    break;
  case 3:
    format = GL_RGB;
    gFilterRecord->outHiPlane = 2;
    break;
  case 4:
  default: // for now we'll handle all unknown cases as RGBA - KB
    format = GL_RGBA;
    gFilterRecord->outHiPlane = 3;
    channels = 4; // just in case
    break;
  }

  beft.bottom = mRelevantTiles.bottom * TileManager::Get()->GetStandardTileSize().v;
  beft.left = mRelevantTiles.left * TileManager::Get()->GetStandardTileSize().h;
  beft.right = mRelevantTiles.right * TileManager::Get()->GetStandardTileSize().h;
  beft.top = mRelevantTiles.top * TileManager::Get()->GetStandardTileSize().v;

  if(beft.right > bigimage.h)
    beft.right = bigimage.h;
  if(beft.bottom > bigimage.v)
    beft.bottom = bigimage.v;
  SetOutRect(beft);

  // Update photoshop's state
	gFilterRecord->advanceState();

  // KB - Adjust this for pixels > one byte!!!
  // Read back from the destination buffer
	GLint storeSize = (type==GL_UNSIGNED_BYTE)?1:((type==GL_SHORT)?2:4);
	glPixelStorei(GL_PACK_ALIGNMENT, storeSize);
  if(gFilterRecord->outRowBytes != ((beft.right - beft.left)*channels*nPixelBytes))
    glPixelStorei(GL_PACK_ROW_LENGTH, gFilterRecord->outRowBytes/channels);
#ifdef _DEBUG
  MessageBox(0,"glReadPixels begin","DoChunkDataRead", false);
#endif

  glReadPixels(
    beft.left - appoffset.h,
    beft.top - appoffset.v,
    beft.right - beft.left,
    beft.bottom - beft.top,
    format,
    type,
    gFilterRecord->outData);
#ifdef _DEBUG
  MessageBox(0,"glPixelStorei begin","DoChunkDataRead", false);
#endif
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
#ifdef _DEBUG
  MessageBox(0,"DoChunkReadData complete","DoChunkDataRead", false);
#endif
}

bool GPUFilter::NextChunkPos(VPoint &chunk)
{
  // The idea here is to sweep back and forth across the image, filtering it
  // as you go. This provides better memory management than going left-to-right
  // across each row, because it uses the least-recently-used idea better.

  ASSERT(TileManager::Get())
  ASSERT(Applicator::Get())

  bool right = mGoingRight;
  TileManager &tileMgr = *TileManager::Get();
  Applicator &applicator = *Applicator::Get();
  VRect workarea = tileMgr.GetContainedTiles(applicator.GetWorkArea());

  workarea.left *= tileMgr.GetStandardTileSize().h;
  workarea.right *= tileMgr.GetStandardTileSize().h;
  workarea.top *= tileMgr.GetStandardTileSize().v;
  workarea.bottom *= tileMgr.GetStandardTileSize().v;

  if(mChunkPos.h == -1 || mChunkPos.v == -1)
  {
    chunk.v = mImageRect.top - mImageRect.top % tileMgr.GetStandardTileSize().v;
    chunk.h = mImageRect.left - mImageRect.left % tileMgr.GetStandardTileSize().h;
    right = true;
  }
  else if(right)
  {
    if(workarea.right >= mImageRect.right)
    {
      right = false;

      if(workarea.bottom >= mImageRect.bottom)
        chunk.h = chunk.v = -1;
      else
      {
        chunk.v = workarea.bottom;
        chunk.h = workarea.right;
      }
    }
    else
    {
      chunk.h = workarea.right;
      chunk.v = mChunkPos.v;
    }
  }
  else
  {
    if(workarea.left <= mImageRect.left)
    {
      right = true;
      if(workarea.bottom >= mImageRect.bottom)
        chunk.h = chunk.v = -1;
      else
      {
        chunk.v = workarea.bottom;
        chunk.h = workarea.left;
      }
    }
    else
    {
      chunk.h = workarea.left;
      chunk.v = mChunkPos.v;
    }
  }

  return right;
}

bool GPUFilter::ShouldRetireTileSource(VPoint &tilepos)
{
  ASSERT(Applicator::Get())
  ASSERT(TileManager::Get())

  Applicator &applicator = *Applicator::Get();
  TileManager &tileMgr = *TileManager::Get();
  VRect workarea = applicator.GetWorkArea();

  if(mNextChunk.h == -1 || mNextChunk.v == -1) return true;

  workarea.top = mChunkPos.v;
  if(mNextGoingRight)
    workarea.left = mChunkPos.h;
  else
    workarea.right = mChunkPos.h;

  workarea = tileMgr.GetContainedTiles(workarea);

  // shift up 1 tile and squish in the sides
  workarea.bottom--;
  workarea.top--;
  workarea.right--;
  workarea.left++;

  if(tilepos.v < workarea.top) return true;

  if(mNextGoingRight)
  {
    if(tilepos.h >= workarea.right) return false;
    else return (tilepos.v < workarea.bottom);
  }
  else
  {
    if(tilepos.h < workarea.left) return false;
    else return (tilepos.v < workarea.bottom);
  }
}