#ifndef VIDEOPUSHER_H
#define VIDEOPUSHER_H

// ----------------------------------------------------------------------------
// 
// Content:
//      Video Image class
//
// Description:
//      Video Image container that handles Video Image data, pixel format and image size.
//
// Author: Eric Young (01/04/05)
//
// Note:
//      Copyright (C) 2004, 2005 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "ImageSource.h"

class NVImageLoader;

// ----------------------------------------------------------------------------
// Video Image class
//
class VideoPusher : public ImageSource
{
	friend class NVImageLoader;

public:
    //
    // Public data
    //
    
    static const char * ClassName;
    static const char * ClassDescription;

    //
    // Construction and destruction
    //

            // Default constructor
            //
    VideoPusher();
            
            // Constructor with TextureDimension support
            //
    VideoPusher(ImageSource::teTextureType eTextureType);

            // Destructor
            //
   ~VideoPusher();

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
        // copyToPixelBuffer
        //
        // Description:
        //      Copies from a dst to a src to the Pixel Buffer
        //
        // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      srcwidth     - length of source image data        
		//      srcheight    - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
    copyToPixelBuffer(void * pDst, void * pSrc, long srcwidth, long srcheight, long numOfBytes);

		// copyToHalfPixelBuffer
        //
        // Description:
        //      Copies from Src -> Dst in half Pixel Buffer region.
        //
        // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      srcwidth     - length of source image data        
		//      srcheight    - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
    copyToHalfPixelBuffer(half * pDst, byte * pSrc, long srcwidth, long srcheight, long numOfBytes);

		// copyTo8bitPixelBuffer
        //
        // Description:
        //      Copies from Src -> Dst in 8-bit Pixel Buffer region.
        //
        // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      srcwidth     - length of source image data        
		//      srcheight    - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
	copyTo8bitPixelBuffer(byte * pDst, byte * pSrc, long srcwidth, long srcheight, long numOfBytes);


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

    HDC   _hDC[2];
    HGLRC _hRC[2];

    unsigned int _nFrameCounter;
};

#endif // IMAGE_PUSHER_H
