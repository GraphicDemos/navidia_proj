
#if defined(WIN32)
#  include <windows.h>
#endif


#include "Mesh.h"

#include <assert.h>
#include <glh/glh_extensions.h>

static const bool sUseVBO = true;

Mesh::Mesh(void)
{
  mVerticesVBO = 0;
  mIndicesVBO = 0;
  mTexture0 = 0;
}

Mesh::~Mesh(void)
{
  if(sUseVBO)
    {
      if(mVerticesVBO > 0)
        {
          glDeleteBuffersARB(1, &mVerticesVBO);
        }

      if(mIndicesVBO > 0)
        {
          glDeleteBuffersARB(1, &mIndicesVBO);
        }
    }
}

void Mesh::render(void) const
{
  if(sUseVBO)
    {
      glDrawElements(GL_TRIANGLES, (GLsizei)mIndices.size(), GL_UNSIGNED_SHORT, 0);
    }
  else
    {
      glDrawElements(GL_TRIANGLES, (GLsizei)mIndices.size(), GL_UNSIGNED_SHORT, &(mIndices[0]));
    }
}

void Mesh::setState(void) const
{
  if(sUseVBO)
    {      
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVerticesVBO);
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mIndicesVBO);
      glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)0);
      glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)12);
      glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)24);
    }
  else
    {
      glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &(mVertices[0].mPosition));
      glNormalPointer(GL_FLOAT, sizeof(Vertex), &(mVertices[0].mNormal));
      glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &(mVertices[0].mTextureCoord0));
    }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

int Mesh::triangleCount(void) const
{
  assert((mIndices.size() % 3) == 0);
  
  return(int(mIndices.size()) / 3);
}

void Mesh::unsetState(void) const
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if(sUseVBO)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}
