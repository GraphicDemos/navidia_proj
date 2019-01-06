#ifndef _NVIO_MODEL
#define _NVIO_MODEL
#include <glh/glh_linear.h>

using namespace glh;

class NvIOModel {
public:
  bool ReadOBJ(char *filename);
  void Rescale();
  void Render();
  void DrawIDs();
  void DrawTriangle(unsigned int tri);
  vec3f GetTriangleNormal(unsigned int tri);

  void Draw();
  void BuildDList();
  void DrawBasis(float length);

  unsigned int m_nverts, m_nindices;

  unsigned int *m_indices;
  glh::vec3f *m_positions;
  glh::vec3f *m_normals;
  glh::vec3f *m_tangents;
  glh::vec3f *m_binormals;
  glh::vec3f *m_texcoords;

  GLuint m_dlist;
};

#endif
