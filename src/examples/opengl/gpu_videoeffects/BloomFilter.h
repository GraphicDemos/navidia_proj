#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H
// -----------------------------------------------------------------------------
// 
// Contents:
//      BloomFilter class
//
// Description:
//      This fiter is the complete implemetation Image PostProcessing Effect
//      of an Overbright glow effect
//          In contrast to simple filters that are build on the ImageFilter base
//      class in a straight forward fashion this filter actually contains
//      a little sub-filter graph. Looking at the constructor and the image()
//      function shows how this is done.
//
// Author:
//      Eric Young (2005)
//
// -----------------------------------------------------------------------------


//
// Include
//

#include <ImageFilter.h>
#include "NVImageLoader.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "ProgramCg.h"


// -----------------------------------------------------------------------------
// BloomDownsample class (stage 1)
//
class BloomDownsample: public ImageFilter
{
public:
    // 
    // Construction and destruction
    //
    
            // Default constructor
            //
    BloomDownsample();

            // Destructor
            //
            virtual
   ~BloomDownsample()
            {
                ; // empty
            }


    //
    // Public methods
    //
            // downsample_scale
            //
            // Description:
            //      Retrieve the filter's downsample scale value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current downsample scale value.
            //
            float
    downsample_scale()
            const
			{
				return _nDownsampleScale;
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
            virtual
            void
    setSourceOperator(SourceOperator * pInputOperator);

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
            virtual
            bool
    dirty();

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
	setDownsampleScale(float nDownsampleScale);

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
            virtual
            Image
    image();

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
            //virtual
           // ImageFBO
    //imageFBO();



protected:
    //
    // Protected methods
    //
            // renderOutput
            //
        //    void
  //  renderOutput(Image * pOutputImage, const Image & rInputImage);

            // renderOutput
            //
        //    void
//    renderOutput(ImageFBO * pOutputImage, const ImageFBO & rInputImage);

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
            virtual
            void
    setCgParameters();


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
            virtual
            CGprofile
    cgFragmentProfile()
            const;

private:
    // 
    // Private data
    //
	float _nDownsampleScale;     // Downsample Scale

	ProgramCg *_pDownsample_Cg;

    //
    // Private static data
    //
	static CGparameter _gDownsampleImage;
};


// -----------------------------------------------------------------------------
// BloomGlowHorz class (stage 2)
//
class BloomGlowHorz : public ImageFilter
{
public:
    // 
    // Construction and destruction
    //
    
            // Default constructor
            //
    BloomGlowHorz();

            // Destructor
            //
            virtual
   ~BloomGlowHorz()
            {
                ; // empty
            }


    //
    // Public methods
    //
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
            virtual
            void
    setSourceOperator(SourceOperator * pInputOperator);

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
            virtual
            bool
    dirty();

            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - Bloom Blur Width
            //
            // Returns:
            //      None
            //
            void
    setBloomBlurWidth(float nBloomBlurWidth);

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
            virtual
            Image
    image();

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
            //virtual
          //  ImageFBO
   // imageFBO();


protected:
    //
    // Protected methods
    //

            // renderOutput
            //
            void
    renderOutput(Image * pOutputImage, const Image & rInputImage);

            // renderOutput
            //
          //  void
  //  renderOutput(ImageFBO * pOutputImage, const ImageFBO & rInputImage);

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
            virtual
            void
    setCgParameters();


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
            virtual
            CGprofile
    cgFragmentProfile()
            const;

private:
    // 
    // Private data
    //
	ProgramCg * _pGlowH_Cg;

    CGparameter _gBloomBlurWidth;

    //
    // Private static data
    //
	static CGparameter _gGlowHImage;


};


// -----------------------------------------------------------------------------
// BloomGlowVert class (stage 3)
//
class BloomGlowVert : public ImageFilter
{
public:
    // 
    // Construction and destruction
    //
    
            // Default constructor
            //
    BloomGlowVert();

            // Destructor
            //
            virtual
   ~BloomGlowVert()
            {
                ; // empty
            }

    //
    // Public methods
    //

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
            virtual
            void
    setSourceOperator(SourceOperator * pInputOperator);

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
            virtual
            bool
    dirty();

            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - Bloom Blur Width
            //
            // Returns:
            //      None
            //
            void
    setBloomBlurWidth(float nBloomBlurWidth);

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
            virtual
            Image
    image();

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
          //  virtual
          //  ImageFBO
   // imageFBO();


protected:
    //
    // Protected methods
    //

            // renderOutput
            //
            void
    renderOutput(Image * pOutputImage, const Image & rInputImage);

            // renderOutput
            //
           // void
   // renderOutput(ImageFBO * pOutputImage, const ImageFBO & rInputImage);

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
            virtual void setCgParameters();


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
            virtual
            CGprofile
    cgFragmentProfile()
            const;

private:
    // 
    // Private data
    //
	ProgramCg * _pGlowV_Cg;

    CGparameter _gBloomBlurWidth;

    //
    // Private static data
    //
	static CGparameter _gGlowVImage;

};


// -----------------------------------------------------------------------------
// BloomFilter class
//
class BloomFilter: public ImageFilter
{
public:
    // 
    // Construction and destruction
    //
    
            // Default constructor
            //
    BloomFilter();

            // Destructor
            //
            virtual
   ~BloomFilter()
            {
                ; // empty
            }


    //
    // Public methods
    //

            // scene_intensity
            //
            // Description:
            //      Get the filter's scene intensity value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current scene intensity value.
            //
            float
    scene_intensity()
            const
			{
				return _nSceneIntensity;
			}

            // glow_intensity
            //
            // Description:
            //      Get the filter's glow intensity value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current glow intensity value.
            //
            float
    glow_intensity()
            const
			{
				return _nGlowIntensity;
			}

            // highlight_threshold
            //
            // Description:
            //      Get the filter's highlight threshold value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current highlight threshold value.
            //
            float
    highlight_threshold()
            const
			{
				return _nHighlightThreshold;
			}

            // highlight_intensity
            //
            // Description:
            //      Get the filter's highlight intensity value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current highlight intensity value.
            //
            float
    highlight_intensity()
            const
			{
				return _nHighlightIntensity;
			}

            // downsample_scale
            //
            // Description:
            //      Get the downsample scale's intensity
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current highlight intensity value.
            //
            float
    downsample_scale()
            const
			{
				return _nDownsampleScale;
			}

            // highlight_intensity
            //
            // Description:
            //      Get the filter's highlight intensity value.
            //
            // Parameters:
            //      None
            //
            // Return:
            //      The current highlight intensity value.
            //
            float
    bloom_blurwidth()
            const
			{
				return _nBloomBlurWidth;
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
    setSceneIntensity(float nSceneIntensity);

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
    setGlowIntensity(float nGlowIntensity);

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
    setHighlightThreshold(float nHighlightThreshold);

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
    setHighlightIntensity(float nHighlightIntensity);


            // setBloomBlurWidth
            //
            // Description:
            //      Set the Bloom Blur Width
            //
            // Parameters:
            //      nBloomBlurWidth - Bloom Blur Width
            //
            // Returns:
            //      None
            //
            void
    setBloomBlurWidth(float nBloomBlurWidth);

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
    setDownsampleScale(float nDownsampleScale);

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
            virtual
            void
    setSourceOperator(SourceOperator * pInputOperator);

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
            virtual
            bool
    dirty();

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
            virtual
            Image
    image();

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
            //virtual
           // ImageFBO
    //imageFBO();

protected:
    //
    // Protected methods
    //

            // renderOutput
            //
            void
    renderOutput(Image * pOutputImage, const Image & rInputImage);

            // renderOutput
            //
          //  void
    //renderOutput(ImageFBO * pOutputImage, const ImageFBO & rInputImage);

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
            virtual
            void
    setCgParameters();


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
            virtual
            CGprofile
    cgFragmentProfile()
            const;


private:
    // 
    // Private data
    //

    float _nSceneIntensity;      // Scene Intensity
    float _nGlowIntensity;       // Glow Intensity
    float _nHighlightThreshold;  // Highlight Threshold
    float _nHighlightIntensity;  // Highlight Intensity
    float _nDownsampleScale;     // Downsample Scale
    float _nBloomBlurWidth;      // Bloom Blur Width

    //
    // Private static data
    //

    SourceOperator  *_pOriginalOperator;
	BloomDownsample _oDownsampleFilter;
	BloomGlowVert	_oGlowVertFilter;
	BloomGlowHorz	_oGlowHorzFilter;

	ProgramCg *_pFinalComp_Cg;

    static CGparameter _gSceneSampler;
    static CGparameter _gBlurredSceneSampler;

};

#endif // BLOOM_FILTER_H
