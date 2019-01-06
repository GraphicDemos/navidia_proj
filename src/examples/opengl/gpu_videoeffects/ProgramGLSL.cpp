
#include "ProgramGLSL.h"
#include "ShaderGLSL.h"

#include <assert.h>

ProgramGLSL::ProgramGLSL(const char* fragmentShaderFilename)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;
  mProgram = 0;

  load(fragmentShaderFilename);

  InitUniforms();
}

ProgramGLSL::ProgramGLSL(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;
  mProgram = 0;

  load(vertexShaderFilename, fragmentShaderFilename);

  InitUniforms();
}


ProgramGLSL::~ProgramGLSL(void)
{
    if (mVertexShader)   glDetachObjectARB(mProgram, (GLhandleARB)(*mVertexShader));
    if (mFragmentShader) glDetachObjectARB(mProgram, (GLhandleARB)(*mFragmentShader));

    if (mVertexShader)   delete(mVertexShader);
    if (mFragmentShader) delete(mFragmentShader);

    if(mProgram != 0)
    {
      glDeleteObjectARB(mProgram);
      mProgram = 0;
    }
}

bool ProgramGLSL::load(const char* fragmentShaderFilename)
{
  char infoLog[4096];
  int  infoLogLength;

  assert(fragmentShaderFilename != NULL);

  // Load the fragment shader
  mFragmentShader = new ShaderGLSL(fragmentShaderFilename, GL_FRAGMENT_SHADER_ARB);

  printf("GLSL::load( %-30s)\n", fragmentShaderFilename);

  // Create the program
  mProgram = glCreateProgramObjectARB();
  assert(mProgram != 0);

  glAttachObjectARB(mProgram, (GLhandleARB)(*mFragmentShader));

  GLint bLinked;

  // Link the shaders in the program
  glLinkProgramARB(mProgram);
  glGetObjectParameterivARB( mProgram, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

  if (bLinked == false) {
    // Print out the info log
    glGetInfoLogARB(mProgram, sizeof(infoLog), &infoLogLength, infoLog);
    if(infoLogLength > 0)
    {
      printf("infoLog\n%s\n", infoLog);
    }
  }

  glUseProgramObjectARB(mProgram);

  // TODO check for errors

  return(true);
}

bool ProgramGLSL::load(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
  assert(vertexShaderFilename != NULL);
  assert(fragmentShaderFilename != NULL);
  
  char infoLog[4096];
  int  infoLogLength;
  
  printf("GLSL::load( %-30s, %-30s)\n", vertexShaderFilename, fragmentShaderFilename);

  // Load the vertex shader
  mVertexShader   = new ShaderGLSL(vertexShaderFilename, GL_VERTEX_SHADER_ARB);
  
  // Load the fragment shader
  mFragmentShader = new ShaderGLSL(fragmentShaderFilename, GL_FRAGMENT_SHADER_ARB);
  
  // Create the program
  mProgram = glCreateProgramObjectARB();
  assert(mProgram != 0);
  
  // Attach the shaders to the program
  glAttachObjectARB(mProgram, (GLhandleARB)(*mVertexShader));
  glAttachObjectARB(mProgram, (GLhandleARB)(*mFragmentShader));
  
  GLint bLinked;

  // Link the shaders in the program
  glLinkProgramARB(mProgram);
  glGetObjectParameterivARB( mProgram, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

  if (bLinked == false) {
    // Print out the info log
    glGetInfoLogARB(mProgram, sizeof(infoLog), &infoLogLength, infoLog);
    if(infoLogLength > 0)
    {
      printf("infoLog\n%s\n", infoLog);
    }
    return(false);
  }

#if 0
  glValidateProgramARB( mProgram );

  GLint validated = false;
  glGetObjectParameterivARB( mProgram , GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
  if (!validated)
  {
        glGetInfoLogARB(mProgram, sizeof(infoLog), &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            printf("infoLog\n%s\n", infoLog);
            printf("Shaders failed to validate, exiting...\n");
            return false;
        }
  }
  glUseProgramObjectARB(mProgram);
#endif

  // TODO check for errors

  return(true);
}

ProgramGLSL::operator GLhandleARB (void) const
{
    return(mProgram);
}


void ProgramGLSL::setWindowSize(int width, int height)
{
    mWinSize.nX = width;
    mWinSize.nY = height;
}

void ProgramGLSL::setTextureSize(int width, int height, int tex_unit)
{
    mTexSize[tex_unit].nX = width;
    mTexSize[tex_unit].nY = height;
}

void ProgramGLSL::bind()
{
    glUseProgramObjectARB(mProgram);

    for (int tex_unit=0; tex_unit < 4; tex_unit++) {
        if (gUniforms.mTexture[tex_unit] != -1)
            glUniform1iARB( gUniforms.mTexture[tex_unit], 0 );
        if (gUniforms.mSceneMap[tex_unit] != -1)
            glUniform1iARB( gUniforms.mSceneMap[tex_unit], 0 );
    }
}

void ProgramGLSL::unbind()
{
    glUseProgramObjectARB(NULL);
}

void ProgramGLSL::InitUniforms()
{
    gUniforms.initUniforms(mProgram);
}

void ProgramGLSL::setTextureID(GLuint & texture, int tex_type, int tex_offset )
{
    gUniforms.setTextureID( texture, tex_type, tex_offset );
}
