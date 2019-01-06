
#include "SphereMesh.h"

#include <assert.h>
#include <glh/glh_extensions.h>
#include <math.h>

static const float PI = 3.14159265358979f;

SphereMesh::SphereMesh(int rows, int columns)
{
  assert(rows > 0);
  assert(columns > 0);

  int    i, j;
  float  theta, phi;
  Vertex v;

  if((rows == 2) && (columns == 2))
    {
      Vertex vertices[] = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
                           {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
                           {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f,  1.0f}, {0.5f, 1.0f}},
                           {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
                           {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.5f, 1.0f}},
                           {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}};
      int indices[] = {0, 1, 2, 3, 4, 5};

      for(i=0; i<6; i++)
        {
          mVertices.push_back(vertices[i]);
        }

      for(i=0; i<6; i++)
        {
          mIndices.push_back(indices[i]);
        }

    }
  else
    {
      // Create the vertices
      for(i=0; i<rows; i++)
        {
          phi = (-PI / 2.0f) + (float(i) / float(rows - 1)) * PI;

          for(j=0; j<columns; j++)
            {
              theta = float(j) / float(columns - 1) * 2.0f * PI; 

              v.mPosition[0] = cos(phi) * cos(theta);
              v.mPosition[1] = cos(phi) * sin(theta);
              v.mPosition[2] = sin(phi);

              v.mNormal[0] = v.mPosition[0];
              v.mNormal[1] = v.mPosition[1];
              v.mNormal[2] = v.mPosition[2];

              v.mTextureCoord0[0] = ((float)j / (columns - 1));
              v.mTextureCoord0[1] = ((float)i / (rows - 1));
          
              mVertices.push_back(v);
            }
        }

      // Create the indices
      for(i=0; i<(rows - 1); i++)
        {
          for(j=0; j<(columns - 1); j++)
            {
              mIndices.push_back((j + 0) + (i + 0) * columns);
              mIndices.push_back((j + 1) + (i + 0) * columns);
              mIndices.push_back((j + 1) + (i + 1) * columns);

              mIndices.push_back((j + 0) + (i + 0) * columns);
              mIndices.push_back((j + 1) + (i + 1) * columns);
              mIndices.push_back((j + 0) + (i + 1) * columns);
            }
        }
    }

  // Create the VBOs
  glGenBuffersARB(1, &mVerticesVBO);
  glGenBuffersARB(1, &mIndicesVBO);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVerticesVBO);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, mVertices.size() * sizeof(mVertices[0]), &(mVertices[0]), GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mIndicesVBO);
  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mIndices.size() * sizeof(mIndices[0]), &(mIndices[0]), GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}
