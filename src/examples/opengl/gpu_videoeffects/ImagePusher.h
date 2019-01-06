#ifndef IMAGE_PUSHER_H
#define IMAGE_PUSHER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImagePusher class
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

#include <OpenEXR/half.h>

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/glew.h>
#include <gl/gl.h>


//
// Forward definitions
//

class OpenEXRLoader;
class ImageTex;


// ----------------------------------------------------------------------------
// ImagePusher class
//
class ImagePusher: public ImageSource
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
    ImagePusher();

            // Constructor with TextureDimension support
            //
    ImagePusher(ImageSource::teTextureType eTextureType);


            // Destructor
            //
   ~ImagePusher();
   
   
    //
    // Public methods
    //
   
            // setImage
            //
            // Description:
            //      Specify a new image.
            //
            // Parameters:
            //      rImage - const reference to the new image.
            //
            // Returns:
            //      None
            //
            virtual
            void
    setImage(const ImageTex & rImage);
   
            // setPixelFormatGL
            //
            // Description:
            //      Specify the internal GL format for texture upload.
            //
            // Parameters:
            //      ePixelFormatGL - the interal OpenGL format for the texture.
            //
            // Returns:
            //      None
            //
            void
    setPixelFormatGL(tePixelFormatGL ePixelFormatGL);
    
            // pixelFormatGL
            //
            // Description:
            //      Get the internal GL format.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The internal OpenGL format used for the texture.
            //
            tePixelFormatGL
    pixelFormatGL()
            const;
   
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
            std::string
    pixelFormatStringGL()
            const;
            
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
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The number of bytes actually pushed across the bus
            //      to the graphics card.
            //
            virtual
            unsigned int
    pushNewFrame();
    
			// frameCounter
            //
            // Description:
            //      Get the current frame count.
            //
            // Paramters:
            //      None
            //
            // Returns:
            //      Frame count.
            //
            unsigned int
    frameCounter()
            const;
            
            // resetFrameCounter
            //
            // Description:
            //      Resets the frame counter to zero.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    resetFrameCounter();
            
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
    

protected:
    //
    // Protected methods
    //
    
            // ImprintPixelData
            //
            // Description:
            //      Imprints binary pattern of a uint in lower left corner.
            //
            // Parameters:
            //      pData        - pointer to the image data
            //      nImprint     - the number to imprint
            //
            // Returns:
            //      None
            //
            void
    imprintPixelData(void * pData, unsigned int nImprint);

            // ImprintHalfPixelData
            //
            // Description:
            //      Imprints binary pattern of a uint in lower left corner.
            //
            // Parameters:
            //      pData        - pointer to the image data
            //      nImprint     - the number to imprint
            //
            // Returns:
            //      None
            //
            // Note:
            //      This method will fail for pixel formats other than FP16_RGB
            //      and FP16_RGBA with an assert(false).
            //
            void
    imprintHalfPixelData(half * pData, unsigned int nImprint);

            // Imprint8bitPixelData
            //
            // Description:
            //      Imprints binary pattern of a uint in lower left corner.
            //
            // Parameters:
            //      pData        - pointer to the image data
            //      nImprint     - the number to imprint
            //
            // Returns:
            //      None
            //
            // Note:
            //      This method will fail for pixel formats other than FX8_RGB,
            //      FX8_RGBA, FX8_BGR, and FX8_BGRA with an assert(false).
            //
            void
    imprint8bitPixelData(unsigned char * pData, unsigned int nImprint);
    
    
    //
    // Protected data
    //
   
    GLuint _hTexture;
    GLint  _nPixelFormatGL;

    unsigned int _nFrameCounter;
};

#endif // IMAGE_PUSHER_H
