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

#include "FastHDRShader.h"
#include <math.h>

#include <gl/gl.h>
#include <glh/glh_extensions.h>

#include <iostream>


// ----------------------------------------------------------------------------
// FastHDRShader class
//

    //
    // Static initialization
    //
    
const char * FastHDRShader::_gzCode = 
    "!!FP1.0\n" \
    "DECLARE exposure;\n" \
    "DECLARE gamma;\n" \
    "TEX  H0.xyz, f[TEX0].xyxx, TEX0, RECT;\n" \
    "MOVH o[COLH].xyz, H0.xyzx;\n" \
    "END\n" \
    "\0";

    //
    // Construction and destruction
    //
    
        // Default constructor
        //
        // Description:
        //      Creates a shader instance.
        //            
FastHDRShader::FastHDRShader(): Shader("Simple Shader")
{
    if (!glh_init_extensions("GL_NV_vertex_program GL_NV_fragment_program")) 
    {
        std::cerr << "Error - required extensions were not supported: " << glh_get_unsupported_extensions()
                  << std::endl;
        exit(-1);
    }

    load(_gzCode);
}

        // Destructor
        //
FastHDRShader::~FastHDRShader()
{
    ;
}
