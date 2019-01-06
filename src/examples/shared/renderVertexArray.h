/*
  Render to vertex array class
  sgreen 2/2004
*/

#ifndef RENDERVERTEXARRAY_H
#define RENDERVERTEXARRAY_H

#include <windows.h>
#include <GL/gl.h>
#include <glh/glh_extensions.h>

class RenderVertexArray {
public:
  RenderVertexArray(int nverts, GLint size, GLenum type = GL_FLOAT);
  ~RenderVertexArray();

  void LoadData(void *data);                // load vertex data from memory
  void Read(GLenum buffer, int w, int h);   // read vertex data from frame buffer
  void SetPointer(GLuint index);

private:
    GLenum m_usage;     // vbo usage flag
    GLuint m_buffer;
    GLuint m_index;
    GLuint m_nverts;
    GLint m_size;       // size of attribute       
    GLenum m_format;    // readpixels image format
    GLenum m_type;      // FLOAT or HALF_FLOAT
    int m_bytes_per_component;
};

#endif