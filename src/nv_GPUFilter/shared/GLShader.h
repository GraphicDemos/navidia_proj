//
// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/inc/shared/GLShader.h#1 $
//

#ifndef _GLSHADER_
#define _GLSHADER_

#include <vector>

// Represents an OpenGL shader object. 

class GLShader
{
  friend class GLProgram; // The GLProgram class needs special access

public:
  enum ShaderType {Vertex, Fragment};

  GLShader(ShaderType type, char *name);
  ~GLShader();

  // Adds code to be compiled
  void AddCode(const char *code);
  void AddCode(const char **code, int numstrings);
  void AddCodeFromFile(const char *file);
  void AddCodeFromFile(const char *file,const char *defaultCode);

  // Clears any previously added code
  void ClearCode();

  // returns true if the shader is usable and false otherwise.
  // Use GetInfo to retrieve detailed information after a validation
  bool Validate();
  char *GetInfo() {return mInfo;}
  char *GetName() {return mName;}

protected:
private:

  // Destroys any openGL stuff that may be using old code
  void Invalidate();

  // The type of shader this is (fragment or vertex)
  ShaderType mType;

  // The actual code for this shader
  std::vector<char *> mCode;

  // The GLPrograms that have had this shader added to them
  std::vector<GLProgram *> mUsedBy;

  // OpenGL's fun little handle
  GLhandleARB mGLObject;

  // Filled upon validation with errors and such
  char *mInfo;

  // Kept for references purposes
  char *mName;

  // True if the code has been validated.
  // Calls to Use and GetUniformID will validate automatically
  // if it has not already been done.
  bool mBeenValidated;
};

#endif
