#ifndef PROGRAM_H
#define PROGRAM_H

#include <glh/glh_extensions.h>


class Shader;


class Program
{
public:
  Program(const char* vertexShaderFilename, const char* fragmentShaderFilename);
  ~Program(void);
  
  operator GLhandleARB (void) const;

private:
  Shader*     mVertexShader;
  Shader*     mFragmentShader;
  GLhandleARB mProgram;

  bool load(const char* vertexShaderFilename, const char* fragmentShaderFilename);
};


#endif


