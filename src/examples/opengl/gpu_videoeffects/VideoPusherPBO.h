#ifndef VIDEO_PUSHER_PBO_H
#define VIDEO_PUSHER_PBO_H
// ----------------------------------------------------------------------------
// 
// Content:
//      VideoPusherPBO class
//
// Description:
//      A class managing Video via OpenGL pixel buffer objects (PBO).
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

#include "VideoPusher.h"
#include "Mutex.h"


// ----------------------------------------------------------------------------
// VideoPusherPBO class
//
class VideoPusherPBO: public VideoPusher
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
    VideoPusherPBO();
    
            // Constructor for different Texture aspect ratios
            //
    VideoPusherPBO(ImageSource::teTextureType eTextureType);

            // Destructor
            //
   ~VideoPusherPBO();
   
   
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

            // cleanup
            //
            // Description:
            //      Cleanup routine for destruction
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    cleanup();

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
    

protected:
    //
    // Private data
    //
    Mutex _mutex;

    GLuint _hPixelBuffer;

	ImageSize imgsize;

    bool _bAllocatedPBO;
};

#endif // IMAGE_PUSHER_PBO_H
