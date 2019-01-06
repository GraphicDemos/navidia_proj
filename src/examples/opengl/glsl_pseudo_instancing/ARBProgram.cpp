
#include "File.h"
#include "ARBProgram.h"

#include <assert.h>

ARBProgram::ARBProgram(GLenum target, const char* programFilename)
{
  // Record the type of program this is
  assert((target == GL_VERTEX_PROGRAM_ARB) || (target == GL_FRAGMENT_PROGRAM_ARB));
  mTarget = target;

  // Generate a handle for the arb program
  glGenProgramsARB(1, &mProgram);
  assert(mProgram > 0);

  // Load the arb program from the file
  load(programFilename);
}

ARBProgram::~ARBProgram(void)
{
  glDeleteProgramsARB(1, &mProgram);
}

bool ARBProgram::load(const char* programFilename)
{
  assert(programFilename != NULL);

  File source;
  
  //printf("ARBProgram::load(\"%s\")\n", programFilename);   #TODO uncomment
  
  // Load the source
  if(!source.load(programFilename))
    {
      return(false);
    }

  // Bind the program
  assert(mProgram > 0);
  glBindProgramARB(mTarget, mProgram);
  
  // Compile the program
  glProgramStringARB(mTarget, GL_PROGRAM_FORMAT_ASCII_ARB, source.length(), ((const char*)source));

  // Check for errors
  if(glGetError() == GL_INVALID_OPERATION)
    {
      GLint          position;
      const GLubyte* string;
      
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);
      string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

      printf("Error at %d\n%s\n", position, string);
    }

  return(true);
}

ARBProgram::operator GLhandleARB (void) const
{
  return(mProgram);
}
