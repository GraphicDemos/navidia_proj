// ----------------------------------------------------------------------------
// 
// Content:
//      OpenEXRLoader class
//
// Description:
//      Load OpenEXR image from disk.
//
// Author: Frank Jargstorff (03/29/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "OpenEXRLoader.h"

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImfRgba.h>

#include <assert.h>


// ----------------------------------------------------------------------------
// OpenEXRLoader abstract base class
//
    //
    // Construction and destruction
    //
            
        // Default constructor
        //
OpenEXRLoader::OpenEXRLoader(const char * zFileName)
{
    load(zFileName);
}
        
        // Destructor
        //
OpenEXRLoader::~OpenEXRLoader()
{
    ; // empty
}
    
   
    //
    // Public methods
    //
   
        // loadImage
        //
        // Description:
        //      Loads a new image from a given file.
        //
        // Parameters:
        //      zFileName - name of the image file to load.
        //
        // Returns:
        //      None
        //
        void
OpenEXRLoader::load(const char * zFileName)
{
    Imf::RgbaInputFile oInputImage(zFileName);
    Imath::Box2i oDataWindow = oInputImage.dataWindow();
    float nPixelRatio = oInputImage.pixelAspectRatio();
    
    unsigned int nWidth  = oDataWindow.max.x - oDataWindow.min.x + 1;
    unsigned int nHeight = oDataWindow.max.y - oDataWindow.min.y + 1;
    
    Imf::Array<Imf::Rgba> aPixels;
    aPixels.resizeErase(nWidth*nHeight);
    
    oInputImage.setFrameBuffer(aPixels + (nHeight-1) * nWidth,
                               1, -static_cast<int>(nWidth));
    try
    {
        oInputImage.readPixels(oDataWindow.min.y, oDataWindow.max.y);
    }
    catch(const std::exception &)
    {
        assert(false);
    }
    
    _oImage.setData(nWidth, nHeight, ImageTex::FP16_RGBA_PIXEL, aPixels);
}