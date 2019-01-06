

//
// Includes
//

#ifdef _WIN32
//  #define NOMINMAX
  #define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#elif defined (UNIX)
#include <GL/glx.h>
#endif

#define USE_TEXTURES 0

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/wglew.h>
#include <GL/glut.h>

#include <string>

//#include <shared/nv_png.h>
#include <shared/data_path.h>
#include <nv_dds.h>

#include "dxutil.h"
#include "GUI_utils.h"

#include "AssertGL.h"
#include "Timer.h"
#include "StaticImage.h"
#include "ImagePusher.h"
#include "ImagePusherPBO.h"
#include "ImagePusherDoublePBO.h"
#include "VideoPusher.h"
#include "VideoPusherPBO.h"
#include "VideoPusherDoublePBO.h"

#include "ImageSinkDummy.h"
#include "ImagePuller.h"
#include "ImagePullerPBO.h"
#include "Scene.h"
#include "OpenEXRLoader.h"
#include "ApplicationInfo.h"
#include "nv_image_processing.h"

#include "File.h"
#include "ShaderGLSL.h"
#include "ProgramGLSL.h"
#include "ProgramCg.h"
#include "uniforms.h"

#include "Mutex.h"
#include "GraphBuilder.h"
#include "defines.h"

#include "InteractionController.h"

#include <AssertGL.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <algorithm>


//
// Macros
//

#define N_COMMAND_QUIT                  0x00

#define N_COMMAND_SOURCE_FILE_IMAGE     0x01
#define N_COMMAND_SOURCE_FILE_VIDEO     0x02
#define N_COMMAND_VIDEO_PLAYPAUSE       0x03
#define N_COMMAND_VIDEO_FF_10SEC		0x04
#define N_COMMAND_VIDEO_REW_10SEC		0x05

#define N_COMMAND_STATIC_IMAGE          0x10
#define N_COMMAND_IMAGE_NORMAL          0x11
#define N_COMMAND_IMAGE_PBO             0x12
#define N_COMMAND_IMAGE_MULTI_PBO       0x13
#define N_COMMAND_VIDEO_NORMAL          0x14
#define N_COMMAND_VIDEO_PBO             0x15
#define N_COMMAND_VIDEO_MULTI_PBO       0x16

#define N_COMMAND_SINK_DUMMY            0x20
#define N_COMMAND_SINK_NORMAL           0x21
#define N_COMMAND_SINK_PBO              0x22

#define N_COMMAND_IMAGE_FP16_RGBA       0x30
#define N_COMMAND_IMAGE_FP16_RGB        0x31
#define N_COMMAND_IMAGE_FX8_RGBA        0x32
#define N_COMMAND_IMAGE_FX8_RGB         0x33
#define N_COMMAND_IMAGE_FX8_BGRA        0x34
#define N_COMMAND_IMAGE_FX8_BGR         0x35
#define N_COMMAND_VIDEO_FX8_BGRA        0x36
#define N_COMMAND_VIDEO_FX8_BGR         0x37
#define N_COMMAND_VIDEO_FX8_YUYV        0x38
#define N_COMMAND_VIDEO_FX8_UYVY        0x39

#define N_COMMAND_INTERNAL_DEFAULT      0x40
#define N_COMMAND_INTERNAL_FP16_RGBA    0x41
#define N_COMMAND_INTERNAL_FP16_RGB     0x42
#define N_COMMAND_INTERNAL_FX8_RGBA     0x43
#define N_COMMAND_INTERNAL_FX8_RGB      0x44
#define N_COMMAND_INTERNAL_FX8_YUYV     0x45
#define N_COMMAND_INTERNAL_FX8_UYVY     0x46

#define N_COMMAND_CG_SIMPLE             0x70
#define N_COMMAND_CG_GAMMA              0x71
#define N_COMMAND_CG_TILES              0x72
#define N_COMMAND_CG_TVNOISE            0x73
#define N_COMMAND_CG_NIGHT				0x74
#define N_COMMAND_CG_SCOTOPIC           0x75
#define N_COMMAND_CG_GAUSSIAN           0x76
#define N_COMMAND_CG_GAUSSIAN_1D        0x77
#define N_COMMAND_CG_GAUSSIAN_2PASS     0x78
#define N_COMMAND_CG_BLOOM				0x79

#define N_COMMAND_GLSL_LUMA             0x80
#define N_COMMAND_GLSL_SIMPLE           0x81
#define N_COMMAND_GLSL_GAMMA            0x82
#define N_COMMAND_GLSL_NEGATIVE         0x83
#define N_COMMAND_GLSL_HALFTONE         0x84
#define N_COMMAND_GLSL_SEPIA            0x85
#define N_COMMAND_GLSL_NOISE            0x86
#define N_COMMAND_GLSL_PERLIN_NOISE     0x87

#define N_COMMAND_GLSL_WATERFALL        0x88
#define N_COMMAND_GLSL_WOBBLE           0x89
#define N_COMMAND_GLSL_TVNOISE          0x8a
#define N_COMMAND_GLSL_TILES            0x8b
#define N_COMMAND_GLSL_BLOOM	        0x8c
#define N_COMMAND_GLSL_OLDCAMERA        0x8d

#define N_COMMAND_GLSL_RADIALBLUR       0x90
#define N_COMMAND_GLSL_EDGEDETECT       0x91
#define N_COMMAND_GLSL_EDGEOVERLAY      0x92
#define N_COMMAND_GLSL_COLORGRADIENT    0x93

#define N_COMMAND_WORKLOAD_ENABLE       0xA0
#define N_COMMAND_WORKLOAD_DISABLE      0xA1

//
// Global constants
//

const char * gzImageName[] = {
    EXR_IMAGE_PATH "MtTamWest.exr",
    EXR_IMAGE_PATH "StillLife.pt.exr",
    NULL
};

const char *gzWMV9_Video[] = {
    VIDEO_FILE_PATH "nvidia1.wmv",
    VIDEO_FILE_PATH "nvidia2.wmv",
    NULL
};

// Function Prototypes
bool InitImages(const char * zImageFileName);
bool InitVideo(const char * zVideoFileName, Scene *pScene);

// 
// Global variables
//
bool bUseFBO = false;

double tCurrent, tPrevious, tElapsed;

unsigned int gnWindowWidth;
unsigned int gnWindowHeight;

int  gnOldX;
int  gnOldY;

bool gpPlayerRunning;
bool gbMoving;
bool gbScaling;
bool gbSliders;
bool gbFullScreen;

int gidMainMenu;
int gidSourceFileMenu;
int gidImageSourceMenu;
int gidImageSinkMenu;
int gidImageFormatMenu;
int gidInternalImageFormatMenu;
int gidPlayerMenu;
int gidShaderCgMenu;
int gidShaderGLSLMenu;
int gidWorkloadMenu;

int gVideoRunning;

GLuint ghFragmentProgram;   // global fragment program handle
GLhandleARB gpProgram;      // handle to the ARB program object

unsigned int gnFrameCounter;
nv::Timer goTimer(100);

nv_dds::CDDSImage dds_dust;
nv_dds::CDDSImage dds_lines;
nv_dds::CDDSImage dds_tv;
nv_dds::CDDSImage dds_noise;

#if 0
glh::tex_object_2D tex_noise_sampler;
glh::tex_object_2D tex_dust;
glh::tex_object_2D tex_lines;
glh::tex_object_2D tex_tv;
glh::tex_object_3D tex_noise;
#endif

std::string pathname;
data_path progpath;

ImageTex            * gpImageFP16RGBA;
ImageTex            * gpImageFP16RGB;
ImageTex            * gpImageFX8RGBA;
ImageTex            * gpImageFX8RGB;
ImageTex            * gpImageFX8BGRA;
ImageTex            * gpImageFX8BGR;
ImageTex 			* gpVideoFX8BGRA;
ImageTex 			* gpVideoFX8BGR;
ImageTex            * gpVideoFX8YUYV;
ImageTex            * gpVideoFX8UYVY;
ImageTex            * gpImage;

StaticImage             * gpStaticImage = NULL;
ImagePusher             * gpImagePusher = NULL;
ImagePusherPBO          * gpImagePusherPBO = NULL;
ImagePusherDoublePBO    * gpImagePusherDoublePBO = NULL;
ImageSource             * gpImageSource = NULL;
VideoPusher             * gpVideoPusher = NULL;
VideoPusherPBO          * gpVideoPusherPBO = NULL;
VideoPusherDoublePBO    * gpVideoPusherDoublePBO = NULL;

ImageSinkDummy			* gpImageSinkDummy = NULL;
ImagePuller				* gpImagePuller = NULL;
ImagePullerPBO			* gpImagePullerPBO = NULL;
ImageSink				* gpImageSink = NULL;

InteractionController * gpInteractionController;

ImageFilter			* gpImageFilter;
LoadOperator        * gpLoadOperator;
NVImageLoader       * gpNVImageLoader;

NightFilter         * gpNightFilter = NULL;;
GaussFilter         * gpGaussFilter = NULL;;
ScotopicFilter      * gpScotopicFilter = NULL;;
GaussFilter1D       * gpGaussFilter1D = NULL;
TwoPassGaussFilter  * gpTwoPassGaussFilter = NULL;
BloomFilter			* gpBloomFilter = NULL;
SaveOperator        * gpSaveOperator;
ImageView           * gpView;

Scene           * gpScene;
ApplicationInfo * gpApplicationInfo;
ShaderGLSL      * gpShaderGLSL;

ProgramCg       * gpProgramCg;
ProgramCg		* gpSimple_Cg;
ProgramCg       * gpGamma_Cg;
ProgramCg       * gpTiles_Cg;
ProgramCg       * gpTVNoise_Cg;

ProgramGLSL     * gpProgramGLSL = NULL;
ProgramGLSL     * gpLuma_GLSL = NULL;
ProgramGLSL     * gpSimple_GLSL = NULL;
ProgramGLSL     * gpGamma_GLSL = NULL;
ProgramGLSL     * gpSepia_GLSL = NULL;
ProgramGLSL     * gpNoise_GLSL = NULL;
ProgramGLSL     * gpPerlinNoise_GLSL = NULL;
ProgramGLSL     * gpEdgeDetect_GLSL = NULL;
ProgramGLSL     * gpEdgeOverlay_GLSL = NULL;
ProgramGLSL     * gpNegative_GLSL = NULL;
ProgramGLSL     * gpBloom_GLSL = NULL;
ProgramGLSL     * gpHalfTone_GLSL = NULL;
ProgramGLSL     * gpTiles_GLSL = NULL;
ProgramGLSL     * gpTVNoise_GLSL = NULL;
ProgramGLSL     * gpRadialBlur_GLSL = NULL;
ProgramGLSL     * gpWaterfall_GLSL = NULL;
ProgramGLSL     * gpOldCamera_GLSL = NULL;
ProgramGLSL     * gpWobble_GLSL = NULL;
ProgramGLSL     * gpColorGrad_GLSL = NULL;

GraphBuilder    * gpGraph = NULL;

Mutex mMutex;

// Function declarations
void menu(int nCommand);

// 
// Implementation
//

#define REQUIRED_EXTENSIONS "GL_VERSION_1_4 " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GL_ARB_shader_objects " \
                            "GL_ARB_vertex_shader " \
                            "GL_ARB_fragment_shader " \
                            "GL_ARB_vertex_program " \
                            "GL_ARB_fragment_program " \
                            "GL_ARB_vertex_buffer_object " \
                            "GL_ARB_multitexture " \
                            "GL_ARB_texture_compression " \
                            "GL_EXT_texture_compression_s3tc " \
                            "GL_NV_texture_rectangle " \
                            "GL_NV_vertex_program " \
                            "GL_NV_fragment_program " \
                            "GL_NV_float_buffer " \
                            "WGL_ARB_extensions_string " \
                            "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format " \
                            "WGL_ARB_render_texture "

#define OPTIONAL_EXTENSIONS "GL_EXT_framebuffer_object "

void initOpenGL(void)
{
	glewInit();
	bUseFBO = false;

//  printf("[GL Extensions]: <%s>\n",  glGetString(GL_EXTENSIONS));
//  printf("[WGL Extensions]: <%s>\n", wglGetExtensionsStringARB(GetDC(0)));

    HDC hDC = wglGetCurrentDC();
    GL_ASSERT_NO_ERROR;

	// Initialize all the OpenGL extension used.
    ShaderManager::initialize();
    GL_ASSERT_NO_ERROR;
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
SetImage(const ImageTex & rImage)
{
    if (gpStaticImage)			gpStaticImage->setImage(rImage);
    if (gpImagePusherPBO)		gpImagePusherPBO->setImage(rImage);
    if (gpVideoPusherPBO)		gpVideoPusherPBO->setImage(rImage);

	if (gpImagePusher)			gpImagePusher->setImage(rImage);
    if (gpImagePusherDoublePBO) gpImagePusherDoublePBO->setImage(rImage);
	if (gpVideoPusher)			gpVideoPusher->setImage(rImage);
    if (gpVideoPusherDoublePBO) gpVideoPusherDoublePBO->setImage(rImage);

	if (gpImageSinkDummy)		gpImageSinkDummy->resize(rImage.width(), rImage.height());
    if (gpImagePuller)			gpImagePuller->resize(rImage.width(), rImage.height());
    if (gpImagePullerPBO)		gpImagePullerPBO->resize(rImage.width(), rImage.height());
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
    if (gpStaticImage)		gpStaticImage->setPixelFormatGL(ePixelFormatGL);
    if (gpImagePusherPBO)	gpImagePusherPBO->setPixelFormatGL(ePixelFormatGL);

    if (gpImagePusher)		gpImagePusher->setPixelFormatGL(ePixelFormatGL);
    if (gpImagePusherDoublePBO) gpImagePusherDoublePBO->setPixelFormatGL(ePixelFormatGL);
}

bool IsVideoSelected()
{
    if (gpImage == gpVideoFX8BGRA ||
        gpImage == gpVideoFX8BGR  ||
        gpImage == gpVideoFX8YUYV ||
        gpImage == gpVideoFX8UYVY)
    {
        return true;
    } else {
        return false;
    }
}

void SetImageDefaults()
{
    SetInternalFormatGL(ImagePusher::GL_FLOAT_RGBA16_NV_PIXEL);
    gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
    gpImage = gpImageFP16RGBA;
    SetImage(*gpImageFP16RGBA);
    gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
    gpScene->setInvertTexCoords(false);
	gpNVImageLoader->setFileType(NVImageLoader::IMAGE_FILE);

	if (gpScene->isGLSL()) {
	    gpScene->setProgramGLSL(gpProgramGLSL);
	} else {
		if (gpScene->isNVImage()) {
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);
		} else {
			gpScene->setProgramCg(gpProgramCg);
		}
	}

    glutReshapeWindow(  max(static_cast<int>(gpScene->minimumWindowWidth()),MIN_WIDTH ),
                            static_cast<int>(gpScene->minimumWindowHeight()));

	if (gpGraph) {
        gpGraph->PauseGraph();
		gpPlayerRunning = false;
	}
}

void SetVideoDefaults()
{
    SetInternalFormatGL(ImagePusher::GL_YUYV_PIXEL);
    gpApplicationInfo->setImageFormatGL(gpImageSource->pixelFormatStringGL());
    gpImage = gpVideoFX8YUYV;
    SetImage(*gpVideoFX8YUYV);
    gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
    gpScene->setInvertTexCoords(true);
	gpNVImageLoader->setFileType(NVImageLoader::VIDEO_FILE);

	if (gpScene->isGLSL()) {
	    gpScene->setProgramGLSL(gpProgramGLSL);
	} else {
		if (gpScene->isNVImage()) {
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);
		} else {
			gpScene->setProgramCg(gpProgramCg);
		}
	}

    glutReshapeWindow(  max(static_cast<int>(gpScene->minimumWindowWidth()), MIN_WIDTH),
                            static_cast<int>(gpScene->minimumWindowHeight()));

    if (gpGraph) {
        gpGraph->RunGraph();
        gpPlayerRunning = true;
    }
}



        // display callback
        //
void display()
{
    gpApplicationInfo->setFramesPerSecond(1.0/goTimer.average());

    if (IsVideoSelected()) {
		if (gpScene->isGLSL()) {
            gpProgramGLSL->gUniforms.updateTexSize(gpGraph->getWidth(), gpGraph->getHeight());
            gpProgramGLSL->gUniforms.updateWinSize(gpGraph->getWidth(), gpGraph->getHeight());
		} else if (!gpScene->isNVImage()) {
            gpProgramCg->gUniforms.updateTexSize(gpGraph->getWidth(), gpGraph->getHeight());
            gpProgramCg->gUniforms.updateWinSize(gpGraph->getWidth(), gpGraph->getHeight());
		}
    } else {
		if (gpScene->isGLSL()) {
            gpProgramGLSL->gUniforms.updateTexSize(gpScene->minimumWindowWidth(), gpScene->minimumWindowHeight());
            gpProgramGLSL->gUniforms.updateWinSize(gpScene->minimumWindowWidth(), gpScene->minimumWindowHeight());
		} else if (!gpScene->isNVImage()) {
            gpProgramCg->gUniforms.updateTexSize(gpScene->minimumWindowWidth(), gpScene->minimumWindowHeight());
            gpProgramCg->gUniforms.updateWinSize(gpScene->minimumWindowWidth(), gpScene->minimumWindowHeight());
		}
    }

	if (gpScene->isGLSL()) {
        gpProgramGLSL->gUniforms.setOffsets(0.5f, 0.5f);
        gpProgramGLSL->gUniforms.setTime(tCurrent/goTimer.frequency());
		gpProgramGLSL->gUniforms.updateUseYUV(IsVideoSelected());
	} else {
		if (!gpScene->isNVImage()) {
			gpProgramCg->gUniforms.setOffsets(0.5f, 0.5f);
			gpProgramCg->gUniforms.setTime(tCurrent/goTimer.frequency());
			gpProgramCg->gUniforms.updateUseYUV(IsVideoSelected());
		}
	}

    // now we can render the scene
    gpScene->render();
    ++gnFrameCounter;
    goTimer.sample();
    tCurrent = goTimer.time();
    tElapsed = (tCurrent - tPrevious) / goTimer.frequency();
    tPrevious = tCurrent;

    if (gpGraph) 
        gpGraph->CheckMovieStatus();
}

        // menu
        //
        void 
menu(int nCommand)
{
    switch (nCommand)
    {
		case N_COMMAND_VIDEO_PLAYPAUSE:
        case '.':
            if (gpGraph) {
                if (gpPlayerRunning) {
                    gpGraph->PauseGraph();
                } else {
                    gpGraph->RunGraph();
                }
                gpPlayerRunning = !gpPlayerRunning;
            }
        break;

		case N_COMMAND_VIDEO_FF_10SEC:
        case '>':
            if (gpGraph) {
                gpGraph->AdvanceTime(10);
            }
        break;

		case N_COMMAND_VIDEO_REW_10SEC:
		case '<':
            if (gpGraph) {
                gpGraph->AdvanceTime(-10);
            }
        break;

        case  27:
        case N_COMMAND_QUIT:
        {
            if (gpGraph) {
				gpGraph->StopGraph();
            }
            
			exit(0);
        }
        break;
    

        case N_COMMAND_SOURCE_FILE_IMAGE:
        {
			char szFileName[256];

			if (gpGraph) {
				gpGraph->StopGraph();
			}
			if (loadImageSource(NULL, EXR_IMAGE_PATH, szFileName)) {
				InitImages(szFileName);
                SetImageDefaults();
                glutReshapeWindow(  max(static_cast<int>(gpScene->minimumWindowWidth()), MIN_WIDTH),
                                        static_cast<int>(gpScene->minimumWindowHeight()));
                gpPlayerRunning = false;
            } else {
                if (gpPlayerRunning) {
                    gpGraph->RunGraph();
                }
            }
        }
        break;

        case N_COMMAND_SOURCE_FILE_VIDEO:
        {
			char szFileName[256];

			if (gpGraph) {
				gpGraph->StopGraph();
			}
			if (loadVideoSource(NULL, VIDEO_FILE_PATH, szFileName)) {
				InitVideo(szFileName, gpScene);
                SetVideoDefaults();
			}
            gpGraph->RunGraph();
        }
        break;

        case N_COMMAND_STATIC_IMAGE:
        {
            gpImageSource = gpStaticImage;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(StaticImage::ClassDescription);
            if (IsVideoSelected()) 
                SetImageDefaults();
        }
        break;
        
        case N_COMMAND_IMAGE_NORMAL:
        {
            gpImageSource = gpImagePusher;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusher::ClassDescription);
			if (IsVideoSelected())
                SetImageDefaults();
        }
        break;
        
        case N_COMMAND_IMAGE_PBO:
        {
            gpImageSource = gpImagePusherPBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusherPBO::ClassDescription);
			if (IsVideoSelected())
                SetImageDefaults();
		}
        break;
        
        case N_COMMAND_IMAGE_MULTI_PBO:
        {
            gpImageSource = gpImagePusherDoublePBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(ImagePusherDoublePBO::ClassDescription);
			if (IsVideoSelected())
                SetImageDefaults();
        }
        break;

        case N_COMMAND_VIDEO_NORMAL:
        {
            gpImageSource = gpVideoPusher;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(VideoPusher::ClassDescription);
			if (!IsVideoSelected()) {
                SetVideoDefaults();
			}
        }
        break;
        
        case N_COMMAND_VIDEO_PBO:
        {
            gpImageSource = gpVideoPusherPBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(VideoPusherPBO::ClassDescription);
			if (!IsVideoSelected()) {
                SetVideoDefaults();
			}
        }
        break;
        
        case N_COMMAND_VIDEO_MULTI_PBO:
        {
            gpImageSource = gpVideoPusherDoublePBO;
            gpScene->setImageSource(gpImageSource);
            gpApplicationInfo->setImageSource(VideoPusherDoublePBO::ClassDescription);
			if (!IsVideoSelected()) {
                SetVideoDefaults();
			}
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
  
		case N_COMMAND_VIDEO_FX8_BGRA:
        {
            gpImage = gpVideoFX8BGRA;
            SetImage(*gpVideoFX8BGRA);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;
        
        case N_COMMAND_VIDEO_FX8_BGR:
        {
            gpImage = gpVideoFX8BGR;
            SetImage(*gpVideoFX8BGR);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;

		case N_COMMAND_VIDEO_FX8_YUYV:
        {
            gpImage = gpVideoFX8YUYV;
            SetImage(*gpVideoFX8YUYV);
            gpApplicationInfo->setImageFormat(gpImage->pixelFormatString());
        }
        break;

		case N_COMMAND_VIDEO_FX8_UYVY:
        {
            gpImage = gpVideoFX8UYVY;
            SetImage(*gpVideoFX8UYVY);
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

        case N_COMMAND_CG_SIMPLE:
        {
			gpProgramCg = gpSimple_Cg;
			gpScene->setProgramCg(gpProgramCg);

            gpInteractionController->setPipelineMode(InteractionController::COLOR_CONTROL_MODE);
            gpApplicationInfo->setShader("Simple Shader (Cg)");
        }
        break;
        
        case N_COMMAND_CG_GAMMA:
        {
			gpProgramCg = gpGamma_Cg;
			gpScene->setProgramCg(gpProgramCg);

            gpInteractionController->setPipelineMode(InteractionController::COLOR_CONTROL_MODE);
            gpApplicationInfo->setShader("Gamma Shader (Cg)");
        }
        break;

        case N_COMMAND_CG_TILES:
        {
			gpProgramCg = gpTiles_Cg;
            gpScene->setProgramCg(gpProgramCg);

            gpInteractionController->setPipelineMode(InteractionController::EFFECT_MODE);
            gpApplicationInfo->setShader("Tiles (Cg)");
        }
        break;

        case N_COMMAND_CG_TVNOISE:
        {
			gpProgramCg = gpTVNoise_Cg;
            gpScene->setProgramCg(gpProgramCg);

            gpInteractionController->setPipelineMode(InteractionController::EFFECT_MODE);
            gpApplicationInfo->setShader("TV-Noise (Cg)");
        }
        break;

		case N_COMMAND_CG_NIGHT:
        {
			gpImageFilter = gpNightFilter;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::NIGHT_MODE);
            gpApplicationInfo->setShader("Night Filter (Cg)");
        }
        break;

        case N_COMMAND_CG_SCOTOPIC:
        {
			gpImageFilter = gpScotopicFilter;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::SCOTOPIC_MODE);
            gpApplicationInfo->setShader("Scotopic Filter (Cg)");
        }
        break;

        case N_COMMAND_CG_GAUSSIAN:
        {
			gpImageFilter = gpGaussFilter;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::GAUSSIAN_MODE);
            gpApplicationInfo->setShader("Gaussian (Cg)");
        }
        break;

        case N_COMMAND_CG_GAUSSIAN_1D:
        {
			gpImageFilter = gpGaussFilter1D;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::GAUSSIAN_1D_MODE);
            gpApplicationInfo->setShader("Gaussian Horizontal (Cg)");
        }
        break;

        case N_COMMAND_CG_GAUSSIAN_2PASS:
        {
			gpImageFilter = gpTwoPassGaussFilter;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::GAUSSIAN_2PASS_MODE);
            gpApplicationInfo->setShader("Gaussian 2-pass (Cg)");
        }
        break;

        case N_COMMAND_CG_BLOOM:
        {
			gpImageFilter = gpBloomFilter;
            gpNVImageLoader->setImagePusher(gpImageSource);
            gpScene->setNVImageFilter(gpImageFilter, gpNVImageLoader);

            gpInteractionController->setPipelineMode(InteractionController::BLOOM_MODE);
			gpApplicationInfo->setShader("Bloom Filter (Cg)");
        }
        break;

		case N_COMMAND_GLSL_LUMA:
        {
			gpProgramGLSL = gpLuma_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
            gpApplicationInfo->setShader("Luminance (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_SIMPLE:
        {
            gpProgramGLSL = gpSimple_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
            gpApplicationInfo->setShader("Simple Shader (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_GAMMA:
        {
            gpProgramGLSL = gpGamma_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::COLOR_CONTROL_MODE);
            gpApplicationInfo->setShader("Gamma Shader (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_NEGATIVE:
        {
			gpProgramGLSL = gpNegative_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
            gpApplicationInfo->setShader("Negative (GLSL)");
        }
        break;


		case N_COMMAND_GLSL_HALFTONE:
        {
            gpScene->enableGLSL(true);
			gpProgramGLSL = gpHalfTone_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);
            gpApplicationInfo->setShader("Half Tone (GLSL)");
        }
        break;

		case N_COMMAND_GLSL_SEPIA:
        {
			gpProgramGLSL = gpSepia_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::POST_PROCESS_MODE);
            gpApplicationInfo->setShader("Sepia (GLSL)");
        }
        break;

		case N_COMMAND_GLSL_PERLIN_NOISE:
        {
			gpProgramGLSL = gpPerlinNoise_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::POST_PROCESS_MODE);
            gpApplicationInfo->setShader("Perlin Noise (GLSL)");

//            gpProgramGLSL->setTextureID( tex_noise_sampler.texture, GL_TEXTURE_2D, 1 );
        }
        break;

		case N_COMMAND_GLSL_NOISE:
        {
			gpProgramGLSL = gpNoise_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::POST_PROCESS_MODE);
            gpApplicationInfo->setShader("Noise (GLSL)");

//            gpProgramGLSL->setTextureID( tex_noise_sampler.texture, GL_TEXTURE_2D, 1 );
        }
        break;

        case N_COMMAND_GLSL_EDGEDETECT:
        {
			gpProgramGLSL = gpEdgeDetect_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::POST_PROCESS_MODE);
            gpApplicationInfo->setShader("Edge Detect (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_EDGEOVERLAY:
        {
			gpProgramGLSL = gpEdgeOverlay_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::POST_PROCESS_MODE);
            gpApplicationInfo->setShader("Edge Overlay (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_COLORGRADIENT:
        {
			gpProgramGLSL = gpColorGrad_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
            gpApplicationInfo->setShader("Color Gradient (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_TILES:
        {
			gpProgramGLSL = gpTiles_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::EFFECT_MODE);
            gpApplicationInfo->setShader("Tiles (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_WOBBLE:
        {
			gpProgramGLSL = gpWobble_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::DISPLAY_MODE);
            gpApplicationInfo->setShader("Wobble (GLSL)");
        }
        break;

		case N_COMMAND_GLSL_TVNOISE:
        {
			gpProgramGLSL = gpTVNoise_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::TV_MODE);
            gpApplicationInfo->setShader("TV-Noise (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_RADIALBLUR:
        {
			gpProgramGLSL = gpRadialBlur_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::BLUR_MODE);
            gpApplicationInfo->setShader("Radial-Blur (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_WATERFALL:
        {
			gpProgramGLSL = gpWaterfall_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::EFFECT_MODE);
            gpApplicationInfo->setShader("Waterfall (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_BLOOM:
        {
			gpProgramGLSL = gpBloom_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::BLOOM_MODE);
            gpApplicationInfo->setShader("Invert (GLSL)");
        }
        break;

        case N_COMMAND_GLSL_OLDCAMERA:
        {
			gpProgramGLSL = gpOldCamera_GLSL;
            gpScene->setProgramGLSL(gpProgramGLSL);

            gpInteractionController->setPipelineMode(InteractionController::CAMERA_EFFECT_MODE);
            gpApplicationInfo->setShader("Old Camera (GLSL)");

//            gpProgramGLSL->setTextureID( tex_dust.texture,  GL_TEXTURE_2D, 1 );
//            gpProgramGLSL->setTextureID( tex_lines.texture, GL_TEXTURE_2D, 2 );
//            gpProgramGLSL->setTextureID( tex_tv.texture,    GL_TEXTURE_2D, 3 );
//            gpProgramGLSL->setTextureID( tex_noise.texture, GL_TEXTURE_3D, 4 );
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

    if (gpScene->isGLSL()) {
    	gpInteractionController->initShaderContext(&gpProgramGLSL);
	} else if (gpScene->isNVImage()) {
    	gpInteractionController->initShaderContext(&gpImageFilter);
	} else {
    	gpInteractionController->initShaderContext(&gpProgramCg);
    } 
}

        // key callback
void keyfunc(unsigned char  nKey, int nPositionX, int nPositionY)
{
    menu(nKey);
}


        // mouse
        //
        void
mouse(int nButton, int nState, int nX, int nY)
{
    if (gbSliders)
        gpInteractionController->mouse((nX - gpApplicationInfo->width()), nY);
    else
    {
        switch (nButton)
        {
            case GLUT_LEFT_BUTTON:
            {
                if (GLUT_DOWN == nState)
                {
                    gnOldX   = nX;
                    gnOldY   = nY;
                    gbMoving = true;
                }   
                else 
                {
                    gbMoving = false;
                }
            }
            break;

            case GLUT_MIDDLE_BUTTON:
            {
                if (GLUT_DOWN == nState)
                {
                    gnOldX = nX;
                    gnOldY = nY;
                    gbScaling = true;
                }
                else
                {
                    gbScaling = false;
                }
            }
            break;

            case GLUT_RIGHT_BUTTON:
            {

            }
            break;
        }
    }
    

    glutPostRedisplay();
}

        // mouseMove
        //
        void
mouseMove(int nX, int nY)
{
    if (gbSliders)
        gpInteractionController->move((nX - gpApplicationInfo->width()), nY);
    else
    {    
        if (gbMoving)
//            gpView->setImagePosition(gpView->imagePositionX() + nX - gnOldX,
//                                    gpView->imagePositionY() + gnOldY - nY);

        if (gbScaling)
//            gpView->setZoomFactor(max(0, (gpView->zoomFactor() + 
//                                ((nX - gnOldX) + (gnOldY - nY))/100.0f)));
            
        gnOldX = nX;
        gnOldY = nY;
    }
    glutPostRedisplay();
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


        // reshaper
        //
        void
reshaper(int nWidth, int nHeight)
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

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, gnWindowWidth, 0, gnWindowHeight);
    GL_ASSERT_NO_ERROR;
}


#define USE_BITMAP 0


        // InitImages
        //
        // Description:
        //      Load image from disk and create all different support formats.
        //
        // Parameters:
        //      zImageFileName - image file to be loaded.
        //
        // Returns:
        //      return true if success, false if failure
        //
        bool
InitImages(const char * zImageFileName)
{
#if USE_BITMAPx
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( (LPCTSTR)"..\\mandrill.bmp" );
    gpImageFX8BGRA   = new ImageTex ();
    gpImageFX8BGRA->setData(pTextureImage->sizeX, pTextureImage->sizeY, ImageTex ::FX8_BGRA_PIXEL, pTextureImage->data);

    gpImageFP16RGBA  = new ImageTex ();
    *gpImageFP16RGBA = ImageTex ::ConvertPixelFormat(ImageTex ::FP16_RGBA_PIXEL, *gpImageFX8BGRA);

    gpImageFX8RGBA   = new ImageTex ();
    *gpImageFX8RGBA  = ImageTex ::ConvertPixelFormat(ImageTex ::FX8_RGBA_PIXEL, *gpImageFX8BGRA);

    gpImage = gpImageFP16RGBA;

#else
    OpenEXRLoader     * pLoadEXR = NULL;
//    AUX_RGBImageRec   * pLoadBMP = NULL;
    nv_dds::CDDSImage * pLoadDDS = NULL;

    int length = (int)strlen(zImageFileName);

    gpImageFP16RGBA  = new ImageTex ();

    if (!_stricmp(&zImageFileName[length-4], ".exr")) {
        pLoadEXR = new OpenEXRLoader(zImageFileName);
        *gpImageFP16RGBA = pLoadEXR->image();
    } else if (!_stricmp(&zImageFileName[length-3], ".bmp")) {
//        pLoadBMP = auxDIBImageLoad( (LPCTSTR)"..\\mandrill.bmp" );
//        *gpImageFP16RGBA = oOpenEXRLoader.image();
    } else if (!_stricmp(&zImageFileName[length-3], ".dds")) {
//        pLoadDDS = new OpenEXRLoader(zImageFileName);
//        *gpImageFP16RGBA = pLoadDDS;
    }

    gpImage = gpImageFP16RGBA;
#endif
    return true;
}

        // InitVideo
        //
        // Description:
        //      Build a FilterGraph with the input file
        //
        // Parameters:
        //      zVideoFileName - video file to be loaded.
        //      pScene         - handle to the Scene class for rendering
        //
        // Returns:
        //      true if success, false if failure
        //
        bool 
InitVideo(const char * zVideoFileName, Scene *pScene)
{
    // now let's initialize some stuff for Video
    // 1) Load DirectShow Graphics, connect pins

    // 2) Initialize Video Buffer so it uses ImageTex  library to swap
    //    your video frames

    WCHAR wstrVideoFile[256];

    DXUtil_ConvertAnsiStringToWide( wstrVideoFile, zVideoFileName, 256 );

    // First clean up the old Filter Graph
    if (gpGraph)
        delete gpGraph;

 	gpGraph = new GraphBuilder(pScene);

    HRESULT hr = gpGraph->InitTextureRenderer(wstrVideoFile, &mMutex, TRUE);
    if (FAILED(hr)) {
        if (hr != VFW_E_NOT_FOUND) {
         Msg(TEXT("Initializing DshowTextureRenderer [pGB=0x%08x] FAILED!!  hr=0x%x"), (DWORD *)gpGraph, hr);
        }
    }

	gpVideoFX8YUYV   = new ImageTex (gpGraph->getWidth(), gpGraph->getHeight(), ImageTex ::FX8_YUYV_PIXEL, NULL);
	gpVideoFX8UYVY   = new ImageTex (gpGraph->getWidth(), gpGraph->getHeight(), ImageTex ::FX8_UYVY_PIXEL, NULL);

    gpImage = gpVideoFX8YUYV;
    return true;
}

        // InitTextures
        //
        // Description:
        //      Set up all the special textures for advanced effecgts
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        bool
InitTextures()
{
#if USE_TEXTURES
	progpath.path.clear();
    progpath.path.push_back(".");
    progpath.path.push_back(MISC_IMAGE_PATH);

    pathname = progpath.get_file("dust.dds");
    if (!pathname.empty()) {
		tex_dust.bind();
        if (dds_dust.load(pathname)) {
            tex_dust.bind();
            dds_dust.upload_texture2D();
            tex_dust.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            tex_dust.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			tex_dust.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			tex_dust.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
	}

    pathname = progpath.get_file("line.dds");
    if (!pathname.empty()) {
		if (dds_lines.load(pathname)) {
            tex_lines.bind();
            dds_lines.upload_texture2D();
            tex_lines.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            tex_lines.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			tex_lines.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			tex_lines.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }

    pathname = progpath.get_file("tv.dds");
    if (!pathname.empty()) {
      if (dds_tv.load(pathname)) {
            tex_tv.bind();
            dds_tv.upload_texture2D();
            tex_tv.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            tex_tv.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			tex_tv.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			tex_tv.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
	}

	pathname = progpath.get_file("tv.dds");
    if (!pathname.empty()) {
      if (dds_noise.load(pathname)) {
            tex_noise.bind();
            dds_noise.upload_texture2D();
			tex_noise.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			tex_noise.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			tex_noise.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
			tex_noise.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
    }

	tex_noise_sampler.bind();
	tex_noise_sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	tex_noise_sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	tex_noise_sampler.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	tex_noise_sampler.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    srand(250673);
#endif
	return true;
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
        bool
InitPipeline(const char * zImageFileName, const char *zVideoFileName)
{
    bool retval = true;

    gpScene = new Scene();
    gpScene->enableFBO(bUseFBO);
    
    gpApplicationInfo = new ApplicationInfo();
    gpScene->setApplicationInfo(gpApplicationInfo); 

    retval = InitImages(zImageFileName);
    if (!retval) {
        return false;
    }

#if 1
    retval = InitVideo(zVideoFileName, gpScene);

    if (!retval) {
        return false;
    }
#endif

	gpStaticImage           = new StaticImage         ();
    gpImagePusherPBO        = new ImagePusherPBO      ();
    gpVideoPusherPBO        = new VideoPusherPBO      ();
	gpImageSinkDummy        = new ImageSinkDummy();
    gpImagePuller           = new ImagePuller();
    gpImagePullerPBO        = new ImagePullerPBO();

    gpNVImageLoader			= new NVImageLoader;
    GL_ASSERT_NO_ERROR;

    gpNightFilter           = new NightFilter;
    GL_ASSERT_NO_ERROR;

	gpScotopicFilter        = new ScotopicFilter;
    GL_ASSERT_NO_ERROR;
    
    gpGaussFilter1D         = new GaussFilter1D;
    GL_ASSERT_NO_ERROR;
    gpTwoPassGaussFilter    = new TwoPassGaussFilter;
    GL_ASSERT_NO_ERROR;

    gpInteractionController = new InteractionController(*gpGaussFilter,
                                                        *gpNightFilter,
                                                        *gpScotopicFilter,
                                                        *gpGaussFilter1D,
                                                        *gpTwoPassGaussFilter,
														*gpBloomFilter,
                                                        *gpSaveOperator
														);

    gpScene->initController(gpInteractionController);

    // Set up the path to load files from
    File::addPath(".");
    File::addPath(SHADER_PATH "gpu_videoeffects/");

    gpLuma_GLSL      = new ProgramGLSL("VS_Quad.glsl",          "PS_luminance.glsl");
    gpSimple_GLSL    = new ProgramGLSL("VS_Quad.glsl",          "PS_simple.glsl");
    gpGamma_GLSL     = new ProgramGLSL("VS_Quad.glsl",          "PS_gamma.glsl");

	gpNegative_GLSL	 = new ProgramGLSL("VS_Quad.glsl",          "PS_negative.glsl");
    gpHalfTone_GLSL  = new ProgramGLSL("VS_Quad.glsl",          "PS_halftone.glsl");
    gpSepia_GLSL     = new ProgramGLSL("VS_Quad.glsl",          "PS_sepia.glsl");

	gpTiles_GLSL     = new ProgramGLSL("VS_Quad.glsl",          "PS_tiles.glsl");
    gpWobble_GLSL    = new ProgramGLSL("VS_wobble.glsl",        "PS_wobble.glsl");
    gpTVNoise_GLSL   = new ProgramGLSL("VS_tvnoise.glsl",       "PS_tvnoise.glsl");

    gpRadialBlur_GLSL= new ProgramGLSL("VS_radialblur.glsl",    "PS_radialblur.glsl");
	gpWaterfall_GLSL = new ProgramGLSL("VS_Quad.glsl",			"PS_waterfall.glsl");

    gpEdgeDetect_GLSL= new ProgramGLSL("VS_edgedetect.glsl",    "PS_edgedetect.glsl");
    gpEdgeOverlay_GLSL=new ProgramGLSL("VS_edgedetect.glsl",    "PS_edgeoverlay.glsl");
    gpColorGrad_GLSL = new ProgramGLSL("VS_Quad.glsl",			"PS_colorGradient.glsl");
    gpOldCamera_GLSL = new ProgramGLSL("VS_oldcamera.glsl",     "PS_oldcamera.glsl");

#if 0
    gpNoise_GLSL     = new ProgramGLSL("VS_Quad.glsl",          "PS_noise.glsl");
    gpPerlinNoise_GLSL=new ProgramGLSL("VS_Quad.glsl",          "PS_noise_perlin.glsl");
#endif

    // Load some textures used for some advanced transitions
    progpath.path.clear();
    progpath.path.push_back("./Textures");
    InitTextures();

    menu(IsVideoSelected() ? N_COMMAND_VIDEO_PBO : N_COMMAND_IMAGE_PBO);
//    menu(N_COMMAND_STATIC_IMAGE);
//    menu(N_COMMAND_SINK_PBO);
	menu(N_COMMAND_SINK_DUMMY);
    menu(IsVideoSelected() ? N_COMMAND_VIDEO_FX8_YUYV : N_COMMAND_IMAGE_FP16_RGBA);
    menu(N_COMMAND_INTERNAL_DEFAULT);
    menu(N_COMMAND_GLSL_GAMMA);

    gpScene->setInvertTexCoords(IsVideoSelected());

    return true;
}


void initMenu()
{
    gidSourceFileMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Load Video",                    N_COMMAND_SOURCE_FILE_VIDEO);
    glutAddMenuEntry("Load Image",                    N_COMMAND_SOURCE_FILE_IMAGE);

    gidImageSourceMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Video PBO",                       N_COMMAND_VIDEO_PBO);
    glutAddMenuEntry("Image PBO",                       N_COMMAND_IMAGE_PBO);

	gidImageSinkMenu = glutCreateMenu(menu);
    glutAddMenuEntry("No Readback",                     N_COMMAND_SINK_DUMMY);
    glutAddMenuEntry("PBO Asynchronous Reads",          N_COMMAND_SINK_PBO);
    glutAddMenuEntry("Normal (glReadPixels)",           N_COMMAND_SINK_NORMAL);
    
    gidImageFormatMenu = glutCreateMenu(menu);
    if (gpImageFP16RGBA)glutAddMenuEntry("FP16 RGBA",                       N_COMMAND_IMAGE_FP16_RGBA);
    if (gpImageFP16RGB) glutAddMenuEntry("FP16 RGB",                        N_COMMAND_IMAGE_FP16_RGB);
    if (gpImageFX8RGBA) glutAddMenuEntry("FX8  RGBA",                       N_COMMAND_IMAGE_FX8_RGBA);
    if (gpImageFX8RGB)  glutAddMenuEntry("FX8  RGB",                        N_COMMAND_IMAGE_FX8_RGB);
    if (gpImageFX8BGRA) glutAddMenuEntry("FX8  BGRA",                       N_COMMAND_IMAGE_FX8_BGRA);
    if (gpImageFX8BGR)  glutAddMenuEntry("FX8  BGR ",                       N_COMMAND_IMAGE_FX8_BGR);
	if (gpVideoFX8BGRA) glutAddMenuEntry("FX8  BGRA (video)",               N_COMMAND_VIDEO_FX8_BGRA);
    if (gpVideoFX8BGR)  glutAddMenuEntry("FX8  BGR  (video)",               N_COMMAND_VIDEO_FX8_BGR);
    if (gpVideoFX8YUYV) glutAddMenuEntry("FX8  YUYV (video)",               N_COMMAND_VIDEO_FX8_YUYV);
    if (gpVideoFX8UYVY) glutAddMenuEntry("FX8  UYVY (video)",               N_COMMAND_VIDEO_FX8_UYVY);

	gidInternalImageFormatMenu = glutCreateMenu(menu);
    glutAddMenuEntry("FP16 RGBA",                       N_COMMAND_INTERNAL_FP16_RGBA);
    glutAddMenuEntry("FP16 RGB",                        N_COMMAND_INTERNAL_FP16_RGB);
    glutAddMenuEntry("FX8 RGBA",                        N_COMMAND_INTERNAL_FX8_RGBA);
    glutAddMenuEntry("FX8 RGB",                         N_COMMAND_INTERNAL_FX8_RGB);


    gidPlayerMenu = glutCreateMenu(menu);
    glutAddMenuEntry("\".\" - Play/Pause",              N_COMMAND_VIDEO_PLAYPAUSE);
	glutAddMenuEntry("\">\" - Jump +10 secs",			N_COMMAND_VIDEO_FF_10SEC);
	glutAddMenuEntry("\"<\" - Jump -10 secs",			N_COMMAND_VIDEO_REW_10SEC);

    gidShaderCgMenu = glutCreateMenu(menu);
    if (gpSimple_Cg)			glutAddMenuEntry("Simple (Cg)",                     N_COMMAND_CG_SIMPLE);
    if (gpGamma_Cg)			    glutAddMenuEntry("Gamma HDR (Cg)",                  N_COMMAND_CG_GAMMA);
    if (gpTiles_Cg)			    glutAddMenuEntry("Tiles (Cg)",                      N_COMMAND_CG_TILES);
    if (gpTVNoise_Cg)		    glutAddMenuEntry("TV-Noise (Cg)",                   N_COMMAND_CG_TVNOISE);

    if (gpNightFilter)			glutAddMenuEntry("Night Filter (Cg)",				N_COMMAND_CG_NIGHT);
	if (gpScotopicFilter)		glutAddMenuEntry("Scotopic Filter (Cg)",			N_COMMAND_CG_SCOTOPIC);
	if (gpGaussFilter)			glutAddMenuEntry("Gaussian (Cg)",					N_COMMAND_CG_GAUSSIAN);
	if (gpGaussFilter1D)		glutAddMenuEntry("Gaussian Horizontal (Cg)",		N_COMMAND_CG_GAUSSIAN_1D);
	if (gpTwoPassGaussFilter)	glutAddMenuEntry("Gaussian 2-pass (Cg)",			N_COMMAND_CG_GAUSSIAN_2PASS);
	if (gpBloomFilter)			glutAddMenuEntry("Bloom Filter (Cg)",				N_COMMAND_CG_BLOOM);

    gidShaderGLSLMenu = glutCreateMenu(menu);
    if (gpLuma_GLSL)            glutAddMenuEntry("Luminance (GLSL)",                N_COMMAND_GLSL_LUMA);
    if (gpSimple_GLSL)			glutAddMenuEntry("Simple HDR (GLSL)",               N_COMMAND_GLSL_SIMPLE);
    if (gpGamma_GLSL)			glutAddMenuEntry("Gamma HDR (GLSL)",                N_COMMAND_GLSL_GAMMA);
    if (gpNegative_GLSL)        glutAddMenuEntry("Negative (GLSL)",                 N_COMMAND_GLSL_NEGATIVE);
    if (gpHalfTone_GLSL)        glutAddMenuEntry("Half Tone (GLSL)",                N_COMMAND_GLSL_HALFTONE);
	if (gpSepia_GLSL)           glutAddMenuEntry("Sepia (GLSL)",                    N_COMMAND_GLSL_SEPIA);
	if (gpNoise_GLSL)           glutAddMenuEntry("Noise (GLSL)",                    N_COMMAND_GLSL_NOISE);
	if (gpPerlinNoise_GLSL)     glutAddMenuEntry("Perlin Noise (GLSL)",             N_COMMAND_GLSL_NOISE);

    if (gpEdgeDetect_GLSL)      glutAddMenuEntry("Edge Detect (GLSL)",              N_COMMAND_GLSL_EDGEDETECT);
    if (gpEdgeOverlay_GLSL)     glutAddMenuEntry("Edge Overlay (GLSL)",             N_COMMAND_GLSL_EDGEOVERLAY);
    if (gpColorGrad_GLSL)       glutAddMenuEntry("Color Gradient (GLSL)",           N_COMMAND_GLSL_COLORGRADIENT);
    if (gpTiles_GLSL)           glutAddMenuEntry("Tiles (GLSL)",                    N_COMMAND_GLSL_TILES);
    if (gpRadialBlur_GLSL)      glutAddMenuEntry("Radial-Blur (GLSL)",              N_COMMAND_GLSL_RADIALBLUR);
    if (gpWaterfall_GLSL)       glutAddMenuEntry("Waterfall (GLSL)",                N_COMMAND_GLSL_WATERFALL);

	if (gpWobble_GLSL)          glutAddMenuEntry("Wobble (GLSL)",                   N_COMMAND_GLSL_WOBBLE);
    if (gpTVNoise_GLSL)         glutAddMenuEntry("TV-Noise (GLSL)",                 N_COMMAND_GLSL_TVNOISE);
    if (gpBloom_GLSL)           glutAddMenuEntry("Bloom (GLSL)",                    N_COMMAND_GLSL_BLOOM);
    if (gpOldCamera_GLSL)       glutAddMenuEntry("Old-Camera (GLSL)",               N_COMMAND_GLSL_OLDCAMERA);

    gidWorkloadMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Enable",                          N_COMMAND_WORKLOAD_ENABLE);
    glutAddMenuEntry("Disable",                         N_COMMAND_WORKLOAD_DISABLE);

    gidMainMenu = glutCreateMenu(menu);
    glutAddSubMenu("Source File",     gidSourceFileMenu);
    glutAddSubMenu("Source Upload",   gidImageSourceMenu);
    glutAddSubMenu("Image Readback",  gidImageSinkMenu);
//    glutAddSubMenu("Image Format",    gidImageFormatMenu);
//    glutAddSubMenu("Internal Image Format", gidInternalImageFormatMenu);
	glutAddSubMenu("Playback Control",	gidPlayerMenu);
    glutAddSubMenu("Shader Effects (Cg)",     gidShaderCgMenu);
    glutAddSubMenu("Shader Effects (GLSL)",   gidShaderGLSLMenu);
    glutAddSubMenu("Dummy Workload",  gidWorkloadMenu);
    glutAddMenuEntry("Quit\t[ESC]",   N_COMMAND_QUIT);

    glutAttachMenu( GLUT_RIGHT_BUTTON );
}

void main(int nArgs, char * aArgs[])
{
    bool retval;

    gbMoving     = false;
    gbScaling    = false;
    gbSliders    = true;
    gbFullScreen = false;

    glutInit(&nArgs, aArgs);
    
    gnWindowWidth  = 100;
    gnWindowHeight = 100;
    
    glutInitWindowSize(gnWindowWidth, gnWindowHeight);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("NVIDIA GPU Video and Effects");
    GL_ASSERT_NO_ERROR;
       
    initOpenGL();

    retval = InitPipeline(gzImageName[0], gzWMV9_Video[1]);

    if (!retval) {
        return;
    }

    initMenu();

    initGL();
    GL_ASSERT_NO_ERROR;

    glutDisplayFunc(display);
    glutReshapeFunc(reshaper);
    glutKeyboardFunc(keyfunc);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    glutIdleFunc(idle);
    GL_ASSERT_NO_ERROR;

    glutReshapeWindow(max(static_cast<int>(gpScene->minimumWindowWidth()), MIN_WIDTH),
                          static_cast<int>(gpScene->minimumWindowHeight()));

	if (gpGraph) {
		gpGraph->RunGraph();
		gpPlayerRunning = true;
	}

    goTimer.start();
    glutMainLoop();
    goTimer.stop();
    
    if (gpGraph) {
        gpGraph->StopGraph();
        gpPlayerRunning = true;
    }
}
