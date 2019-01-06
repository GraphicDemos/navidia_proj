// ----------------------------------------------------------------------------
// 
// Content:
//      VideoPusher class
//
// Description:
//      A class managing Video images via OpenGL pixel buffer objects (PBO).
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

#include <GL/glew.h>
#include <GL/gl.h>
//#include <glh/glh_extensions.h>

#include "VideoPusher.h"
#include "OpenEXRLoader.h"
#include <assert.h>

//
// Global constants
// 

        // Used by the imprint functions to determine the size of the
        // squares representing the bits of the frame-counter.
        //    The actual size of the square is cnSquareSize x cnSquareSize
        // pixels.
        //
const int cnSquareSize = 16;

// ----------------------------------------------------------------------------
// VideoPusher class
//

    //
    // Public data
    //
    
const char * VideoPusher::ClassName        = "VideoPusher";
const char * VideoPusher::ClassDescription = "Video (glTexSubimage)";

    //
    // Construction and destruction
    //

        // Default constructor
        //s
VideoPusher::VideoPusher(): ImageSource(ImageSource::GL_TEX_RECT)
                          , _hTexture(0)
                          , _nPixelFormatGL(0)
                          , _nFrameCounter(0)
{
    glGenTextures(1, &_hTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // need this since RGB/BGR idmages are stored 
                                           // without padding.
}
        // Constructor for defining custom Shader Types
        //
VideoPusher::VideoPusher(ImageSource::teTextureType eTextureType): ImageSource(eTextureType)
						  , _hTexture(0)
                          , _nPixelFormatGL(0)
                          , _nFrameCounter(0)
{
    glGenTextures(1, &_hTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // need this since RGB/BGR images are stored 
                                           // without padding.
}

        // Destructor
        //
VideoPusher::~VideoPusher()
{
    unbind();
    glDeleteTextures(1, &_hTexture);

    for (int i=0; i < 2; i++) {
        if ( _hRC[i] != NULL )
        {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext( _hRC[i] );
            _hRC[i] = NULL;
        }

        if ( _hDC[i] != NULL )
        {
            ReleaseDC ( NULL, _hDC[i] );
            _hDC[i] = NULL;
        }
    }
}


    //
    // Public methods
    //

        // setImage
        //
        // Description:
        //      Specify a new image.
        //
        // Parameters:
        //      rImage - image object.
        //
        // Returns:
        //      None
        //
        void
VideoPusher::setImage(const ImageTex & rImage)
{
    ImageSource::setImage(rImage);
    
    GLint nInternalFormat = _nPixelFormatGL;
    if (nInternalFormat == 0)
        nInternalFormat = _oImage.glInternalTextureFormat();


    glBindTexture(_nTextureTarget, _hTexture);
    glTexImage2D(_nTextureTarget, 
                 0, 
                 _oImage.glInternalTextureFormat(),
                 _oImage.width(),
                 _oImage.height(),
                 0, 
                 _oImage.glTextureFormat(),
                 _oImage.glTextureType(),
                 _oImage.data());

    _hDC[0]   = wglGetCurrentDC( );
    _hRC[0]   = wglCreateContext( _hDC[0] );
}
        
        // setPixelFormatGL
        //
        // Description:
        //      Specify the internal GL format in which the image is processed.
        //
        // Parameters:
        //      ePixelFormatGL - the interal OpenGL format for the texture.
        //          Use UNKNOWN_PIXEL to reset to internal format matching the
        //      image's representation.
        //
        // Returns:
        //      None
        //
        void
VideoPusher::setPixelFormatGL(VideoPusher::tePixelFormatGL ePixelFormatGL)
{
    switch (ePixelFormatGL)
    {
        case GL_FLOAT_RGBA16_NV_PIXEL:
            _nPixelFormatGL = GL_FLOAT_RGBA16_NV;
        break;
        case GL_FLOAT_RGB16_NV_PIXEL:
            _nPixelFormatGL = GL_FLOAT_RGB16_NV;
        break;
        case GL_RGBA_PIXEL:
            _nPixelFormatGL = GL_RGBA;
        break;
        case GL_RGB_PIXEL:
            _nPixelFormatGL = GL_RGB;
        break;
        case GL_YUYV_PIXEL:
        case GL_UYVY_PIXEL:
            _nPixelFormatGL = GL_LUMINANCE_ALPHA;
        break;
        case GL_UNDEFINED_PIXEL:
            _nPixelFormatGL = 0;
        break;
        default:
            assert(false);
    }
    glBindTexture(_nTextureTarget, _hTexture);
    
    GLint nInternalFormat = _nPixelFormatGL;
    if (nInternalFormat == 0)
        nInternalFormat = _oImage.glInternalTextureFormat();

    glTexImage2D(_nTextureTarget, 
                 0, 
                 nInternalFormat,
                 _oImage.width(),
                 _oImage.height(),
                 0, 
                 _oImage.glTextureFormat(),
                 _oImage.glTextureType(),
                 _oImage.data());
}

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
        VideoPusher::tePixelFormatGL
VideoPusher::pixelFormatGL()
        const
{
    switch (_nPixelFormatGL)
    {
        case GL_FLOAT_RGBA16_NV:
            return GL_FLOAT_RGBA16_NV_PIXEL;
        case GL_FLOAT_RGB16_NV:
            return GL_FLOAT_RGB16_NV_PIXEL;
        case GL_RGBA:
            return GL_RGBA_PIXEL;
        case GL_RGB:
            return GL_RGB_PIXEL;
        case 0:
            return GL_UNDEFINED_PIXEL;
        default:
            assert(false);
    }
    return GL_UNDEFINED_PIXEL;
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
        std::string
VideoPusher::pixelFormatStringGL()
        const
{
    GLint nInternalFormat = _nPixelFormatGL;
    if (nInternalFormat == 0)
        nInternalFormat = _oImage.glInternalTextureFormat();

    switch (nInternalFormat)
    {
        case GL_FLOAT_RGBA16_NV:
            return "GL_FLOAT_RGBA16_NV";
        case GL_FLOAT_RGB16_NV:
            return "GL_FLOAT_RGB16_NV";
        case GL_RGBA:
            return "GL_RGBA";
        case GL_RGB:
            return "GL_RGB";
        case GL_LUMINANCE_ALPHA:
            return "GL_YUYV";
        default:
            assert(false);
    }
    return 0;
}
            
        // bind
        //
        // Description:
        //      Bind source's texture object to active texture unit.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
VideoPusher::bind()
{
    glBindTexture(_nTextureTarget, _hTexture);
}

        // unbind
        //
        // Description:
        //      Unbinds the texture object.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
VideoPusher::unbind()
{
    glBindTexture(_nTextureTarget, 0);
}


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
        unsigned int 
VideoPusher::initVideoCopyContext()
{
    _hDC[1]   = wglGetCurrentDC( );
    _hRC[1]   = wglCreateContext( _hDC[1] );

    return 0;
}

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
        void
VideoPusher::initVideoBuffers()
{
}

        // copyVideoFrame
        //
        // Description:
        //      Copies data from a Video Frame to the graphics board.
        //
        // Parameters:
        //      pSrc - the frame number to imprint.
        //      numOfBytes - number of 
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int 
VideoPusher::copyVideoFrame(void *pSrc, ImageSize imginfo)
{
    copyToPixelBuffer(_oImage.data(), pSrc, _oImage.width(), _oImage.height(), imginfo.imageDataSize());

    bind();
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), _oImage.data());

    _nFrameCounter++;
    
    return _oImage.imageDataSize();
}



        // pushNewFrame
        //
        // Description:
        //      Pushes a new frame up to the graphics board.
        //          This specific image pusher imprints a time stamp
        //      bit-pattern in the left-bottom corner of the image.
        //
        // Parameters:
        //      nFrameStamp - the frame number to imprint.
        //
        // Returns:
        //      The number of bytes actually pushed across the bus
        //      to the graphics card.
        //
        unsigned int 
VideoPusher::pushNewFrame()
{
    imprintPixelData(_oImage.data(), _nFrameCounter);
    bind();
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), _oImage.data());

    _nFrameCounter++;
    
    return _oImage.imageDataSize();
}
        

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
VideoPusher::frameCounter()
        const
{
    return _nFrameCounter;
}
        
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
VideoPusher::resetFrameCounter()
{
    _nFrameCounter = 0;
}


    //
    // Protected methods
    //

	
        // copyToPixelBuffer
        //
        // Description:
        //      Copies from Dst to Src
        //
        // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      width        - length of source image data        
		//      height       - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
VideoPusher::copyToPixelBuffer(void * pDst, void * pSrc, long width, long height, long numOfBytes)
{
    switch (_oImage.pixelFormat())
    {
        case ImageTex::FP16_RGB_PIXEL:
        case ImageTex::FP16_RGBA_PIXEL:
            copyToHalfPixelBuffer(  static_cast<half *>(pDst), 
                                    static_cast<unsigned char *>(pSrc), 
                                    width, 
                                    height, 
                                    numOfBytes);
        break;
        case ImageTex::FX8_BGR_PIXEL:
        case ImageTex::FX8_BGRA_PIXEL:
        case ImageTex::FX8_RGB_PIXEL:
        case ImageTex::FX8_RGBA_PIXEL:
            copyTo8bitPixelBuffer(  static_cast<unsigned char *>(pDst), 
                                    static_cast<unsigned char *>(pSrc), 
                                    width, height, numOfBytes);
        break;
        case ImageTex::FX8_YUYV_PIXEL:
        case ImageTex::FX8_UYVY_PIXEL:
            copyTo8bitPixelBuffer(  static_cast<unsigned char *>(pDst), 
                                    static_cast<unsigned char *>(pSrc), 
                                    width, height/2, numOfBytes);
        break;
        default:
            assert(false); // fail if pixel format is not handled above
    }
}

        // copyToHalfPixelBuffer
        //
        // Description:
        //      Copies from Src -> Dst to the HALF Pixel Buffer region.
        //
	    // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      width        - length of source image data        
		//      height       - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
VideoPusher::copyToHalfPixelBuffer(half * pDst, byte * pSrc, long srcwidth, long srcheight, long numOfBytes)
{
	int imgheight = _oImage.height();
	int imgwidth  = _oImage.width();

	int clip_width, clip_height;

	clip_width = (imgwidth < srcwidth) ? imgwidth : srcwidth;
	clip_height= (imgheight< srcheight)? imgheight : srcheight;

	for (int y=0; y < clip_height; y++) {
		for (int x=0; x < clip_width; x++) {
            switch (_oImage.pixelFormat())
            {
                case ImageTex::FP16_RGB_PIXEL:
                {
                    pDst[y*imgheight+x*3+0] = (half)pSrc[y*srcheight+x*3+0];
                    pDst[y*imgheight+x*3+1] = (half)pSrc[y*srcheight+x*3+1];
                    pDst[y*imgheight+x*3+2] = (half)pSrc[y*srcheight+x*3+2];
                }
                break;
                case ImageTex::FP16_RGBA_PIXEL: 
                {
                    pDst[y*imgheight+x*4+0] = (half)pSrc[y*srcheight+x*3+0];
                    pDst[y*imgheight+x*4+1] = (half)pSrc[y*srcheight+x*3+1];
                    pDst[y*imgheight+x*4+2] = (half)pSrc[y*srcheight+x*3+2];
                    pDst[y*imgheight+x*4+3] = (half)1.0f;
                }
                break;
                default:
                    assert(false); // this method is only meant to handle FP16 pixels
            }
		}
	}
}
        // copyTo8bitPixelBuffer
        //
        // Description:
        //      Copies from Src -> Dst in 8-bit Pixel Buffer region.
        //
        // Parameters:
        //      pDst         - pointer to the destination image data
        //      pSrc         - pointer to the source image data
		//      width        - length of source image data        
		//      height       - length of source image data        
        //      numOfBytes   - the number of bytes to copy
        //
        // Returns:
        //      None
        //
        void
VideoPusher::copyTo8bitPixelBuffer(byte * pDst, byte * pSrc, long srcwidth, long srcheight, long numOfBytes)
{
	int imgheight = _oImage.height();
	int imgwidth  = _oImage.width();

	int clip_width, clip_height;

	clip_width = (imgwidth < srcwidth) ? imgwidth : srcwidth;
	clip_height= (imgheight< srcheight)? imgheight : srcheight;

	for (int y=0; y < clip_height; y++) {
		for (int x=0; x < clip_width; x++) {
            switch (_oImage.pixelFormat())
            {
                case ImageTex::FX8_RGB_PIXEL:
                case ImageTex::FX8_BGR_PIXEL:
                {
					pDst[y*imgheight+x*3+0] = pSrc[y*srcheight+x*3+0];
					pDst[y*imgheight+x*3+1] = pSrc[y*srcheight+x*3+0];
					pDst[y*imgheight+x*3+2] = pSrc[y*srcheight+x*3+0];
                }
                break;
                case ImageTex::FX8_RGBA_PIXEL:
                case ImageTex::FX8_BGRA_PIXEL:
				case ImageTex::FX8_YUYV_PIXEL:
				case ImageTex::FX8_UYVY_PIXEL:
                {
					pDst[y*imgheight+x*4+0] = pSrc[y*srcheight+x*3+0];
					pDst[y*imgheight+x*4+1] = pSrc[y*srcheight+x*3+1];
					pDst[y*imgheight+x*4+2] = pSrc[y*srcheight+x*3+2];
					pDst[y*imgheight+x*4+3] = 0xff;
                }
                break;
                default:
                    assert(false); // this method is only meant to handle FX8 pixels
			}
		}
	}
}


        // imprintPixelData
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
VideoPusher::imprintPixelData(void * pData, unsigned int nImprint)
{
    switch (_oImage.pixelFormat())
    {
        case ImageTex::FP16_RGB_PIXEL:
        case ImageTex::FP16_RGBA_PIXEL:
            imprintHalfPixelData(static_cast<half *>(pData), nImprint);
        break;
        case ImageTex::FX8_BGR_PIXEL:
        case ImageTex::FX8_BGRA_PIXEL:
        case ImageTex::FX8_RGB_PIXEL:
        case ImageTex::FX8_RGBA_PIXEL:
        case ImageTex::FX8_UYVY_PIXEL:
        case ImageTex::FX8_YUYV_PIXEL:
            imprint8bitPixelData(static_cast<unsigned char *>(pData), nImprint);
        break;
        default:
            assert(false); // fail if pixel format is not handled above
    }
}

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
VideoPusher::imprintHalfPixelData(half * pData, unsigned int nImprint)
{
                                // 32 bits in the imprint and 4 halfs for RGBA
                                // for RGB format only part of the array is used.
    half aBitPixels[cnSquareSize*32*4]; 
                                // set the memory to "black"
    memset(aBitPixels, 0, cnSquareSize*32*4*sizeof(half)); 
    
    unsigned int nImprintCopy = nImprint;
    for (int iBit = 0; iBit < 32; ++iBit, nImprintCopy /= 2)
    {
        if (nImprintCopy & 0x00000001)
        {
                                // loop only over cnSquareSize-1 pixels
                                // to leave a one pixel black diveder between
                                // squares.
            for (int iPixel = 0; iPixel < cnSquareSize-1; ++iPixel)
            {
                switch (_oImage.pixelFormat())
                {
                    case ImageTex::FP16_RGB_PIXEL:
                    {
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+0] = (half)1.0f;
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+1] = (half)1.0f;
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+2] = (half)1.0f;
                    }
                    break;
                    case ImageTex::FP16_RGBA_PIXEL: 
                    {
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+0] = (half)1.0f;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+1] = (half)1.0f;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+2] = (half)1.0f;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+3] = (half)1.0f;
                    }
                    break;
                    default:
                        assert(false); // this method is only meant to handle FP16 pixels
                }
            }
        }
    }
    
    for (int iPixel = 0; iPixel < cnSquareSize-1; ++iPixel)
    {
        switch (_oImage.pixelFormat())
        {
            case ImageTex::FP16_RGB_PIXEL:
            {
                memcpy(pData + iPixel*_oImage.width()*3, aBitPixels, cnSquareSize*32*3*sizeof(half));
            }
            break;
            case ImageTex::FP16_RGBA_PIXEL: 
            {
                memcpy(pData + iPixel*_oImage.width()*4, aBitPixels, cnSquareSize*32*4*sizeof(half));
            }
            break;
            default:
                assert(false); // this method is only meant to handle FP16 pixels
        }
    }
}

        // imprint8bitPixelData
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
VideoPusher::imprint8bitPixelData(unsigned char * pData, unsigned int nImprint)
{
    assert(sizeof(unsigned char) == 1);
    
                                // 32 bits in the imprint and 4 bytes for RGBA
                                // for RGB format only part of the array is used.
    unsigned char aBitPixels[cnSquareSize*32*4];
                                // set the memory contents to "black"
    memset(aBitPixels, 0, cnSquareSize*32*4*sizeof(unsigned char));
    
    unsigned int nImprintCopy = nImprint;
    for (int iBit = 0; iBit < 32; ++iBit, nImprintCopy /= 2)
    {
        if (nImprintCopy & 0x00000001)
        {
            for (int iPixel = 0; iPixel < cnSquareSize-1; ++iPixel)
            {
                switch (_oImage.pixelFormat())
                {
                    case ImageTex::FX8_RGB_PIXEL:
                    case ImageTex::FX8_BGR_PIXEL:
                    {
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+0] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+1] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*3+iPixel*3+2] = 0xFF;
                    }
                    break;
                    case ImageTex::FX8_RGBA_PIXEL:
                    case ImageTex::FX8_BGRA_PIXEL:
                    {
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+0] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+1] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+2] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*4+iPixel*4+3] = 0xFF;
                    }
                    break;
                    case ImageTex::FX8_UYVY_PIXEL:
                    case ImageTex::FX8_YUYV_PIXEL:
                    {
                        aBitPixels[iBit*cnSquareSize*2+iPixel*2+0] = 0xFF;
                        aBitPixels[iBit*cnSquareSize*2+iPixel*2+1] = 0xFF;
                    }
                    break;
                    default:
                        assert(false); // this method is only meant to handle FX8 pixels
                }
            }
        }
    }
    
    for (int iPixel = 0; iPixel < cnSquareSize-1; ++iPixel)
    {
        switch (_oImage.pixelFormat())
        {
            case ImageTex::FX8_RGB_PIXEL:
            case ImageTex::FX8_BGR_PIXEL:
            {
                memcpy(pData + iPixel*_oImage.width()*3, aBitPixels, cnSquareSize*32*3*sizeof(unsigned char));
            }
            break;
            case ImageTex::FX8_RGBA_PIXEL:
            case ImageTex::FX8_BGRA_PIXEL:
            {
                memcpy(pData + iPixel*_oImage.width()*4, aBitPixels, cnSquareSize*32*4*sizeof(unsigned char));
            }
            break;
            case ImageTex::FX8_UYVY_PIXEL:
            case ImageTex::FX8_YUYV_PIXEL:
            {
                memcpy(pData + iPixel*_oImage.width()*2, aBitPixels, cnSquareSize*32*2*sizeof(unsigned char));
            }
            break;
            default:
                assert(false); // this method is only meant to handle FP16 pixels
        }
    }

}



