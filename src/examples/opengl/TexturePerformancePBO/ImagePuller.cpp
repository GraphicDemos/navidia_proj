// ----------------------------------------------------------------------------
// 
// Content:
//      ImagePuller class
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

#include "ImagePuller.h"

#include "ImageSink.h"

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/gl.h>

#include <assert.h>


//
// Macros
//

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// ----------------------------------------------------------------------------
// ImagePuller class
//
    
    //
    // Public data
    //
    
const char * ImagePuller::ClassName = "ImagePuller";
const char * ImagePuller::ClassDescription = "Normal (glReadPixels)";

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
ImagePuller::ImagePuller(unsigned int nWidth, unsigned int nHeight)
            : ImageSink(nWidth, nHeight)
            , _pPixels(0)
            , _pProcessedPixels(0)
{
    if (imageSize() != 0)
    {
        _pPixels = new tsBGRA8[imageSize()];
        _pProcessedPixels = new tsBGRA8[imageSize()];
    }
}

        // Destructor
        //
ImagePuller::~ImagePuller()
{
    delete _pProcessedPixels;
    delete _pPixels;
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
ImagePuller::resize(unsigned int nWidth, unsigned int nHeight)
{
    ImageSink::resize(nWidth, nHeight);
    
    delete _pProcessedPixels;
    delete _pPixels;
    
    if (imageSize() != 0)
    {
        _pPixels = new tsBGRA8[imageSize()];
        _pProcessedPixels = new tsBGRA8[imageSize()];
    }
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
ImagePuller::pull(unsigned int nPositionX, unsigned int nPositionY)
{
    glReadBuffer(GL_FRONT);
                                // make sure our reinterpret cast 
                                // below is valid
    assert(sizeof(tsBGRA8) == sizeof(GLuint));
    
    glReadPixels(nPositionX, nPositionY, width(), height(), GL_BGRA, 
                 GL_UNSIGNED_BYTE, _pPixels);
                 
    if (_bDummyWorkload)
        processPixels();
                 
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
ImagePuller::processPixels()
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


