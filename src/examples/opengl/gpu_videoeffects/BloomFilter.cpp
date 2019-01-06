// -----------------------------------------------------------------------------
// 
// Contents:
//      BloomFilter class
//
// Description:
//      This fiter is the complete implemetation of an algorithm described in 
//      "A Spatial Post-Processing Algorithm for Images of Night Scenes" by 
//      William B. Thompson et al.
//          In contrast to simple filters that are build on the ImageFilter base
//      class in a straight forward fashion this filter actually contains
//      a little sub-filter graph. Looking at the constructor and the image()
//      function shows how this is done.
//
// Author:
//      Frank Jargstorff (2003)
//
// -----------------------------------------------------------------------------


//
// Include
//

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "BloomFilter.h"
#include "defines.h"

#include <iostream>

#include <ShaderManager.h>
#include <AssertGL.h>
#include <AssertCG.h>

#include "ProgramCg.h"

#include <math.h>


    //
    // Private static data initialization
    //

CGparameter BloomDownsample::_gDownsampleImage	= 0;

        // Default constructor
        //
BloomDownsample::BloomDownsample(): _nDownsampleScale(0.25f)
{
	_pDownsample_Cg = new ProgramCg(SHADER_PATH "gpu_videoeffects/VS_bloom_downsamp.cg",	NULL, NULL,
									SHADER_PATH "gpu_videoeffects/PS_bloom_downsamp.cg",    NULL, NULL);

	// Grab the necessary parameters
    if (_gDownsampleImage == 0)
        _gDownsampleImage		= cgGetNamedParameter(_pDownsample_Cg->fragment(), "DownsampleTex");

	CG_ASSERT_NO_ERROR;

    _oCgFragmentProgram = _pDownsample_Cg->fragment();
    _hoInputImage = _gDownsampleImage;
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
BloomDownsample::dirty()
{
    return true;
}

            // setDownsampleScale
            //
            // Description:
            //      Set the Downsample Scale Intensity
            //
            // Parameters:
            //      nDownsampleScale - the downscaling value.
            //
            // Returns:
            //      None
            //
            void
BloomDownsample::setDownsampleScale(float nDownsampleScale)
{
	this->_nDownsampleScale = nDownsampleScale;
}
	
		// setSourceOperator
        //
        // Description:
        //      Register the input image operator.
        //      The filter acts on the image it retrives
        //      from the input operator.
        //      
        // Paramters:
        //      pInputOperator - pointer to an image class object.
        //          To unregister an input operator set 
        //          pInputOperator to 0.
        //
        // Returns:
        //      None
        //
        void
BloomDownsample::setSourceOperator(SourceOperator * pSourceOperator)
{
    ImageFilter::setSourceOperator(pSourceOperator);
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
BloomDownsample::image()
{
    Image DownsampleImage;

    if (_pSourceOperator != 0)
    {
        Image oInputImage  = _pSourceOperator->image();

        DownsampleImage.setSize(oInputImage.width(), oInputImage.height());

        DownsampleImage.renderBegin();
        {
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) DownsampleImage.width(), (GLsizei) DownsampleImage.height());
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, DownsampleImage.width(), 0, DownsampleImage.height());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

                                // Set Cg state
            cgGLEnableProfile(_pDownsample_Cg->vertex_profile());
            cgGLBindProgram(_pDownsample_Cg->vertex());

            cgGLEnableProfile(_pDownsample_Cg->fragment_profile());
            cgGLBindProgram(_pDownsample_Cg->fragment());

            cgGLSetTextureParameter(_gDownsampleImage, oInputImage.textureID());
            cgGLEnableTextureParameter(_gDownsampleImage);

                            // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float nWidth  = static_cast<float>(oInputImage.width());
            const float nHeight = static_cast<float>(oInputImage.height());

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(nWidth, 0.0f);     glVertex3f(nWidth, 0.0f,    0.0f);
                glTexCoord2f(nWidth, nHeight);  glVertex3f(nWidth, nHeight, 0.0f);
                glTexCoord2f(0.0f,   nHeight);  glVertex3f(  0.0f, nHeight, 0.0f);
            glEnd();

            cgGLDisableTextureParameter(_gDownsampleImage);

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        DownsampleImage.renderEnd();

        _bDirty = false;
    }

    return DownsampleImage;
}

        // imageFBO
        //
        // Description:
        //      Gets the operator's output image via FBO.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image via FBO.
        //
//        ImageFBO
//BloomDownsample::imageFBO()
//{
//    ImageFBO oOutputImage;
//
//	return oOutputImage;
//}

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
BloomDownsample::setCgParameters()
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
BloomDownsample::cgFragmentProfile()
        const
{
    return CG_PROFILE_FP30;
}

	//
    // Private static data initialization
    //
const char * aGlowH[] = { "7", "1", "0" };
CGparameter BloomGlowHorz::_gGlowHImage		= 0;

        // Default constructor
        //
BloomGlowHorz::BloomGlowHorz()
{
    const char * aArguments[4];

    char zNSamples[128];

    sprintf(zNSamples, "-DN_SAMPLES=%i", 7 );

    aArguments[0] = zNSamples;
    aArguments[1] = 0x00000000;

	_pGlowH_Cg		= new ProgramCg(SHADER_PATH "gpu_videoeffects/VS_bloom_horz.cg",	NULL, &aArguments[0], 
									SHADER_PATH "gpu_videoeffects/PS_bloom_horz.cg",  NULL, NULL);

	CG_ASSERT_NO_ERROR;

    if (_gGlowHImage == 0)
        _gGlowHImage			= cgGetNamedParameter( _pGlowH_Cg->fragment(), "HBlurTex");

    if (_gBloomBlurWidth == 0)
        _gBloomBlurWidth 		= cgGetNamedParameter( _pGlowH_Cg->vertex(), "BloomBlurWidth");

    _oCgFragmentProgram = _pGlowH_Cg->fragment();
    _hoInputImage = _gGlowHImage;
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
BloomGlowHorz::dirty()
{
    return true;
}

            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - the Bloom Blur Widt
            //
            // Returns:
            //      None
            //
            void
BloomGlowHorz::setBloomBlurWidth(float nBloomBlurWidth)
{
    if (_gBloomBlurWidth)
        cgSetParameter1f(_gBloomBlurWidth, nBloomBlurWidth);
}

        // setSourceOperator
        //
        // Description:
        //      Register the input image operator.
        //      The filter acts on the image it retrives
        //      from the input operator.
        //      
        // Paramters:
        //      pInputOperator - pointer to an image class object.
        //          To unregister an input operator set 
        //          pInputOperator to 0.
        //
        // Returns:
        //      None
        //
        void
BloomGlowHorz::setSourceOperator(SourceOperator * pSourceOperator)
{
    ImageFilter::setSourceOperator(pSourceOperator);
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
BloomGlowHorz::image()
{
	Image GlowHImage;

    if (_pSourceOperator != 0)
    {
        Image oInputImage  = _pSourceOperator->image();

        GlowHImage.setSize(oInputImage.width(), oInputImage.height());

        GlowHImage.renderBegin();
        {
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) oInputImage.width(), (GLsizei) oInputImage.height());
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, oInputImage.width(), 0, oInputImage.height());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

                                // Set Cg state
            cgGLEnableProfile(_pGlowH_Cg->vertex_profile());
            cgGLBindProgram(_pGlowH_Cg->vertex());

            cgGLEnableProfile(_pGlowH_Cg->fragment_profile());
            cgGLBindProgram(_pGlowH_Cg->fragment());

            cgGLSetTextureParameter(_gGlowHImage, oInputImage.textureID());
            cgGLEnableTextureParameter(_gGlowHImage);

                                // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float nWidth  = static_cast<float>(oInputImage.width());
            const float nHeight = static_cast<float>(oInputImage.height());

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(nWidth, 0.0f);     glVertex3f(nWidth, 0.0f,    0.0f);
                glTexCoord2f(nWidth, nHeight);  glVertex3f(nWidth, nHeight, 0.0f);
                glTexCoord2f(0.0f,   nHeight);  glVertex3f(  0.0f, nHeight, 0.0f);
            glEnd();

            cgGLDisableTextureParameter(_gGlowHImage);

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        GlowHImage.renderEnd();

        _bDirty = false;
    }

    return GlowHImage;
}

        // imageFBO
        //
        // Description:
        //      Gets the operator's output image via FBO.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image via FBO.
        //
       // ImageFBO
//BloomGlowHorz::imageFBO()
//{
  //  ImageFBO oOutputImage;

	//return oOutputImage;
//}

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
BloomGlowHorz::setCgParameters()
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
BloomGlowHorz::cgFragmentProfile()
        const
{
    return CG_PROFILE_FP30;
}

    //
    // Private static data initialization
    //
CGparameter BloomGlowVert::_gGlowVImage		= 0;

        // Default constructor
        //
BloomGlowVert::BloomGlowVert()
{
    const char * aArguments[4];
    char zNSamples[256];

    sprintf(zNSamples, "-DN_SAMPLES=%i", 7 );

    aArguments[0] = zNSamples;
    aArguments[1] = 0;

	_pGlowV_Cg		= new ProgramCg(SHADER_PATH "gpu_videoeffects/VS_bloom_vert.cg",	NULL, &aArguments[0], 
									SHADER_PATH "gpu_videoeffects/PS_bloom_vert.cg",	NULL, NULL);
    if (_gGlowVImage == 0)
        _gGlowVImage			= cgGetNamedParameter( _pGlowV_Cg->fragment(), "VBlurTex");

    if (_gBloomBlurWidth == 0)
        _gBloomBlurWidth 		= cgGetNamedParameter( _pGlowV_Cg->vertex(), "BloomBlurWidth");

    _oCgFragmentProgram = _pGlowV_Cg->fragment();
    _hoInputImage = _gGlowVImage;
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
BloomGlowVert::dirty()
{
    return true;
}

            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - the Bloom Blur Widt
            //
            // Returns:
            //      None
            //
            void
BloomGlowVert::setBloomBlurWidth(float nBloomBlurWidth)
{
    if (_gBloomBlurWidth)
        cgSetParameter1f(_gBloomBlurWidth, nBloomBlurWidth);
}

        // setSourceOperator
        //
        // Description:
        //      Register the input image operator.
        //      The filter acts on the image it retrives
        //      from the input operator.
        //      
        // Paramters:
        //      pInputOperator - pointer to an image class object.
        //          To unregister an input operator set 
        //          pInputOperator to 0.
        //
        // Returns:
        //      None
        //
        void
BloomGlowVert::setSourceOperator(SourceOperator * pSourceOperator)
{
    ImageFilter::setSourceOperator(pSourceOperator);
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
BloomGlowVert::image()
{
	Image GlowVImage;

    if (_pSourceOperator != 0)
    {
        Image oInputImage  = _pSourceOperator->image();

        GlowVImage.setSize(oInputImage.width(), oInputImage.height());

        GlowVImage.renderBegin();
        {
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) oInputImage.width(), (GLsizei) oInputImage.height());
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, oInputImage.width(), 0, oInputImage.height());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

                                // Set Cg state
            cgGLEnableProfile(_pGlowV_Cg->vertex_profile());
            cgGLBindProgram(_pGlowV_Cg->vertex());

            cgGLEnableProfile(_pGlowV_Cg->fragment_profile());
            cgGLBindProgram(_pGlowV_Cg->fragment());

            cgGLSetTextureParameter(_gGlowVImage, oInputImage.textureID());
            cgGLEnableTextureParameter(_gGlowVImage);

                                // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float nWidth  = static_cast<float>(oInputImage.width());
            const float nHeight = static_cast<float>(oInputImage.height());

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(nWidth, 0.0f);     glVertex3f(nWidth, 0.0f,    0.0f);
                glTexCoord2f(nWidth, nHeight);  glVertex3f(nWidth, nHeight, 0.0f);
                glTexCoord2f(0.0f,   nHeight);  glVertex3f(  0.0f, nHeight, 0.0f);
            glEnd();

            cgGLDisableTextureParameter(_gGlowVImage);

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        GlowVImage.renderEnd();

        _bDirty = false;
    }

    return GlowVImage;
}

        // imageFBO
        //
        // Description:
        //      Gets the operator's output image via FBO.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image via FBO.
        //
//        ImageFBO
//BloomGlowVert::imageFBO()
//{
//    ImageFBO oOutputImage;
//
//	return oOutputImage;
//}

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
BloomGlowVert::setCgParameters()
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
BloomGlowVert::cgFragmentProfile()
        const
{
    return CG_PROFILE_FP30;
}

// -----------------------------------------------------------------------------
// BloomFilter implementation
//

    //
    // Private static data initialization
    //
CGparameter BloomFilter::_gSceneSampler		= 0;
CGparameter BloomFilter::_gBlurredSceneSampler = 0;


// 
// Construction and destruction
//


        // Default constructor
        //
BloomFilter::BloomFilter(): _nSceneIntensity(0.5f),
							_nGlowIntensity(0.5f),
							_nHighlightThreshold(0.9f),
							_nHighlightIntensity(0.5f),
                            _nDownsampleScale(0.25f),
                            _nBloomBlurWidth(2.0f)
{
    _oGlowHorzFilter.setSourceOperator(&_oDownsampleFilter);
    _oGlowVertFilter.setSourceOperator(&_oGlowHorzFilter);
	ImageFilter::setSourceOperator(&_oGlowVertFilter);

	_pFinalComp_Cg	= new ProgramCg(SHADER_PATH "gpu_videoeffects/VS_bloom.cg",	NULL, NULL,
									SHADER_PATH "gpu_videoeffects/PS_bloom_comp.cg",NULL, NULL);

	// Grab the necessary parameters
    if (_gSceneSampler == 0)
        _gSceneSampler			= cgGetNamedParameter( _pFinalComp_Cg->fragment(), "sceneSampler");  
    if (_gBlurredSceneSampler == 0)
        _gBlurredSceneSampler	= cgGetNamedParameter( _pFinalComp_Cg->fragment(), "blurredSceneSampler");  

	CG_ASSERT_NO_ERROR;
                                // Set the fragment program.
    _oCgFragmentProgram = _pFinalComp_Cg->fragment();
    _hoInputImage = _gSceneSampler;
}



//
// Public methods
//

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
BloomFilter::dirty()
{
    return true;
}


			// setSceneIntensity
            //
            // Description:
            //      Set the Scene Intensity
            //
            // Parameters:
            //      nSceneIntensity - the new intensity value.
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setSceneIntensity(float nSceneIntensity)
{
	this->_nSceneIntensity = nSceneIntensity;
}


            // setGlowIntensity
            //
            // Description:
            //      Set the Glow Intensity
            //
            // Parameters:
            //      nGlowIntensity - the new glow value.
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setGlowIntensity(float nGlowIntensity)
{
	this->_nGlowIntensity = nGlowIntensity;
}

            // setHighlightThreshold
            //
            // Description:
            //      Set the Highlight Threshold
            //
            // Parameters:
            //      nHighlightThreshold - the new threshold value.
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setHighlightThreshold(float nHighlightThreshold)
{
	this->_nHighlightThreshold = nHighlightThreshold;
}

            // setHighlightIntensity
            //
            // Description:
            //      Set the Highlight Intensity
            //
            // Parameters:
            //      nHighlightIntensity - the new intensity value.
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setHighlightIntensity(float nHighlightIntensity)
{
	this->_nHighlightIntensity = nHighlightIntensity;
}

            // setDownsampleScale
            //
            // Description:
            //      Set the Downsample Scale Intensity
            //
            // Parameters:
            //      nDownsampleScale - the downscaling value.
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setDownsampleScale(float nDownsampleScale)
{
    this->_nDownsampleScale = nDownsampleScale;
    _oDownsampleFilter.setDownsampleScale(nDownsampleScale);
}

            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - the Bloom Blur Widt
            //
            // Returns:
            //      None
            //
            void
BloomFilter::setBloomBlurWidth(float nBloomBlurWidth)
{
    this->_nBloomBlurWidth = nBloomBlurWidth;
    _oGlowHorzFilter.setBloomBlurWidth(nBloomBlurWidth);
    _oGlowVertFilter.setBloomBlurWidth(nBloomBlurWidth);
}

		// setSourceOperator
        //
        // Description:
        //      Register the input image operator.
        //      The filter acts on the image it retrives
        //      from the input operator.
        //      
        // Paramters:
        //      pInputOperator - pointer to an image class object.
        //          To unregister an input operator set 
        //          pInputOperator to 0.
        //
        // Returns:
        //      None
        //
        void
BloomFilter::setSourceOperator(SourceOperator * pSourceOperator)
{
	_oDownsampleFilter.setSourceOperator(pSourceOperator);

    _pOriginalOperator = pSourceOperator;
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
BloomFilter::image()
{
	Image FinalImage;

    if (_pSourceOperator != 0)
    {
        Image oInputImage  = _pSourceOperator->image();
        Image oSourceImage = _pOriginalOperator->image();

        FinalImage.setSize(oInputImage.width(), oInputImage.height());

        FinalImage.renderBegin();
        {
                                // Set OpenGL state
            glViewport(0, 0, (GLsizei) oInputImage.width(), (GLsizei) oInputImage.height());
  
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);       
            glDisable(GL_CULL_FACE);     
    
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, oInputImage.width(), 0, oInputImage.height());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

                                // Set Cg state
            cgGLEnableProfile(_pFinalComp_Cg->vertex_profile());
            cgGLBindProgram(_pFinalComp_Cg->vertex());

            cgGLEnableProfile(_pFinalComp_Cg->fragment_profile());
            cgGLBindProgram(_pFinalComp_Cg->fragment());

            cgGLSetTextureParameter(_gSceneSampler, oSourceImage.textureID());
            cgGLEnableTextureParameter(_gSceneSampler);
            cgGLSetTextureParameter(_gBlurredSceneSampler, oInputImage.textureID());
            cgGLEnableTextureParameter(_gBlurredSceneSampler);

                                // Set the model view matrix for the vertex program.
            cgGLSetStateMatrixParameter(ShaderManager::gVertexIdentityModelView, 
                                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                        CG_GL_MATRIX_IDENTITY);

            setCgParameters();

            const float nWidth  = static_cast<float>(oInputImage.width());
            const float nHeight = static_cast<float>(oInputImage.height());

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,   0.0f);     glVertex3f(  0.0f, 0.0f,    0.0f);
                glTexCoord2f(nWidth, 0.0f);     glVertex3f(nWidth, 0.0f,    0.0f);
                glTexCoord2f(nWidth, nHeight);  glVertex3f(nWidth, nHeight, 0.0f);
                glTexCoord2f(0.0f,   nHeight);  glVertex3f(  0.0f, nHeight, 0.0f);
            glEnd();

            cgGLDisableTextureParameter(_gSceneSampler);
            cgGLDisableTextureParameter(_gBlurredSceneSampler);

            cgGLDisableProfile(CG_PROFILE_VP20);
            cgGLDisableProfile(cgFragmentProfile());
        }
        FinalImage.renderEnd();

        _bDirty = false;
    }

    return FinalImage;
}

        // imageFBO
        //
        // Description:
        //      Gets the operator's output image via FBO.
        //          This method will usually result in a complete
        //      reevaluation of the pipeline!
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      A new image via FBO.
        //
      //  ImageFBO
//BloomFilter::imageFBO()
//{
//    ImageFBO oOutputImage;

//	return oOutputImage;
//}

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
BloomFilter::setCgParameters()
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
BloomFilter::cgFragmentProfile()
        const
{
    return CG_PROFILE_FP30;
}
