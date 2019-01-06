#ifndef SIMPLE_HDR_SHADER_H
#define SIMPLE_HDR_SHADER_H
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

#include "Shader.h"


// ----------------------------------------------------------------------------
// SimpleHDRShader class
//
class SimpleHDRShader: public Shader
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
    SimpleHDRShader();
    
            // Destructor
            //
            virtual
   ~SimpleHDRShader();
   
   
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
    setExposure(float nExposure);
    
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
    setGamma(float nGamma);            
 
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

private:
    //
    // Private data
    //
    
    ParamBase * _pExposureParameter;
    ParamBase * _pGammaParameter;

    
    float _nExposure;
    float _nGamma;

    
    static const char * _gzCode; // zero terminated string with the shader code.
};

#endif // SIMPLE_HDR_SHADER_H
