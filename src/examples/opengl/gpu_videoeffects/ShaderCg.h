#ifndef SHADER_CG_H
#define SHADER_CG_H
// ----------------------------------------------------------------------------
// 
// Content:
//      Shader Cg class
//
// Description:
//      A class managing the Cg fragment shaders for image compositing.
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

#include <GL/glew.h>
#include <GL/gl.h>

#include <paramgl.h>

#include "uniforms.h"


// ----------------------------------------------------------------------------
// ShaderCg class
//
class ShaderCg
{
    struct sRectSize
    {
        int nX, nY;
    };

public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
            // Description:
            //      Creates a ShaderCg instance.
            //            
    ShaderCg(const char * szName);
    
            // Destructor
            //
            virtual
   ~ShaderCg();
   
   
    //
    // Public methods
    //
   
            // setWindowSize
            //
            // Description:
            //      Sets the Window size of Shader Context
            //
            // Parameters:
            //      width, height
            //
            // Returns:
            //      None
            //
            void
    setWindowSize(int width, int height);

            // setTextureSize
            //
            // Description:
            //      Sets the Texture size of Shader Context
            //
            // Parameters:
            //      width, height, tex_unit
            //
            // Returns:
            //      None
            //
            void
    setTextureSize(int width, int height, int tex_unit = 0);

            // bind
            //
            // Description:
            //      Bind the Cg Shader to the currently active state
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
            //      Unbind the Cg Shader from the currently active state
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
             void
    unbind();

        // InitUniforms
        //
        // Description:
        //      Initialize all the uniforms for the current Shader program
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
			void 
	InitUniforms();

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
    ShaderCg();
    
            // Copy constructor
            //
            // Description:
            //      Not implemented.
            //
    ShaderCg(const ShaderCg &);
    
            // assignment operator
            //
            // Description:
            //      Not implemented.
            //
            ShaderCg &
    operator=(const ShaderCg &);
    
            // load
            //
            // Description:
            //      Load a given ShaderCg program.
            //
            // Parameters:
            //      zProgram - the program as zero terminated string.
            //
            // Returns:
            //      None
            //
            void
    load(const char * zProgram);

            // load
            //
            // Description:
            //      Load a given ShaderCg program using the device context
            //
            // Parameters:
            //      zProgram - the program as zero terminated string.
            //
            // Returns:
            //      None
            //
            void
    loadCg(const char * zProgram);

    //
    // Protected data
    //
    
    const char    * _szName;
    GLuint          _hProgram; 
    ParamListGL	  * _pSliders;

public:
    UniformsCG gUniforms;

private:
    sRectSize mWinSize;
    sRectSize mTexSize[16];

};

#endif // SHADER_CG_H
