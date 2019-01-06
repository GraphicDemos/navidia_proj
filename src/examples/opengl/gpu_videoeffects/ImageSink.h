#ifndef IMAGE_SINK_H
#define IMAGE_SINK_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImageSink class
//
// Description:
//      A class reading pixels into an image via pixel buffer objects (PBO).
//
// Author: Frank Jargstorff (03/11/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//



// ----------------------------------------------------------------------------
// ImageSink class
//
class ImageSink
{
public:
    //
    // Public types
    //
    
    struct tsBGRA8
    {
        unsigned char nB; // blue
        unsigned char nG; // green
        unsigned char nR; // red
        unsigned char nA; // alpha
    };
    
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
            // Description:
            //      Creates image puller for given size images.
            //
            // Parameters:
            //      nWidth  - image width.
            //      nHeight - image height.
            //
    ImageSink(unsigned int nWidth = 0, unsigned int nHeight = 0): _bDummyWorkload(false)
                                                                , _nWidth(nWidth)
                                                                , _nHeight(nHeight)
            { ; }
                
            // Destructor
            //
            virtual
   ~ImageSink()
            { ; }
   
   
    //
    // Public methods
    //
    
            // enableDummyWorkload
            //
            // Description:
            //      Perform a dummy task after downloading image.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    enableDummyWorkload()
            {
                _bDummyWorkload = true;
            };
    
            // disableDummyWorkload
            //
            // Description:
            //      Don't perform a dummy task after downloading image.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      None
            //
            void
    disableDummyWorkload()
            {
                _bDummyWorkload = false;
            };
   
            // resize
            //
            // Description:
            //      Resizes the buffer size.
            //          The content of the new image is undefined.
            //
            // Parameters:
            //      nWidth  - the new width.
            //      nHeight - the new height.
            //
            // Returns:
            //      None
            //
            virtual
            void 
    resize(unsigned int nWidth, unsigned int nHeight)
            {
                _nWidth  = nWidth;
                _nHeight = nHeight;
            }
    
            // width
            //
            // Description:
            //      Get current image width.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The current withd.
            //
            unsigned int
    width()
            const
            {
                return _nWidth;
            }
            
            // height
            //
            // Description:
            //      Get current image height.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The current height.
            //
            unsigned int
    height()
            const
            {
                return _nHeight;
            }
   
            
            // imageSize
            //
            // Description:
            //      Get the image's size in pixels.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      Image size in pixels.
            //
            unsigned int
    imageSize()
            const
            {
                return _nWidth * _nHeight;
            }
            
            // imageDataSize
            //
            // Description:
            //      Get the image data's size in bytes.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      Image data size.
            //
            unsigned int
    imageDataSize()
            const
            {
                return _nWidth * _nHeight * sizeof(tsBGRA8);
            }
            
            // pull
            //
            // Description:
            //      Reads pixels from frame buffer.
            //          The buffer read from is always the currently 
            //      displayed buffer.
            //          If the position is such that the image size
            //      exceeds the frame buffer size the outcome of the
            //      operation is undefined and might stop the program.
            //
            // Parameters:
            //      nPositionX - x-position in FB where to start reading from.
            //      nPositionY - y-position in FB where to start reading from.
            //
            // Returns:
            //      The number of bytes read back to system memory. 
            //      
            virtual
            unsigned int
    pull(unsigned int nPositionX, unsigned int nPositionY) = 0;

protected:
    //
    // Protected data
    //
    
    bool _bDummyWorkload;

private:
    //
    // Private data
    //
    
    unsigned int _nWidth;
    unsigned int _nHeight;
    
 };

#endif // IMAGE_SINK_H
