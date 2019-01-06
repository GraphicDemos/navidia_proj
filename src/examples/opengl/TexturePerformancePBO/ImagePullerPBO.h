#ifndef IMAGE_PULLER_PBO_H
#define IMAGE_PULLER_PBO_H
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

#include "ImageSink.h"

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/gl.h>


//
// Defines
//

#define N_MAX_BUFFERS 2


// ----------------------------------------------------------------------------
// ImagePullerPBO class
//
class ImagePullerPBO: public ImageSink
{
public:
    //
    // Public data
    //
    
    static const char * ClassName;
    static const char * ClassDescription;

public:
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
    ImagePullerPBO(unsigned int nWidth = 0, unsigned int nHeight = 0);
    
            // Destructor
            //
            virtual
   ~ImagePullerPBO();
   
   
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
            virtual
            void 
    resize(unsigned int nWidth, unsigned int nHeight);
    
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
            virtual
            unsigned int
    pull(unsigned int nPositionX, unsigned int nPositionY);
   
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
    processPixels();
   
private:
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
    initialize();
    
    
    //
    // Private data
    //
    
    tsBGRA8 * _pPixels;
    tsBGRA8 * _pProcessedPixels;
 
    GLuint          _aPixelBuffer[N_MAX_BUFFERS];
    unsigned int    _iCurrentBuffer;
};

#endif // IMAGE_PULLER_PBO_H
