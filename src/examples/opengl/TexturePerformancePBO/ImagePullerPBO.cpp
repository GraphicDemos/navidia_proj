// ----------------------------------------------------------------------------
// 
// Content:
//      ImagePullerPBO class
//
// Description:
//      A class reading pixels into an image via pixel buffer objects (PBO).
//
// Author: Frank Jargstorff (03/11/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "ImagePullerPBO.h"

#include <glh/glh_extensions.h>

#include <iostream>
#include <assert.h>


//
// Macros
//

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// ----------------------------------------------------------------------------
// ImagePullerPBO class
//

    //
    // Public data
    //
    
const char * ImagePullerPBO::ClassName = "ImagePullerPBO";
const char * ImagePullerPBO::ClassDescription = "PBO Readback";

    //
    // Construction and destruction
    //
    
        // Default constructor
        //
        // Description:
        //      Creates image puller for given size images.
        //
        // Parameters:
        //      nWidth  - image width.
        //      nHeight - image height.
        //
ImagePullerPBO::ImagePullerPBO(unsigned int nWidth, unsigned int nHeight)
            : ImageSink(nWidth, nHeight)
            , _pPixels(0)
            , _pProcessedPixels(0)
            , _iCurrentBuffer(0)
{
                                // Initialize the PBO extension. 
    if (!glh_init_extensions("GL_ARB_vertex_buffer_object "
                             "GL_EXT_pixel_buffer_object ")) 
    {
        std::cerr << "Error - required extensions were not supported: " << glh_get_unsupported_extensions()
                  << std::endl;
        exit(-1);
    }

    glGenBuffersARB(N_MAX_BUFFERS, _aPixelBuffer);
    initialize();

    if (imageSize() != 0)
        _pProcessedPixels = new tsBGRA8[imageSize()];
}

        // Destructor
        //
ImagePullerPBO::~ImagePullerPBO()
{
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);
    glDeleteBuffersARB(N_MAX_BUFFERS, _aPixelBuffer);
}

   
    //
    // Public methods
    //
   
        // resize
        //
        // Description:
        //      Resizes the buffer size.
        //          The content of the new image is undefined.
        //
        // Parameters:
        //      nWidth  - the new width.
        //      nHeight - the new height.
        //
        // Returns:
        //      None
        //
        void 
ImagePullerPBO::resize(unsigned int nWidth, unsigned int nHeight)
{
    ImageSink::resize(nWidth, nHeight);
    initialize();    
    
    delete _pProcessedPixels;
    
    if (imageSize() != 0)
        _pProcessedPixels = new tsBGRA8[imageSize()];
}


        // pull
        //
        // Description:
        //      Reads pixels from frame buffer.
        //          The buffer read from is always the currently 
        //      displayed buffer.
        //          If the position is such that the image size
        //      exceeds the frame buffer size the outcome of the
        //      operation is undefined and might stop the program.
        //
        // Parameters:
        //      nPositionX - x-position in FB where to start reading from.
        //      nPositionY - y-position in FB where to start reading from.
        //
        // Returns:
        //      The number of bytes read back to system memory. 
        //      
        unsigned int
ImagePullerPBO::pull(unsigned int nPositionX, unsigned int nPositionY)
{
                                // make sure our reinterpret cast 
                                // below is valid
    assert(sizeof(tsBGRA8) == sizeof(GLuint));
                                // get next buffer's index
    unsigned int iNextBuffer = (_iCurrentBuffer + 1) % N_MAX_BUFFERS;
                                // kick of readback of current front-buffer
                                // into the next buffer
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, _aPixelBuffer[iNextBuffer]);
    glReadPixels(nPositionX, nPositionY, width(), height(), GL_BGRA, 
                 GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
                                // map the current buffer containing the
                                // image read back the previous time around
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, _aPixelBuffer[_iCurrentBuffer]);
    _pPixels = static_cast<tsBGRA8 *>(glMapBufferARB(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY_ARB));
                                // perform dummy-task on the image.
    if (_bDummyWorkload)
        processPixels();
                                // unmap the buffer
    if (!glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_EXT))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // unbind readback buffer to not interfere with
                                // any other (traditional) readbacks.
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);
                                // make next-buffer the current buffer
    _iCurrentBuffer = iNextBuffer;
                 
    return imageDataSize();
}

        // processPixels
        //
        // Description:
        //      Divide all pixels in the lastest image by two and
        //      store in new location.
        //          This is used as a dummy workload on the downloaded
        //      image.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
ImagePullerPBO::processPixels()
{
    tsBGRA8 * pInputPixel;
    tsBGRA8 * pEndInput = _pPixels + imageSize();
    tsBGRA8 * pOutputPixel;
    
    for (pInputPixel = _pPixels, pOutputPixel = _pProcessedPixels;
         pInputPixel < pEndInput; 
         ++pInputPixel, ++pOutputPixel)
    {
        pOutputPixel->nB = pInputPixel->nB / 2;
        pOutputPixel->nG = pInputPixel->nG / 2;
        pOutputPixel->nR = pInputPixel->nR / 2;
        pOutputPixel->nA = pInputPixel->nA;
    }
}



    //
    // Private methods
    //
    
        // initialize 
        //
        // Description:
        //      Initializes the pixel array.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
ImagePullerPBO::initialize()
{
    for (unsigned int iBuffer = 0; iBuffer < N_MAX_BUFFERS; ++iBuffer)
    {
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, _aPixelBuffer[iBuffer]);
        glBufferDataARB(GL_PIXEL_PACK_BUFFER_EXT, imageDataSize(), NULL, GL_STATIC_READ);
    }   
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);
    glReadBuffer(GL_FRONT);
}
