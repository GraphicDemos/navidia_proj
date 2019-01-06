#ifndef IMAGE_SINK_DUMMY_H
#define IMAGE_SINK_DUMMY_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ImageSinkDummy class
//
// Description:
//      A class that simply does nothing but has the ImageSink interface.
//
// Author: Frank Jargstorff (03/18/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Includes
//

#include "ImageSink.h"

// ----------------------------------------------------------------------------
// ImageSinkDummy class
//
class ImageSinkDummy: public ImageSink
{
public:
    //
    // Public data
    //
    
    static const char * ClassName;
    static const char * ClassDescription;
    

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
    ImageSinkDummy(unsigned int nWidth = 0, unsigned int nHeight = 0): ImageSink(nWidth, nHeight)
            { ; }
                
            // Destructor
            //
            virtual
   ~ImageSinkDummy()
            { ; }
   
   
    //
    // Public methods
    //
            
            // pull
            //
            // Description:
            //      Reads pixels from frame buffer.
            //          This impelementation in fact (remeber the class
            //      is called dummy) doesn't read anything.
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
    pull(unsigned int nPositionX, unsigned int nPositionY)
            { 
                nPositionX; // avoid warnings
                nPositionY; // avoid warnings
                
                return 0;
            }

 };

#endif // IMAGE_SINK_DUMMY_H
