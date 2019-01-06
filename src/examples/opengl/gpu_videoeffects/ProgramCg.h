#ifndef PROGRAM_CG_H
#define PROGRAM_CG_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <paramgl.h>

#include "uniforms.h"

class ShaderCg;

class ProgramCg
{
    struct sRectSize
    {
        int nX, nY;
    };

public:
  ProgramCg(const char* fragmentShaderFilename);
  ProgramCg(const char* vertexShaderFilename, const char* fragmentShaderFilename);
  ProgramCg(const char* vertexShaderFilename, const char* vertexMain, const char **vertexArgs,
			const char* fragmentShaderFilename, const char* fragMain, const char **fragArgs);
  ~ProgramCg(void);
  
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
#if 0
  void InitSpecialTextures(glh::tex_object_2D & tex_dust,
                           glh::tex_object_2D & tex_lines,
                           glh::tex_object_2D & tex_tv,
                           glh::tex_object_3D & tex_noise);
#endif

  CGprogram vertex() { return mVertexShader; }
  CGprogram fragment() { return mFragmentShader; }

  CGprofile vertex_profile() { return mVertexProfile; }
  CGprofile fragment_profile() { return mFragmentProfile; }

public:
  UniformsCG gUniforms;

private:
    CGcontext Context;
    CGprogram mVertexShader;
    CGprogram mFragmentShader;

    CGprofile mVertexProfile;
    CGprofile mFragmentProfile;

    CGerror Ret;

  sRectSize mWinSize;
  sRectSize mTexSize[16];

  bool bUseVertex;
  bool bUseFragment;

  bool load(const char* fragmentShaderFilename, const char *fragMain);
  bool load(const char* vertexShaderFilename, const char *vertexMain, const char **vertexArgs,
			const char* fragmentShaderFilename, const char *fragMain, const char **fragArgs);
};

	
#endif


