// -----------------------------------------------------------------------------
// 
// Contents:
//      NVImageLoader class
//
// Description:
//      NVImageLoader loads DDS images from files.
//          The operator has some infrastructure to support additional image
//      formats in the future.
//
// Author:
//      Frank Jargstorff (2003)
//
// -----------------------------------------------------------------------------


//
// Includes
//
#define USE_RTCLASS 1

#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glu.h>
#include <GL/glut.h>

#include "StaticImage.h"
#include "ImagePusher.h"
#include "ImagePusherPBO.h"
#include "ImagePusherDoublePBO.h"
#include "VideoPusher.h"
#include "VideoPusherPBO.h"
#include "VideoPusherDoublePBO.h"
#include "AssertGL.h"

#include "NVImageLoader.h"
#include "defines.h"

#include <nv_dds.h>
#include <shared/data_path.h>
//#include <shared/pbuffer.h>

#include "RenderTexture2.h"
#include <ShaderManager.h>

#include <AssertCG.h>

#define USE_NVIMAGE_PROC 1

//
// Namespaces
//

using namespace nv_dds;


// -----------------------------------------------------------------------------
// NVImageLoader implementation
//


// 
// Construction and destruction
//

        // Default constructor
        //
NVImageLoader::NVImageLoader(): _bDirty(false)
							, _bUseYUV(true)
                            , _sFileName("")
                            , _eFileType(NVImageLoader::VIDEO_FILE)
                            , _goYUV2RGBShader(0)
                            , _hYUVImage(0)
                            , _goRGBAShader(0)
                            , _hRGBAImage(0)
							, _hRenderTarget(NULL)
//							, pbuffer(NULL)
{
    if (_goYUV2RGBShader == 0)
    {                         // Set up the fragment program
        _goYUV2RGBShader = cgCreateProgramFromFile(ShaderManager::gCgContext, CG_SOURCE, 
                               SHADER_PATH "gpu_videoeffects/yuv2rgb.cg",
                               cgFragmentProfile(), 
                               0, 0
                             );
        CG_ASSERT_NO_ERROR;

        cgGLLoadProgram(_goYUV2RGBShader);
        CG_ASSERT_NO_ERROR;
    }
    if (_hYUVImage == 0)
        _hYUVImage = cgGetNamedParameter(_goYUV2RGBShader, "tex0");

	if (_goRGBAShader == 0)
	{
        _goRGBAShader = cgCreateProgramFromFile(ShaderManager::gCgContext, CG_SOURCE, 
                               SHADER_PATH "gpu_videoeffects/texture.cg", 
                               cgFragmentProfile(), 
                               0, 0
                             );
        CG_ASSERT_NO_ERROR;

        cgGLLoadProgram(_goRGBAShader);
        CG_ASSERT_NO_ERROR;
	}
    if (_hRGBAImage == 0)
        _hRGBAImage = cgGetNamedParameter(_goRGBAShader, "oImage");

    // Set the Fragment Program
    _oCgFragmentProgram = _goYUV2RGBShader;

    // Set the input image parameter
    _hoInputImage = _hYUVImage;
    GL_ASSERT_NO_ERROR;
}

NVImageLoader::~NVImageLoader()
{
//	if (pbuffer)
//	    delete pbuffer;
	if (_hRenderTarget)
		delete _hRenderTarget;
}


//
// Public methods
//

        // setFilename
        //
        // Description:
        //      Set the name of the image to be loaded.
        //
        // Parameters:
        //      sFileName - Name (and path) of the image to load.
        //
        // Returns:
        //      None
        //
        void
NVImageLoader::setFileName(std::string sFileName)
{
    _sFileName = sFileName;
    _bDirty = true;
}

        // setFileType
        //
        // Description:
        //      Set the image file type of the image to be loaded.
        //
        // Parameters:
        //      eFileType - One of the file types listed in the 
        //          file-type enum.
        //
        // Returns:
        //      None
        //
        // Note:
        //      The class's default filetype is DDS_FILE.
        //
        void
NVImageLoader::setFileType(NVImageLoader::teFileType eFileType)
{
    _eFileType = eFileType;
    _bDirty = true;

	if (VIDEO_FILE == _eFileType) {
		_oCgFragmentProgram	= _goYUV2RGBShader;
		_hoInputImage		= _hYUVImage;
	} else {
		_oCgFragmentProgram = _goRGBAShader;
		_hoInputImage		= _hRGBAImage;
	}
}

        // fileType
        //
        // Description:
        //      Returns the current file type.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      The current filetype.
        //
        NVImageLoader::teFileType
NVImageLoader::fileType()
        const
{
    return _eFileType;
}

        // dirty
        //
        // Description:
        //      Has the state of the operator or any operators
        //      that this operator draws data from changed?
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A boolean telling whether the operator is dirty.
        //
        bool
NVImageLoader::dirty()
{
    return _bDirty;
}

        // image
        //
        // Description:
        //      Gets the operator's output image.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image.
        //
        Image
NVImageLoader::image()
{
    if (_eFileType == DDS_FILE) {
        if (_bDirty)
        {
            if (!loadImage(&_oImage))
            {
                std::cerr << "Couldn't load image." << std::endl;
                assert(false);
            }
            
            _bDirty = false;
        }
        return _oImage;
	} else {
//		render_to_pbuffer();
//		render_to_screen();
		_oImage = render_to_image_buffer();
        return _oImage;
    }
}

        // imagefBO
        //
        // Description:
        //      Gets the operator's output image.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image via FBO.
        //
		/*
        ImageFBO
NVImageLoader::imageFBO()
{
    if (_eFileType == DDS_FILE) {
        if (_bDirty)
        {
            if (!loadImage(&_oImage))
            {
                std::cerr << "Couldn't load image." << std::endl;
                assert(false);
            }
            
            _bDirty = false;
        }
        return _oImageFBO;
    } else {
//		render_to_pbuffer();
//		render_to_screen();
		_oImageFBO = render_to_image_FBO();
        return _oImageFBO;
    }
}
*/
        // render_to_image_buffer
        //
        // Description:
        //
        // Parameters:
        //
        // Returns:
        //
        Image
NVImageLoader::render_to_image_buffer()
{
	Image oOutputImage;
    int nWidth =  _pImagePusher->image().width();
    int nHeight = _pImagePusher->image().height();

    {
        oOutputImage.setSize(nWidth, nHeight);
		oOutputImage.renderBegin();
        {
    		_pImagePusher->pushNewFrame();
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) nWidth, (GLsizei) nHeight);
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, nWidth, 0, nHeight);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
                                // Set Cg state
            cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
            cgGLBindProgram(ShaderManager::gVertexIdentityShader);

            cgGLEnableProfile(cgFragmentProfile());
            cgGLBindProgram(_oCgFragmentProgram);

                                // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float fWidth  = static_cast<float>(nWidth);
            const float fHeight = static_cast<float>(nHeight);

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(fWidth, 0.0f);     glVertex3f(fWidth, 0.0f,    0.0f);
                glTexCoord2f(fWidth, fHeight);  glVertex3f(fWidth, fHeight, 0.0f);
                glTexCoord2f(0.0f,   fHeight);  glVertex3f(  0.0f, fHeight, 0.0f);
            glEnd();

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        oOutputImage.renderEnd();

        _bDirty = false;
    }

    return oOutputImage;
}

        // render_to_image_buffer
        //
        // Description:
        //
        // Parameters:
        //
        // Returns:
        //
        //ImageFBO
		/*
NVImageLoader::render_to_image_FBO()
{
	ImageFBO oOutputImage;
    int nWidth =  _pImagePusher->image().width();
    int nHeight = _pImagePusher->image().height();

    {
        oOutputImage.setSize(nWidth, nHeight);
		oOutputImage.renderBegin();
        {
    		_pImagePusher->pushNewFrame();
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) nWidth, (GLsizei) nHeight);
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, nWidth, 0, nHeight);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
                                // Set Cg state
            cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
            cgGLBindProgram(ShaderManager::gVertexIdentityShader);

            cgGLEnableProfile(cgFragmentProfile());
            cgGLBindProgram(_oCgFragmentProgram);

                                // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float fWidth  = static_cast<float>(nWidth);
            const float fHeight = static_cast<float>(nHeight);

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(fWidth, 0.0f);     glVertex3f(fWidth, 0.0f,    0.0f);
                glTexCoord2f(fWidth, fHeight);  glVertex3f(fWidth, fHeight, 0.0f);
                glTexCoord2f(0.0f,   fHeight);  glVertex3f(  0.0f, fHeight, 0.0f);
            glEnd();

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        oOutputImage.renderEnd();

        _bDirty = false;
    }

    return oOutputImage;
}
*/
        // render_to_screen
        //
        // Description:
        //
        // Parameters:
        //
        // Returns:
        //
        void
NVImageLoader::render_to_screen()
{
    // here lets us render the image to do our YUV->RGB conversion
    // before calling this function, make sure the texture has been bound
    // and uploaded
    int nWidth =  _pImagePusher->image().width();
    int nHeight = _pImagePusher->image().height();

	_pImagePusher->pushNewFrame();
    {
        glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);       
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set OpenGL state
        glViewport(0, 0, nWidth, nHeight);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluOrtho2D(0, nWidth, 0, nHeight);

        // Set Cg state
        cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                    CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                    CG_GL_MATRIX_IDENTITY);

        cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
        cgGLBindProgram(ShaderManager::gVertexIdentityShader);

        cgGLEnableProfile(cgFragmentProfile());
        cgGLBindProgram(_oCgFragmentProgram);
		
//		cgGLEnableTextureParameter(_gInputImage); // Very important to allow Cg to setup the units

        const float fWidth  = static_cast<float>(nWidth);
        const float fHeight = static_cast<float>(nHeight);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
            glTexCoord2f(fWidth, 0.0f);     glVertex3f(fWidth, 0.0f,    0.0f);
            glTexCoord2f(fWidth, fHeight);  glVertex3f(fWidth, fHeight, 0.0f);
            glTexCoord2f(0.0f,   fHeight);  glVertex3f(  0.0f, fHeight, 0.0f);
        glEnd();

        cgGLDisableProfile(CG_PROFILE_VP20);
        cgGLDisableProfile(cgFragmentProfile());
    }
}

        // render_to_pbuffer
        //
        // Description:
        //
        // Parameters:
        //
        // Returns:
        //
        void
NVImageLoader::render_to_pbuffer()
{
    // here lets us render the image to do our YUV->RGB conversion
    // before calling this function, make sure the texture has been bound
    // and uploaded
    int nWidth =  _pImagePusher->image().width();
    int nHeight = _pImagePusher->image().height();

	_pImagePusher->pushNewFrame();  // this will bind the appropriate texture

#if USE_RTCLASS
	_hRenderTarget->BeginCapture();
#else
//    pbuffer->Release(WGL_FRONT_LEFT_ARB);
    pbuffer->Activate();
#endif

    {
        glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);       
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set OpenGL state
        glViewport(0, 0, nWidth, nHeight);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluOrtho2D(0, nWidth, 0, nHeight);

        // Set Cg state
        cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                    CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                    CG_GL_MATRIX_IDENTITY);

        cgGLEnableProfile(ShaderManager::gVertexIdentityProfile);
        cgGLBindProgram(ShaderManager::gVertexIdentityShader);

        cgGLEnableProfile(cgFragmentProfile());
        cgGLBindProgram(_oCgFragmentProgram);
		
//		cgGLEnableTextureParameter(_gInputImage); // Very important to allow Cg to setup the units

        const float fWidth  = static_cast<float>(nWidth);
        const float fHeight = static_cast<float>(nHeight);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
            glTexCoord2f(fWidth, 0.0f);     glVertex3f(fWidth, 0.0f,    0.0f);
            glTexCoord2f(fWidth, fHeight);  glVertex3f(fWidth, fHeight, 0.0f);
            glTexCoord2f(0.0f,   fHeight);  glVertex3f(  0.0f, fHeight, 0.0f);
        glEnd();

        cgGLDisableProfile(CG_PROFILE_VP20);
        cgGLDisableProfile(cgFragmentProfile());

        // Copy the results to a output p-buffer
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, pbuffer_tex);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, nWidth, nHeight);
    }
#if USE_RTCLASS
	_hRenderTarget->EndCapture();
#else
    pbuffer->Deactivate();
#endif
}


        // displayPbuffer
        //
        // Description:
        //      This shows the p-buffer object (render to screen)
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        // 
        void
NVImageLoader::displayPbuffer(int win_width, int win_height)
{
#if USE_NVIMAGE_PROC
  int nWidth = _oImage.width();
  int nHeight = _oImage.height();
#else
  int nWidth = pbuffer->GetWidth();
  int nHeight = pbuffer->GetHeight();

  // display pbuffer contents by drawing a textured quad to window
#if USE_RTCLASS
//  _hRenderTarget->EndCapture();
#else
  pbuffer->Deactivate();
  pbuffer->Bind(WGL_FRONT_LEFT_ARB);
#endif

  // bind pbuffer texture to the current texture unit
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, pbuffer_tex);
#endif

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);       
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, (GLsizei) win_width, (GLsizei) win_height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluOrtho2D(0, win_width, 0,  win_height);

#if USE_NVIMAGE_PROC
  cgGLSetTextureParameter(_hYUVImage, _oImage.textureID());
  cgGLEnableTextureParameter(_hYUVImage);
#else
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, display_fprog);
#endif

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
     glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
     glTexCoord2f(nWidth, 0.0f);     glVertex3f(nWidth, 0.0f,    0.0f);
     glTexCoord2f(nWidth, nHeight);  glVertex3f(nWidth, nHeight, 0.0f);
     glTexCoord2f(0.0f,   nHeight);  glVertex3f(  0.0f, nHeight, 0.0f);
  glEnd();

#if USE_NVIMAGE_PROC
    cgGLDisableTextureParameter(_hYUVImage);
    cgGLDisableProfile(ShaderManager::gVertexIdentityProfile);
    cgGLDisableProfile(CG_PROFILE_FP30);
#else
  #if USE_RTCLASS
    _hRenderTarget->UnBind();
  #else
    pbuffer->Release(WGL_FRONT_LEFT_ARB);	// release the texture binding
  #endif
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);
#endif

  glFinish();
}

        // bindPBuffer
        //
        // Description:
        //      This shows the p-buffer object (render to screen)
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        // 
        void
NVImageLoader::bindPBuffer()
{
  // display pbuffer contents by drawing a textured quad to window
#if USE_RTCLASS
	_hRenderTarget->Bind();
#else
//  pbuffer->Deactivate();
  pbuffer->Bind(WGL_FRONT_LEFT_ARB);	// let's bind it to a texture now
#endif
}

        // releasePBuffer
        //
        // Description:
        //      This shows the p-buffer object (render to screen)
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        // 
        void
NVImageLoader::releasePBuffer()
{
  // display pbuffer contents by drawing a textured quad to window
  #if USE_RTCLASS
    _hRenderTarget->UnBind();
  #else
    pbuffer->Release(WGL_FRONT_LEFT_ARB);	// release the texture binding
  #endif
}

        // setImagePusher
        //
        // Description:
        //      This is a helper method that defines the ImageSource class
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        // 
        void
NVImageLoader::setImagePusher(ImageSource * pImage)
{
    _eFileType = VIDEO_FILE;
    _pImagePusher = pImage;

    int nWidth =  _pImagePusher->image().width();
    int nHeight = _pImagePusher->image().height();

#if USE_NVIMAGE_PROC
    _oImage.setSize(nWidth, nHeight);
#else
    // Setups up nv_image_processing class (for 16-bit pbuffer rendering size)
	if(pbuffer)
		delete pbuffer;

	pbuffer = new PBuffer("float=16 rgba textureRECT");
    pbuffer->Initialize(nWidth, nHeight, false, true);
	pbuffer->Activate();
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pbuffer_tex = createTexture(GL_TEXTURE_RECTANGLE_NV, GL_FLOAT_RGBA16_NV, nWidth, nHeight, GL_RGBA, GL_FLOAT);
//    pbuffer->Deactivate();

    glViewport(0, 0, nWidth, nHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif

}

        // createTexture
        //
        // Description:
        //      This is a helper method to help create textures for pbuffers
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        // 
        GLuint
NVImageLoader::createTexture(GLenum target, GLenum internalformat, int width, int height, GLenum format, GLenum type)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(target, 0, internalformat, width, height, 0, format, type, 0);
    return tex;
}

        // loadProgram
        //
        // Description:
		//		Load a vertex or fragment program from a string
        //
        // Parameters:
        //      program_type - the method gets its information from the class state.
        //      code - string code
        //
        // Returns:
		//	The Shader Program ID
        // 
		GLuint 
NVImageLoader::loadProgram(GLenum program_type, const char *code)
{
	GLuint program_id;
	glGenProgramsARB(1, &program_id);
	glBindProgramARB(program_type, program_id);
	glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);

	GLint error_pos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
	if (error_pos != -1) {
		const GLubyte *error_string;
		error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		fprintf(stderr, "Program error at position: %d\n%s\n", error_pos, error_string);
	}
	return program_id;
}


//
// Protected methods
//

        // setCgParameters
        //
        // Description:
        //      This method is used in the image() method to 
        //      set provide the Cg programs with the correct
        //      parameters
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
NVImageLoader::setCgParameters()
{

}

        // cgFragmentProfile
        //
        // Description:
        //      Get the fragment profile required for this filter's
        //      fragment program.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      CGprofile
        //
        CGprofile
NVImageLoader::cgFragmentProfile()
        const
{
    return CG_PROFILE_FP30;
}

//
// Privatemethods
//

        // loadImage
        //
        // Description:
        //      This is a helper method that loads the actual image.
        //
        // Parameters:
        //      None - the method gets its information from the class state.
        //
        // Returns:
        //      true  - on success,
        //      false - otherwise. 
        // 
        bool
NVImageLoader::loadImage(Image * pImage)
{
    switch (_eFileType)
    {
        case DDS_FILE:
        {
            CDDSImage oPicture;

            if (!oPicture.load(_sFileName, false))
                return false;

            int nWidth  = oPicture.get_width();
            int nHeight = oPicture.get_height();

            pImage->setSize(nWidth, nHeight);
            GL_ASSERT_NO_ERROR;
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, pImage->textureID());
            GL_ASSERT_NO_ERROR;
            bool bSuccess = oPicture.upload_textureRectangle();
            GL_ASSERT_NO_ERROR;
            
            return bSuccess;
        }
        break;
    }

    return true;
}
