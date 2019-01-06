#ifndef IMAGE_PUSHER_PBO_H
#define IMAGE_PUSHER_PBO_H
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

#include "ImagePusher.h"


// ----------------------------------------------------------------------------
// ImagePusherPBO class
//
class ImagePusherPBO: public ImagePusher
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
    ImagePusherPBO();
    
            // Constructor for different Texture aspect ratios
            //
    ImagePusherPBO(ImageSource::teTextureType eTextureType);

            // Destructor
            //
   ~ImagePusherPBO();
   
   
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
    
    

protected:
    //
    // Private data
    //
   
    GLuint _hPixelBuffer;
};

#endif // IMAGE_PUSHER_PBO_H