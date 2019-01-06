#ifndef OPEN_EXR_LOADER_H
#define OPEN_EXR_LOADER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      OpenEXRLoader class
//
// Description:
//      Load OpenEXR image from disk.
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

#include "ImageLoader.h"


// ----------------------------------------------------------------------------
// OpenEXRLoader abstract base class
//
class OpenEXRLoader: public ImageLoader
{
public:
    //
    // Construction and destruction
    //
            
            // Default constructor
            //
    OpenEXRLoader(const char * zFileName);
            
            // Destructor
            //
            virtual
   ~OpenEXRLoader();
    
   
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
            void
    load(const char * zFileName);
};

#endif // OPEN_EXR_LOADER_H
