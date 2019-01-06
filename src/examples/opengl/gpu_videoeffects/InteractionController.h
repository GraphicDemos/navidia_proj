#ifndef INTERACTION_CONTROLLER_H
#define INTERACTION_CONTROLLER_H
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

#include <gl/glew.h>
#include <gl/gl.h>

//
// Forward declarations
//

class ImageFilter;
class NVImageLoader;
class LoadOperator;
class SaveOperator;
class GaussFilter;
class NightFilter;
class ScotopicFilter;
class GaussFilter1D;
class TwoPassGaussFilter;
class BloomFilter;
class ImageView;

class ParamBase;
class ParamListGL;

class ShaderGLSL;
class ShaderCg;
class ProgramGLSL;
class ProgramCg;

// -----------------------------------------------------------------------------
// InteractionController class
//
class InteractionController
{
public:
    //
    // Public typdefs
    //

    enum tePipelineMode
    {
        DISPLAY_MODE,
		NIGHT_MODE,
		SCOTOPIC_MODE,
		GAUSSIAN_MODE,
		GAUSSIAN_1D_MODE,
		GAUSSIAN_2PASS_MODE,
		BLOOM_MODE,
        EFFECT_MODE,
        BLUR_MODE, 
        POST_PROCESS_MODE, 
        TV_MODE, 
        PAINT_MODE,
        COLOR_CONTROL_MODE,
        CAMERA_EFFECT_MODE
    };


    // 
    // Construction and destruction
    //
    
            // Constructors
            //

            // Default constructor
    InteractionController();

            // Constructors
            //
    InteractionController(//LoadOperator          & rLoadOperator,
                          //NVImageLoader         & rNVImageLoader,
                          GaussFilter           & rGaussFilter,
                          NightFilter           & rNightFilter,
                          ScotopicFilter        & rScotopicFilter,
                          GaussFilter1D         & rGaussFilter1D,
                          TwoPassGaussFilter    & rTwoPassGaussFilter,
                          BloomFilter           & rBloomFilter,
                          SaveOperator          & rSaveOperator//, ImageView             & rImageView
						  );

			// Destructor
            //
            virtual
   ~InteractionController();


    //
    // Public methods
    //

            void
	InitParams();

			// initShaderContext
            //
            // Description:
            //      Initialize the Shader Context to be GLSL, Cg, or NVImageFilter
            //
            // Parameters:
            //      ppGLSL	- pointer to pointer of GLSL Program
            //      ppCg	 - pointer to pointer of GLSL Program
            //
            // Returns:
            //      None
            //
            void
	initShaderContext(ProgramGLSL **ppGLSL);

			void
	initShaderContext(ProgramCg **ppCg);

			void
	initShaderContext(ImageFilter **ppNVImageFilter);

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
    mouse(int nX, int nY);
    
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
    move(int nX, int nY);
            
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
    special(int key, int x, int y);
    
            // setPipelineMode
            //
            void
    setPipelineMode(tePipelineMode ePipelineMode);

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
    save();

			// renderSliders
			//
			// Description:
			//		Renders the sliders controlling parameters.
			//
			// Parameters:
			//		nX, nY (offset parameters)
			//
			// Returns:
			//		None
			//
			void
	renderSliders(unsigned int nX, unsigned int nY)
			const;


            // updateUniforms
			//
			// Description:
			//		Refreshes all the uniforms
			//
			// Parameters:
			//		None
			//
			// Returns:
			//		None
			//
			void
	updateUniforms();


private:
    //
    // Private methods
    //

            // Copy constructor (not implemented).
    InteractionController(const InteractionController &);

            // Assignment operator (not implemented).
    void operator= (const InteractionController &);

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
    updateDisplayParameters();

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
    updateEffectParameters();
    

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
    updateBlurParameters();

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
    updateBloomParameters();

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
    updatePostProcParameters();

            // updateTVParameters
            //
            // Description:
            //      Set the TV filter's parameters with slider values.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    updateTVParameters();

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
    updatePaintParameters();

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
    updateColorControlParameters();

            // updateCameraEffectParameters
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
    updateCameraEffectParameters();


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
    updateNightParameters();
    
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
    updateScotopicParameters();

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
    updateGaussParameters();

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
    updateGauss1dParameters();
    
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
    updateTwoPassGaussParameters();
    

    //
    // Private data
    //

    tePipelineMode    _ePipelineMode;

//    LoadOperator        * _pLoadOperator;

    // For special effects
    ParamBase           * _pSamples;
    ParamBase           * _pTiles;
    ParamBase           * _pEdgeWidth;
    ParamBase           * _pDisplacement;
    ParamBase           * _pCurrentAngle;
    ParamBase           * _pRadius;
    ParamBase           * _pEffectScale;
    ParamListGL         * _pEffectSlider;

	ParamBase           * _pSpeed;
    ParamBase           * _pScanlines;
    ParamListGL         * _pTVSlider;

    // for display stuff
    ParamBase           * _pFrequency;
    ParamBase           * _pAmplitude;
    ParamBase           * _pStartRad;
    ParamListGL         * _pDisplaySlider;


    // Operations for Blurring operations
    ParamBase           * _pBlurWidth;
    ParamBase           * _pBlurStart;
    ParamBase           * _pBlurCenter;
    ParamListGL         * _pBlurSlider;

    // Operations Post Processing operations
    ParamBase           * _pDesaturate; // for Sepia
    ParamBase           * _pToning;     // for Sepia
    ParamBase           * _pGlowness;
    ParamBase           * _pEdgePixelSteps;
    ParamBase           * _pEdgeThreshold;
    ParamListGL         * _pPostProcSlider;

    ParamBase           * _pOpacity;
    ParamBase           * _pBrushsizestart;
    ParamBase           * _pBrushsizeend;
    ParamBase           * _pBrushpressure;
    ParamBase           * _pEffectStrength;
    ParamBase           * _pFadeout;
    ParamBase           * _pFadetime;
    ParamBase           * _pFadein;
    ParamBase           * _pFadeintime;
    ParamListGL         * _pPaintSlider;

    ParamBase           * _pTexWidth;
    ParamBase           * _pTexHeight;

    ParamBase           * _pBrightness;
    ParamBase           * _pContrast;
    ParamBase           * _pHue;
    ParamBase           * _pSaturation;
    ParamBase           * _pExposure;
    ParamBase           * _pGamma;
    ParamBase           * _pDefog;
    ParamListGL         * _pColorControlSlider;

    ParamBase           * _pOverExposure;
    ParamBase           * _pDustAmount;
    ParamBase           * _pFrameJitter;
    ParamBase           * _pMaxFrameJitter;
    ParamBase           * _pGrainThickness;
    ParamBase           * _pGrainAmount;
    ParamBase           * _pScratchesAmount;
    ParamBase           * _pScratchesLevel;
    ParamListGL         * _pCameraEffectSlider;

    GaussFilter         * _pGaussFilter;
    NightFilter         * _pNightFilter;
    ScotopicFilter      * _pScotopicFilter;
    GaussFilter1D       * _pGaussFilter1D;
    TwoPassGaussFilter  * _pTwoPassGaussFilter;
	BloomFilter			* _pBloomFilter;

    ParamBase           * _pScotopicSigmaParameter;
    ParamBase           * _pScotopicGammaParameter;
    ParamBase           * _pScotopicBrightnessParameter;
	ParamListGL		    * _pScotopicSliders;
	
	ParamBase           * _pNightBrightnessParameter;
	ParamListGL         * _pNightSliders;
	
	ParamBase           * _pGaussSigmaParameter;
	ParamListGL         * _pGaussSliders;
	
	ParamBase           * _pGauss1dSigmaParameter;
	ParamListGL         * _pGauss1dSliders;
	
	ParamBase           * _pTwoPassGaussSigmaParameter;
	ParamListGL         * _pTwoPassGaussSliders;

    // Operations Bloom Effect operations
    ParamBase           * _pClearDepth;
    ParamBase           * _pSceneIntensity;
    ParamBase           * _pGlowIntensity;
    ParamBase           * _pHighlightThreshold;
    ParamBase           * _pHighlightIntensity;
    ParamBase           * _pDownsampleScale;
    ParamBase           * _pBloomBlurWidth;
    ParamListGL         * _pBloomSliders;

	SaveOperator        * _pSaveOperator;
    ImageView           * _pImageView;

    ProgramGLSL         ** _ppProgramGLSL;
    ProgramCg			** _ppProgramCg;
	ImageFilter			** _ppImageFilter;
};

#endif // INTERACTION_CONTROLLER_H