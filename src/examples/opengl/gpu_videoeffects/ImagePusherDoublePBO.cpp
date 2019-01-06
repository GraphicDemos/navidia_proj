// ----------------------------------------------------------------------------
// 
// Content:
//      ImagePusherDoublePBO class
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

#include "ImagePusherDoublePBO.h"
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
    
const char * ImagePusherDoublePBO::ClassName = "ImagePusherDoublePBO";
const char * ImagePusherDoublePBO::ClassDescription = "Image Double PBO";

    //
    // Construction and destruction
    //

        // Default constructor
        //
ImagePusherDoublePBO::ImagePusherDoublePBO(): ImagePusher()
                                            , _nCurrentBuffer(0)
{
    init();
}

        // Constructor for textures with different aspect ratios
        //
ImagePusherDoublePBO::ImagePusherDoublePBO(ImageSource::teTextureType eTextureType): ImagePusher(eTextureType)
                                            , _nCurrentBuffer(0)
{
    init();
}

        // Destructor
        //
ImagePusherDoublePBO::~ImagePusherDoublePBO()
{
    unbind();
    glDeleteBuffersARB(N_MAX_BUFFERS, _haPixelBuffer);
}


    //
    // Public methods
    //
void ImagePusherDoublePBO::init()
{
	glewInit();

	glGenBuffersARB(N_MAX_BUFFERS, _haPixelBuffer);
}

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
ImagePusherDoublePBO::setImage(const ImageTex & rImage)
{
                                // Bind texture and create local image copy
    ImagePusher::setImage(rImage);
    
    glBindTexture(_nTextureTarget, _hTexture);
 
                                // Now create the corresponding PBOs
    for (int iBuffer = 0; iBuffer < N_MAX_BUFFERS; ++iBuffer)
    {
                                // bind pixel-buffer object
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _haPixelBuffer[iBuffer]);
                                // create buffer's data container
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _oImage.imageDataSize(), NULL, GL_STREAM_DRAW_ARB);
        
        void * pPixelsPBO = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
                                // copy original image into the buffer
        memcpy(pPixelsPBO, _oImage.data(), _oImage.imageDataSize());
    
        if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
        {
            std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
            assert(false);
        }
                                // unbind pixel-buffer object
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    }
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
ImagePusherDoublePBO::initVideoCopyContext()
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
ImagePusherDoublePBO::copyVideoFrame(void *pSrc, ImageSize imginfo)
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
ImagePusherDoublePBO::pushNewFrame()
{
    _nCurrentBuffer = (_nCurrentBuffer + 1) % N_MAX_BUFFERS;
    unsigned int iModifyBuffer = (_nCurrentBuffer + 1) % N_MAX_BUFFERS;
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _haPixelBuffer[iModifyBuffer]);
    
    void * pPixelData = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
    
    imprintPixelData(pPixelData, _nFrameCounter);
    
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // bind the texture object
    bind();   
                                
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _haPixelBuffer[_nCurrentBuffer]);                
                                // copy buffer contents into the texture
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), BUFFER_OFFSET(0));
                    
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    _nFrameCounter++;

    return _oImage.imageDataSize();
}
 
