// -----------------------------------------------------------------------------
// 
// Contents:
//      InteractionController class
//
// Description:
//      The interaction controller provides a simple interface to setup
//      and reconfigure the demo application's image processing pipeline.
//      It also provides a set of methods that allow to easily and in a uniform
//      way forward GUI commands for tweaking parameters to the pipeline.
//          The InteractionController is part of a model-view-controller 
//      pattern. Other classes involved in this collaboration are 
//          - the ImageView
//          - hierarchies of ImageOperators
//      ImageView is the only implementation of a view in this MVC pattern.
//      The hierarchies of ImageOperators are the model.
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
#include <GL/wglew.h>

#include "File.h"
#include "ProgramGLSL.h"
#include "ProgramCg.h"
#include "ShaderCg.h"
#include "NV_image_processing.h"

#include "uniforms.h"

#include <paramgl.h>

#include "InteractionController.h"

#include <iostream>


//
// Namespaces
//

using namespace std;


// 
// Constants
//

const float gnDeltaSigma = 0.1f;
const float gnDeltaGamma = 0.1f;
const float gnDeltaBrightness = 0.05f;


// -----------------------------------------------------------------------------
// InteractionController implementation
//


// 
// Construction and destruction
//
            // Default Constructor
InteractionController::InteractionController() :	_ppProgramGLSL(NULL),
													_ppProgramCg(NULL)
{
	InitParams();
}


            // Constructor//
InteractionController::InteractionController(GaussFilter        & rGaussFilter,
                                             NightFilter        & rNightFilter,
                                             ScotopicFilter     & rScotopicFilter,
                                             GaussFilter1D      & rGaussFilter1D,
                                             TwoPassGaussFilter & rTwoPassGaussFilter,
											 BloomFilter		& rBloomFilter,
                                             SaveOperator       & rSaveOperator
                                             )
                :	_ePipelineMode(InteractionController::DISPLAY_MODE),
					_ppProgramGLSL(NULL),
					_ppProgramCg(NULL)
                , _pGaussFilter(        &rGaussFilter       )
                , _pNightFilter(        &rNightFilter       )
                , _pScotopicFilter(     &rScotopicFilter    )
                , _pGaussFilter1D(      &rGaussFilter1D     )
                , _pTwoPassGaussFilter( &rTwoPassGaussFilter)
				, _pBloomFilter(        &rBloomFilter       )
                , _pScotopicSigmaParameter(0)
                , _pScotopicGammaParameter(0)
                , _pScotopicBrightnessParameter(0)
				, _pScotopicSliders(0)
				, _pGauss1dSigmaParameter(0)
				, _pGauss1dSliders(0)
				, _pTwoPassGaussSigmaParameter(0)
				, _pTwoPassGaussSliders(0)
				, _pBloomSliders(0)
                , _pSaveOperator(   &rSaveOperator   )
{
	InitParams();

	_pGaussSliders                  = new ParamListGL("Gauss Filter Parameters");
	_pGaussSigmaParameter           = new Param<float>("Sigma", 0.25f, 0.0f, 10.0f, 0.1f);
    _pGaussSliders->AddParam(_pGaussSigmaParameter);
    
	_pNightSliders                  = new ParamListGL("Night Filter Parameters");
	_pNightBrightnessParameter      = new Param<float>("Brightness", 1.0f, 0.0f, 2.0f, 0.05f);
	_pNightSliders->AddParam(_pNightBrightnessParameter);

	_pScotopicSliders               = new ParamListGL("Scotopic Filter Parameters");
    _pScotopicSigmaParameter        = new Param<float>("Sigma",      0.25f, 0.0f, 10.0f, 0.1f);
    _pScotopicGammaParameter        = new Param<float>("Gamma",      1.0f,  0.0f, 10.0f, 0.1f);
    _pScotopicBrightnessParameter   = new Param<float>("Brightness", 1.0f,  0.0f,  2.0f, 0.05f);
 	_pScotopicSliders->AddParam(_pScotopicSigmaParameter);
	_pScotopicSliders->AddParam(_pScotopicGammaParameter);
	_pScotopicSliders->AddParam(_pScotopicBrightnessParameter);
	
	_pGauss1dSliders                = new ParamListGL("Gauss 1D Filter Parameters");
	_pGauss1dSigmaParameter         = new Param<float>("Sigma", 1.0f, 0.0f, 10.0f, 0.1f);
	_pGauss1dSliders->AddParam(_pGauss1dSigmaParameter);

	_pGaussSliders					= new ParamListGL("Gauss Filter Parameters");
	_pGaussSliders->AddParam(_pGauss1dSigmaParameter);

	_pTwoPassGaussSliders           = new ParamListGL("2-pass Gauss Filter Parameters");
	_pTwoPassGaussSigmaParameter    = new Param<float>("Sigma", 1.0f, 0.0f, 10.0f, 0.1f);
	_pTwoPassGaussSliders->AddParam(_pTwoPassGaussSigmaParameter);

	if (_pBloomFilter) {
		_pBloomSliders					= new ParamListGL("Bloom Parameters");
		_pClearDepth					= new Param<float>("ClearDepth"     , 1.0f, 0.0f, 1.0f, 0.05f);
		_pSceneIntensity				= new Param<float>("Scene Intensity", 0.5f, 0.0f, 2.0f, 0.05f);
		_pGlowIntensity					= new Param<float>("Glow Intensity" , 0.5f, 0.0f, 2.0f, 0.05f);
		_pHighlightThreshold			= new Param<float>("Highlight Threshold", 0.9f, 0.0f, 1.0f, 0.1f);
		_pHighlightIntensity			= new Param<float>("Highlight Intensity", 0.5f, 0.0f,10.0f, 0.1f);
		_pBloomBlurWidth				= new Param<float>("Bloom Blur Width",2.0f, 0.0f, 10.0f, 0.01f);
		_pDownsampleScale				= new Param<float>("Downsample Scale",1.0f, 0.0f, 10.0f, 0.1f);
		_pBloomSliders->AddParam(_pClearDepth);
		_pBloomSliders->AddParam(_pSceneIntensity);
		_pBloomSliders->AddParam(_pGlowIntensity);
		_pBloomSliders->AddParam(_pHighlightThreshold);
		_pBloomSliders->AddParam(_pHighlightIntensity);
		_pBloomSliders->AddParam(_pBloomBlurWidth);
		_pBloomSliders->AddParam(_pDownsampleScale);
	}
}    


        // Destructor
        //
InteractionController::~InteractionController() 
{
    ; // empty
}


void
InteractionController::InitParams()
{
    // Common to both vertex and fragment programs
    _pTiles             = new Param<float>("Tiles"        ,  64.0f,  1.0f,  200.0f, 1.0f);
    _pEdgeWidth         = new Param<float>("Edge Width"   ,  0.15f,  0.0f,    2.0f, 0.001f);
    _pSamples           = new Param<float>("Samples"      ,   1.0f,  0.0f,   16.0f, 0.5f);
    _pDisplacement      = new Param<float>("Displacement" ,   1.0f,  0.0f,   10.0f, 0.1f);
    _pCurrentAngle      = new Param<float>("Current Angle",   0.0f,  0.0f,   90.0f, 1.0f);
    _pRadius            = new Param<float>("Radius"       ,   0.6f,  0.0f,    1.0f, 0.01f);
    _pEffectScale       = new Param<float>("Effect Scale" ,  0.75f,  0.0f,    1.0f, 0.01f);
    _pEffectSlider      = new ParamListGL("Effect Parameters");
    _pEffectSlider->AddParam(_pTiles);
    _pEffectSlider->AddParam(_pEdgeWidth);
    _pEffectSlider->AddParam(_pSamples);
    _pEffectSlider->AddParam(_pDisplacement);
    _pEffectSlider->AddParam(_pCurrentAngle);
    _pEffectSlider->AddParam(_pRadius);
    _pEffectSlider->AddParam(_pEffectScale);

    // display parameters
    _pFrequency         = new Param<float>("Frequency"    ,   30.0 , 0.0f,   60.0f, 0.1f);
    _pAmplitude         = new Param<float>("Amplitude"    ,   0.05 , 0.0f,   0.25f, 0.01f);
    _pStartRad          = new Param<float>("StartRad"     ,   70.0f, 0.0f,   120.0f, 1.0f);
    _pDisplaySlider     = new ParamListGL("Display Parameters");
    _pDisplaySlider->AddParam(_pFrequency);
    _pDisplaySlider->AddParam(_pAmplitude);
    _pDisplaySlider->AddParam(_pStartRad);

    _pBlurStart         = new Param<float>("Blur Start"     , 1.0f, 0.0f, 1.0f, 0.01f);
    _pBlurWidth         = new Param<float>("Blur Width"     ,-0.2f,-1.0f, 1.0f, 0.01f);
    _pBlurCenter        = new Param<float>("Blur Center"    , 0.15f, 0.0f, 1.0f, 0.01f);
    _pBlurSlider        = new ParamListGL("Blur Parameters");
    _pBlurSlider->AddParam(_pBlurStart);
    _pBlurSlider->AddParam(_pBlurWidth);
    _pBlurSlider->AddParam(_pBlurCenter);

    // Operations for post processing operations #1
    _pDesaturate        = new Param<float>("Desaturate"     , 0.5f, 0.0f, 1.0f, 0.01f); // for Sepia
    _pToning            = new Param<float>("Toning"         , 1.0f, 0.0f, 1.0f, 0.01f); // for Sepia
    _pGlowness          = new Param<float>("Glowness"       , 1.0f, 0.0f, 1.0f, 0.05f);
    _pEdgePixelSteps    = new Param<float>("Pixel Steps"    , 1.5f, 1.0f, 5.0f, 0.5f);   // for Edge Detection
    _pEdgeThreshold     = new Param<float>("Edge Threshold" , 0.2f, 0.01f, 0.5f, 0.01f); // for Edge Detection
    _pPostProcSlider    = new ParamListGL("PostProcess Parameters");
    _pPostProcSlider->AddParam(_pDesaturate);
    _pPostProcSlider->AddParam(_pToning);
    _pPostProcSlider->AddParam(_pGlowness);
    _pPostProcSlider->AddParam(_pEdgePixelSteps);
    _pPostProcSlider->AddParam(_pEdgeThreshold);

	// Operations for post processing operations #2
	_pSpeed             = new Param<float>("Speed"        ,  0.15f,  0.0f,    1.0f, 0.01f);
    _pScanlines         = new Param<float>("Scanlines"    , 386.0f,  0.0f,  512.0f, 1.0f);
    _pTVSlider			= new ParamListGL("TV Parameters");
    _pTVSlider->AddParam(_pSpeed);
    _pTVSlider->AddParam(_pScanlines);

    _pOpacity           = new Param<float>("Opacity"         , 1.0f, 0.0f, 10.0f, 0.1f);
    _pBrushsizestart    = new Param<float>("Brushsize Start" , 0.045f, 0.001, 0.15f, 0.001f);
    _pBrushsizeend      = new Param<float>("Brushsize End"   , 0.01f,  0.001, 0.15f, 0.001f);
    _pBrushpressure     = new Param<float>("Brush Pressure"  , 1.0f, 0.0f, 1.0f, 0.001f);
    _pEffectStrength    = new Param<float>("3D Effect Strength",1.0f,-1.0f, 1.0f, 0.001f);
    _pFadeout           = new Param<float>("Fade-Out"        , 1.0f, 0.0f, 10.0f, 0.1f);
    _pFadetime          = new Param<float>("Fade Time"       , 1.10f, 0.1f, 10.0f, 0.1f);
    _pFadein            = new Param<float>("Fade-In"         , 0.15f, 0.001f, 0.3f, 0.001f);
    _pFadeintime        = new Param<float>("Fade-In Time"    , 0.15f, 0.001f, 0.3f, 0.001f);
    _pPaintSlider       = new ParamListGL("Paint Parameters");
    _pPaintSlider->AddParam(_pOpacity);
    _pPaintSlider->AddParam(_pBrushsizestart);
    _pPaintSlider->AddParam(_pBrushsizeend);
    _pPaintSlider->AddParam(_pBrushpressure);
    _pPaintSlider->AddParam(_pEffectStrength);
    _pPaintSlider->AddParam(_pFadeout);
    _pPaintSlider->AddParam(_pFadetime);
    _pPaintSlider->AddParam(_pFadein);
    _pPaintSlider->AddParam(_pFadeintime);

    _pBrightness        = new Param<float>("Brightness"      , 1.0f, 0.0f, 10.0f, 0.01f);
    _pContrast          = new Param<float>("Contrast"        , 1.0f, 0.0f, 10.0f, 0.01f);
    _pHue               = new Param<float>("Hue"             , 1.0f, 0.0f, 10.0f, 0.1f);
    _pSaturation        = new Param<float>("Saturation"      , 1.0f, 0.0f, 10.0f, 0.1f);
    _pExposure          = new Param<float>("Exposure"        , 1.0f, 0.0f, 5.0f, 0.01f);
    _pGamma             = new Param<float>("Gamma"           , 0.5f, 2.5f, 0.0f, 0.01f);
    _pDefog             = new Param<float>("Defog"           , 1.0f, 0.0f, 10.0f, 0.1f);
    _pColorControlSlider= new ParamListGL("Color Control");

//    _pColorControlSlider->AddParam(_pBrightness);
//    _pColorControlSlider->AddParam(_pContrast);
//    _pColorControlSlider->AddParam(_pHue);
    _pColorControlSlider->AddParam(_pSaturation);
    _pColorControlSlider->AddParam(_pExposure);
    _pColorControlSlider->AddParam(_pGamma);
    _pColorControlSlider->AddParam(_pDefog);

    _pOverExposure      = new Param<float>("OverExposure"    , 1.5f, 0.0f, 10.0f, 0.1f);
    _pDustAmount        = new Param<float>("Dust Amount"     , 4.0f, 0.0f, 10.0f, 1.0f);
    _pFrameJitter       = new Param<float>("Frame Jitter"    , 4.7f, 0.0f, 6.0f, 0.1f);
    _pMaxFrameJitter    = new Param<float>("Max Frame Jitter", 1.4f, 0.0f, 10.0f, 0.1f);
    _pGrainThickness    = new Param<float>("Grain Thickness" , 1.0f, 0.0f, 4.0f, 0.1f);
    _pGrainAmount       = new Param<float>("Grain Amount"    , 0.8f, 0.0f, 1.0f, 0.1f);
    _pScratchesAmount   = new Param<float>("Scraches Amount" , 3.0f, 0.0f, 3.0f, 1.0f);
    _pScratchesLevel    = new Param<float>("Scraches Level"  , 0.7f, 0.0f, 1.0f, 0.1f);
    _pCameraEffectSlider= new ParamListGL("Old Camera Parameters");;
    _pCameraEffectSlider->AddParam(_pOverExposure);
    _pCameraEffectSlider->AddParam(_pDustAmount);
    _pCameraEffectSlider->AddParam(_pFrameJitter);
    _pCameraEffectSlider->AddParam(_pMaxFrameJitter);
    _pCameraEffectSlider->AddParam(_pGrainThickness);
    _pCameraEffectSlider->AddParam(_pGrainAmount);
    _pCameraEffectSlider->AddParam(_pScratchesAmount);
    _pCameraEffectSlider->AddParam(_pScratchesLevel);
}


//
// Public methods
//

            // initShaderContext
            //
            // Description:
            //      Initialize the Shader Context of GLSL or Cg
            //
            // Parameters:
            //      ppGLSL	- pointer to pointer of GLSL Program
            //      ppCg	 - pointer to pointer of GLSL Program
            //
            // Returns:
            //      None
            //
void
InteractionController::initShaderContext(ProgramGLSL **ppGLSL)
{
	_ppImageFilter	= NULL;
	_ppProgramCg	= NULL;
	_ppProgramGLSL	= ppGLSL;
}

void
InteractionController::initShaderContext(ProgramCg **ppCg)
{
	_ppImageFilter	= NULL;
	_ppProgramCg	= ppCg;
	_ppProgramGLSL	= NULL;
}

void
InteractionController::initShaderContext(ImageFilter **ppNVImageFilter)
{
	_ppImageFilter	= ppNVImageFilter;
	_ppProgramCg	= NULL;
	_ppProgramGLSL	= NULL;
}

        // mouse
        //
        // Description:
        //      Handle mouse events
        //
        // Parameters:
        //      nX - x-coordinate where event happened.
        //      nY - y-coordinate where event happened.
        //
        // Returns:
        //      None
        //
void
InteractionController::mouse(int nX, int nY)
{
    switch (_ePipelineMode)
    {
        case DISPLAY_MODE:
            _pDisplaySlider->Mouse(nX, nY);
            updateDisplayParameters();
        break;
        case EFFECT_MODE:
            _pEffectSlider->Mouse(nX, nY);
            updateEffectParameters();
        break;
        case BLUR_MODE:
            _pBlurSlider->Mouse(nX, nY);
            updateBlurParameters();
        break;
        case POST_PROCESS_MODE:
            _pPostProcSlider->Mouse(nX, nY);
            updatePostProcParameters();
        break;
        case TV_MODE:
            _pTVSlider->Mouse(nX, nY);
            updateTVParameters();
        break;
        case PAINT_MODE:
            _pPaintSlider->Mouse(nX, nY);
            updatePaintParameters();
        break;
        case COLOR_CONTROL_MODE:
            _pColorControlSlider->Mouse(nX, nY);
            updateColorControlParameters();
        break;
        case CAMERA_EFFECT_MODE:
            _pCameraEffectSlider->Mouse(nX, nY);
            updateCameraEffectParameters();
        break;
		case NIGHT_MODE:
            _pNightSliders->Mouse(nX, nY);
            updateNightParameters();
		break;
		case SCOTOPIC_MODE:
            _pScotopicSliders->Mouse(nX, nY);
            updateScotopicParameters();
		break;
		case GAUSSIAN_MODE:
            _pGaussSliders->Mouse(nX, nY);
            updateGaussParameters();
		break;
		case GAUSSIAN_1D_MODE:
            _pGauss1dSliders->Mouse(nX, nY);
            updateGauss1dParameters();
		break;
		case GAUSSIAN_2PASS_MODE:
            _pTwoPassGaussSliders->Mouse(nX, nY);
            updateTwoPassGaussParameters();
		break;
		case BLOOM_MODE:
            _pBloomSliders->Mouse(nX, nY);
            updateBloomParameters();
		break;
    }
}

        // special
        //
        // Description:
        //      Handle special keys
        //
        // Parameters:
        //      nKey - key code.
        //      nX - cursor position x.
        //      nY - cursor position y.
        //
        // Returns:
        //      None
        //
        void
InteractionController::special(int nKey, int nX, int nY)
{
    switch (_ePipelineMode)
    {
        case DISPLAY_MODE:
            _pDisplaySlider->Special(nKey, nX, nY);
            updateDisplayParameters();
        break;
        case EFFECT_MODE:
            _pEffectSlider->Special(nKey, nX, nY);
            updateEffectParameters();
        break;
        case BLUR_MODE:
            _pBlurSlider->Special(nKey, nX, nY);
            updateBlurParameters();
        break;
        case POST_PROCESS_MODE:
            _pPostProcSlider->Special(nKey, nX, nY);
            updatePostProcParameters();
        break;
        case TV_MODE:
            _pTVSlider->Special(nKey, nX, nY);
            updateTVParameters();
        break;
        case PAINT_MODE:
            _pPaintSlider->Special(nKey, nX, nY);
            updatePaintParameters();
        break;
        case COLOR_CONTROL_MODE:
            _pColorControlSlider->Special(nKey, nX, nY);
            updateColorControlParameters();
        break;
        case CAMERA_EFFECT_MODE:
            _pCameraEffectSlider->Special(nKey, nX, nY);
            updateCameraEffectParameters();
        break;
		case NIGHT_MODE:
            _pNightSliders->Special(nKey, nX, nY);
            updateNightParameters();
		break;
		case SCOTOPIC_MODE:
            _pScotopicSliders->Special(nKey, nX, nY);
            updateScotopicParameters();
		break;
		case GAUSSIAN_MODE:
            _pGaussSliders->Special(nKey, nX, nY);
            updateGaussParameters();
		break;
		case GAUSSIAN_1D_MODE:
            _pGauss1dSliders->Special(nKey, nX, nY);
            updateGauss1dParameters();
		break;
		case GAUSSIAN_2PASS_MODE:
            _pTwoPassGaussSliders->Special(nKey, nX, nY);
            updateTwoPassGaussParameters();
		break;
		case BLOOM_MODE:
            _pBloomSliders->Special(nKey, nX, nY);
            updateBloomParameters();
		break;
    }
}

        // move
        //
        // Description:
        //      Handle mouse movements
        //
        // Parameters:
        //      nX - mouse position
        //      nY - mouse position
        //
        // Returns:
        //      None
        //
        void
InteractionController::move(int nX, int nY)
{
    switch (_ePipelineMode)
    {
        case DISPLAY_MODE:
            _pDisplaySlider->Motion(nX, nY);
            updateDisplayParameters();
        break;
        case EFFECT_MODE:
            _pEffectSlider->Motion(nX, nY);
            updateEffectParameters();
        break;
        case BLUR_MODE:
            _pBlurSlider->Motion(nX, nY);
            updateBlurParameters();
        break;
        case POST_PROCESS_MODE:
            _pPostProcSlider->Motion(nX, nY);
            updatePostProcParameters();
        break;
        case TV_MODE:
            _pTVSlider->Motion(nX, nY);
            updateTVParameters();
        break;
        case PAINT_MODE:
            _pPaintSlider->Motion(nX, nY);
            updatePaintParameters();
        break;
        case COLOR_CONTROL_MODE:
            _pColorControlSlider->Motion(nX, nY);
            updateColorControlParameters();
        break;
        case CAMERA_EFFECT_MODE:
            _pCameraEffectSlider->Motion(nX, nY);
            updateCameraEffectParameters();
        break;
		case NIGHT_MODE:
            _pNightSliders->Motion(nX, nY);
            updateNightParameters();
		break;
		case SCOTOPIC_MODE:
            _pScotopicSliders->Motion(nX, nY);
            updateScotopicParameters();
		break;
		case GAUSSIAN_MODE:
            _pGaussSliders->Motion(nX, nY);
            updateGaussParameters();
		break;
		case GAUSSIAN_1D_MODE:
            _pGauss1dSliders->Motion(nX, nY);
            updateGauss1dParameters();
		break;
		case GAUSSIAN_2PASS_MODE:
            _pTwoPassGaussSliders->Motion(nX, nY);
            updateTwoPassGaussParameters();
		break;
		case BLOOM_MODE:
            _pBloomSliders->Motion(nX, nY);
            updateBloomParameters();
		break;
    }
}

        // setPiplineMode
        //
        void
InteractionController::setPipelineMode(InteractionController::tePipelineMode ePipelineMode)
{
    _ePipelineMode = ePipelineMode;
}

        // save
        //
        // Description:
        //      Trigger a save operation on the save operator.
        //          All saves are done to the file Scotopic.dds
        //      that gets created in the current working director.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::save()
{
//    _pSaveOperator->save("Scotopic.dds");
}

		// renderSliders
		//
		// Description:
		//		Renders the sliders controlling parameters.
		//
		// Parameters:
		//		None
		//
		// Returns:
		//		None
		//
		void
InteractionController::renderSliders(unsigned int nX, unsigned int nY)
		const
{
    switch (_ePipelineMode)
    {
        case DISPLAY_MODE:
            _pDisplaySlider->Render(nX, nY);
        break;
        case EFFECT_MODE:
            _pEffectSlider->Render(nX, nY);
        break;
        case BLUR_MODE:
            _pBlurSlider->Render(nX, nY);
        break;
        case POST_PROCESS_MODE:
            _pPostProcSlider->Render(nX, nY);
        break;
        case TV_MODE:
            _pTVSlider->Render(nX, nY);
        break;
        case PAINT_MODE:
            _pPaintSlider->Render(nX, nY);
        break;
        case COLOR_CONTROL_MODE:
            _pColorControlSlider->Render(nX, nY);
        break;
        case CAMERA_EFFECT_MODE:
            _pCameraEffectSlider->Render(nX, nY);
        break;
		case NIGHT_MODE:
            _pNightSliders->Render(nX, nY);
		break;
		case SCOTOPIC_MODE:
            _pScotopicSliders->Render(nX, nY);
		break;
		case GAUSSIAN_MODE:
            _pGaussSliders->Render(nX, nY);
		break;
		case GAUSSIAN_1D_MODE:
            _pGauss1dSliders->Render(nX, nY);
		break;
		case GAUSSIAN_2PASS_MODE:
            _pTwoPassGaussSliders->Render(nX, nY);
		break;
		case BLOOM_MODE:
            _pBloomSliders->Render(nX, nY);
		break;
    }
}


			// updateUniforms
			//
			// Description:
			//		Refreshes the data from all the uniforms
			//
			// Parameters:
			//		None
			//
			// Returns:
			//		None
			//
			void
InteractionController::updateUniforms()
{
	if (!_ppImageFilter) {
		switch (_ePipelineMode) {
			case DISPLAY_MODE:          updateDisplayParameters();
				break;
			case EFFECT_MODE:			updateEffectParameters();
				break;
			case BLUR_MODE:				updateBlurParameters();
				break;
			case POST_PROCESS_MODE:		updatePostProcParameters();
				break;
			case TV_MODE:				updateTVParameters();
				break;
			case BLOOM_MODE:			updateBloomParameters();
				break;
			case PAINT_MODE:			updatePaintParameters();
				break;
			case COLOR_CONTROL_MODE:	updateColorControlParameters();
				break;
			case CAMERA_EFFECT_MODE:	updateCameraEffectParameters();
				break;
		}
    }
}


        // updateDisplayParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateDisplayParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateTime();
        (*_ppProgramGLSL)->gUniforms.updateFrequency(   _pFrequency->GetFloatValue(),
                                                        _pFrequency->GetFloatValue() );
        (*_ppProgramGLSL)->gUniforms.updateAmplitude(   _pAmplitude->GetFloatValue(),
                                                        _pAmplitude->GetFloatValue() );
        (*_ppProgramGLSL)->gUniforms.updateStartRad(    _pStartRad->GetFloatValue() );
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateTime();
        (*_ppProgramCg)->gUniforms.updateFrequency( _pFrequency->GetFloatValue(),
                                                    _pFrequency->GetFloatValue() );
        (*_ppProgramCg)->gUniforms.updateAmplitude( _pAmplitude->GetFloatValue(),
                                                    _pAmplitude->GetFloatValue() );
        (*_ppProgramCg)->gUniforms.updateStartRad(  _pStartRad->GetFloatValue() );
    }
}

        // updateEffectParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateEffectParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateTime();
        (*_ppProgramGLSL)->gUniforms.updateSpeed(_pSpeed->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateOffsets( (*_ppProgramGLSL)->gUniforms.offsets[0],
                                                    (*_ppProgramGLSL)->gUniforms.offsets[1]);
        (*_ppProgramGLSL)->gUniforms.updateTiles(_pTiles->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateEdgeWidth(_pEdgeWidth->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateSamples(_pSamples->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateDisplacement(_pDisplacement->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateCurrentAngle(_pCurrentAngle->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateRadius(_pRadius->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateEffectScale(_pEffectScale->GetFloatValue());
    }

    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateTime();
        (*_ppProgramCg)->gUniforms.updateSpeed(_pSpeed->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateOffsets( (*_ppProgramCg)->gUniforms.offsets[0],
                                                  (*_ppProgramCg)->gUniforms.offsets[1]);
        (*_ppProgramCg)->gUniforms.updateTiles(_pTiles->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateEdgeWidth(_pEdgeWidth->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateSamples(_pSamples->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateDisplacement(_pDisplacement->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateCurrentAngle(_pCurrentAngle->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateRadius(_pRadius->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateEffectScale(_pEffectScale->GetFloatValue());
    }
}

        // updateBlurParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateBlurParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateBlurCenter(_pBlurCenter->GetFloatValue(), _pBlurCenter->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBlurStart(_pBlurStart->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBlurWidth(_pBlurWidth->GetFloatValue());
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateBlurCenter(_pBlurCenter->GetFloatValue(), _pBlurCenter->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBlurStart(_pBlurStart->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBlurWidth(_pBlurWidth->GetFloatValue());
    }
}

        // updateBloomParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateBloomParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateClearDepth(_pClearDepth->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateSceneIntensity(_pSceneIntensity->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateGlowIntensity(_pGlowIntensity->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateHighlightThreshold(_pHighlightThreshold->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateHighlightIntensity(_pHighlightIntensity->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBloomBlurWidth(_pBloomBlurWidth->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateDownsampleScale(_pDownsampleScale->GetFloatValue());
    }
    if (_ppProgramCg) {
		_pBloomFilter->setSceneIntensity(_pSceneIntensity->GetFloatValue());
		_pBloomFilter->setGlowIntensity(_pGlowIntensity->GetFloatValue());
		_pBloomFilter->setHighlightThreshold(_pHighlightThreshold->GetFloatValue());
		_pBloomFilter->setHighlightIntensity(_pHighlightIntensity->GetFloatValue());
		_pBloomFilter->setDownsampleScale(_pDownsampleScale->GetFloatValue());
		_pBloomFilter->setBloomBlurWidth(_pBloomBlurWidth->GetFloatValue());

		(*_ppProgramCg)->gUniforms.updateClearDepth(_pClearDepth->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBloomBlurWidth(_pBloomBlurWidth->GetFloatValue());
    }
}

        // updatePostProcParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updatePostProcParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateDesaturate(_pDesaturate->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateToning(_pToning->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateGlowness(_pGlowness->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateEdgePixelSteps(_pEdgePixelSteps->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateEdgeThreshold(_pEdgeThreshold->GetFloatValue());
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateDesaturate(_pDesaturate->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateToning(_pToning->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateGlowness(_pGlowness->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateEdgePixelSteps(_pEdgePixelSteps->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateEdgeThreshold(_pEdgeThreshold->GetFloatValue());
    }
}

        // updateTVParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateTVParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateTime();
        (*_ppProgramGLSL)->gUniforms.updateSpeed(_pSpeed->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateScanlines(_pScanlines->GetFloatValue());
   }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateTime();
        (*_ppProgramCg)->gUniforms.updateSpeed(_pSpeed->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateScanlines(_pScanlines->GetFloatValue());
    }
}

            // updatePaintParameters
            //
            // Description:
            //      Set the filter's parameters with slider values.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
InteractionController::updatePaintParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateOpacity(_pOpacity->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBrushsizestart(_pBrushsizestart->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBrushsizeend(_pBrushsizeend->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateBrushpressure(_pBrushpressure->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateEffectStrength(_pEffectStrength->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateFadeout(_pFadeout->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateFadetime(_pFadetime->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateFadein(_pFadein->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateFadeintime(_pFadeintime->GetFloatValue());
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateOpacity(_pOpacity->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBrushsizestart(_pBrushsizestart->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBrushsizeend(_pBrushsizeend->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateBrushpressure(_pBrushpressure->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateEffectStrength(_pEffectStrength->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateFadeout(_pFadeout->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateFadetime(_pFadetime->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateFadein(_pFadein->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateFadeintime(_pFadeintime->GetFloatValue());
    }
}


            // updateColorControlParameters
            //
            // Description:
            //      Set the filter's parameters with slider values.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
InteractionController::updateColorControlParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateBrightness(_pBrightness->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateContrast(_pContrast->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateHue(_pHue->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateSaturation(_pSaturation->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateExposure(_pExposure->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateGamma(_pGamma->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateDefog(_pDefog->GetFloatValue());
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateBrightness(_pBrightness->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateContrast(_pContrast->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateHue(_pHue->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateSaturation(_pSaturation->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateExposure(_pExposure->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateGamma(_pGamma->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateDefog(_pDefog->GetFloatValue());
    }
}

            // updateCameraEffects
            //
            // Description:
            //      Set the filter's parameters with slider values.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
InteractionController::updateCameraEffectParameters()
{
    if (_ppProgramGLSL) {
        (*_ppProgramGLSL)->gUniforms.updateOverExposure(_pOverExposure->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateDustAmount(_pDustAmount->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateFrameJitter(_pFrameJitter->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateMaxFrameJitter(_pMaxFrameJitter->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateGrainThickness(_pGrainThickness->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateGrainAmount(_pGrainAmount->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateScratchesAmount(_pScratchesAmount->GetFloatValue());
        (*_ppProgramGLSL)->gUniforms.updateScratchesLevel(_pScratchesLevel->GetFloatValue());
    }
    if (_ppProgramCg) {
        (*_ppProgramCg)->gUniforms.updateOverExposure(_pOverExposure->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateDustAmount(_pDustAmount->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateFrameJitter(_pFrameJitter->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateMaxFrameJitter(_pMaxFrameJitter->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateGrainThickness(_pGrainThickness->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateGrainAmount(_pGrainAmount->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateScratchesAmount(_pScratchesAmount->GetFloatValue());
        (*_ppProgramCg)->gUniforms.updateScratchesLevel(_pScratchesLevel->GetFloatValue());
    }
}

        // updateNightParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateNightParameters()
{
    _pNightFilter->setBrightness(_pNightBrightnessParameter->GetFloatValue());
}

        // updateScotopicParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateScotopicParameters()
{
    _pScotopicFilter->setSigma(_pScotopicSigmaParameter->GetFloatValue());
    _pScotopicFilter->setGamma(_pScotopicGammaParameter->GetFloatValue());
    _pScotopicFilter->setBrightness(_pScotopicBrightnessParameter->GetFloatValue());
}

        // updateGaussParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateGaussParameters()
{
    _pGaussFilter->setSigma(_pGauss1dSigmaParameter->GetFloatValue());
}

        // updateGauss1dParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateGauss1dParameters()
{
    _pGaussFilter1D->setSigma(_pGauss1dSigmaParameter->GetFloatValue());
}

        // updateTwoPassGaussParameters
        //
        // Description:
        //      Set the filter's parameters with slider values.
        //
        // Parameters:
        //      None
        //
        // Returns:
        //      None
        //
        void
InteractionController::updateTwoPassGaussParameters()
{
    _pTwoPassGaussFilter->setSigma(_pTwoPassGaussSigmaParameter->GetFloatValue());
}
