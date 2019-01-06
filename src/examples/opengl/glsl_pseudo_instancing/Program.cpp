
#include "Program.h"
#include "Shader.h"

#include <assert.h>

Program::Program(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
  mVertexShader = NULL;
  mFragmentShader = NULL;
  mProgram = 0;

  load(vertexShaderFilename, fragmentShaderFilename);
}

Program::~Program(void)
{
  delete(mVertexShader);
  delete(mFragmentShader);

  if(mProgram != 0)
    {
      glDeleteObjectARB(mProgram);
      mProgram = 0;
    }
}

bool Program::load(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
  assert(vertexShaderFilename != NULL);
  assert(fragmentShaderFilename != NULL);
  
  char infoLog[4096];
  int  infoLogLength;
  
  printf("Program::load(\"%s\", \"%s\")\n", vertexShaderFilename, fragmentShaderFilename);

  // Load the vertex shader
  mVertexShader = new Shader(vertexShaderFilename, GL_VERTEX_SHADER_ARB);
  
  // Load the fragment shader
  mFragmentShader = new Shader(fragmentShaderFilename, GL_FRAGMENT_SHADER_ARB);
  
  // Create the program
  mProgram = glCreateProgramObjectARB();
  assert(mProgram != 0);
  
  // Attach the shaders to the program
  glAttachObjectARB(mProgram, (GLhandleARB)(*mVertexShader));
  glAttachObjectARB(mProgram, (GLhandleARB)(*mFragmentShader));
  
  // Link the shaders in the program
  glLinkProgramARB(mProgram);
  
  // Print out the info log
  glGetInfoLogARB(mProgram, sizeof(infoLog), &infoLogLength, infoLog);
  if(infoLogLength > 0)
    {
      printf("infoLog\n%s\n", infoLog);
    }

  // TODO check for errors

  return(true);
}

Program::operator GLhandleARB (void) const
{
  return(mProgram);
}
