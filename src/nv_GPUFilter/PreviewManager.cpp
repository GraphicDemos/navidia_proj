/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/PreviewManager.cpp#1 $

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
	Implementation of PvwMgr - calls Applicator to do actual work

******************************************************************************/

#include <glh/glh_extensions.h>
#include "PreviewManager.h"
#include "GPUFilter.h"
#include "shared/TileManager.h"
#include "GPUFilterData.h"
#include "GLManager.h"
#include "MyGLShader.h"
#include "shared/ErrorHandling.h"
#include <shared/pbuffer.h>
#include <FilterBigDocument.h>
#include <PIColorSpaceSuite.h>
#include <PIDLLInstance.h>
PreviewManager *PreviewManager::me = 0;

INT_PTR CALLBACK GlobalGUIProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  ASSERT(GPUFilterData::Get())
  ASSERT(PreviewManager::Get())

  INT_PTR ret = GPUFilterData::Get()->HandlePreviewMessages(hwnd, msg, wparam, lparam);

  switch(msg)
  {
  case WM_CHARTOITEM:
    if(ret < 0) return ret;
    break;
  case WM_COMPAREITEM:
    return ret;
    break;
  case WM_CTLCOLORBTN:
  case WM_CTLCOLORDLG:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORLISTBOX:
  case WM_CTLCOLORSCROLLBAR:
  case WM_CTLCOLORSTATIC:
    if(ret) return ret;
    break;
  case WM_INITDIALOG:
    PreviewManager::Get()->HandlePreviewMessages(hwnd, msg, wparam, lparam);
    return ret;
    break;
  case WM_QUERYDRAGICON:
    if(ret) return ret;
    break;
  case WM_VKEYTOITEM:
    if(ret == -2) return ret;
    break;
  default:
    if(ret) return ret;
    break;
  }

  return PreviewManager::Get()->HandlePreviewMessages(hwnd, msg, wparam, lparam);
}

PreviewManager::PreviewManager()
  : mBlitFS(GLShader::Fragment, "Blit FS"),
    mBlitVS(GLShader::Vertex, "Blit VS"), mBlit("Blit"),
    mGridTex(0)
{
  if(me)
    throw "You've already got a preview manager!";
  me = this;

  // Set up the initial area
  mArea.bottom = 0;
  mArea.top = 0;
  mArea.left = 0;
  mArea.right = 0;

  // load an appropriate fragment shader for blitting, depending on the PS image mode
  switch(gFilterRecord->imageMode)
  {
  case plugInModeBitmap:
  case plugInModeIndexedColor:
    throw "Unsupported format";
    break;
  case plugInModeGrayScale:
  case plugInModeGray16:
    mBlitFS.AddCodeFromFile("GrayBlit_fs.glsl",GrayBlit_fs_src);
    break;
  case plugInModeRGBColor:
  case plugInModeRGB48:
  case plugInModeRGB96:
    mBlitFS.AddCodeFromFile("RGBBlit_fs.glsl",RGBBlit_fs_src);
    break;
  case plugInModeCMYKColor:
  case plugInModeCMYK64:
    mBlitFS.AddCodeFromFile("CMYKBlit_fs.glsl",CMYKBlit_fs_src);
    break;
  case plugInModeHSLColor:
    mBlitFS.AddCodeFromFile("HSLBlit_fs.glsl" /*,HSLBlit_fs_src */);
    break;
  case plugInModeHSBColor:
    mBlitFS.AddCodeFromFile("HSBBlit_fs.glsl" /* ,HSBBlit_fs_src */);
    break;
  case plugInModeMultichannel:
  case plugInModeDeepMultichannel:
    mBlitFS.AddCodeFromFile("Standard_fs.glsl",standard_fs_src);
    break;
  case plugInModeDuotone:
  case plugInModeDuotone16:
    mBlitFS.AddCodeFromFile("Standard_fs.glsl",standard_fs_src);
    break;
  case plugInModeLabColor:
  case plugInModeLab48:
    mBlitFS.AddCodeFromFile("LABBlit_fs.glsl",LABBlit_fs_src);
    break;
  }

  // regardless of mode, load the "standard" (screen quad) vertex shader
  mBlitVS.AddCodeFromFile("Standard_vs.glsl",standard_vs_src);

  mTexPos.h = 0;
  mTexPos.v = 0;

  mTileArea.h = 0;
  mTileArea.v = 0;

  mTextures = 0;
  mDirty = 0;
  mColShifts = 0;
  mRowShifts = 0;

  mPreviewInitialized = false;

  for(int x = 0;x < 8;x++)
    mKTextures[x] = 0;
}

PreviewManager::~PreviewManager()
{
  me = 0;
}

int PreviewManager::ShowPreview()
{
  ASSERT(GPUFilterData::Get())

  return DialogBox(
    GetDLLInstance((SPPluginRef)gFilterRecord->plugInRef),
    MAKEINTRESOURCE(GPUFilterData::Get()->GetGUIResID()),
    GetActiveWindow(),
    GlobalGUIProc);
}

void PreviewManager::Pan(VPoint &pan)
{
  ASSERT(TileManager::Get())

  VPoint bigimage = GetImageSize();
  TileManager &tiler = *TileManager::Get();

  // Do the pan
  mArea.left += pan.h;
  mArea.right += pan.h;
  mArea.top += pan.v;
  mArea.bottom += pan.v;

  // Make sure you stay in bounds
  if(mArea.left < 0)
  {
    mArea.right -= mArea.left;
    mArea.left = 0;
  }
  if(mArea.top < 0)
  {
    mArea.bottom -= mArea.top;
    mArea.top = 0;
  }
  if(mArea.right > bigimage.h)
  {
    mArea.left -= mArea.right - bigimage.h;
    mArea.right = bigimage.h;
  }
  if(mArea.bottom > bigimage.v)
  {
    mArea.top -= mArea.bottom - bigimage.v;
    mArea.bottom = bigimage.v;
  }

  Draw();
}

void PreviewManager::Resize(VPoint &size)
{
  VPoint bigimage = GetImageSize();

  // Resize, making sure you don't get too big
  mArea.right  = mArea.left + min(min(size.h, MaxViewArea.h), bigimage.h);
  mArea.bottom = mArea.top  + min(min(size.v, MaxViewArea.v), bigimage.v);

  // Make sure you stay in bounds
  if(mArea.right > bigimage.h)
  {
    mArea.left -= mArea.right - bigimage.h;
    mArea.right = bigimage.h;
  }
  if(mArea.bottom > bigimage.v)
  {
    mArea.top -= mArea.bottom - bigimage.v;
    mArea.bottom = bigimage.v;
  }

  GLASSERT()

  // Tell OpenGL about it
  glViewport(0, 0, size.h, size.v);

  GLASSERT()

  // The window should send a WM_PAINT message on a resize,
  // making an Draw() call here unnecessary
}

void PreviewManager::Draw()
{
  ASSERT(TileManager::Get())
  ASSERT(GPUFilterData::Get())

  TileManager &tileMgr = *TileManager::Get();
  Applicator &applicator = *Applicator::Get();
  VPoint tile;
  bool didone = false;
  GLint param;

  // See if the view area has escape the pre-filtered area
  if(mArea.left < mTexPos.h * tileMgr.GetStandardTileSize().h)
  {
    // Go one tile to the left
    mTexPos.h--;
    // All the row shifts should now be to the right
    for(int i = 0;i < mTileArea.v;i++)
    {
      if(mRowShifts[i]) // If it's already to the right, mark the left tile as dirty
        mDirty[(mTexPos.h + mTileArea.h) % mTileArea.h][i] = true;
      mRowShifts[i] = true;   // true = right
    }
  }

  if(mArea.top < mTexPos.v * tileMgr.GetStandardTileSize().v)
  {
    // Go one tile up
    mTexPos.v--;
    // All the column shifts should now be down
    for(int i = 0;i < mTileArea.h;i++)
    {
      if(mColShifts[i]) // If it's already down, mark the top tile as dirty
        mDirty[i][(mTexPos.v + mTileArea.h) % mTileArea.v] = true;
      mColShifts[i] = true;   // true = down
    }
  }

  if(mArea.right > (mTexPos.h + mTileArea.h - 1) * tileMgr.GetStandardTileSize().h)
  {
    // Go one tile to the right
    mTexPos.h++;
    // All the row shifts should now be to the left
    for(int i = 0;i < mTileArea.v;i++)
    {
      if(!mRowShifts[i]) // If it's already to the left, mark the right tile as dirty
        mDirty[(mTexPos.h + mTileArea.h - 2) % mTileArea.h][i] = true;
      mRowShifts[i] = false;   // false = left
    }
  }

  if(mArea.bottom > (mTexPos.v + mTileArea.v - 1) * tileMgr.GetStandardTileSize().v)
  {
    // Go one tile down
    mTexPos.v++;
    // All the column shifts should now be up
    for(int i = 0;i < mTileArea.h;i++)
    {
      if(!mColShifts[i]) // If it's already down, mark the top tile as dirty
        mDirty[i][(mTexPos.v + mTileArea.h - 2) % mTileArea.v] = true;
      mColShifts[i] = false;   // false = up
    }
  }

  // Nice amortized updating algorithm here... updates occur just beyond the viewable area
  VRect inner;
  VRect outer =
  {
    mTexPos.v * tileMgr.GetStandardTileSize().v,
    mTexPos.h * tileMgr.GetStandardTileSize().h,
    (mTexPos.v + mTileArea.v - 1) * tileMgr.GetStandardTileSize().v,
    (mTexPos.h + mTileArea.h - 1) * tileMgr.GetStandardTileSize().h
  };

  VPoint blend =
  {
    ((outer.bottom - outer.top) - (mArea.bottom - mArea.top)) / 2,
    ((outer.right - outer.left) - (mArea.right - mArea.left)) / 2
  };

  memcpy(&inner, &outer, sizeof(VRect));
  inner.top += blend.v;
  inner.bottom -= blend.v;
  inner.left += blend.h;
  inner.right -= blend.h;

  if(mArea.left < inner.left)
  {
    int thresh = ((inner.left - mArea.left - 1) * mTileArea.h) / blend.h + 1;

    for(int i = 0;i < thresh;i++)
    {
      if(mRowShifts[i])
        mDirty[(mTexPos.h + mTileArea.h - 1) % mTileArea.h][i] = true;
      mRowShifts[i] = false;
    }
  }

  if(mArea.top < inner.top)
  {
    int thresh = ((inner.top - mArea.top - 1) * mTileArea.v) / blend.v + 1;

    for(int i = 0;i < thresh;i++)
    {
      if(mColShifts[i])
        mDirty[i][(mTexPos.v + mTileArea.v - 1) % mTileArea.v] = true;
      mColShifts[i] = false;
    }
  }

  if(mArea.right > inner.right)
  {
    int thresh = ((mArea.right - inner.right - 1) * mTileArea.h) / blend.h + 1;

    for(int i = 0;i < thresh;i++)
    {
      if(!mRowShifts[i])
        mDirty[(mTexPos.h + mTileArea.h - 1) % mTileArea.h][i] = true;
      mRowShifts[i] = true;
    }
  }

  if(mArea.bottom > inner.bottom)
  {
    int thresh = ((mArea.bottom - inner.bottom - 1) * mTileArea.v) / blend.v + 1;

    for(int i = 0;i < thresh;i++)
    {
      if(!mColShifts[i])
        mDirty[i][(mTexPos.v + mTileArea.v - 1) % mTileArea.v] = true;
      mColShifts[i] = true;
    }
  }

  VRect visibletiles = tileMgr.GetTouchingTiles(mArea);

  GLASSERT()

  Applicator::Get()->Activate();
  for(tile.h = visibletiles.left;tile.h < visibletiles.right;tile.h++)
    for(tile.v = visibletiles.top;tile.v < visibletiles.bottom;tile.v++)
    {
      VPoint tex = {tile.v % mTileArea.v, tile.h % mTileArea.h};
      if(mDirty[tex.h][tex.v])
        ApplyFilter(tex);
    }

  for(tile.h = 0;tile.h < mTileArea.h;tile.h++)
    for(tile.v = 0;tile.v < mTileArea.v;tile.v++)
      if(mDirty[tile.h][tile.v])
      {
        // Just render these one at a time...
        ApplyFilter(tile);
        tile.h = mTileArea.h;
        tile.v = mTileArea.v;
      }

  Applicator::Get()->Deactivate();

  glClear(GL_COLOR_BUFFER_BIT);

  // Set the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(
    mArea.left,
    mArea.right,
    mArea.bottom,
    mArea.top,
    -1, 1);

  mBlit.Use();
  glActiveTextureARB(GL_TEXTURE0_ARB);

  GLASSERT()

  // !!!! Might want to test for Max. texture units!!!!!!
  char name[] = "k[#]";
  for(int i = 0;i < 8;i++)
  {
    glActiveTextureARB(GL_TEXTURE1_ARB + i);
    glBindTexture(GL_TEXTURE_3D, mKTextures[i]);
    name[2] = i + '0';    // Don't do more than 10!
    param = mBlit.GetUniformID(name);
    if(param != -1)
      glUniform1iARB(param, 1 + i);
  }

  // Render each visible tile to the frame buffer
  for(tile.h = visibletiles.left;tile.h < visibletiles.right;tile.h++)
    for(tile.v = visibletiles.top;tile.v < visibletiles.bottom;tile.v++)
    {
      VRect drawme = tileMgr.GetTileArea(tile);

      if(!mTextures[tile.h % mTileArea.h][tile.v % mTileArea.v])
        continue;

      // Bind the texture and set the parameters
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(
        GL_TEXTURE_RECTANGLE_NV,
        mTextures[tile.h % mTileArea.h][tile.v % mTileArea.v]);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, mGridTex);

      GLASSERT()

      param = mBlit.GetUniformID("grid");
      if(param != -1)
        glUniform1iARB(param, 1);
      param = mBlit.GetUniformID("chunk");
      if(param != -1)
        glUniform1iARB(param, 0);
      param = mBlit.GetUniformID("chunkPos");
      if(param != -1)
        glUniform2fARB(param, (float)drawme.left, (float)drawme.top);
      param = mBlit.GetUniformID("hasAlpha");
      if(param != -1)
        glUniform1iARB(param, gFilterRecord->outTransparencyMask);

      // Draw
      glRecti(drawme.left, drawme.top, drawme.right, drawme.bottom);

      GLASSERT()
    }

  SwapBuffers(GLManager::Get()->GetRootDC());

  GLASSERT()
}

void PreviewManager::ParamsChanging()
{
  ASSERT(TileManager::Get())
  ASSERT(Applicator::Get())

  // mark all visible tiles as dirty (invisible ones are dirtied on ParamsChanged())
  VRect visible = TileManager::Get()->GetTouchingTiles(mArea);
  VPoint tile;

  for(tile.h = visible.left;tile.h < visible.right;tile.h++)
    for(tile.v = visible.top;tile.v < visible.bottom;tile.v++)
      mDirty[tile.h % mTileArea.h][tile.v % mTileArea.v] = true;

  // The filter distance may have changed, so reset the work area
  Applicator::Get()->ResetWorkArea();
}

void PreviewManager::ParamsChanged()
{
  ASSERT(Applicator::Get())

  // mark all tiles as dirty
  for(int i = 0;i < mTileArea.h;i++)
    memset(mDirty[i], true, sizeof(bool) * mTileArea.v);

  // The filter distance may have changed, so reset the work area
  Applicator::Get()->ResetWorkArea();
}

void PreviewManager::InitPreview()
{
  ASSERT(TileManager::Get())

  VPoint appsize;
  TileManager &tiler = *TileManager::Get();
  VPoint bigimage = GetImageSize();
  PSColorSpaceSuite1 *colorspace;
  gFilterRecord->sSPBasic->AcquireSuite(
    (const char *)kPSColorSpaceSuite,
    kPSColorSpaceSuiteVersion1,
    (const void **)&colorspace);
  Color8 CMY[32][32][32];

  if(!mBlitFS.Validate())
    throw mBlitFS.GetInfo();
  if(!mBlitVS.Validate())
    throw mBlitVS.GetInfo();

  mBlit.AddShader(mBlitFS);
  mBlit.AddShader(mBlitVS);

  if(!mBlit.Validate())
    throw mBlit.GetInfo();

  glGenTextures(8, mKTextures);
  for(int texindex = 0;texindex < 8;texindex++)
  {
    for(int i = 0;i < 32;i++)
      for(int j = 0;j < 32;j++)
        for(int k = 0;k < 32;k++)
        {
          CMY[k][j][i][0] = (i * 255) / 31;
          CMY[k][j][i][1] = (j * 255) / 31;
          CMY[k][j][i][2] = (k * 255) / 31;
          CMY[k][j][i][3] = (texindex * 255) / 7;
        }

    // Initialize the CMYK conversion textures
    glBindTexture(GL_TEXTURE_3D, mKTextures[texindex]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    colorspace->Convert8(plugIncolorServicesCMYKSpace, plugIncolorServicesRGBSpace, (Color8 *)CMY, 32767);
    colorspace->Convert8(plugIncolorServicesCMYKSpace, plugIncolorServicesRGBSpace, (Color8 *)CMY + 32767, 1);
    glTexImage3DEXT(
      GL_TEXTURE_3D,
      0,
      GL_RGBA8,
      32, 32, 32,
      0,
      GL_BGRA,
      GL_UNSIGNED_INT_8_8_8_8_REV,
      CMY);

    GLASSERT()
  }

  Color8 grid[4] = 
  {
    {255, 255, 255, 255}, {204, 204, 204, 255},
    {204, 204, 204, 255}, {255, 255, 255, 255}
  };

  GLASSERT()
  glGenTextures(1, &mGridTex);
  glBindTexture(GL_TEXTURE_2D, mGridTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, grid);

  GLASSERT()

  appsize.h = tiler.GetStandardTileSize().h + 2*(MaxFilterSize + 1);
  appsize.v = tiler.GetStandardTileSize().v + 2*(MaxFilterSize + 1);

  ASSERT(!Applicator::Get())
  new Applicator(appsize);
  ASSERT(Applicator::Get())

  // Calculate the maximum visible tile area
  mTileArea.h = (MaxViewArea.h - 2) / tiler.GetStandardTileSize().h + 2;
  mTileArea.v = (MaxViewArea.v - 2) / tiler.GetStandardTileSize().v + 2;

  // Fluff (one tile on each edge, and then one more for the "shifting")
  mTileArea.h += 1;
  mTileArea.v += 1;

  // If there just aren't that many tiles, crop mTileArea to fit the image
  mTileArea.h = min(mTileArea.h, tiler.GetNumTiles().h + 1);
  mTileArea.v = min(mTileArea.h, tiler.GetNumTiles().v + 1);
  mTextures = new GLuint *[mTileArea.h];
  ASSERT(mTextures)
  mDirty = new bool *[mTileArea.h];
  ASSERT(mDirty)

  for(int i = 0;i < mTileArea.h;i++)
  {
    mTextures[i] = new GLuint[mTileArea.v];
    ASSERT(mTextures[i])
    memset(mTextures[i], 0, sizeof(GLuint) * mTileArea.v);
    mDirty[i] = new bool[mTileArea.v];
    ASSERT(mDirty[i])
    memset(mDirty[i], true, sizeof(bool) * mTileArea.v);
  }

  mColShifts = new bool[mTileArea.h];
  ASSERT(mColShifts)
  memset(mColShifts, true, sizeof(bool) * mTileArea.h);
  mRowShifts = new bool[mTileArea.v];
  ASSERT(mRowShifts)
  memset(mRowShifts, true, sizeof(bool) * mTileArea.v);

  mPreviewInitialized = true;
}

void PreviewManager::CleanUpPreview()
{
  mPreviewInitialized = false;

  glDeleteTextures(1, &mGridTex);
  glDeleteTextures(8, mKTextures);

  for(int i = 0;i < mTileArea.h;i++)
  {
    for(int j = 0;j < mTileArea.v;j++)
      if(mTextures[i][j])
        glDeleteTextures(1, &mTextures[i][j]);

    delete [] mTextures[i];
    mTextures[i] = 0;
    delete [] mDirty[i];
    mDirty[i] = 0;
  }
  delete [] mTextures;
  mTextures = 0;
  delete [] mColShifts;
  mColShifts = 0;
  delete [] mRowShifts;
  mRowShifts = 0;
  delete [] mDirty;
  mDirty = 0;

  if(Applicator::Get())
    delete Applicator::Get();

  mBlit.RemoveShader(mBlitFS);
  mBlit.RemoveShader(mBlitVS);
  mBlit.Clear();
  mBlitFS.ClearCode();
  mBlitVS.ClearCode();

  GLManager::Get()->Update(GetDLLInstance((SPPluginRef)gFilterRecord->plugInRef));
}

INT_PTR PreviewManager::HandlePreviewMessages(
  HWND hwnd,
  UINT msg,
  WPARAM wparam,
  LPARAM lparam)
{
  GPUFilterData &data = *GPUFilterData::Get();

  try
  {
    switch(msg)
    {
    case WM_INITDIALOG:
      {
        ASSERT(GLManager::Get())

        VPoint size;

        // Grab the preview window...
        if(data.GetPreviewResID())
        {
          RECT previewRect;
          HWND preview;

          preview = GetDlgItem(hwnd, data.GetPreviewResID());
          GLManager::Get()->Update(preview, false);

          GetClientRect(preview, &previewRect);
          size.h = previewRect.right - previewRect.left + 1;
          size.v = previewRect.bottom - previewRect.top + 1;
        }
        else
          GLManager::Get()->Update(hwnd, false);

        // ... and initialize it
        InitPreview();

        if(data.GetPreviewResID())
          Resize(size);   // Sizes up the preview window, and draws it
        return true;
      }

    case WM_DESTROY:
      // If the preview is still initialized, clean it up
      if(mPreviewInitialized)
        CleanUpPreview();
      return true;

    case WM_PAINT:
      Draw();
      break;

    // The docs for dialog boxes point out that these messages are handled
    // differently than the rest, and the easiest way to get it done right
    // is to just pass them off to DefWindowProc
    case WM_CHARTOITEM:
    case WM_COMPAREITEM:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
    case WM_QUERYDRAGICON:
    case WM_VKEYTOITEM:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
  }
  catch(const char *err)
  {
    MessageBox(0, err, "Error", MB_OK);
    PostMessage(hwnd, WM_CLOSE, 0, 0);
  }

  return false;
}

void PreviewManager::ApplyFilter(VPoint &tile)
{
  ASSERT(TileManager::Get())
  ASSERT(Applicator::Get())

  VPoint target;
  VPoint base;  // All part of the math
  TileManager &tiler = *TileManager::Get();
  Applicator &app = *Applicator::Get();

  // calculate the absolute position of the tile based on the given relative offset
  target.h = mTexPos.h - (mRowShifts[tile.v] ? 0 : 1);
  target.v = mTexPos.v - (mColShifts[tile.h] ? 0 : 1);
  base.h = target.h - target.h % mTileArea.h;
  base.v = target.v - target.v % mTileArea.v;
  if(tile.h < target.h % mTileArea.h)
    base.h += mTileArea.h;
  if(tile.v < target.v % mTileArea.v)
    base.v += mTileArea.v;
  target.h = base.h + tile.h;
  target.v = base.v + tile.v;

  // If the target is out of bounds, just return
  if(
    target.h < 0 ||
    target.v < 0 ||
    target.h >= tiler.GetNumTiles().h ||
    target.v >= tiler.GetNumTiles().v)
  {
    mDirty[tile.h][tile.v] = false;
    return;
  }

  base.h = target.h * tiler.GetStandardTileSize().h;
  base.v = target.v * tiler.GetStandardTileSize().v;

  app.Target(base);

  // Render source tiles to applicator
  VPoint region;
  for(
    region.h = max(0, target.h - 1);
    region.h <= min(tiler.GetNumTiles().h - 1, target.h + 1);
    region.h++)
    for(
      region.v = max(0, target.v - 1);
      region.v <= min(tiler.GetNumTiles().v - 1, target.v + 1);
      region.v++)
      tiler.RenderSource(region, app.GetArea());

  // Do the filter
  app.FilterTo(tiler.GetTileArea(target), false);


  GLASSERT()

  // Extract the results
  glActiveTextureARB(GL_TEXTURE0_ARB);
  if(!mTextures[tile.h][tile.v])
  {
    glGenTextures(1, &mTextures[tile.h][tile.v]);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, mTextures[tile.h][tile.v]);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  else
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, mTextures[tile.h][tile.v]);

  GLASSERT()

  glCopyTexImage2D(
    GL_TEXTURE_RECTANGLE_NV,
    0,
    GL_RGBA8,
    tiler.GetTilePos(target).h - app.GetArea().left,
    tiler.GetTilePos(target).v - app.GetArea().top,
    tiler.GetTileSize(target).h,
    tiler.GetTileSize(target).v,
    0);

  GLASSERT()

  // Mark this tile as now clean
  mDirty[tile.h][tile.v] = false;
}