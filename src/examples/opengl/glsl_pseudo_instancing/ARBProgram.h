#ifndef ARB_PROGRAM_H
#define ARB_PROGRAM_H

#include <glh/glh_extensions.h>


class ARBProgram
{
public:
  ARBProgram(GLenum target, const char* programFilename);
  ~ARBProgram(void);
  
  operator GLhandleARB (void) const;

  bool load(const char* programFilename);   // TODO make private again

private:
  GLuint mProgram;
  GLenum mTarget;
};


#endif


