#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImageLoader class
//
// Description:
//      A abstract class for loading images from disk and feeding them 
//      into the compositor.
//
// Author: Frank Jargstorff (03/29/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "Image.h"


// ----------------------------------------------------------------------------
// ImageLoader abstract base class
//
class ImageLoader
{
public:
    //
    // Construction and destruction
    //
            
            // Default constructor
            //
    ImageLoader()
            { ; }
            
            // Destructor
            //
            virtual
   ~ImageLoader()
            { ; }
   
   
    //
    // Public methods
    //
   
            // loadImage
            //
            // Description:
            //      Loads a new image from a given file.
            //
            // Parameters:
            //      zFileName - name of the image file to load.
            //
            // Returns:
            //      None
            //
            virtual
            void
    load(const char * zFileName) = 0;
    
            
            // iamge
            //
            // Description:
            //      Get image.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      Const reference to the internally stored image.
            //
            const
            Image &
    image()
            const
            {
                return _oImage;
            }
            
                  
protected:
    //
    // Protected data
    //
    
    Image _oImage;
};

#endif // IMAGE_LOADER_H
