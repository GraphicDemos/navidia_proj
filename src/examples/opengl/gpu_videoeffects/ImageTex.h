#ifndef IMAGETEX_H
#define IMAGETEX_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImageTex class
//
// Description:
//      ImageTex container storing the pixel data, pixel format and image size.
//
// Author: Frank Jargstorff (10/25/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/glew.h>
#include <gl/gl.h>

#include <OpenEXR/half.h>


class ImageSize
{
public:
    int imageDataSize() { return _width*_height*_bpp/8; }
    int imageStride()   { return _width*_bpp/8; }
	int width()			{ return _width;  }
	int height()		{ return _height; }

public:
    int _width;
    int _height;
    int _bpp;
};



// ----------------------------------------------------------------------------
// ImageTex class
//
class ImageTex
{
public:
    //
    // Public data types
    //
    
    enum tePixelFormat
    {
        FP16_RGB_PIXEL,
        FP16_RGBA_PIXEL,
        FX8_RGB_PIXEL,
        FX8_RGBA_PIXEL,
        FX8_BGR_PIXEL,
        FX8_BGRA_PIXEL,
        FX8_YUYV_PIXEL,
        FX8_UYVY_PIXEL,
        UNDEFINED_PIXEL
    };
    
    //
    // Construction and destruction
    //
            
            // Default constructor
            //
    ImageTex();
            
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
    ImageTex(unsigned int nWidth, unsigned int nHeight, ImageTex::tePixelFormat ePixelFormat, const void * pData);
            
            // Copy constructor
            //
            // Description:
            //      Make a deep copy of the image.
            //          Copies the image data into the new instance.
            //
            // Parameters:
            //      rImage - const reference to the input image.
            //
    ImageTex(const ImageTex & rImage);
            
            // Destructor
            //
            // Description:
            //      Delete the image data from the heap.
            //
            virtual
   ~ImageTex();
   
   
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
    operator = (const ImageTex & rImage);
    
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
    setData(unsigned int nWidth, unsigned int nHeight, tePixelFormat ePixelFormat, const void * pData);
    
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
    width()
            const;
            
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
    height()
            const;
            
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
    imageSize()
            const;
            
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
    imageDataSize()
            const;
            
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
    data()
            const;
            
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
    data();

            
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
            tePixelFormat
    pixelFormat()
            const;
            
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
    pixelFormatString()
            const;
            
            // imprintPixelData
            //
            // Description:
            //      Imprints binary pattern of a uint in lower left corner.
            //
            // Parameters:
            //      pPixelData - an array of half representing the image.
            //      nImpring   - the number to imprint
            //
            // Returns:
            //      None
            //
            void
    imprintPixelData(unsigned int nImprint);
   
            // glInternalTextureFormat
            //
            // Description:
            //      Returns GL texture format.
            //      An image can specify an internal texture format
            //      that OpenGL should use when the image is bound as 
            //      a texture.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The GL internal texture format.
            //
            GLint
    glInternalTextureFormat()
            const;
            
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
    glTextureFormat()
            const;
            
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
    glTextureType()
            const;


            // getpixelDataSize
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
    getPixelDataSize()
            const;

    //
    // Static public members
    //

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
            static
            unsigned int
    PixelDataSize(tePixelFormat ePixelFormat);
           
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
            static
            ImageTex
    ConvertPixelFormat(tePixelFormat ePixelFormat, const ImageTex & rImage);
                  
                  
protected:
    //
    // Protected methods
    //
    
            static
            unsigned char
    HalfToByte(half nValue);
    
            static
            half
    ByteToHalf(unsigned char nValue);
    
            static
            ImageTex
    FP16_RGB_to_FP16_RGBA(const ImageTex & rImage);
    
            static 
            ImageTex
    FP16_RGBA_to_FP16_RGB(const ImageTex & rImage);
    
            static 
            ImageTex
    FP16_RGBA_to_FX8_RGB(const ImageTex & rImage);
    
            static 
            ImageTex
    FP16_RGBA_to_FX8_RGBA(const ImageTex & rImage);
    
            static 
            ImageTex
    FP16_RGBA_to_FX8_BGR(const ImageTex & rImage);
    
            static
            ImageTex
    FP16_RGBA_to_FX8_BGRA(const ImageTex & rImage);
    
            static
            ImageTex
    FX8_RGB_to_FP16_RGBA(const ImageTex & rImage);
    
            static
            ImageTex
    FX8_RGBA_to_FP16_RGBA(const ImageTex & rImage);
    
          static
            ImageTex
    FX8_BGR_to_FP16_RGBA(const ImageTex & rImage);
    
            static
            ImageTex
    FX8_BGRA_to_FP16_RGBA(const ImageTex & rImage);
    
    //
    // Protected data
    //
    
    unsigned int    _nWidth;
    unsigned int    _nHeight;  
    
    tePixelFormat   _ePixelFormat;
    
    void          * _pData;  
};

#endif // IMAGETEX_H
