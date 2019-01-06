#ifndef FAST_HDR_SHADER_H
#define FAST_HDR_SHADER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      FastHDRShader class
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
// FastHDRShader class
//
class FastHDRShader: public Shader
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
    FastHDRShader();
    
            // Destructor
            //
            virtual
   ~FastHDRShader();
   
private:
    //
    // Private data
    //
    
    static const char * _gzCode; // zero terminated string with the shader code.
};

#endif // SIMPLE_HDR_SHADER_H
