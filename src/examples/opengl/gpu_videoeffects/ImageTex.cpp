// ----------------------------------------------------------------------------
// 
// Content:
//      ImageTex class
//
// Description:
//      ImageTex container storing the pixel data, pixel format and image size.
//
// Author: Frank Jargstorff (10/25/2004)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include <GL/glew.h>
#include <GL/gl.h>

#include "ImageTex.h"

#include <assert.h>
#include <memory.h>
#include <memory>

#include <OpenEXR/half.h>
#include <math.h>


// ----------------------------------------------------------------------------
// Image class
//

    //
    // Construction and destruction
    //
            

        // Default constructor
        //
ImageTex::ImageTex(): _nWidth(0)
                , _nHeight(0)
                , _ePixelFormat(UNDEFINED_PIXEL)
                , _pData(0)
{
    ; // empty
}

        // Constructor
        //
        // Description:
        //      Construct an image from the given data.
        //
        // Parameters:
        //      nWidth - image width (in pixel).
        //      nHeight - image height (in pixel).
        //      ePixelFormat - pixel format descriptor.
        //      pData - pointer to the raw pixel data.
        //
ImageTex::ImageTex(unsigned int nWidth, unsigned int nHeight, tePixelFormat ePixelFormat, const void * pData)
                : _nWidth(nWidth)
                , _nHeight(nHeight)
                , _ePixelFormat(ePixelFormat)
                , _pData(0)
{ 
    assert(sizeof(char) == 1);
    size_t nDataSize = nWidth * nHeight * PixelDataSize(ePixelFormat);
    
    _pData = new unsigned char[nDataSize];
	if (pData) {
	    memcpy(_pData, pData, nDataSize);
	} else {
		memset(_pData, 0, nDataSize);
	}
}
        
        // Copy constructor
        //
        // Description:
        //      Make a deep copy of the image.
        //          Copies the image data into the new instance.
        //
        // Parameters:
        //      rImage - const reference to the input image.
        //
ImageTex::ImageTex(const ImageTex & rImage)
{
    _nWidth = rImage._nWidth;
    _nHeight = rImage._nHeight;
    _ePixelFormat = rImage._ePixelFormat;
    
    assert(sizeof(char) == 1);
    size_t nDataSize = _nWidth * _nHeight * PixelDataSize(_ePixelFormat);
    
    _pData = new unsigned char[nDataSize];
    assert(_pData);
    memcpy(_pData, rImage._pData, nDataSize);
}
        
        // Destructor
        //
        // Description:
        //      Delete the image data from the heap.
        //
ImageTex::~ImageTex()
{
    delete[] _pData;
}
   
   
    //
    // Public methods
    //
    
        // Assignment operator
        //
        // Description:
        //      Copy one image into an other.
        //
        // Parameters:
        //      rImage - a const reference to the source image.
        //
        // Returns:
        //      A reference to itself.
        //
        const 
        ImageTex &
ImageTex::operator = (const ImageTex & rImage)
{
                                // Test for self-assignment
    if (this == & rImage)
        return * this;
        
    
    _nWidth = rImage._nWidth;
    _nHeight = rImage._nHeight;
    _ePixelFormat = rImage._ePixelFormat;
    
    assert(sizeof(char) == 1);
    size_t nDataSize = _nWidth * _nHeight * PixelDataSize(_ePixelFormat);
    
                                // free existing memory
    delete[] _pData;
    _pData = new unsigned char[nDataSize];
    assert(_pData);
    memcpy(_pData, rImage._pData, nDataSize);
    
    return *this;
}

        // setData
        //
        // Description:
        //      Specify new image data.
        //         This operation makes a copy of the given data.
        //
        // Parameters:
        //      nWidth - image width (in pixel).
        //      nHeight - image height (in pixel).
        //      ePixelFormat - pixel format descriptor.
        //      pData - pointer to the raw image data.
        //
        // Returns:
        //      None
        //
        void
ImageTex::setData(unsigned int nWidth, unsigned int nHeight, tePixelFormat ePixelFormat, const void * pData)
{
                                // Sanity chek on the given data pointer
    assert(pData);
    
    _nWidth = nWidth;
    _nHeight = nHeight;
    _ePixelFormat = ePixelFormat;
    
    assert(sizeof(char) == 1);
    size_t nDataSize = _nWidth * _nHeight * PixelDataSize(_ePixelFormat);
    
                                // free existing memory
    delete[] _pData;
    _pData = new unsigned char[nDataSize];
    assert(_pData);
    memcpy(_pData, pData, nDataSize);
}

        
        // width
        //
        // Description:
        //      The image's width.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      Image width. If no image is loaded 0 is returned.
        //
        unsigned int
ImageTex::width()
        const
{
    return _nWidth;
}
        
        // height
        //
        // Description:
        //      The image's height.
        //
        // Parametesr:
        //      None
        //
        // Returns:
        //      Image height. If no image is loaded 0 is returned.
        //
        unsigned int
ImageTex::height()
        const
{
    return _nHeight;
}
        
        // imageSize
        //
        // Description:
        //      Get the image's size in pixel.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      Image size in pixels.
        //
        unsigned int
ImageTex::imageSize()
        const
{
    return _nWidth * _nHeight;
}
        
        // imageDataSize
        // 
        // Description:
        //      Get the image's size in bytes.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The image's size in bytes.
        //
        unsigned int
ImageTex::imageDataSize()
        const
{
    return imageSize() * PixelDataSize(_ePixelFormat);
}
        
        // data
        //
        // Description:
        //      Pointer to the internal data.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A pointer to the internal image data.
        //
        const
        void *
ImageTex::data()
        const
{
    return _pData;
}
            
        // data
        //
        // Description:
        //      Pointer to the internal data.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A pointer to the internal image data.
        //
        void *
ImageTex::data()
{
    return _pData;
}

        // pixelFormat
        //
        // Description:
        //      The image's pixel format.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The pixel format.
        //
        ImageTex::tePixelFormat
ImageTex::pixelFormat()
        const
{
    return _ePixelFormat;
}
        
        // pixelFormatString
        //
        // Description:
        //      String description of pixel format.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      Pixel format description string.
        //
        std::string
ImageTex::pixelFormatString()
        const
{
    switch (_ePixelFormat)
    {
        case FP16_RGBA_PIXEL:
            return "FP16 RGBA";
        case FP16_RGB_PIXEL:
            return "FP16 RGB";
        case FX8_RGBA_PIXEL:
            return "FX8 RGBA";
        case FX8_RGB_PIXEL:
            return "FX8_RGB";
        case FX8_BGRA_PIXEL:
            return "FX8_BGRA";
        case FX8_BGR_PIXEL:
            return "FX8_BGR";
		case FX8_YUYV_PIXEL:
            return "FX8_YUYV";
		case FX8_UYVY_PIXEL:
            return "FX8_UYVY";
        default:
            assert(false);
    }
    
    return "";

}
            
        // glInternalTextureFormat
        //
        // Description:
        //      Returns GL texture format.
        //          An image can specify an internal texture format
        //      that OpenGL should use when the image is bound as 
        //      a texture vie setTextureFormatGL call. If set to 
        //      UNKNOWN_PIXEL this method automatically returns an
        //      OpenGL internal format matching the image's storage
        //      format.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The GL internal texture format.
        //
        GLint
ImageTex::glInternalTextureFormat()
        const
{
    switch (_ePixelFormat)
    {
        case FP16_RGBA_PIXEL:
            return GL_FLOAT_RGBA16_NV;
        case FP16_RGB_PIXEL:
            return GL_FLOAT_RGB16_NV;
        case FX8_RGBA_PIXEL:
            return GL_RGBA;
        case FX8_RGB_PIXEL:
            return GL_RGB;
        case FX8_BGRA_PIXEL:
            return GL_RGBA;
        case FX8_BGR_PIXEL:
            return GL_RGB;
		case FX8_YUYV_PIXEL:
            return GL_LUMINANCE_ALPHA;
		case FX8_UYVY_PIXEL:
            return GL_LUMINANCE_ALPHA;
        default:
            assert(false);
    }

    return 0;
}
        
        // glTextureFormat
        //
        // Description:
        //      GL texture format matching the image's pixel format.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The GL texture format.
        //
        GLenum
ImageTex::glTextureFormat()
        const
{
    switch (_ePixelFormat)
    {
        case FP16_RGBA_PIXEL:
            return GL_RGBA;
        case FP16_RGB_PIXEL:
            return GL_RGB;
        case FX8_RGBA_PIXEL:
            return GL_RGBA;
        case FX8_RGB_PIXEL:
            return GL_RGB;
        case FX8_BGRA_PIXEL:
            return GL_BGRA;
        case FX8_BGR_PIXEL:
            return GL_BGR;
		case FX8_YUYV_PIXEL:
            return GL_LUMINANCE_ALPHA;
		case FX8_UYVY_PIXEL:
            return GL_LUMINANCE_ALPHA;
        default:
            assert(false);
    }
    
    return 0;
}
        
        // glTextureType
        //
        // Description:
        //      The GL datatype matching the image's pixel format.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The GL datatype.
        //
        GLenum
ImageTex::glTextureType()
        const
{
    switch (_ePixelFormat)
    {
        case FP16_RGBA_PIXEL:
            return GL_HALF_FLOAT_NV;
        case FP16_RGB_PIXEL:
            return GL_HALF_FLOAT_NV;
        case FX8_RGBA_PIXEL:
            return GL_UNSIGNED_BYTE;
        case FX8_RGB_PIXEL:
            return GL_UNSIGNED_BYTE;
        case FX8_BGRA_PIXEL:
            return GL_UNSIGNED_BYTE;
        case FX8_BGR_PIXEL:
            return GL_UNSIGNED_BYTE;
		case FX8_YUYV_PIXEL:
            return GL_UNSIGNED_BYTE;
		case FX8_UYVY_PIXEL:
            return GL_UNSIGNED_BYTE;
        default:
            assert(false);
    }
    
    return 0;
}


    //
    // Static public members
    //

        // getPixelDataSize
        //
        // Description:
        //      Size of a single pixel in bytes.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The pixel-format's pixel size in bytes.
        //
        unsigned int
ImageTex::getPixelDataSize()
        const
{
    switch (_ePixelFormat)
    {
        case FP16_RGB_PIXEL:
            return 6;
        case FP16_RGBA_PIXEL:
            return 8;
        case FX8_RGB_PIXEL:
            return 3;
        case FX8_RGBA_PIXEL:
            return 4;
        case FX8_BGR_PIXEL:
            return 3;
        case FX8_BGRA_PIXEL:
            return 4;
		case FX8_YUYV_PIXEL:
		case FX8_UYVY_PIXEL:
            return 2;
        default:
            assert(false);
    }
    return 0;
}


        // pixelDataSize
        //
        // Description:
        //      Size of a single pixel in bytes.
        //
        // Parameters:
        //      ePixelFormat - the pixel size will be given for this 
        //                     pixel format.
        //
        // Returns:
        //      The pixel-format's pixel size in bytes.
        //
       unsigned int
ImageTex::PixelDataSize(tePixelFormat ePixelFormat)
{
    switch (ePixelFormat)
    {
        case FP16_RGB_PIXEL:
            return 6;
        case FP16_RGBA_PIXEL:
            return 8;
        case FX8_RGB_PIXEL:
            return 3;
        case FX8_RGBA_PIXEL:
            return 4;
        case FX8_BGR_PIXEL:
            return 3;
        case FX8_BGRA_PIXEL:
            return 4;
		case FX8_YUYV_PIXEL:
		case FX8_UYVY_PIXEL:
            return 2;
        default:
            assert(false);
    }
    
    return 0;
}

        
        // ConvertPixelFormat
        //
        // Description:
        //      Make new image with different pixel format.
        // 
        // Parameters:
        //      tePixelFormat - the target pixel format.
        //      rImage        - const reference to the source image.
        //
        // Returns:
        //      A new image with the target pixel format.
        //
        ImageTex
ImageTex::ConvertPixelFormat(tePixelFormat ePixelFormat, const ImageTex & rImage)
{
    if (ePixelFormat == rImage._ePixelFormat)
        return ImageTex(rImage);
   
    switch (rImage._ePixelFormat)
    {
        case FP16_RGB_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGBA_PIXEL:
                    return FP16_RGB_to_FP16_RGBA(rImage);
                case FX8_RGB_PIXEL:
                    return FP16_RGBA_to_FX8_RGB(FP16_RGB_to_FP16_RGBA(rImage));
                case FX8_RGBA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(FP16_RGB_to_FP16_RGBA(rImage));
                case FX8_BGR_PIXEL:
                    return FP16_RGBA_to_FX8_BGR(FP16_RGB_to_FP16_RGBA(rImage));
                case FX8_BGRA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(FP16_RGB_to_FP16_RGBA(rImage));
                default:
                    assert(false);
            }
        case FP16_RGBA_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGB_PIXEL:
                    return FP16_RGBA_to_FP16_RGB(rImage);
                case FX8_RGB_PIXEL:
                    return FP16_RGBA_to_FX8_RGB(rImage);
                case FX8_RGBA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(rImage);
                case FX8_BGR_PIXEL:
                    return FP16_RGBA_to_FX8_BGR(rImage);
                case FX8_BGRA_PIXEL:
                    return FP16_RGBA_to_FX8_BGRA(rImage);
                default:
                    assert(false);
            }
        case FX8_RGB_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGB_PIXEL:
                    return FP16_RGBA_to_FP16_RGB(FX8_RGB_to_FP16_RGBA(rImage));
                case FP16_RGBA_PIXEL:
                    return FX8_RGB_to_FP16_RGBA(rImage);
                case FX8_RGBA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(FX8_RGB_to_FP16_RGBA(rImage));
                case FX8_BGR_PIXEL:
                    return FP16_RGBA_to_FX8_BGR(FX8_RGB_to_FP16_RGBA(rImage));
                case FX8_BGRA_PIXEL:
                    return FP16_RGBA_to_FX8_BGRA(FX8_RGB_to_FP16_RGBA(rImage));
                default:
                    assert(false);
            }
        case FX8_RGBA_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGB_PIXEL:
                    return FP16_RGBA_to_FP16_RGB(FX8_RGBA_to_FP16_RGBA(rImage));
                case FP16_RGBA_PIXEL:
                    return FX8_RGBA_to_FP16_RGBA(rImage);
                case FX8_RGB_PIXEL:
                    return FP16_RGBA_to_FX8_RGB(FX8_RGBA_to_FP16_RGBA(rImage));
                case FX8_BGR_PIXEL:
                    return FP16_RGBA_to_FX8_BGR(FX8_RGBA_to_FP16_RGBA(rImage));
                case FX8_BGRA_PIXEL:
                    return FP16_RGBA_to_FX8_BGRA(FX8_RGBA_to_FP16_RGBA(rImage));
                default:
                    assert(false);
            }
        case FX8_BGR_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGB_PIXEL:
                    return FP16_RGBA_to_FP16_RGB(FX8_BGR_to_FP16_RGBA(rImage));
                case FP16_RGBA_PIXEL:
                    return FX8_BGR_to_FP16_RGBA(rImage);
                case FX8_RGB_PIXEL:
                    return FP16_RGBA_to_FX8_RGB(FX8_BGR_to_FP16_RGBA(rImage));
                case FX8_RGBA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(FX8_BGR_to_FP16_RGBA(rImage));
                case FX8_BGRA_PIXEL:
                    return FP16_RGBA_to_FX8_BGRA(FX8_BGR_to_FP16_RGBA(rImage));
                default:
                    assert(false);
            }
        case FX8_BGRA_PIXEL:
            switch (ePixelFormat)
            {
                case FP16_RGB_PIXEL:
                    return FP16_RGBA_to_FP16_RGB(FX8_BGRA_to_FP16_RGBA(rImage));
                case FP16_RGBA_PIXEL:
                    return FX8_BGRA_to_FP16_RGBA(rImage);
                case FX8_RGB_PIXEL:
                    return FP16_RGBA_to_FX8_RGB(FX8_BGRA_to_FP16_RGBA(rImage));
                case FX8_RGBA_PIXEL:
                    return FP16_RGBA_to_FX8_RGBA(FX8_BGRA_to_FP16_RGBA(rImage));
                case FX8_BGR_PIXEL:
                    return FP16_RGBA_to_FX8_BGR(FX8_BGRA_to_FP16_RGBA(rImage));
                default:
                    assert(false);
            }
//        case FX8_L8A8_PIXEL:
//            return FX8_L8A8(rImage);
        default:
            assert(false);
        
        return ImageTex();
    } 
}


    //
    // Protected methods
    //

        inline
        unsigned char
ImageTex::HalfToByte(half nValue)
{
    unsigned int nInteger = static_cast<unsigned int>(nValue * 255.0f + .5f);
    if (nInteger > 255)
        nInteger = 255;

    return static_cast<unsigned char>(nInteger);
}

        inline
        half
ImageTex::ByteToHalf(unsigned char nValue)
{
    return half(nValue * 255.0f);
}

       ImageTex
ImageTex::FP16_RGB_to_FP16_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGB_PIXEL);

    const half * pRGB = static_cast<const half *>(rImage._pData);
    assert(pRGB);
    const half * pEndOfRGB = pRGB + rImage.imageSize() * PixelDataSize(FP16_RGB_PIXEL) / 2;
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGBA = pNewData.get(); pRGB < pEndOfRGB; )
    {
        *pRGBA++ = *pRGB++; // copy red component
        *pRGBA++ = *pRGB++; // copy green component
        *pRGBA++ = *pRGB++; // copy blue component
        *pRGBA++ = 1.0f;    // fill 1.0 into alpha channel
    }
       
    return ImageTex(rImage.width(), rImage.height(), FP16_RGBA_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FP16_RGBA_to_FP16_RGB(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGBA_PIXEL);

    const half * pRGBA = static_cast<const half *>(rImage._pData);
    assert(pRGBA);
    const half * pEndOfRGBA = pRGBA + rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2;
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGB_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGB = pNewData.get(); pRGBA < pEndOfRGBA; )
    {
        *pRGB++ = *pRGBA++; // copy red component
        *pRGB++ = *pRGBA++; // copy green component
        *pRGB++ = *pRGBA++; // copy blue component
                   pRGBA++; // skip alpha channel
    }
       
    return ImageTex(rImage.width(), rImage.height(), FP16_RGB_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FP16_RGBA_to_FX8_RGB(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGBA_PIXEL);

    const half * pFP16RGBA = static_cast<const half *>(rImage._pData);
    assert(pFP16RGBA);
    const half * pEndOfFP16RGBA = pFP16RGBA + rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2;
    
    std::auto_ptr<unsigned char> pNewData(new unsigned char[rImage.imageSize() * PixelDataSize(FX8_RGB_PIXEL)]);
    assert(pNewData.get());
    
    for (unsigned char * pRGB = pNewData.get(); pFP16RGBA < pEndOfFP16RGBA; )
    {
        *pRGB++ = HalfToByte(*pFP16RGBA++); // copy red component
        *pRGB++ = HalfToByte(*pFP16RGBA++); // copy green component
        *pRGB++ = HalfToByte(*pFP16RGBA++); // copy blue component
                              pFP16RGBA++;  // skip alpha channel
    }
       
    return ImageTex(rImage.width(), rImage.height(), FX8_RGB_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FP16_RGBA_to_FX8_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGBA_PIXEL);

    const half * pFP16RGBA = static_cast<const half *>(rImage._pData);
    assert(pFP16RGBA);
    const half * pEndOfFP16RGBA = pFP16RGBA + rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2;
    
    std::auto_ptr<unsigned char> pNewData(new unsigned char[rImage.imageSize() * PixelDataSize(FX8_RGBA_PIXEL)]);
    assert(pNewData.get());
    
    for (unsigned char * pRGBA = pNewData.get(); pFP16RGBA < pEndOfFP16RGBA; )
    {
        *pRGBA++ = HalfToByte(*pFP16RGBA++); // copy red component
        *pRGBA++ = HalfToByte(*pFP16RGBA++); // copy green component
        *pRGBA++ = HalfToByte(*pFP16RGBA++); // copy blue component
        *pRGBA++ = HalfToByte(*pFP16RGBA++); // skip alpha channel
    }
       
    return ImageTex(rImage.width(), rImage.height(), FX8_RGBA_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FP16_RGBA_to_FX8_BGR(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGBA_PIXEL);

    const half * pFP16RGBA = static_cast<const half *>(rImage._pData);
    assert(pFP16RGBA);
    const half * pEndOfFP16RGBA = pFP16RGBA + rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2;
    
    std::auto_ptr<unsigned char> pNewData(new unsigned char[rImage.imageSize() * PixelDataSize(FX8_BGR_PIXEL)]);
    assert(pNewData.get());
    
    for (unsigned char * pBGR = pNewData.get(); pFP16RGBA < pEndOfFP16RGBA; )
    {
         pBGR += 2;
        *pBGR-- = HalfToByte(*pFP16RGBA++); // copy red component
        *pBGR-- = HalfToByte(*pFP16RGBA++); // copy green component
        *pBGR   = HalfToByte(*pFP16RGBA++); // copy blue component
         pBGR += 3;
                              pFP16RGBA++;  // skip the 16bit alpha component
    }
       
    return ImageTex(rImage.width(), rImage.height(), FX8_BGR_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FP16_RGBA_to_FX8_BGRA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FP16_RGBA_PIXEL);

    const half * pFP16RGBA = static_cast<const half *>(rImage._pData);
    assert(pFP16RGBA);
    const half * pEndOfFP16RGBA = pFP16RGBA + rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2;
    
    std::auto_ptr<unsigned char> pNewData(new unsigned char[rImage.imageSize() * PixelDataSize(FX8_BGRA_PIXEL)]);
    assert(pNewData.get());
    
    for (unsigned char * pBGRA = pNewData.get(); pFP16RGBA < pEndOfFP16RGBA; )
    {
         pBGRA += 2;
        *pBGRA-- = HalfToByte(*pFP16RGBA++);; // copy red component
        *pBGRA-- = HalfToByte(*pFP16RGBA++); // copy green component
        *pBGRA   = HalfToByte(*pFP16RGBA++); // copy blue component
         pBGRA += 3;
        *pBGRA++ = HalfToByte(*pFP16RGBA++); // skip alpha channel
    }
       
    return ImageTex(rImage.width(), rImage.height(), FX8_BGRA_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FX8_RGB_to_FP16_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FX8_RGB_PIXEL);

    const unsigned char * pFX8RGB = static_cast<const unsigned char *>(rImage._pData);
    assert(pFX8RGB);
    const unsigned char * pEndOfFX8RGB = pFX8RGB + rImage.imageSize() * PixelDataSize(FX8_RGB_PIXEL);
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGBA = pNewData.get(); pFX8RGB < pEndOfFX8RGB; )
    {
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = 1.0f;
    }
       
    return ImageTex(rImage.width(), rImage.height(), FP16_RGBA_PIXEL, pNewData.get());
}

        ImageTex
ImageTex::FX8_RGBA_to_FP16_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FX8_RGBA_PIXEL);

    const unsigned char * pFX8RGB = static_cast<const unsigned char *>(rImage._pData);
    assert(pFX8RGB);
    const unsigned char * pEndOfFX8RGB = pFX8RGB + rImage.imageSize() * PixelDataSize(FX8_RGB_PIXEL);
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGBA = pNewData.get(); pFX8RGB < pEndOfFX8RGB; )
    {
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
        *pRGBA++ = (*pFX8RGB++) / 255.0f;
    }
       
    return ImageTex();
}

        ImageTex
ImageTex::FX8_BGR_to_FP16_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FX8_BGR_PIXEL);

    const unsigned char * pFX8RGB = static_cast<const unsigned char *>(rImage._pData);
    assert(pFX8RGB);
    const unsigned char * pEndOfFX8RGB = pFX8RGB + rImage.imageSize() * PixelDataSize(FX8_RGB_PIXEL);
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGBA = pNewData.get(); pFX8RGB < pEndOfFX8RGB; )
    {
        pFX8RGB += 2;
        *pRGBA++ = (*pFX8RGB--) / 255.0f;
        *pRGBA++ = (*pFX8RGB--) / 255.0f;
        *pRGBA++ = (*pFX8RGB  ) / 255.0f;
        pFX8RGB += 3;
        *pRGBA++ = 1.0f;
    }
       
    return ImageTex();
}

        ImageTex
ImageTex::FX8_BGRA_to_FP16_RGBA(const ImageTex & rImage)
{
    assert(rImage._ePixelFormat == FX8_BGRA_PIXEL);

    const unsigned char * pFX8RGB = static_cast<const unsigned char *>(rImage._pData);
    assert(pFX8RGB);
    const unsigned char * pEndOfFX8RGB = pFX8RGB + rImage.imageSize() * PixelDataSize(FX8_RGB_PIXEL);
    
    std::auto_ptr<half> pNewData(new half[rImage.imageSize() * PixelDataSize(FP16_RGBA_PIXEL) / 2]);
    assert(pNewData.get());
    
    for (half * pRGBA = pNewData.get(); pFX8RGB < pEndOfFX8RGB; )
    {
        pFX8RGB += 2;
        *pRGBA++ = (*pFX8RGB--) / 255.0f;
        *pRGBA++ = (*pFX8RGB--) / 255.0f;
        *pRGBA++ = (*pFX8RGB  ) / 255.0f;
        pFX8RGB += 3;
        *pRGBA++ = (*pFX8RGB  ) / 255.0f;
    }
       
    return ImageTex();
}

 