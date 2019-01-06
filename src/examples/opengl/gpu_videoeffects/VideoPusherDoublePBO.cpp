// ----------------------------------------------------------------------------
// 
// Content:
//      VideoPusherDoublePBO class
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

#include "VideoPusherDoublePBO.h"
#include "OpenEXRLoader.h"
#include "defines.h"

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
    
const char * VideoPusherDoublePBO::ClassName = "VideoPusherDoublePBO";
const char * VideoPusherDoublePBO::ClassDescription = "Video (Double PBO)";

    //
    // Construction and destruction
    //

        // Default constructor
        //
VideoPusherDoublePBO::VideoPusherDoublePBO(): ImagePusher()
                                            , _nCurrentBuffer(0)
{
    init();
}

        // Constructor for textures with different aspect ratios
        //
VideoPusherDoublePBO::VideoPusherDoublePBO(ImageSource::teTextureType eTextureType): ImagePusher(eTextureType)
                                            , _nCurrentBuffer(0)
{
    init();
}

        // Destructor
        //
VideoPusherDoublePBO::~VideoPusherDoublePBO()
{
    unbind();
    glDeleteBuffersARB(N_MAX_BUFFERS, _haPixelBuffer);
}


    //
    // Public methods
    //
void VideoPusherDoublePBO::init()
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
VideoPusherDoublePBO::setImage(const ImageTex & rImage)
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
VideoPusherDoublePBO::pushNewFrame()
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
 

        // initVideoCopyContext
        //
        // Description:
        //      Copies the contents of new frame up to the graphics board.
        //
        // Parameters:
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int 
VideoPusherDoublePBO::initVideoCopyContext()
{
//    _hDC[1]   = wglGetCurrentDC( );
//    _hRC[1]   = wglCreateContext( _hDC[1] );

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
VideoPusherDoublePBO::initVideoBuffers()
{
}

        // copyVideoFrame
        //
        // Description:
        //      Copies data from a Video Frame to the graphics board.
        //
        // Parameters:
        //      pSrc - the frame number to imprint.
        //      numOfBytes - number of 
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int 
VideoPusherDoublePBO::copyVideoFrame(void *pSrc, ImageSize imginfo)
{
#if 0
	copyToPixelBuffer(_oImage.data(), pSrc, _oImage.width(), _oImage.height(), imginfo.imageDataSize());

    bind();
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), _oImage.data());

    _nFrameCounter++;
#endif    
    return _oImage.imageDataSize();
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
VideoPusherDoublePBO::pushNewFrame(BYTE *src, LONG numOfBytes)
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
 
