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

#include "Scene.h"

#include "ImageSource.h"
#include "ImageSink.h"
#include "ApplicationInfo.h"
#include "Shader.h"

#include <glh/glh_extensions.h>
#include <GL/glut.h>


#include <sstream>
#include <iomanip>

#include <assert.h>



// ----------------------------------------------------------------------------
// Scene class
//
    
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
              , _pImageSink(0)
              , _pApplicationInfo(0)
              , _bNeedUpdate(false)
{
    ;
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
             ApplicationInfo * pApplicationInfo)
            : _nWindowWidth(nWindowWidth)
            , _nWindowHeight(nWindowHeight)
            , _pImageSource(pImageSource)
            , _nImagePositionX(0)
            , _nImagePositionY(0)
            , _pImageSink(pImageSink)
            , _pApplicationInfo(pApplicationInfo)
            , _bNeedUpdate(true)
{
    ;
}

        // Destructor
Scene::~Scene()
{
    ;
}
   
   
   //
   // Public methods
   //
            
        // render
        //
        // Description:
        //      Renders the scene.
        //          This call issues the OpenGL call to render the
        //      scene.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
Scene::render()
{        
    assert(_pImageSource != 0);
    unsigned int nDownloadedBytes = _pImageSource->pushNewFrame();
    
    float nImageWidth = static_cast<float>(_pImageSource->image().width());
    float nImageHeight = static_cast<float>(_pImageSource->image().height());
    
    assert(_pShader != 0);

    glClear(GL_COLOR_BUFFER_BIT);

                                // draw the image as a textured quad
    glViewport(0, 0, (GLsizei) _nWindowWidth, (GLsizei) _nWindowHeight - _pApplicationInfo->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0,  _nWindowHeight - _pApplicationInfo->height());


    glEnable (GL_FRAGMENT_PROGRAM_NV);
    glEnable (GL_TEXTURE_RECTANGLE_NV);
    glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);                    glVertex2f(0.0, 0.0);
        glTexCoord2f (nImageWidth, 0.0);            glVertex2f(nImageWidth, 0.0);
        glTexCoord2f (nImageWidth, nImageHeight);   glVertex2f(nImageWidth, nImageHeight);
        glTexCoord2f (0.0, nImageHeight);           glVertex2f(0.0, nImageHeight);
    glEnd ();
    glDisable (GL_TEXTURE_RECTANGLE_NV);
    glDisable (GL_FRAGMENT_PROGRAM_NV);

    assert(_pApplicationInfo != 0);
    _pApplicationInfo->setBytesDownloaded(nDownloadedBytes);
                                // draw the GUI elements
    glViewport(0, _nWindowHeight - _pApplicationInfo->height(), _nWindowWidth,  _pApplicationInfo->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, _nWindowWidth, 0, _pApplicationInfo->height());
    _pApplicationInfo->render(0, 0);
    
    assert(_pShader);
    glViewport(0, 0, _nWindowWidth, _nWindowHeight);
    _pShader->renderSliders(_pApplicationInfo->width(), 0);

    glutSwapBuffers();

    assert(_pImageSink != 0);
                                // readback 
    unsigned int nReadBytes = _pImageSink->pull(0, 0);
                                // number of bytes read back will be displayed 
                                // in successive frame
    _pApplicationInfo->setBytesRead(nReadBytes);
    
    
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

        // setShader
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
Scene::setShader(Shader * pShader)
{
    _pShader = pShader;
    _pShader->bind();
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


