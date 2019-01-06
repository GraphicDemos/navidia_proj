// ----------------------------------------------------------------------------
// 
// Content:
//      VideoPusherPBO class
//
// Description:
//      A class that manages Video streams through OpenGL pixel buffer objects (PBO).
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

#include "VideoPusherPBO.h"
#include "OpenEXRLoader.h"
#include "defines.h"

#include <assert.h>


#define USE_YUYV_32BPP     0
#define GL_FILTERING_MODE  GL_NEAREST


//
// Macros
//

#define min(v1,v2) (v1<v2) ? v1 : v2;
#define max(v1,v2) (v1>v2) ? v1 : v2;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// ----------------------------------------------------------------------------
// VideoPBO class
//

    //
    // Public data
    //
    
const char * VideoPusherPBO::ClassName = "VideoPusherPBO";
const char * VideoPusherPBO::ClassDescription = "Video (PBO)";

    //
    // Construction and destruction
    //

        // Default constructor
        //
VideoPusherPBO::VideoPusherPBO(): VideoPusher()
                                , _hPixelBuffer(0)
                                , _bAllocatedPBO(false)
{
	init();
}

        // Constructor for textures with different aspect ratios
        //
VideoPusherPBO::VideoPusherPBO(ImageSource::teTextureType eTextureType): VideoPusher(eTextureType)
                                , _hPixelBuffer(0)
                                , _bAllocatedPBO(false)
{
    init();
}

        // Destructor
        //
VideoPusherPBO::~VideoPusherPBO()
{
	cleanup();
}

void VideoPusherPBO::init()
{
	glewInit();

    glGenBuffersARB(1, &_hPixelBuffer);

#if DECODER_DOES_RENDER 
    _hDC[0] = wglGetCurrentDC();
    _hRC[0] = wglGetCurrentContext();
#endif
}

void VideoPusherPBO::cleanup()
{
    unbind();
    glDeleteBuffersARB(1, &_hPixelBuffer);
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
        //      rImage - const reference to image object.
        //
        // Returns:
        //      None
        //
        void
VideoPusherPBO::setImage(const ImageTex & rImage)
{
	imgsize._height = rImage.height();
	imgsize._width  = rImage.width();
    imgsize._bpp    = rImage.getPixelDataSize() * 8;
                                // create texture object and local image copy
    VideoPusher::setImage(rImage);

#if DECODER_DOES_RENDER
	wglMakeCurrent(_hDC[0], _hRC[0]);
#endif

    // bind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
                                // create pixel-buffer data container
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _oImage.imageDataSize(), NULL, GL_STREAM_DRAW_ARB);
    unsigned char * pPixelsPBO = static_cast<unsigned char *>(glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY));
                                // copy image data into the buffer
    memcpy(pPixelsPBO, _oImage.data(), _oImage.imageDataSize());
        
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // unbind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
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
VideoPusherPBO::pushNewFrame()
{
    Lock l(&_mutex);

#if DECODER_DOES_RENDER
    wglMakeCurrent(_hDC[0], _hRC[0]);
#endif

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
    
    void * pPixelData = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);

    memcpy(pPixelData, _oImage.data(), _oImage.imageDataSize());

    // here we can copy the data from the Video frame over to the PBO buffer
//  imprintPixelData(pPixelData, _nFrameCounter);
    
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // bind the texture object
    bind();

	glTexParameteri(_nTextureTarget, GL_TEXTURE_MIN_FILTER, GL_FILTERING_MODE);
	glTexParameteri(_nTextureTarget, GL_TEXTURE_MAG_FILTER, GL_FILTERING_MODE);

    // copy buffer contents into the texture
    glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
                    _oImage.glTextureFormat(), _oImage.glTextureType(), BUFFER_OFFSET(0));

    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    _nFrameCounter++;
    
    return _oImage.imageDataSize();
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
VideoPusherPBO::initVideoCopyContext()
{
#if DECODER_DOES_RENDER
    _hDC[1] = wglGetCurrentDC();
	_hDC[1] = _hDC[0];
    _hRC[1] = wglCreateContext( _hDC[1] );

	if(!wglShareLists(_hRC[0], _hRC[1])) {
        std::cerr << "wglShareLists() call failed!\n";
        return false;
	}
#endif
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
VideoPusherPBO::initVideoBuffers()
{
	initVideoCopyContext();

	wglMakeCurrent(_hDC[1], _hRC[1]);

    // bind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
                                // create pixel-buffer data container
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _oImage.imageDataSize(), NULL, GL_STREAM_DRAW_ARB);
    unsigned char * pPixelsPBO = static_cast<unsigned char *>(glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY));
                                // copy image data into the buffer
    memcpy(pPixelsPBO, _oImage.data(), _oImage.imageDataSize());
        
    if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
    {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }
                                // unbind pixel-buffer object
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
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
VideoPusherPBO::copyVideoFrame(void *pSrc, ImageSize imginfo)
{
    BYTE * pDest   = NULL, 
         * pSource = NULL;

	int x, xoffset  = 0;
    int y, yoffset  = 0;
    int clip_width  = min((int)_oImage.width(), (int)imginfo.width());
    int clip_height = min((int)_oImage.height(),(int)imginfo.height());

    Lock l(&_mutex);

#if DECODER_DOES_RENDER 
    wglMakeCurrent(_hDC[1], _hRC[1]);
#endif

    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _hPixelBuffer);
    
    void * pPixelData = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
//    void * pPixelData = NULL;

    pSource = (BYTE *)pSrc;

    if (pPixelData != NULL) {
        pDest = (BYTE *)pPixelData;
    } else {
        pDest = (BYTE *)_oImage.data();
    }

    if (imginfo.height() < (int)_oImage.height()) 
		yoffset = (_oImage.height() - imginfo.height());

    if (imginfo.width() < (int)_oImage.width())
        xoffset = (_oImage.width() - imginfo.width()) / 2;

    switch (_oImage.pixelFormat()) {
		case ImageTex::FX8_UYVY_PIXEL:
		case ImageTex::FX8_YUYV_PIXEL:
            memcpy(pDest, pSource, imginfo.imageDataSize());
            break;
        case ImageTex::FX8_RGB_PIXEL:     // 24bpp
        case ImageTex::FX8_BGR_PIXEL:
			if (_oImage.width() == imginfo.width()) {
				memcpy(pDest, pSource, clip_height*clip_width*3);
			} else {
				for (y=0; y < clip_height; y++) {
					memcpy(&pDest[((y+yoffset)*_oImage.width()+xoffset)*3], &pSource[y*imginfo.imageStride()], clip_width*3);
				}
			}
            break;
        case ImageTex::FX8_RGBA_PIXEL:    // 32bpp
        case ImageTex::FX8_BGRA_PIXEL:
            for (y=0; y < clip_height; y++) {
                for (x=0; x < clip_width; x++) {
                    pDest[((y+yoffset)*_oImage.width()+(x+xoffset))*4 + 0] = pSource[(y*imginfo.width()+x)*3 + 0];
                    pDest[((y+yoffset)*_oImage.width()+(x+xoffset))*4 + 1] = pSource[(y*imginfo.width()+x)*3 + 1];
                    pDest[((y+yoffset)*_oImage.width()+(x+xoffset))*4 + 2] = pSource[(y*imginfo.width()+x)*3 + 2];
                    pDest[((y+yoffset)*_oImage.width()+(x+xoffset))*4 + 3] = 0xff;
                }
            }
            break;
    }

    if (pPixelData != NULL) {
		if (!glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
		{
			std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
			assert(false);
		}

		// bind the texture object
		bind();

#if USE_YUYV_32BPP
		glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width()/2, _oImage.height(), 
						GL_BGRA, _oImage.glTextureType(), BUFFER_OFFSET(0));
#else
		// copy buffer contents into the texture
		glTexSubImage2D(_nTextureTarget, 0, 0, 0, _oImage.width(), _oImage.height(), 
						_oImage.glTextureFormat(), _oImage.glTextureType(), BUFFER_OFFSET(0));
#endif

		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	}

    return imginfo.imageDataSize();
}
