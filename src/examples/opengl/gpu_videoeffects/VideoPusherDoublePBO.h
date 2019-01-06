#ifndef VIDEO_PUSHER_DOUBLE_PBO_H
#define VIDEO_PUSHER_DOUBLE_PBO_H
// ----------------------------------------------------------------------------
// 
// Content:
//      VideoPusherDoublePBO class
//
// Description:
//      A class managing OpenEXR images via OpenGL pixel buffer objects (PBO).
//
// Author: Eric Young (02/16/05)
//
// Note:
//      Copyright (C) 2005 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "ImagePusher.h"

//
// Defines
//

#define N_MAX_BUFFERS 2


// ----------------------------------------------------------------------------
// VideoPusherDoublePBO class
//
class VideoPusherDoublePBO: public ImagePusher
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
            //      Loads OpenEXR image if given.
            //
            // Parameters:
            //      zFileName - name of the OpenEXR image to load.
            //
            // Note:
            //      As a side-effect the constructor leaves the image
            //      bound to the OpenGL texture unit active upon invocation
            //      of this constructor.
            //            
    VideoPusherDoublePBO();

            // Constructor for different Texture aspect ratios
            //
    VideoPusherDoublePBO(ImageSource::teTextureType eTextureType);

            // Destructor
            //
   ~VideoPusherDoublePBO();
   
   
    //
    // Public methods
    //
   
            // init
            //
            // Description:
            //      Init routine during construction
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    init();

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
            virtual
            void
    setImage(const ImageTex & rImage);

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
            virtual
            void
    initVideoBuffers();

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
            virtual
            unsigned int 
    initVideoCopyContext();

            // copyVideoFrame
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
            virtual
            unsigned int 
    copyVideoFrame(void *pSrc, ImageSize imginfo);

            // pushNewFrame
            //
            // Description:
            //      Pushes a new frame up to the graphics board.
            //          This specific image pusher imprints a time stamp
            //      bit-pattern in the left-bottom corner of the image.
            //      Pushing a new frame increments the frame counter.
            //
            // Parameters:
            //      nFrameStamp - the frame number to imprint.
            //
            // Returns:
            //      The number of bytes actually pushed across the bus
            //      to the graphics card.
            //
            virtual
            unsigned int 
    pushNewFrame();
    
            // pushNewFrame
            //
            // Description:
            //      Pushes a new frame up to the graphics board.
            //          This specific image pusher imprints a time stamp
            //      bit-pattern in the left-bottom corner of the image.
            //      Pushing a new frame increments the frame counter.
            //
            // Parameters:
            //      nFrameStamp - the frame number to imprint.
            //
            // Returns:
            //      The number of bytes actually pushed across the bus
            //      to the graphics card.
            //
            virtual
            unsigned int 
    pushNewFrame(BYTE *src, LONG numOfBytes);

protected:
    //
    // Protected data
    //
   
    GLuint _haPixelBuffer[N_MAX_BUFFERS];
    unsigned int _nCurrentBuffer;
};

#endif // IMAGE_PUSHER_DOUBLE_PBO_H
