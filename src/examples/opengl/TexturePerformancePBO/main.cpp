

//
// Includes
//

#ifdef _WIN32
#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <GL/glut.h>

#include "AssertGL.h"
#include "Timer.h"
#include "StaticImage.h"
#include "ImagePusher.h"
#include "ImagePusherPBO.h"
#include "ImagePusherDoublePBO.h"
#include "ImageSinkDummy.h"
#include "ImagePuller.h"
#include "ImagePullerPBO.h"
#include "Scene.h"
#include "OpenEXRLoader.h"
#include "ApplicationInfo.h"
#include "SimpleHDRShader.h"
#include "FastHDRShader.h"

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <algorithm>


//
// Macros
//

#define N_COMMAND_QUIT                  0x00

#define N_COMMAND_SOURCE_STATIC_IMAGE   0x10
#define N_COMMAND_SOURCE_NORMAL         0x11
#define N_COMMAND_SOURCE_PBO            0x12
#define N_COMMAND_SOURCE_MULTI_PBO      0x13

#define N_COMMAND_SINK_DUMMY            0x20
#define N_COMMAND_SINK_NORMAL           0x21
#define N_COMMAND_SINK_PBO              0x22

#define N_COMMAND_IMAGE_FP16_RGBA       0x30
#define N_COMMAND_IMAGE_FP16_RGB        0x31
#define N_COMMAND_IMAGE_FX8_RGBA        0x32
#define N_COMMAND_IMAGE_FX8_RGB         0x33
#define N_COMMAND_IMAGE_FX8_BGRA        0x34
#define N_COMMAND_IMAGE_FX8_BGR         0x35

#define N_COMMAND_INTERNAL_DEFAULT      0x40
#define N_COMMAND_INTERNAL_FP16_RGBA    0x41
#define N_COMMAND_INTERNAL_FP16_RGB     0x42
#define N_COMMAND_INTERNAL_FX8_RGBA     0x43
#define N_COMMAND_INTERNAL_FX8_RGB      0x44


#define N_COMMAND_SHADER_SIMPLE         0x50
#define N_COMMAND_SHADER_GAMMA          0x51

#define N_COMMAND_WORKLOAD_ENABLE       0x60
#define N_COMMAND_WORKLOAD_DISABLE      0x61


//
// Global constants
//

const char * gzImageName = "../../../../MEDIA/textures/hdr/MtTamWest.exr";


// 
// Global variables
//

unsigned int gnWindowWidth;
unsigned int gnWindowHeight;

int gidMainMenu;
int gidImageSourceMenu;
int gidImageSinkMenu;
int gidImageFormatMenu;
int gidInternalImageFormatMenu;
int gidShaderMenu;
int gidWorkloadMenu;

GLuint ghFragmentProgram;   // global fragment program handle

unsigned int gnFrameCounter;
nv::Timer goTimer(100);

Image           * gpImageFP16RGBA;
Image           * gpImageFP16RGB;
Image           * gpImageFX8RGBA;
Image           * gpImageFX8RGB;
Image           * gpImageFX8BGRA;
Image           * gpImageFX8BGR;
Image           * gpImage;

StaticImage             * gpStaticImage;
ImagePusher             * gpImagePusher;
ImagePusherPBO          * gpImagePusherPBO;
ImagePusherDoublePBO    * gpImagePusherDoublePBO;
ImageSource             * gpImageSource;

ImageSinkDummy  * gpImageSinkDummy;
ImagePuller     * gpImagePuller;
ImagePullerPBO  * gpImagePullerPBO;
ImageSink       * gpImageSink;

Scene           * gpScene;
ApplicationInfo * gpApplicationInfo;
SimpleHDRShader * gpSimpleShader;
FastHDRShader   * gpFastShader;
Shader          * gpShader;



// 
// Implementation
//

void getExtensions(void)
{
    if (!glh_init_extensions("GL_NV_vertex_program GL_NV_fragment_program")) 
    {
        std::cerr << "Error - required extensions were not supported: " << glh_get_unsupported_extensions()
                  << std::endl;
        exit(-1);
    }
}

void loadFragmentProgram(const char * zFragmentProgram)
{
    int nFragmentProgramLength = static_cast<int>(strlen(zFragmentProgram));

    //
    // load the fragment shader into the card
    //
    
    glGenProgramsNV (1, &ghFragmentProgram);
    glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, ghFragmentProgram);
    glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, ghFragmentProgram, nFragmentProgramLength, 
		     (const GLubyte *) zFragmentProgram);

    //
    // check for errors
    //
    
    GLint nErrorPosition;
    glGetIntegerv (GL_PROGRAM_ERROR_POSITION_NV, &nErrorPosition);
    if (nErrorPosition != -1)
    {
        std::cerr << "Fragment shader error:" << std::endl << std::endl;

        const char * pCharacter = zFragmentProgram;
        const char * pLine = pCharacter;
        while (*pCharacter != '\0' && (pCharacter - zFragmentProgram) < nErrorPosition)
        {
	        if (*pCharacter == '\n')
	            pLine = pCharacter + 1;
	        pCharacter++;
        }
        char zErrorLine[81];

        strncpy (zErrorLine, pLine, 80);
        zErrorLine[80] = '\0';

        std::cerr << zErrorLine << std::endl;
    }
}



        double
framesPerSecond()
{
    return 1.0 / goTimer.average();
}

        // SetImage
        //
        // Description:
        //      Set up a new image.
        //          All image pushers are provided with the new input
        //      image and the image sinks are resized to accomodate the
        //      new iamge.
        //
        // Parameters:
        //      rImage - reference to the new image.
        //
        // Returns:
        //      None
        //
        void
SetImage(const Image & rImage)
{
    gpStaticImage->setImage(rImage);
    gpImagePusher->setImage(rImage);
    gpImagePusherPBO->setImage(rImage);
    gpImagePusherDoublePBO->setImage(rImage);
    
    gpImageSinkDummy->resize(rImage.width(), rImage.height());
    gpImagePuller->resize(rImage.width(), rImage.height());
    gpImagePullerPBO->resize(rImage.width(), rImage.height());
}
        
        // SetInternalFormatGL
        //
        // Description:
        //      Sets all images to specific internal GL format.
        //
        // Parameters:
        //      ePixelFormatGL - the OpenGL internal pixel format.
        //
        // Returns:
        //      None
        //
        void
SetInternalFormatGL(ImagePusher::tePixelFormatGL ePixelFormatGL)
{
    gpStaticImage->setPixelFormatGL(ePixelFormatGL);
    gpImagePusher->setPixelFormatGL(ePixelFormatGL);
    gpImagePusherPBO->setPixelFormatGL(ePixelFormatGL);
    gpImagePusherDoublePBO->setPixelFormatGL(ePixelFormatGL);
}

        // display callback
        //
void display()
{
    gpApplicationInfo->setFramesPerSecond(1.0/goTimer.average());
    gpScene->render();
    ++gnFrameCounter;
    goTimer.sample();
}

        // menu
        //
        void 
menu(int nCommand)
{
    switch (nCommand)
    {
        case  27:
        case N_COMMAND_QUIT:
        {
            exit(0);
        }
        break;
    
        case N_COMMAND_SOURCE_STATIC_IMAGE:
        {
            gpImageSource = gpStaticImage;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(StaticImage::ClassDescription);
        }
        break;
        
        case N_COMMAND_SOURCE_NORMAL:
        {
            gpImageSource = gpImagePusher;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusher::ClassDescription);
        }
        break;
        
        case N_COMMAND_SOURCE_PBO:
        {
            gpImageSource = gpImagePusherPBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusherPBO::ClassDescription);
        }
        break;
        
        case N_COMMAND_SOURCE_MULTI_PBO:
        {
            gpImageSource = gpImagePusherDoublePBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusherDoublePBO::ClassDescription);
        }
        break;
        
        case N_COMMAND_SINK_DUMMY:
        {
            gpImageSink = gpImageSinkDummy;
            gpScene->setImageSink(gpImageSink);
            gpApplicationInfo->setImageSink(ImageSinkDummy::ClassDescription);
        }
        break;
        
        case N_COMMAND_SINK_NORMAL:
        {
            gpImageSink = gpImagePuller;
            gpScene->setImageSink(gpImageSink);
            gpApplicationInfo->setImageSink(ImagePuller::ClassDescription);
        }
        break;

        case N_COMMAND_SINK_PBO:
        {
            gpImageSink = gpImagePullerPBO;
            gpScene->setImageSink(gpImagePullerPBO);
            gpApplicationInfo->setImageSink(ImagePullerPBO::ClassDescription);
        }
        break;

        case N_COMMAND_IMAGE_FP16_RGBA:
        {
            gpImage = gpImageFP16RGBA;
            SetImage(*gpImageFP16RGBA);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_IMAGE_FP16_RGB:
        {
            gpImage = gpImageFP16RGB;
            SetImage(*gpImageFP16RGB);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_IMAGE_FX8_RGBA:
        {
            gpImage = gpImageFX8RGBA;
            SetImage(*gpImageFX8RGBA);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_IMAGE_FX8_RGB:
        {
            gpImage = gpImageFX8RGB;
            SetImage(*gpImageFX8RGB);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_IMAGE_FX8_BGRA:
        {
            gpImage = gpImageFX8BGRA;
            SetImage(*gpImageFX8BGRA);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_IMAGE_FX8_BGR:
        {
            gpImage = gpImageFX8BGR;
            SetImage(*gpImageFX8BGR);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_INTERNAL_DEFAULT:
        {
            SetInternalFormatGL(ImagePusher::GL_UNDEFINED_PIXEL);
            SetImage(*gpImage);
            gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
        }
        break;
        
        case N_COMMAND_INTERNAL_FP16_RGBA:
        {
            SetInternalFormatGL(ImagePusher::GL_FLOAT_RGBA16_NV_PIXEL);
            SetImage(*gpImage);        
            gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
       }
        break;
        
        case N_COMMAND_INTERNAL_FP16_RGB:
        {
             SetInternalFormatGL(ImagePusher::GL_FLOAT_RGB16_NV_PIXEL);
             SetImage(*gpImage);      
            gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
        }
        break;
        
        case N_COMMAND_INTERNAL_FX8_RGBA:
        {
            SetInternalFormatGL(ImagePusher::GL_RGBA_PIXEL);
            SetImage(*gpImage);       
            gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
       }
        break;
        
        case N_COMMAND_INTERNAL_FX8_RGB:
        {
            SetInternalFormatGL(ImagePusher::GL_RGB_PIXEL);
            SetImage(*gpImage);        
            gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
        }
        break;

        
        case N_COMMAND_SHADER_SIMPLE:
        {
            gpShader = gpFastShader;
            gpScene->setShader(gpShader);
            gpApplicationInfo->setShader("Simple Shader");
        }
        break;
        
        case N_COMMAND_SHADER_GAMMA:
        {
            gpShader = gpSimpleShader;
            gpScene->setShader(gpShader);
            gpApplicationInfo->setShader("Gamma Shader");
        }
        break;
        
        case N_COMMAND_WORKLOAD_ENABLE:
        {
            gpImageSink->enableDummyWorkload();
        }
        break;
        
        case N_COMMAND_WORKLOAD_DISABLE:
        {
            gpImageSink->disableDummyWorkload();
        }
        break;

    } 
}

        // key callback
void key(unsigned char  nKey, int nPositionX, int nPositionY)
{
    menu(nKey);
}

        // special 
        //
        // Description:
        //      Special keys (arrows, etc.) handler.
        //
        void
special(int nKey, int nX, int nY)
{
    // gpShader->special(nKey, nX, nY);
}


        // reshape
        //
        void
reshape(int nWidth, int nHeight)
{
    gpScene->setWindowSize(nWidth, nHeight);

    display();
}

        // idle
        //
        void
idle()
{
    display();
}

        // visibility
        //
        void
visibility(int eVisibility)
{
    if (eVisibility == GLUT_NOT_VISIBLE)
        glutIdleFunc(NULL);
    else
        glutIdleFunc(idle);
}


void initGL()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    GL_ASSERT_NO_ERROR;
    
    glViewport(0, 0, (GLsizei) gnWindowWidth, (GLsizei) gnWindowHeight);
    GL_ASSERT_NO_ERROR;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, gnWindowWidth, 0, gnWindowHeight);
    GL_ASSERT_NO_ERROR;
}

        // InitImages
        //
        // Description:
        //      Load image from disk and create all different support formats.
        //
        // Parameters:
        //      zImageFileName - image file to be loaded.
        //
        // Returns:
        //      None
        //
        void 
InitImages(const char * zImageFileName)
{
    OpenEXRLoader oOpenEXRLoader(zImageFileName);
    
    gpImageFP16RGBA  = new Image();
    *gpImageFP16RGBA = oOpenEXRLoader.image();

    gpImageFP16RGB   = new Image();
    *gpImageFP16RGB  = Image::ConvertPixelFormat(Image::FP16_RGB_PIXEL, *gpImageFP16RGBA);
    
    gpImageFX8RGBA   = new Image();
    *gpImageFX8RGBA  = Image::ConvertPixelFormat(Image::FX8_RGBA_PIXEL, *gpImageFP16RGBA);
    
    gpImageFX8RGB    = new Image();
    *gpImageFX8RGB   = Image::ConvertPixelFormat(Image::FX8_RGB_PIXEL,  *gpImageFP16RGBA);
    
    gpImageFX8BGRA   = new Image();
    * gpImageFX8BGRA = Image::ConvertPixelFormat(Image::FX8_BGRA_PIXEL, *gpImageFP16RGBA);
    
    gpImageFX8BGR    = new Image();
    * gpImageFX8BGR  = Image::ConvertPixelFormat(Image::FX8_BGR_PIXEL,  *gpImageFP16RGBA);
    
    gpImage = gpImageFP16RGBA;
}

        // InitPipeline
        //
        // Description:
        //      Set up all sources and sinks and default pipeline.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InitPipeline(const char * zImageFileName)
{
    gpScene = new Scene();
    
    gpApplicationInfo = new ApplicationInfo();
    gpScene->setApplicationInfo(gpApplicationInfo); 
    
    gpSimpleShader = new SimpleHDRShader();
    gpFastShader   = new FastHDRShader();

    InitImages(zImageFileName);

    gpStaticImage           = new StaticImage();
    gpImagePusher           = new ImagePusher();
    gpImagePusherPBO        = new ImagePusherPBO();
    gpImagePusherDoublePBO  = new ImagePusherDoublePBO();
    
    gpImageSinkDummy        = new ImageSinkDummy();
    gpImagePuller           = new ImagePuller();
    gpImagePullerPBO        = new ImagePullerPBO();
    
    menu(N_COMMAND_SOURCE_PBO);
    menu(N_COMMAND_SINK_DUMMY);
    menu(N_COMMAND_IMAGE_FP16_RGBA);
    menu(N_COMMAND_INTERNAL_DEFAULT);
    menu(N_COMMAND_SHADER_SIMPLE);
 }


void initMenu()
{
    gidImageSourceMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Static Image",                    N_COMMAND_SOURCE_STATIC_IMAGE);
    glutAddMenuEntry("Normal (glTexSubimage)",          N_COMMAND_SOURCE_NORMAL);
    glutAddMenuEntry("PBO",                             N_COMMAND_SOURCE_PBO);
    glutAddMenuEntry("Multi PBO",                       N_COMMAND_SOURCE_MULTI_PBO);
    
    gidImageSinkMenu = glutCreateMenu(menu);
    glutAddMenuEntry("No Readback",                     N_COMMAND_SINK_DUMMY);
    glutAddMenuEntry("Normal (glReadPixels)",           N_COMMAND_SINK_NORMAL);
    glutAddMenuEntry("PBO Asynchronous",                N_COMMAND_SINK_PBO);
    
    gidImageFormatMenu = glutCreateMenu(menu);
    glutAddMenuEntry("FP16 RGBA",                       N_COMMAND_IMAGE_FP16_RGBA);
    glutAddMenuEntry("FP16 RGB",                        N_COMMAND_IMAGE_FP16_RGB);
    glutAddMenuEntry("FX8  RGBA",                       N_COMMAND_IMAGE_FX8_RGBA);
    glutAddMenuEntry("FX8  RGB",                        N_COMMAND_IMAGE_FX8_RGB);
    glutAddMenuEntry("FX8  BGRA",                       N_COMMAND_IMAGE_FX8_BGRA);
    glutAddMenuEntry("FX8  BGR]",                       N_COMMAND_IMAGE_FX8_BGR);
    
    gidInternalImageFormatMenu = glutCreateMenu(menu);
    glutAddMenuEntry("FP16 RGBA",                       N_COMMAND_INTERNAL_FP16_RGBA);
    glutAddMenuEntry("FP16 RGB",                        N_COMMAND_INTERNAL_FP16_RGB);
    glutAddMenuEntry("FX8 RGBA",                        N_COMMAND_INTERNAL_FX8_RGBA);
    glutAddMenuEntry("FX8 RGB",                         N_COMMAND_INTERNAL_FX8_RGB);
    
    gidShaderMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Simple HDR",                      N_COMMAND_SHADER_SIMPLE);
    glutAddMenuEntry("Gamma HDR",                       N_COMMAND_SHADER_GAMMA);
    
    gidWorkloadMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Enable",                          N_COMMAND_WORKLOAD_ENABLE);
    glutAddMenuEntry("Disable",                         N_COMMAND_WORKLOAD_DISABLE);

    gidMainMenu = glutCreateMenu(menu);
    glutAddSubMenu("Image Upload", gidImageSourceMenu);
    glutAddSubMenu("Image Readback", gidImageSinkMenu);
    glutAddSubMenu("Image Format", gidImageFormatMenu);
    glutAddSubMenu("Internal Image Format", gidInternalImageFormatMenu);
    glutAddSubMenu("Shader", gidShaderMenu);
    glutAddSubMenu("Dummy Workload", gidWorkloadMenu);
    glutAddMenuEntry("Quit\t[ESC]",                     N_COMMAND_QUIT);

    glutAttachMenu( GLUT_RIGHT_BUTTON );
}

void main(int nArgs, char * aArgs[])
{
    glutInit(&nArgs, aArgs);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    
    gnWindowWidth  = 100;
    gnWindowHeight = 100;
    
    glutInitWindowSize(gnWindowWidth, gnWindowHeight);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("PBO Test");
    GL_ASSERT_NO_ERROR;
       
    getExtensions();
    
    initMenu();
    InitPipeline(gzImageName);
        
    initGL();
    GL_ASSERT_NO_ERROR;

    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    GL_ASSERT_NO_ERROR;
       
    glutReshapeWindow(static_cast<int>(gpScene->minimumWindowWidth()),
                      static_cast<int>(gpScene->minimumWindowHeight()));
    goTimer.start();
    glutMainLoop();
    goTimer.stop();
    
}