
#include "File.h"
#include "ShaderGLSL.h"

#include <gl/glew.h>
#include <gl/gl.h>
#include <assert.h>


ShaderGLSL::ShaderGLSL(const char* filename, GLenum shaderType) :
            _pSliders(0)
{
    assert(filename != NULL);

    File             file;
    const GLcharARB* text;
    int              length;
    char             infoLog[4096];
    int              infoLogLength;

    _pSliders = new ParamListGL(const_cast<char *>(filename));

    // Create the shader object
    mShader = glCreateShaderObjectARB(shaderType);
    assert(mShader != 0);

    // Load the file
    file.load(filename);
    text = const_cast<const GLcharARB*>((const char*)file);
    length = file.length();

    GLint bFragCompiled;

    // Load the shader source
    glShaderSourceARB(mShader, 1, &text, &length);

    // Compile the shader
    glCompileShaderARB(mShader);

    glGetObjectParameterivARB( mShader, GL_OBJECT_COMPILE_STATUS_ARB, 
                               &bFragCompiled );

    if ( bFragCompiled == false) {
        // Print out the info log
        glGetInfoLogARB(mShader, sizeof(infoLog), &infoLogLength, infoLog);
        if(infoLogLength > 0)
        {
           printf("CompileShaderARB() infoLog   \"%s\"\n%s\n", filename, infoLog);
        }
    }
}

ShaderGLSL::~ShaderGLSL(void)
{
    delete _pSliders;

    if(mShader != 0)
    {
      glDeleteObjectARB(mShader);
      mShader = 0;
    }
}


ShaderGLSL::operator GLhandleARB (void) const
{
  return(mShader);
}

        // renderSliders
        //
        // Description:
        //      Render the ShaderCg's parameter sliders.
        //
        // Parameters:
        //      nX - x-coordinate
        //      nY - y-coordinate
        //
        // Returns:
        //      None
        //
        void
ShaderGLSL::renderSliders(unsigned int nX, unsigned int nY) const
{
    _pSliders->Render(nX, nY);
}


    
        // special
        //
        // Description:
        //      Handle special keys
        //
        // Parameters:
        //      nKey - key code.
        //      nX - cursor position x.
        //      nY - cursor position y.
        //
        // Returns:
        //      None
        //
        void
ShaderGLSL::special(int nKey, int nX, int nY)
{
    _pSliders->Special(nKey, nX, nY);
}
