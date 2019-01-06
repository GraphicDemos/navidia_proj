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

#include <gl/glew.h>
#include <gl/gl.h>

#include "StaticImage.h"

#include "OpenEXRLoader.h"

#include <assert.h>


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

        // Constructor with types
        //
StaticImage::StaticImage(ImageSource::teTextureType eTextureType): ImagePusher(eTextureType)
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
StaticImage::setImage(const ImageTex & rImage)
{
    ImageSource::setImage(rImage);
    
    glBindTexture(_nTextureTarget, _hTexture);

    glTexImage2D(_nTextureTarget, 
                 0, 
                 _oImage.glInternalTextureFormat(),
                 _oImage.width(),
                 _oImage.height(), 
                 0, 
                 _oImage.glTextureFormat(),
                 _oImage.glTextureType(),
                 _oImage.data());
}
        
        // initVideoCopyContext
        //
        // Description:
        //      Initializes the Video Copy Context (for copying Video Frames)
        //      to a Texture
        //
        // Parameters:
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int 
StaticImage::initVideoCopyContext()
{
	return 0;
}

		// copyVideoFrame
        //
        // Description:
        //      Specify a new image.
        //
        // Parameters:
        //      rImage - image object.
        //
        // Returns:
        //      None
        //
        unsigned int 
StaticImage::copyVideoFrame(void *pSrc, ImageSize imginfo)
{
    return 0;
}

        // init the Video Buffers to the render stage
        //
        // Description:
        //      This function will create the PBO for Video Copying, so the
		//		DirectShow thread will run properly
        //
        // Parameters:
        //
        // Returns:
        //      None
        //
        void
StaticImage::initVideoBuffers()
{
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
    glBindTexture(_nTextureTarget, _hTexture);
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
    glBindTexture(_nTextureTarget, 0);
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
