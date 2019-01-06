// ----------------------------------------------------------------------------
// 
// Content:
//      SimpleHDRShader class
//
// Description:
//      A shader for simple 16bit fp rendering.
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

#include "SimpleHDRShader.h"
#include <math.h>

#include <gl/gl.h>
#include <glh/glh_extensions.h>

#include <iostream>


// ----------------------------------------------------------------------------
// SimpleHDRShader class
//

    //
    // Static initialization
    //
    
const char * SimpleHDRShader::_gzCode = 
    "!!FP1.0\n"\
    "DECLARE exposure;\n"
    "DECLARE gamma;\n"
    "TEX H0, f[TEX0].xyxx, TEX0, RECT;\n"
    "MULH H0, H0, exposure.x;\n"
    "POWH H1.z, H0.z, gamma.x;\n"
    "POWH H1.y, H0.y, gamma.x;\n"
    "POWH H1.x, H0.x, gamma.x;\n"
    "MOVH H1.w, H0.w;\n"
    "MOVH o[COLH], H1;\n"
    "END\n"
    "\0";

    //
    // Construction and destruction
    //
    
        // Default constructor
        //
        // Description:
        //      Creates a shader instance.
        //            
SimpleHDRShader::SimpleHDRShader(): Shader("Gamma Shader")
                                  , _pExposureParameter(0)
                                  , _pGammaParameter(0)
                                  , _nExposure(1.0f)
                                  , _nGamma(0.4545f)
{
    if (!glh_init_extensions("GL_NV_vertex_program GL_NV_fragment_program")) 
    {
        std::cerr << "Error - required extensions were not supported: " << glh_get_unsupported_extensions()
                  << std::endl;
        exit(-1);
    }

    load(_gzCode);

    _pExposureParameter = new Param<float>("Exposure", _nExposure, 0.0f, 10.0f, 0.1f);
    _pGammaParameter    = new Param<float>("Gamma",    _nGamma,    0.0f, 10.0f, 0.1f);
 	_pSliders->AddParam(_pExposureParameter);
	_pSliders->AddParam(_pGammaParameter);

    setExposure(_nExposure);
    setGamma(_nGamma);
}

        // Destructor
        //
SimpleHDRShader::~SimpleHDRShader()
{
    ;
}

      
    //
    // Public methods
    //
    
        // setExposure
        //
        // Description:
        //      Set exposure value
        //
        // Parameters:
        //      nExposure - the exposure value.
        //
        // Returns:
        //      None
        //
        void
SimpleHDRShader::setExposure(float nExposure)
{
    _nExposure = nExposure;
    glProgramNamedParameter4fNV(_hProgram, static_cast<GLsizei>(strlen("exposure")), reinterpret_cast<GLubyte *>("exposure"), 
                                _nExposure, 0, 0, 0);
}

        // setGamma
        //
        // Description:
        //      Set a gamma value for image display.
        //
        // Parameters:
        //      nGamme - the gamma value.
        //
        // Returns:
        //      None
        //
        void
SimpleHDRShader::setGamma(float nGamma)
{
    _nGamma = nGamma;
    glProgramNamedParameter4fNV(_hProgram, static_cast<GLsizei>(strlen("gamma")), reinterpret_cast<GLubyte *>("gamma"), 
                                _nGamma, 0, 0, 0);
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
SimpleHDRShader::special(int nKey, int nX, int nY)
{
    Shader::special(nKey, nX, nY);
    setExposure(_pExposureParameter->GetFloatValue());
    setGamma(_pGammaParameter->GetFloatValue());
}
    
      
