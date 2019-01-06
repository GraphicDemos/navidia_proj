#ifndef SHADER_GLSL_H
#define SHADER_GLSL_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <paramgl.h>

class ShaderGLSL
{
public:
  ShaderGLSL(const char* filename, GLenum shaderType);
  ~ShaderGLSL(void);

  operator GLhandleARB (void) const;

            // renderSliders
            //
            // Description:
            //      Render the ShaderGLSL's parameter sliders.
            //
            // Parameters:
            //      nX - x-coordinate
            //      nY - y-coordinate
            //
            // Returns:
            //      None
            //
            void
    renderSliders(unsigned int nX, unsigned int nY)
            const;

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
    special(int nKey, int nX, int nY);

private:
    GLhandleARB mShader;

    const char    * _szName;
    ParamListGL	  * _pSliders;
};


#endif
