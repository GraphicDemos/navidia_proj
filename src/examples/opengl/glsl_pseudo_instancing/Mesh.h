#ifndef MESH_H
#define MESH_H

#include "Vertex.h"

#include <vector>

class Mesh
{
public:
  Mesh(void);
  ~Mesh(void);

  std::vector<Vertex>         mVertices;
  std::vector<unsigned short> mIndices;
  unsigned int                mVerticesVBO;
  unsigned int                mIndicesVBO;
  unsigned int                mTexture0;

  void render(void) const;
  void setState(void) const;
  int  triangleCount(void) const;
  void unsetState(void) const;
};

#endif
