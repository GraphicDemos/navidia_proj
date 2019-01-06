#ifndef IMAGE_SOURCE_H
#define IMAGE_SOURCE_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImageSource class
//
// Description:
//      A base class for image sources.
//
// Author: Frank Jargstorff (03/19/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Include
// 

#include "Image.h"


// ----------------------------------------------------------------------------
// ImageSource class
//
class ImageSource
{
public:
    //
    // Public types
    //
    
    enum tePixelFormatGL
    {
        GL_FLOAT_RGBA16_NV_PIXEL,
        GL_FLOAT_RGB16_NV_PIXEL, 
        GL_RGBA_PIXEL,
        GL_RGB_PIXEL,
        GL_UNDEFINED_PIXEL
    };
 
    
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
    ImageSource()
            { ; }
    
            // Destructor
            //
            virtual
   ~ImageSource()
            { ; }
   
   
    //
    // Public methods
    //
   
            // setImage
            //
            // Description:
            //      Specify a new image.
            //
            // Parameters:
            //      rImage - const reference to the image to set.
            //
            // Returns:
            //      None
            //
            virtual
            void
    setImage(const Image & rImage)
            {
                _oImage = rImage;
            }
            
            // image
            //
            // Description:
            //      Access to the internal image.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      A const reference to the internally stored image.
            //
            const
            Image &
    image()
            const
            {
                return _oImage;
            }
            
            // pixelFormatStringGL
            //
            // Description:
            //      String description of internal pixel format.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      Pixel format description string.
            //
            virtual
            std::string
    pixelFormatStringGL()
            const = 0;
 
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
    pushNewFrame() = 0;
    
            // bind
            //
            // Description:
            //      Bind the iamge to the currently active texture unit.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            virtual
            void
    bind() = 0;
    
            // unbind
            //
            // Description:
            //      Unbinds the image.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            virtual
            void
    unbind() = 0;

protected:
    //
    // Protected data
    //
   
    Image _oImage;
};

#endif // IMAGE_SOURCE_H
