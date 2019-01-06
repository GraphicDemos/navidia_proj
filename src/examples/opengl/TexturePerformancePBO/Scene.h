#ifndef SCENE_H
#define SCENE_H
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

#include <string>

//
// Forward declarations
//

class ImageSource;
class ImageSink;
class ApplicationInfo;
class Shader;


// ----------------------------------------------------------------------------
// Scene class
//
class Scene
{
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
    Scene();
    
            // Constructor 1
            //
            // Description:
            //      Creates a ready to render setup.
            //
            // Parameters:
            //      nWindowWidth  - width  of the OpenGL window.
            //      nWindowHeight - height of the OpenGL window.
            //      pRenderImage  - point to the image pusher.
            //      pReadbackImage - pointer to the image puller.
            //      
    Scene(unsigned int nWindowWidth, unsigned int nWindowHeight, 
          ImageSource * pRenderImage, ImageSink * pReadbackImage,
          ApplicationInfo * pApplicationInfo);
    
            // Destructor
   ~Scene();
   
   
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
    render();
            
            // setImageSource
            //
            // Description:
            //      Sets the image-source object uploading and binding
            //      the renderable texture.
            //
            // Parameters:
            //      pRenderImage - the render image.
            //
            // Returns:
            //      None
            //
            void
    setImageSource(ImageSource * pImageSource);
    
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
    setShader(Shader * pShader);
           
            // setImageSink
            //
            // Description:
            //      Sets the image-sink object reading the frame buffer.
            //
            // Parameters:
            //      pReadbackImage - the readback image.
            //
            // Returns:
            //      None
            //
            void
    setImageSink(ImageSink * pImageSink);
    
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
    setApplicationInfo(ApplicationInfo * pApplicationInfo);
            
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
    minimumWindowWidth()
            const;
            
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
    minimumWindowHeight()
            const;
            
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
    width();
    
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
    height();
    
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
    setWindowSize(unsigned int nWindowWidth , unsigned int nWindowHeight);
    
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
    notify();
   
private:
    //
    // Private data
    //
    
    unsigned int _nWindowWidth;
    unsigned int _nWindowHeight;
    
    ImageSource * _pImageSource;
    unsigned int _nImagePositionX;
    unsigned int _nImagePositionY;
    
    Shader * _pShader;
    
    ImageSink * _pImageSink;
    
    ApplicationInfo * _pApplicationInfo;
    
    bool _bNeedUpdate;
};


#endif // SCENE_H