// ----------------------------------------------------------------------------
// 
// Content:
//      ImagePusherPBO class
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

#include <GL/glew.h>
#include <GL/gl.h>

#include "ImagePusherPBO.h"
#include "OpenEXRLoader.h"
#include <assert.h>


//
// Macros
//

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// ----------------------------------------------------------------------------
// ImagePBO class
//

    //
    // Public data
    //
    
const char * ImagePusherPBO::ClassName = "ImagePusherPBO";
const char * ImagePusherPBO::ClassDescription = "Image PBO";

    //
    // Construction and destruction
    //

        // Default constructor
        //
ImagePusherPBO::ImagePusherPBO(): ImagePusher()
                                , _hPixelBuffer(0)
{
    init();
}

        // Constructor for textures with different aspect ratios
        //
ImagePusherPBO::ImagePusherPBO(ImageSource::teTextureType eTextureType): ImagePusher(eTextureType)
                                , _hPixelBuffer(0)
{
    init();
}

        // Destructor
        //
ImagePusherPBO::~ImagePusherPBO()
{
    unbind();
    glDeleteBuffersARB(1, &_hPixelBuffer);
}

void ImagePusherPBO::init()
{
	glewInit();

	glGenBuffersARB(1, &_hPixelBuffer);
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
        //      rImage - const reference to image object.
        //
        // Returns:
        //      None
        //
        void
ImagePusherPBO::setImage(const ImageTex & rImage)
{
                                // create texture object and local image copy
    ImagePusher::setImage(rImage);
                                // bind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
                                // create pixel-buffer data container
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_EXT, _oImage.imageDataSize(), NULL, GL_STREAM_DRAW_ARB);
    unsigned char * pPixelsPBO = static_cast<unsigned char *>(glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY));
                                // copy image data into the buffer
    memcpy(pPixelsPBO, _oImage.data(), _oImage.imageDataSize());
    
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_EXT))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // unbind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
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
ImagePusherPBO::initVideoCopyContext()
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
ImagePusherPBO::copyVideoFrame(void *pSrc, ImageSize imginfo)
{
    return 0;
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
ImagePusherPBO::pushNewFrame()
{
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
    
    void * pPixelData = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
    
    imprintPixelData(pPixelData, _nFrameCounter);
    
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // bind the texture object
    bind();                   
                                // copy buffer contents into the texture
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), BUFFER_OFFSET(0));

    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    _nFrameCounter++;
    
    return _oImage.imageDataSize();
}
        
