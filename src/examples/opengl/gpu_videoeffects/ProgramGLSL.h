#ifndef PROGRAM_GLSL_H
#define PROGRAM_GLSL_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <paramgl.h>
#include "uniforms.h"

class ShaderGLSL;

class ProgramGLSL
{
    struct sRectSize
    {
        int nX, nY;
    };

public:
  ProgramGLSL(const char* fragmentShaderFilename);
  ProgramGLSL(const char* vertexShaderFilename, const char* fragmentShaderFilename);
  ~ProgramGLSL(void);
  
  operator GLhandleARB (void) const;

            // setWindowSize
            //
            // Description:
            //      Sets the Window size of Shader Context
            //
            // Parameters:
            //      width, height
            //
            // Returns:
            //      None
            //
            void
    setWindowSize(int width, int height);

            // setTextureSize
            //
            // Description:
            //      Sets the Texture size of Shader Context
            //
            // Parameters:
            //      width, height, tex_unit
            //
            // Returns:
            //      None
            //
            void
    setTextureSize(int width, int height, int tex_unit = 0);

            // bind
            //
            // Description:
            //      Bind the Shader Program to the currently active texture unit.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
  bind();

            // unbind
            //
            // Description:
            //      Unbinds the Shader Program
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
  unbind();

  void InitUniforms();

  void setTextureID(GLuint & texture, int tex_type, int tex_offset );

public:
  UniformsGLSL gUniforms;

private:
  ShaderGLSL* mVertexShader;
  ShaderGLSL* mFragmentShader;
  GLhandleARB mProgram;

  sRectSize mWinSize;
  sRectSize mTexSize[16];

  bool load(const char* fragmentShaderFilename);
  bool load(const char* vertexShaderFilename, const char* fragmentShaderFilename);
};


#endif


