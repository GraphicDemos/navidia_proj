#ifndef SHADER_H
#define SHADER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      Shader class
//
// Description:
//      A class managing the fragment shaders for image compositing.
//
// Author: Frank Jargstorff (03/28/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>

#include <paramgl/paramgl.h>



// ----------------------------------------------------------------------------
// Shader class
//
class Shader
{
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
            // Description:
            //      Creates a shader instance.
            //            
    Shader(const char * szName);
    
            // Destructor
            //
            virtual
   ~Shader();
   
   
    //
    // Public methods
    //
   
            
            // bind
            //
            // Description:
            //      Bind the PBO to the currently active texture unit.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    bind();
    
            // unbind
            //
            // Description:
            //      Unbinds the PBO.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
             void
    unbind();

            // renderSliders
            //
            // Description:
            //      Render the shader's parameter sliders.
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
            virtual
            void
    special(int nKey, int nX, int nY);


protected:
    //
    // Protected methods
    //
    
            // Default constructor
            //
            // Description:
            //      Not implemented.
            //
    Shader();
    
            // Copy constructor
            //
            // Description:
            //      Not implemented.
            //
    Shader(const Shader &);
    
            // assignment operator
            //
            // Description:
            //      Not implemented.
            //
            Shader &
    operator=(const Shader &);
    
            // load
            //
            // Description:
            //      Load a given shader program.
            //
            // Parameters:
            //      zProgram - the program as zero terminated string.
            //
            // Returns:
            //      None
            //
            void
    load(const char * zProgram);
    
    //
    // Protected data
    //
    
    const char    * _szName;
    GLuint          _hProgram; 
    ParamListGL	  * _pSliders;

};

#endif // SHADER_H
