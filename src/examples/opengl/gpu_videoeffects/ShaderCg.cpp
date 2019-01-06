// ----------------------------------------------------------------------------
// 
// Content:
//      ShaderCg class
//
// Description:
//      A class managing the fragment ShaderCgs for image compositing.
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

#include "ShaderCg.h"

#include <iostream>
#include <assert.h>



// ----------------------------------------------------------------------------
// ShaderCg class
//

    //
    // Construction and destruction
    //
    
        // Constructor
        //
        // Description:
        //      Creates a ShaderCg instance.
        //
        // Parameters:
        //      zsName - the ShaderCg's name.
        //            
ShaderCg::ShaderCg(const char * szName): _szName(szName)
                                   , _hProgram(0)
                                   , _pSliders(0)
{
    _pSliders       = new ParamListGL(const_cast<char *>(_szName));
    glGenProgramsNV (1, &_hProgram);
}

        // Destructor
        //
ShaderCg::~ShaderCg()
{
    delete _pSliders;
    glDeleteProgramsNV(1, &_hProgram);
}


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
ShaderCg::setWindowSize(int width, int height)
{
    mWinSize.nX = width;
    mWinSize.nY = height;
}

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
ShaderCg::setTextureSize(int width, int height, int tex_unit)
{
    mTexSize[tex_unit].nX = width;
    mTexSize[tex_unit].nY = height;
}

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
ShaderCg::bind()
{
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _hProgram);

    for (int tex_unit=0; tex_unit < 4; tex_unit++) {
        if (cgIsParameter(gUniforms.mTexture[tex_unit]))
            cgGLSetParameter1d( gUniforms.mTexture[tex_unit], 0 );
        if (cgIsParameter(gUniforms.mSceneMap[tex_unit]))
            cgGLSetParameter1d( gUniforms.mSceneMap[tex_unit], 0 );
    }

    if (cgIsParameter(gUniforms.mTexSize))
        cgGLSetParameter2f( gUniforms.mTexSize, mTexSize[0].nX, mTexSize[0].nY );

    int i;
    i = cgIsParameter(gUniforms.mGamma);
    i = cgIsParameter(gUniforms.mExposure);
}

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
ShaderCg::unbind()
{
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
}

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
ShaderCg::InitUniforms()
{
//    gUniforms.initUniforms(_hProgram);
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
ShaderCg::renderSliders(unsigned int nX, unsigned int nY)
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
ShaderCg::special(int nKey, int nX, int nY)
{
    _pSliders->Special(nKey, nX, nY);
}


    
    //
    // Protected methods
    //

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
ShaderCg::loadCg(const char * zProgram)
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
        std::cerr << "Fragment ShaderCg error:" << std::endl << std::endl;

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

        // load
        //
        // Description:
        //      Load a given ShaderCg program, using the cgGL function calls instead.
        //
        // Parameters:
        //      zProgram - the program as zero terminated string.
        //
        // Returns:
        //      None
        //
        void
ShaderCg::load(const char * zProgram)
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
        std::cerr << "Fragment ShaderCg error:" << std::endl << std::endl;

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
