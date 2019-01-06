#ifndef SHADER_H
#define SHADER_H

#include <glh/glh_extensions.h>

class Shader
{
public:
  Shader(const char* filename, GLenum shaderType);
  ~Shader(void);

  operator GLhandleARB (void) const;

private:
  GLhandleARB mShader;
};


#endif
