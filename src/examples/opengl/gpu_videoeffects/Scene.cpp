// ----------------------------------------------------------------------------
// 
// Content:
//      Scene class
//
// Description:
//      Class representing the different elements of the scene drawn.
//      I.e. the image, the transfer performance information, logo, etc.
//
// Author: Frank Jargstorff (03/17/04)
//
// Note:
//      Copyright (C) by 2004 NVIDIA Croporartion. All right reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/wglew.h>
#include <GL/glut.h>

#include "defines.h"
#include "Scene.h"

#include "ImageSource.h"
#include "ImageSink.h"
#include "ApplicationInfo.h"
#include "ShaderCg.h"
#include "ShaderGLSL.h"
#include "ProgramGLSL.h"
#include "ProgramCg.h"
#include "InteractionController.h"
#include "Timer.h"

#include "NV_image_processing.h"


#include <sstream>
#include <iomanip>

#include <assert.h>
#include <AssertCG.h>
#include <AssertGL.h>


// ----------------------------------------------------------------------------
// Scene class
//

nv::Timer goTimer2(100);

    //
    // Construction and destruction
    //

        // Default constructor
        //
Scene::Scene(): _nWindowWidth(0)
              , _nWindowHeight(0)
              , _pImageSource(0)
              , _nImagePositionX(0)
              , _nImagePositionY(0)
              , _nZoomFactor(1.0f)
              , _nBrightness(1.0f)
              , _pImageSink(0)
              , _pApplicationInfo(0)
			  , _pInteractionController(0)
			  , _pShaderCg(0)
			  , _pShaderGLSL(0)
			  , _pProgramCg(0)
			  , _pProgramGLSL(0)
              , _bInvertTexCoords(false)
              , _bNeedUpdate(false)
              , _bUseGLSL(false)
              , _bUseNVImage(false)
			 // , _bUseFBO(false)
{
    initCg();
}

        // Constructor 1
        //
        // Description:
        //      Creates a ready to render setup.
        //
        // Parameters:
        //      nWindowWidth  - width  of the OpenGL window.
        //      nWindowHeight - height of the OpenGL window.
        //      pImageSource  - point to the image pusher.
        //      pImageSink - pointer to the image puller.
        //      
Scene::Scene(unsigned int nWindowWidth, unsigned int nWindowHeight, 
             ImageSource * pImageSource, ImageSink * pImageSink,
             ApplicationInfo * pApplicationInfo, bool bInvertUV = FALSE)
            : _nWindowWidth(nWindowWidth)
            , _nWindowHeight(nWindowHeight)
            , _pImageSource(pImageSource)
            , _nImagePositionX(0)
            , _nImagePositionY(0)
            , _nZoomFactor(1.0f)
            , _nBrightness(1.0f)
            , _pImageSink(pImageSink)
            , _pApplicationInfo(pApplicationInfo)
			, _pInteractionController(0)
			, _pShaderCg(0)
			, _pShaderGLSL(0)
			, _pProgramCg(0)
			, _pProgramGLSL(0)
            , _bInvertTexCoords(bInvertUV)
            , _bNeedUpdate(true)
            , _bUseGLSL(false)
            , _bUseNVImage(false)
{
    initCg();

    cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
    CG_ASSERT_NO_ERROR;
    cgGLBindProgram(ShaderManager::gVertexIdentityShader);
    CG_ASSERT_NO_ERROR;
}

        // Destructor
Scene::~Scene()
{
//    goTimer2.stop();
}
   
   
   //
   // Public methods
   //

            // init the Cg functions
            //
            // Description:
            //      This initializes all the Cg display routines
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
        void
Scene::initCg()
{
                                // Set up the fragment program
    _oCgFragmentProgram = cgCreateProgramFromFile(ShaderManager::gCgContext, CG_SOURCE, 
                                               SHADER_PATH "gpu_videoeffects/texture.cg", 
                                                 CG_PROFILE_FP30, 0, 0);
    cgGLLoadProgram(_oCgFragmentProgram);
    cgGLEnableProfile(CG_PROFILE_FP30);
    cgGLBindProgram(_oCgFragmentProgram);

    _hImage          = cgGetNamedParameter(_oCgFragmentProgram,  "oImage"    );
    CG_ASSERT_NO_ERROR;

//    goTimer2.start();
}

            // init the Interaction Controller
            //
            // Description:
            //      This function passes the Interaction Controller
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
        void
Scene::initController(InteractionController *pController)
{
	_pInteractionController = pController;	
}

		// initVideoBuffers
        //
        // Description:
        //      This will be a callback function for the TextureRenderer 
        //      to copy the Video Frame over to the Scene Renderer
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
Scene::initVideoBuffers()
{
    _pImageSource->initVideoBuffers();
}

        // initVideoCopyContext
        //
        // Description:
        //      This will be a callback function for the TextureRenderer 
        //      to copy the Video Frame over to the Scene Renderer
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
Scene::initVideoCopyContext()
{
    _pImageSource->initVideoCopyContext();
}

        // copyVideoFrame
        //
        // Description:
        //      This will be a callback function for the TextureRenderer 
        //      to copy the Video Frame over to the Scene Renderer
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
Scene::copyVideoFrame(BYTE *source, ImageSize imginfo)
{
    _pImageSource->copyVideoFrame(source, imginfo);
}


        // render
        //
        // Description:
        //      Renders the scene depending on bUseGLSL flag
        //          This call issues the OpenGL call to render the
        //      scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        double
Scene::render()
{
    double time;

    if (_bUseGLSL) {
        time = renderGLSL();
	} else {
		if (!_bUseNVImage) {
	        time = renderCg();
		} else {
			time = renderNVImageProc();
		}
    }
    return time;
}

        // renderNVImageProc
        //
        // Description:
        //      Renders the scene with the NV Imaging Library
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        double
Scene::renderNVImageProc()
{
    assert(_pImageSource != 0);
    unsigned int nDownloadedBytes = _pImageSource->pushNewFrame();
    float nX = 0.0f, nY = 0.0f;

    float nImageWidth  = static_cast<float>(_pImageSource->image().width());
    float nImageHeight = static_cast<float>(_pImageSource->image().height());

    if (nImageWidth < MIN_WIDTH) {
        nX = (_nWindowWidth - nImageWidth) / 2;
    }
    if (nImageHeight < (_nWindowHeight - _pApplicationInfo->height())) {
        nY = (_nWindowHeight - _pApplicationInfo->height() - nImageHeight) / 2;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);       
    glClear(GL_COLOR_BUFFER_BIT);

#if 1
    if (false) {
        //_oImageFBO  = _pNVImageFilter->imageFBO();
    } else {
        _oImage     = _pNVImageFilter->image();
    }
#else
    _oImage = _pNVImageLoader->image();
#endif

    glViewport(0, 0, (GLsizei) _nWindowWidth, (GLsizei) _nWindowHeight - _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0,  _nWindowHeight - _pApplicationInfo->height());

	cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
    cgGLBindProgram(ShaderManager::gVertexIdentityShader);

    cgGLEnableProfile(CG_PROFILE_FP30);
	cgGLBindProgram(_oCgFragmentProgram);

    if (_pInteractionController)
        _pInteractionController->updateUniforms();

    glEnable(GL_FRAGMENT_PROGRAM_NV);

#if 0
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, (_bUseFBO ? _oImageFBO.textureID() : _oImage.textureID()));
    glEnable(GL_TEXTURE_RECTANGLE_NV);
#else
	//cgGLSetTextureParameter(_hImage, (_bUseFBO ? _oImageFBO.textureID() : _oImage.textureID()));
	cgGLSetTextureParameter(_hImage, (_oImage.textureID()));
    cgGLEnableTextureParameter(_hImage);
#endif
    cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                CG_GL_MATRIX_IDENTITY);

    // draw the image as a textured quad
    glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);                    glVertex2f(nX,             (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, 0.0);            glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, nImageHeight);   glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
        glTexCoord2f (0.0, nImageHeight);           glVertex2f(nX,             (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
    glEnd ();

#if 0
    glDisable(GL_TEXTURE_RECTANGLE_NV);
#else
    cgGLDisableTextureParameter(_hImage);
#endif

    cgGLDisableProfile(ShaderManager::gVertexIdentityProfile);
    cgGLDisableProfile(CG_PROFILE_FP30);

    assert(_pApplicationInfo != 0);
    _pApplicationInfo->setBytesDownloaded(nDownloadedBytes);

    glDisable(GL_FRAGMENT_PROGRAM_NV);

    // draw the GUI elements
    glViewport(0, _nWindowHeight - _pApplicationInfo->height(), _nWindowWidth,  _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0, _pApplicationInfo->height());
    _pApplicationInfo->render(0, 0);

    // Now render the slider bars
    glViewport(0, 0, _nWindowWidth, _nWindowHeight);
    if (_pInteractionController)
		_pInteractionController->renderSliders(_pApplicationInfo->width(), 0);

    glutSwapBuffers();

    assert(_pImageSink != 0);
                                // readback 
    unsigned int nReadBytes = _pImageSink->pull(0, 0);
                                // number of bytes read back will be displayed 
                                // in successive frame
    _pApplicationInfo->setBytesRead(nReadBytes);

    return 0.0;
}

		// renderCg
        //
        // Description:
        //      Renders the scene with Cg shaders
        //          This call issues the OpenGL call to render the
        //      scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        double
Scene::renderCg()
{        
    assert(_pImageSource != 0);
    unsigned int nDownloadedBytes = _pImageSource->pushNewFrame();

    float nX = 0.0f, nY = 0.0f;

    float nImageWidth  = static_cast<float>(_pImageSource->image().width());
    float nImageHeight = static_cast<float>(_pImageSource->image().height());

    if (nImageWidth < MIN_WIDTH) {
        nX = (_nWindowWidth - nImageWidth) / 2;
    }
    if (nImageHeight < (_nWindowHeight - _pApplicationInfo->height())) {
        nY = (_nWindowHeight - _pApplicationInfo->height() - nImageHeight) / 2;
    }

    glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, (GLsizei) _nWindowWidth, (GLsizei) _nWindowHeight - _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0,  _nWindowHeight - _pApplicationInfo->height());

    assert(_pProgramCg != 0);
	_pProgramCg->setWindowSize(_nWindowWidth, _nWindowHeight);
    _pProgramCg->setTextureSize( (int)nImageWidth, (int)nImageHeight, 0 );

    _pProgramCg->bind();

    if (_pInteractionController)
        _pInteractionController->updateUniforms();

    glEnable(GL_FRAGMENT_PROGRAM_NV);
    glEnable(GL_TEXTURE_RECTANGLE_NV);

    // draw the image as a textured quad
    glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);                    glVertex2f(nX,             (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, 0.0);            glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, nImageHeight);   glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
        glTexCoord2f (0.0, nImageHeight);           glVertex2f(nX,             (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
    glEnd ();

    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_FRAGMENT_PROGRAM_NV);

    _pProgramCg->unbind();

    assert(_pApplicationInfo != 0);
    _pApplicationInfo->setBytesDownloaded(nDownloadedBytes);

    // draw the GUI elements
    glViewport(0, _nWindowHeight - _pApplicationInfo->height(), _nWindowWidth,  _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0, _pApplicationInfo->height());
    _pApplicationInfo->render(0, 0);

    // Now Render the UI slider bars
    glViewport(0, 0, _nWindowWidth, _nWindowHeight);
    if (_pInteractionController)
		_pInteractionController->renderSliders(_pApplicationInfo->width(), 0);

    glutSwapBuffers();

    assert(_pImageSink != 0);
                                // readback 
    unsigned int nReadBytes = _pImageSink->pull(0, 0);
                                // number of bytes read back will be displayed 
                                // in successive frame
    _pApplicationInfo->setBytesRead(nReadBytes);

    return 0.0;
}


        // renderGLSL
        //
        // Description:
        //      Renders the scene with GLSL shaders
        //          This call issues the OpenGL call to render the
        //      scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        double
Scene::renderGLSL()
{
    assert(_pImageSource != 0);
    unsigned int nDownloadedBytes = _pImageSource->pushNewFrame();

    float nX = 0.0f, nY = 0.0f;

    float nImageWidth  = static_cast<float>(_pImageSource->image().width());
    float nImageHeight = static_cast<float>(_pImageSource->image().height());

    if (nImageWidth < MIN_WIDTH) {
        nX = (_nWindowWidth - nImageWidth) / 2;
    }
    if (nImageHeight < (_nWindowHeight - _pApplicationInfo->height())) {
        nY = (_nWindowHeight - _pApplicationInfo->height() - nImageHeight) / 2;
    }

    double timer = 0.0;

    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, (GLsizei) _nWindowWidth, (GLsizei) _nWindowHeight - _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0,  _nWindowHeight - _pApplicationInfo->height());

    assert(_pProgramGLSL != 0);
    _pProgramGLSL->setWindowSize(_nWindowWidth, _nWindowHeight);
    _pProgramGLSL->setTextureSize( (int)nImageWidth, (int)nImageHeight, 0 );

    _pProgramGLSL->bind();  // bind our vertex/pixel shader program

    if (_pInteractionController)
        _pInteractionController->updateUniforms();

    // draw the image as a textured quad
    glEnable (GL_FRAGMENT_PROGRAM_NV);
    glEnable (GL_TEXTURE_RECTANGLE_NV);

    // draw the image as a textured quad
    glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);                    glVertex2f(nX,             (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, 0.0);            glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? nImageHeight : 0.0) + nY);
        glTexCoord2f (nImageWidth, nImageHeight);   glVertex2f(nImageWidth+nX, (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
        glTexCoord2f (0.0, nImageHeight);           glVertex2f(nX,             (_bInvertTexCoords ? 0.0 : nImageHeight) + nY);
    glEnd ();

    glDisable (GL_TEXTURE_RECTANGLE_NV);
    glDisable (GL_FRAGMENT_PROGRAM_NV);

    _pProgramGLSL->unbind();

    assert(_pApplicationInfo != 0);
    _pApplicationInfo->setBytesDownloaded(nDownloadedBytes);

    // draw the GUI elements
    glViewport(0, _nWindowHeight - _pApplicationInfo->height(), _nWindowWidth,  _pApplicationInfo->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0, _pApplicationInfo->height());
    _pApplicationInfo->render(0, 0);

    // Now render the slider bars
    glViewport(0, 0, _nWindowWidth, _nWindowHeight);
	if (_pInteractionController)
		_pInteractionController->renderSliders(_pApplicationInfo->width(), 0);

	glutSwapBuffers();

    assert(_pImageSink != 0);
                                // readback 
    unsigned int nReadBytes = _pImageSink->pull(0, 0);
                                // number of bytes read back will be displayed 
                                // in successive frame
    _pApplicationInfo->setBytesRead(nReadBytes);

    return timer;
}

        
        // setImageSource
        //
        // Description:
        //      Sets the image-source object uploading and binding
        //      the renderable texture.
        //
        // Parameters:
        //      pImageSource - the render image.
        //
        // Returns:
        //      None
        //
        void
Scene::setImageSource(ImageSource * pImageSource)
{
    _pImageSource = pImageSource;
}

        // setNVImageFilter
        //
        // Description:
        //      Sets up the NVImageFilter to load the filter and images
        //
        // Parameters:
        //      pImageFilter - pointer to the NVImageFilter input filter.
        //      pImageLoader - image loader operations
        //
        // Returns:
        //      None
        //
        void
Scene::setNVImageFilter(ImageFilter* pImageFilter, NVImageLoader *pImageLoader)
{
	enableNVImage(true);
	enableGLSL(false);
    _pNVImageFilter = pImageFilter;
    _pNVImageLoader = pImageLoader;

	_pNVImageFilter->setSourceOperator(_pNVImageLoader);
}


		// setShaderCg
        //
        // Description:
        //      Sets the shader for the input image.
        //
        // Parameters:
        //      pShader - pointer to the shader.
        //
        // Returns:
        //      None
        //
        void
Scene::setShaderCg(ShaderCg * pShader)
{
	enableNVImage(false);
	enableGLSL(false);
    _pShaderCg = pShader;
    _pShaderCg->bind();
}
           
        // setProgramCg
        //
        // Description:
        //      Sets the Cg shaders for the input image.
        //
        // Parameters:
        //      pShader - pointer to the Cg shaders.
        //
        // Returns:
        //      None
        //
        void
Scene::setProgramCg(ProgramCg * pProgram)
{
	enableNVImage(false);
	enableGLSL(false);
    _pProgramCg = pProgram;
    _pProgramCg->bind();
}


        // setProgramGLSL
        //
        // Description:
        //      Sets the shader for the input image.
        //
        // Parameters:
        //      pShader - pointer to the shader.
        //
        // Returns:
        //      None
        //
        void
Scene::setProgramGLSL(ProgramGLSL * pProgram)
{
	enableNVImage(false);
	enableGLSL(true);
    _pProgramGLSL = pProgram;
    _pProgramGLSL->bind();
}


        // setImageSink
        //
        // Description:
        //      Sets the image-sink object reading the frame buffer.
        //
        // Parameters:
        //      pImageSink - the image-sink.
        //
        // Returns:
        //      None
        //
        void
Scene::setImageSink(ImageSink * pImageSink)
{
    _pImageSink = pImageSink;
}

        // setApplicationInfo
        //
        // Description:
        //      Sets the application info object displaying performance
        //      information.
        //
        // Parameters:
        //      pApplicationInfo - the application info object.
        //
        // Returns:
        //      None
        //
        void
Scene::setApplicationInfo(ApplicationInfo * pApplicationInfo)
{
    _pApplicationInfo = pApplicationInfo;
}
            
        // minimumWindowWidth
        //
        // Description:
        //      Get the minimum window width required to display the scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The minium required window width.
        //
        unsigned int
Scene::minimumWindowWidth()
        const
{
    return _pImageSource->image().width();
}
        
        // minimumWindowHeight
        //
        // Description:
        //      Get the minimum window height required to display the scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The minimum required window height.
        //
        unsigned int
Scene::minimumWindowHeight()
        const
{
    return _pImageSource->image().height() + _pApplicationInfo->height();
}

        // width()
        //
        // Description:
        //      Return the current scene width.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The scene width.
        //
        unsigned int
Scene::width()
{
    return _nWindowWidth;
}

        // height
        //
        // Description:
        //      Return the current scene height.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The scene height.
        //
        unsigned int
Scene::height()
{
    return _nWindowHeight;
}

        // setWindowSize
        //
        // Description:
        //      Set a new window size.
        //
        // Parameters:
        //      nWindowWidth  - the new window width.
        //      nWindowHeight - the new window height.
        //      bInvertUV
        //
        // Returns:
        //      None
        //
        void
Scene::setWindowSize(unsigned int nWindowWidth , unsigned int nWindowHeight)
{
    _nWindowWidth  = nWindowWidth;
    _nWindowHeight = nWindowHeight;
}

        // setInvertTexCoords
        //
        // Description:
        //      Set to invert Texture Coordinates
        //
        // Parameters:
        //      bInvertTexCoords
        //
        // Returns:
        //      None
        //
        void
Scene::setInvertTexCoords(bool bInvertTexCoords)
{
    _bInvertTexCoords = bInvertTexCoords;
}


        // notify
        //
        // Description:
        //      Notifies the scene to perform an update.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
Scene::notify()
{
    _bNeedUpdate = true;
}


