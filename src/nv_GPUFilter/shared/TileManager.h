//
// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/inc/shared/TileManager.h#1 $
//

#ifndef _TILEMANAGER_
#define _TILEMANAGER_

#include <PIFilter.h>
#include <utility>
#include "GLProgram.h"

#define TILE(t) mTiles[t.h][t.v]

const int AllocateThisManyTextures = 2352; // Multiple of 3 and 4 makes happy
const int MaxColorChannels = 4;            // Only up to 4 color channels for now

class TileManager
{
  struct PSTile;

public:
  static TileManager *Get() {return me;}  // Singleton getter

  TileManager();
  ~TileManager();

	int GetBitsPerChannel() {return mBitsPerChannel;}
  int GetNumChannels() {return mNumChannels;}
  const VPoint &GetNumTiles() {return mNumTiles;}
  const VPoint &GetStandardTileSize() {return mTileSize;}
  GLProgram &GetComposer() {return mComposition;}

  VRect GetTouchingTiles(VRect &area); // Gets the range of tiles touching a given area of pixels
  VRect GetContainedTiles(VRect &area); // Gets the range of tiles wholly within the given area
  GLuint GetTexture(int handle); // Gets an OGL texture associated with the handle, or -1 if unallocated
  int CreateTexture(VPoint &tilepos, int channel); // Creates a tile texture. Needs a tile and color channel
  void DeleteTexture(int handle); // Gets rid of a texture
  void Use(int handle); // Tells the manager that a texture is being used

	VPoint GetTileSize(VPoint &t) {return TILE(t).size;}
  VPoint GetTilePos(VPoint &t) {VPoint p = {TILE(t).area.top, TILE(t).area.left}; return p;}
  VRect GetTileArea(VPoint &t) {return TILE(t).area;}

	void RenderSource(VPoint &tile, VRect &imageArea, VRect &targetArea = mZero); // Renders the tile
	void RetireSource(VPoint &tile);      // Disposes of the source textures

  // Renders a regions of tiles. The "Once" version destroys textures after it has used them
  void RenderRegion(VRect targetArea, bool invertY, VRect &imageArea = mZero);
  void RenderRegionOnce(VRect targetArea, bool invertY, VRect &imageArea = mZero);

protected:
private:
  static TileManager *me;

  struct PSTile
  {
    PSTile() {}
    PSTile(int x, int y, int width, int height)
    {
      memset(channels, 0, sizeof(int) * TileManager::Get()->GetNumChannels());
      area.right = (size.h = width) + (area.left = x);
      area.bottom = (size.v = height) + (area.top = y);
    }
    PSTile &operator =(PSTile &tile)
    {
      memcpy(this, &tile, sizeof(PSTile));
      return *this;
    }
    PSTile(PSTile &tile) {*this = tile;}

    int channels[MaxColorChannels]; // Channel textures (LUMINANCE8 or LUMINANCE16)
    VRect area;           // The position of the tile
    VPoint size;          // dimensions of the tile
  };

  PSTile **mTiles;        // The big array of tiles

  VPoint mTileSize;       // The size (in pixels) of a normal (uncropped) tile
  VPoint mNumTiles;       // The number of tiles in the image

	int mBitsPerChannel;	  // 8 or 16
  int mNumChannels;       // The number of color channels (will be 3, 4, or 5, most likely)

  // Texture Management stuff - recycles old tile textures
  // The LRU (Least Recently Used) table is an array of pairs of integers.
  // The first integer in each pair is the index of the next texture in the
  // list, and the second integer is the index of the previous. The beginning
  // and ends of the list are denoted with -1. There are two independent lists
  // stored in mLRUTable: The list of allocated textures (in least-recently-used
  // order) and the list of unallocated textures (in no particular order). The first
  // members of these lists are obtained through mMRUTexture and mUnallocated,
  // respectively. mMRUTexture points to the most recently used texture, and mLRUTexture
  // points to the least recently used texture. When there are no more unallocated slots,
  // attempts to create a new texture will overwrite the least recently used texture.
  typedef std::pair<int,int> LRUData;

  short *mMagic;          // Array of magic numbers to identify handles
  GLuint *mTextures;      // Array of tile-sized textures
  LRUData *mLRUTable;     // The Least Recently Used (and unallocated) table

  int mMRUTexture;        // The most recently used texture in mTextures
  int mLRUTexture;        // The least recently used texture in mTextures
  int mUnallocated;       // The first unallocated texture in mTextures
  int mNumTextures;       // The number of textures in the manager

  // The shaders that blit from tile to applicator
  GLShader mCompositionVS;
  GLShader mCompositionFS;
  GLProgram mComposition;

  WORD *mBSTexData;       // A useless array used only to appease glTexImage2D when necessary

  static VRect mZero;
};

#endif
