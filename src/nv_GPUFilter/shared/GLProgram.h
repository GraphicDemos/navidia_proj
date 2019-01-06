//
// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/inc/shared/GLProgram.h#1 $
//

#ifndef _GLPROGRAM_
#define _GLPROGRAM_

#include "GLShader.h"

class GLProgram
{
public:
  GLProgram(char *name);
  ~GLProgram();

  // These add and remove shader objects from the program
  // They automatically invalidate the program
  bool AddShader(GLShader &shader);
  bool RemoveShader(GLShader &shader);

  // This sets up the OpenGL render state to begin using the shader
  // returns false if the shader was unable to validate
  bool Use();

  // returns true if the program is usable in the current render state
  // and false otherwise. Use GetInfo to retrieve detailed information
  bool Validate();
  char *GetInfo() {return mInfo;}
  bool IsValidated() {return mBeenValidated;}

  // Clears any previous shaders attached to this program.
  void Clear();

  // Get the ID of a uniform parameter.
  // Use with one of the glUniform functions to set uniform parameters
  GLint GetUniformID(const char *var);

  char *GetName() {return mName;}

protected:
private:

  // Deletes the program handle and clears mInfo
  void Invalidate();

  // OpenGL's program handle
  GLhandleARB mGLProgram;

  // The list of GLShaders this program is using
  std::vector<GLShader *> mUsing;

  // Filled upon validation with errors and such
  char *mInfo;

  // Stored for reference purposes
  char *mName;

  // True if the code has been validated.
  // Calls to Use and GetUniformID will validate automatically
  // if it has not already been done.
  bool mBeenValidated;
};

#endif
