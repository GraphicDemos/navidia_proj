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

#include "Shader.h"

#include <gl/gl.h>
#include <glh/glh_extensions.h>

#include <iostream>
#include <assert.h>



// ----------------------------------------------------------------------------
// Shader class
//

    //
    // Construction and destruction
    //
    
        // Constructor
        //
        // Description:
        //      Creates a shader instance.
        //
        // Parameters:
        //      zsName - the shader's name.
        //            
Shader::Shader(const char * szName): _szName(szName)
                                   , _hProgram(0)
                                   , _pSliders(0)
{
    _pSliders = new ParamListGL(const_cast<char *>(_szName));
    glGenProgramsNV (1, &_hProgram);
}

        // Destructor
        //
Shader::~Shader()
{
    delete _pSliders;
    glDeleteProgramsNV(1, &_hProgram);
}

   
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
Shader::bind()
{
    glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, _hProgram);
}

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
Shader::unbind()
{
    glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, 0);
}

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
Shader::renderSliders(unsigned int nX, unsigned int nY)
        const
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
Shader::special(int nKey, int nX, int nY)
{
    _pSliders->Special(nKey, nX, nY);
}


    
    //
    // Protected methods
    //

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
Shader::load(const char * zProgram)
{
    bind();
    
    unsigned int nProgramLength = static_cast<unsigned int>(strlen(zProgram));
    glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, _hProgram, nProgramLength, 
		        (const GLubyte *) zProgram);

    //
    // check for errors
    //

    GLint nErrorPosition;
    glGetIntegerv (GL_PROGRAM_ERROR_POSITION_NV, &nErrorPosition);
    if (nErrorPosition != -1)
    {
        std::cerr << "Fragment shader error:" << std::endl << std::endl;

        const char * pCharacter = zProgram;
        const char * pLine = pCharacter;
        while (*pCharacter != '\0' && (pCharacter - zProgram) < nErrorPosition)
        {
	        if (*pCharacter == '\n')
	            pLine = pCharacter + 1;
	        pCharacter++;
        }
        char zErrorLine[81];

        strncpy (zErrorLine, pLine, 80);
        zErrorLine[80] = '\0';

        std::cerr << zErrorLine << std::endl;
    }
}

