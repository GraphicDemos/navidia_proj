
#include "File.h"
#include "Shader.h"

#include <assert.h>
#include <glh/glh_extensions.h>


Shader::Shader(const char* filename, GLenum shaderType)
{
  assert(filename != NULL);
   
  File             file;
  const GLcharARB* text;
  int              length;
  char             infoLog[4096];
  int              infoLogLength;


  // Create the shader object
  mShader = glCreateShaderObjectARB(shaderType);
  assert(mShader != 0);

  // Load the file
  file.load(filename);
  text = const_cast<const GLcharARB*>((const char*)file);
  length = file.length();

  // Load the shader source
  glShaderSourceARB(mShader, 1, &text, &length);

  // Compile the shader
  glCompileShaderARB(mShader);

  // Print out the info log
  glGetInfoLogARB(mShader, sizeof(infoLog), &infoLogLength, infoLog);
  if(infoLogLength > 0)
    {
      printf("infoLog   \"%s\"\n%s\n", filename, infoLog);
    }  
}

Shader::~Shader(void)
{
  if(mShader != 0)
    {
      glDeleteObjectARB(mShader);
      mShader = 0;
    }
}

Shader::operator GLhandleARB (void) const
{
  return(mShader);
}
