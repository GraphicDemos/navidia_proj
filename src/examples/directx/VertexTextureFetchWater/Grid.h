#ifndef GRID_H
#define GRID_H

#include <d3d9.h>
#include <vector>

struct IDirect3DDevice9;
struct IDirect3DVertexBuffer;
struct IDirect3DIndexBuffer;
struct D3DXVECTOR3;

class HeightMap;

class Grid
{
public:
	Grid(void);
  Grid(int width, int height);

	void clear(float value);
  void copy(const Grid& grid);
  void createVertexAndIndexBuffers(IDirect3DDevice9* device, IDirect3DVertexBuffer9*& vertexBuffer, IDirect3DIndexBuffer9*& indexBuffer,
                                   const D3DXVECTOR3& scale, const D3DXVECTOR3& offset, HeightMap* heightMap);
	void resize(int width, int height);

  // Accessor methods
  int height(void) const;
  int size(void) const;
  int width(void) const;
  int primitiveCount(void) const;

  // Operators
	float& operator ()(int x, int y);
	float  operator ()(int x, int y) const;
  void   operator += (float value);
  void   operator *= (float value);

protected:
	int                _width;
	int                _height;
	std::vector<float> _data;

  float smoothStep(float t0, float f0, float t1, float f1, float t);
};


#endif