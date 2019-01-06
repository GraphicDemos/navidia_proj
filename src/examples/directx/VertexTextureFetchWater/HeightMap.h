#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>


class HeightMap
{
public:
  HeightMap(const char* heightFilename, const D3DXVECTOR3& scale, const D3DXVECTOR3& offset);
  ~HeightMap(void);

  void  createIndexBuffer(LPDIRECT3DDEVICE9 device);
  void  createVertexBuffer(LPDIRECT3DDEVICE9 device);
  void  createHeightTexture(LPDIRECT3DDEVICE9 device, LPDIRECT3DTEXTURE9& texture);
  float height(float x, float z, bool useWorldSpace = true, bool useNormalizedSpace = false) const;
  void  loadColorTexture(LPDIRECT3DDEVICE9 device, const char* textureFilename);
  void  render(LPDIRECT3DDEVICE9 device, bool disableStateSetup = false);
  void  setTexture(LPDIRECT3DTEXTURE9 texture);

protected:
  std::vector<float>      _data;
  int                     _width;               // Width of heightmap
  int                     _depth;               // Depth of heightmap
  D3DXVECTOR3             _scale;
  D3DXVECTOR3             _offset;

  LPDIRECT3DVERTEXBUFFER9 _vertexBuffer;
  LPDIRECT3DINDEXBUFFER9  _indexBuffer;
  LPDIRECT3DTEXTURE9      _texture;

  void computePosition(int x, int z, D3DXVECTOR3& position);
  bool load(const char* filename);
  void resize(int width, int depth);

  inline int    computeIndex(int x, int z) const;
  inline float  operator ()(int x, int z) const;
  inline float& operator ()(int x, int z);
  inline float  operator ()(int x, int z, bool clamp) const;
};



////////////////////////////////////////////////////////////////////////////////
//
//                            Inline methods
//
////////////////////////////////////////////////////////////////////////////////

int HeightMap::computeIndex(int x, int z) const
{
  assert(x >= 0);
  assert(x < _width);
  assert(z >= 0);
  assert(z < _depth);

  return(z * _width + x);
}

float HeightMap::operator ()(int x, int z) const
{
  assert(x >= 0);
  assert(x < _width);
  assert(z >= 0);
  assert(z < _depth);

  int index;

  index = computeIndex(x, z);
  assert(index < (int)_data.size());

  return(_data[index]);
}

float& HeightMap::operator ()(int x, int z)
{
  assert(x >= 0);
  assert(x < _width);
  assert(z >= 0);
  assert(z < _depth);

  int index;

  index = computeIndex(x, z);
  assert(index < (int)_data.size());

  return(_data[index]);
}

float HeightMap::operator ()(int x, int z, bool clamp) const
{
  if(clamp)
    {
      if(x < 0)
        {
          x = 0;
        }
      else if(x > (_width - 1))
        {
          x = _width - 1;
        }

      if(z < 0)
        {
          z = 0;
        }
      else if(z > (_depth - 1))
        {
          z = _depth - 1;
        }
    }

  return((*this)(x, z));
}


#endif
