#ifndef NVIMAGE_SOURCE_H
#define NVIMAGE_SOURCE_H
// -----------------------------------------------------------------------------
// 
// Contents:
//      NVImageLoader class
//
// Description:
//      NVImageLoader loads DDS images and Video from files.
//          The operator has some infrastructure to support additional image
//      formats in the future.
//
// Author:
//      Eric Young (2005)
//
// -----------------------------------------------------------------------------


//
// Include
//

class PBuffer;
class RenderTexture;

class ImageSource;
class ImagePusher;
class ImagePusherPBO;
class ImagePusherDoublePBO;

class VideoPusher;
class VideoPusherPBO;
class VideoPusherDoublePBO;

#include <SourceOperator.h>
#include <ImageFilter.h>

#include <iostream>



// -----------------------------------------------------------------------------
// NVImageLoader class
//
class NVImageLoader: public ImageFilter
{
	friend class Scene;
    friend class ImageSource;

    friend class ImagePusher;
    friend class ImagePusherPBO;
    friend class ImagePusherDoublePBO;

    friend class VideoPusher;
    friend class VideoPusherPBO;
    friend class VideoPusherDoublePBO;


public:
    //
    // Public types
    //

    enum teFileType
    {
        DDS_FILE,
		IMAGE_FILE,
        VIDEO_FILE
    };


    // 
    // Construction and destruction
    //
    
            // Default constructor
            //
    NVImageLoader();

            // Destructor
            //
            virtual
   ~NVImageLoader();


    //
    // Public methods
    //

            // setFileName
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
    setFileName(std::string sFileName);

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
    setFileType(teFileType eFileType);

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
            teFileType
    fileType()
            const;

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
         /*   ImageFBO
    imageFBO();*/

			// render_to_image_buffer
			//
			// Description:
			//
			// Parameters:
			//
			// Returns:
			//
			Image
	render_to_image_buffer();

			// render_to_image_buffer
			//
			// Description:
			//
			// Parameters:
			//
			// Returns:
			//
			/*ImageFBO
	render_to_image_FBO();
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
	render_to_screen();

			// render_to_pbuffer
			//
			// Description:
			//
			// Parameters:
			//
			// Returns:
			//
			void
	render_to_pbuffer();

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
            virtual 
            void
    setImagePusher(ImageSource * pImage);


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
	displayPbuffer(int win_width, int win_height);


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
	bindPBuffer();

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
	releasePBuffer();

            // getTextureID
            //
            // Description:
            //      This is a method retrieves the texture ID from this class
            //
            // Parameters:
            //      None - the method gets its information from the class state.
            //
            // Returns:
            // 
            GLuint
    getTextureID() { return pbuffer_tex; }

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
    createTexture(GLenum target, GLenum internalformat, int width, int height, GLenum format, GLenum type);

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
	NVImageLoader::loadProgram(GLenum program_type, const char *code);

protected:
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
    // Private methods
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
    loadImage(Image * pImage);


private:
    // 
    // Private data
    //

    bool _bDirty;
	bool _bUseYUV;

    std::string _sFileName;
    teFileType  _eFileType;

	RenderTexture *_hRenderTarget;
    PBuffer     * pbuffer;
    GLuint      pbuffer_tex;
    GLuint      image_tex;

    Image       _oImage;
    //ImageFBO    _oImageFBO;

    ImageSource  * _pImagePusher;

	GLuint display_fprog, render_fprog, white_fprog;

	CGprogram   _goYUV2RGBShader;
    CGparameter _hYUVImage;

	CGprogram   _goRGBAShader;
	CGparameter	_hRGBAImage;
};

#endif // IMAGE_LOAD_H
