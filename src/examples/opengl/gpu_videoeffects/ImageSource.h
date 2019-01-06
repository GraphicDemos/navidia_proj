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

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/glew.h>
#include <gl/gl.h>

#include "ImageTex.h"

// ----------------------------------------------------------------------------
// ImageSource class
//
class ImageSource
{
public:
    //
    // Public types
    //
    enum teTextureType
    {
        GL_TEX_RECT,
        GL_TEX_2D,
        GL_TEX_UNDEFINED
    };

    enum tePixelFormatGL
    {
        GL_FLOAT_RGBA16_NV_PIXEL,
        GL_FLOAT_RGB16_NV_PIXEL, 
        GL_RGBA_PIXEL,
        GL_RGB_PIXEL,
        GL_YUYV_PIXEL,
        GL_UYVY_PIXEL,
        GL_UNDEFINED_PIXEL
    };
 
    
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
    ImageSource()
            { 
                setTextureType(GL_TEX_RECT);
            }
    
            // Constructor with texture aspect ratio support
            //
    ImageSource(teTextureType eTextureType)
            { 
                setTextureType(eTextureType);
            }

			// Destructor
            //
            virtual
   ~ImageSource()
            { ; }
   
   
    //
    // Public methods
    //

            // setTextureType
            //
            // Description:
            //      Specify a the texture type, so we know which GL_TEXTURE format to use.
            //
            // Parameters:
            //      rImage - const reference to the image to set.
            //
            // Returns:
            //      None
            //
            virtual
            void
    setTextureType(teTextureType eTextureType)
            {
                _eTextureType = eTextureType; 
                _nTextureTarget = (_eTextureType == GL_TEX_RECT) ? GL_TEXTURE_RECTANGLE_NV : GL_TEXTURE_2D;
            }

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
    setImage(const ImageTex & rImage)
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
            ImageTex &
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
    initVideoBuffers() = 0;

			// initVideoCopyContext
            //
            // Description:
            //      Initializes the Video Copy Context (for copying Video Frames)
            //      to a PBO or a Texture
            //
            // Parameters:
            //
            // Returns:
            //      The number of bytes actually pushed across the bus
            //      to the graphics card.
            //
            virtual
            unsigned int 
    initVideoCopyContext() = 0;

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
    copyVideoFrame(void *pSrc, ImageSize imginfo) = 0;

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
   
    ImageTex _oImage;

	GLenum       _nTextureTarget;
	teTextureType _eTextureType;
};

#endif // IMAGE_SOURCE_H
