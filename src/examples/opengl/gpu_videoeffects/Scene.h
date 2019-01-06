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

#include <GL/glew.h>
#include <GL/gl.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <Image.h>
//#include <ImageFBO.h>

#include <string>


//
// Forward declarations
//

class Image;
//class ImageFBO;
class ImageSource;
class ImageSink;
class ApplicationInfo;

class LoadOperator;
class NVImageLoader;
class SourceOperator;
class ImageFilter;
class ShaderCg;
class ShaderGLSL;
class ProgramGLSL;
class ProgramCg;
class ImageSize;
class InteractionController;

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
          ApplicationInfo * pApplicationInfo, bool bInvertUV);
    
            // Destructor
   ~Scene();
   
   
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
    initCg();


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
    initController(InteractionController *pController);


            // init the Video Buffers to the render stage
            //
            // Description:
            //      This function will create the PBO for Video Copying, so the
			//		DirectShow thread will run properly
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    initVideoBuffers();


            // init the Video Copy Context to the render stage
            //
            // Description:
            //      This will be a callback function for the the Video Copy
            //      Context.  So we can have the DirectShow thread copy to the 
            //      the PBO or Texture Buffer.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    initVideoCopyContext();


            // copy Video Frame over to the render stage
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
    copyVideoFrame(unsigned char *source, ImageSize imginfo);

            // enableGLSL
            //
            // Description:
            //      sets a Flag indicating to use GLSL
            //
            // Parameters:
            //      None
            //
            void
	enableGLSL(bool bUseGLSL) { _bUseGLSL = bUseGLSL; }

            // enableNVImage
            //
            // Description:
            //      sets a Flag indicating to enable NV Imaging
            //
            // Parameters:
            //      None
            //
            void
	enableNVImage(bool bUseNVImage) { _bUseNVImage = bUseNVImage; }

            // enableFBO
            //
            // Description:
            //      sets a Flag indicating to enable FBO
            //
            // Parameters:
            //      None
            //
            void
	enableFBO(bool bUseFBO) { /*_bUseFBO = bUseFBO; */}

			// isGLSL
            //
            // Description:
            //      checks the GLSL flag
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      _bUseGLSL
            //
            bool
    isGLSL() { return _bUseGLSL; }

            // isNVImage
            //
            // Description:
            //      checks the NVImageProcessing flag
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      _bUseGLSL
            //
            bool
    isNVImage() { return _bUseNVImage; }

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
			//      the time it took to render the frame
            //
            double
    render();

			// renderNVImageProc
			//
			// Description:
			//      Renders the scene with the NV Imaging Library
			//
			// Parameters:
			//      None
			//
			// Returns:
			//      the time it took to render the frame
			//
			double
	renderNVImageProc();

			// renderCg
            //
            // Description:
            //      Renders the scene with Cg Shaders.
            //          This call issues the OpenGL call to render the
            //      scene.
            //
            // Parameters:
            //      None
            //
            // Returns:
			//      the time it took to render the frame
            //
            double
    renderCg();
            

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
			//      the time it took to render the frame
            //
            double
    renderGLSL();

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
    setNVImageFilter(ImageFilter* pImageFilter, NVImageLoader *pImageLoader);
//    setNVImageFilter(ImageFilter* pImageFilter, LoadOperator *pImageLoader);

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
    setShaderCg(ShaderCg * pShader);
           
            // setProgramCg
            //
            // Description:
            //      Sets the Cg shaders for the input image.
            //
            // Parameters:
            //      pProgram - pointer to the Cg Program.
            //
            // Returns:
            //      None
            //
            void
    setProgramCg(ProgramCg * pProgram);

			// setProgramGLSL
            //
            // Description:
            //      Sets the GLSL program for the input image.
            //
            // Parameters:
            //      pProgram - pointer to the GLSL Program .
            //
            // Returns:
            //      None
            //
            void
    setProgramGLSL(ProgramGLSL * pProgram);

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
    
            // Size
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
    setInvertTexCoords(bool bInvertTexCoords);

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
    
    ImageSource * _pImageSource;    // this is the Video that will uploaded through PBO's
    unsigned int _nImagePositionX;
    unsigned int _nImagePositionY;

    float _nZoomFactor;
    float _nBrightness;
    float _aBackgroundColor[3];

    NVImageLoader   * _pNVImageLoader;  // these is the loader 
    ImageFilter     * _pNVImageFilter;  // this is the image filtering
    Image       _oImage;        // received after rendering to the pbuffer
    //ImageFBO    _oImageFBOImageFBO;     // received after rendering to the pbuffer

    ShaderCg    * _pShaderCg;
    ShaderGLSL  * _pShaderGLSL;
	ProgramCg   * _pProgramCg;
    ProgramGLSL * _pProgramGLSL;
    ImageSink   * _pImageSink;

    ApplicationInfo			* _pApplicationInfo;
	InteractionController	* _pInteractionController;
    
    CGprogram   _oCgFragmentProgram;
    CGprofile   _oCgFragmentProfile;
    CGparameter  _hImage;

    bool _bInvertTexCoords;
    bool _bNeedUpdate;
    bool _bUseGLSL;
	bool _bUseNVImage;
};


#endif // SCENE_H
