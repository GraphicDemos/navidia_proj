#ifndef STATIC_IMAGE_H
#define STATIC_IMAGE_H
// ----------------------------------------------------------------------------
// 
// Content:
//      StaticImage class
//
// Description:
//      A class managing OpenEXR images via OpenGL.
//
// Author: Frank Jargstorff (03/19/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "ImageSource.h"
#include "ImagePusher.h"



// ----------------------------------------------------------------------------
// StaticImage class
//
class StaticImage: public ImagePusher
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
    StaticImage();
    
            // Constructor with texture aspect ratio support
            //
    StaticImage(ImageSource::teTextureType eTextureType);

            // Destructor
            //
   ~StaticImage();
   
   
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
            virtual
            void
    bind();
    
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
            virtual
            void
    unbind();
};

#endif // STATIC_IMAGE_H
