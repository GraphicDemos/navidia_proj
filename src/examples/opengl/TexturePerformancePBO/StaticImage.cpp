// ----------------------------------------------------------------------------
// 
// Content:
//      StaticImage class
//
// Description:
//      A class managing OpenEXR images via OpenGL pixel buffer objects (PBO).
//
// Author: Frank Jargstorff (03/10/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "StaticImage.h"

#include "OpenEXRLoader.h"

#include <assert.h>

#include <glh/glh_extensions.h>


// ----------------------------------------------------------------------------
// ImagePBO class
//

    //
    // Public data
    //
    
const char * StaticImage::ClassName = "StaticImage";
const char * StaticImage::ClassDescription = "Static Image";

    //
    // Construction and destruction
    //

        // Default constructor
        //
StaticImage::StaticImage(): ImagePusher()
{
    ;
}

        // Destructor
        //
StaticImage::~StaticImage()
{
    unbind();
    glDeleteTextures(1, &_hTexture);
}


    //
    // Public methods
    //
    
        // setImage
        //
        // Description:
        //      Specify a new image.
        //
        // Parameters:
        //      rImage - const reference to the image object.
        //
        // Returns:
        //      None
        //
        void
StaticImage::setImage(const Image & rImage)
{
    ImageSource::setImage(rImage);
    
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, _hTexture);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 
                 0, 
                 _oImage.glInternalTextureFormat(),
                 _oImage.width(),
                 _oImage.height(), 
                 0, 
                 _oImage.glTextureFormat(),
                 _oImage.glTextureType(),
                 _oImage.data());
}
        
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
StaticImage::bind()
{
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, _hTexture);
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
StaticImage::unbind()
{
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);
}

        // pushNewFrame
        //
        // Description:
        //      Pushes a new frame up to the graphics board.
        //          This specific image pusher imprints a time stamp
        //      bit-pattern in the left-bottom corner of the image.
        //
        // Parameters:
        //      nFrameStamp - the frame number to imprint.
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int
StaticImage::pushNewFrame()
{
    bind();
    return 0;
}
