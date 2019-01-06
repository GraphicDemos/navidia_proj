#ifndef APPLICATION_INFO_H
#define APPLICATION_INFO_H
// ----------------------------------------------------------------------------
// 
// Content:
//      ApplicationInfo class
//
// Description:
//      A base class for image sources.
//
// Author: Frank Jargstorff (03/19/04)
//
// Note:
//      Copyright (C) 2004 by NVIDIA Croporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Include
//

#include <string>
#include <map>


//
// Defines
//

#define N_ITEM_IMAGE_SOURE          0
#define N_ITEM_IMAGE_SINK           1
#define N_ITEM_IMAGE_FORMAT         2
#define N_ITEM_IMAGE_FORMAT_GL      3
#define N_ITEM_SHADER               4
#define N_ITEM_FRAMES_PER_SECOND    5
#define N_ITEM_TRANSFER_DOWNLOAD    6
#define N_ITEM_TRANSFER_READBACK    7
#define N_ITEM_TRANSFER_TOTAL       8   

#define N_INFO_ITEMS                9



// ----------------------------------------------------------------------------
// ApplicationInfo class
//
class ApplicationInfo
{
public:
    //
    // Construction and destruction
    //
    
            // Default constructor
            //
    ApplicationInfo();
    
            // Destructor
            //
   ~ApplicationInfo();
 
    
    //
    // Public methods
    //
   
            // width
            //
            // Description:
            //      Get the image width in pixel.
            //
            // Parameters:
            //      None
            //
            // Returns:
            //      The image width.
            //
            unsigned int
    width()
            const;
            
            // height
            //
            // Description:
            //      Get the image height in pixel.
            // 
            // Parameters:
            //      None
            //
            // Returns:
            //      The image height.
            //
            unsigned int
    height()
            const;
             
            // render
            //
            // Description:
            //      Render the information at given position.
            //
            // Parameters:
            //      nPositionX - x-position.
            //      nPositionY - y-position.
            //
            // Returns:
            //      None
            //
            void
    render(unsigned int nPositionX, unsigned int nPositionY);
    
            // setImageSource
            //
            // Description:
            //      Set image-source description string.
            //
            // Parameters:
            //      sImageSource - description string.
            //
            // Returns:
            //      None
            //
            void
    setImageSource(std::string sImageSource);
    
            // setImageSink
            //
            // Description:
            //      Set image-sink description string.
            //
            // Parameters:
            //      sImageSink - description string.
            //
            // Returns:
            //      None
            //
            void
    setImageSink(std::string sImageSink);
    
            // setImageFormat
            //
            // Description:
            //      Set image-format description string.
            //
            // Parameters:
            //      sImageFormat - description string.
            //
            // Returns:
            //      None
            //
            void
    setImageFormat(std::string sImageFormat);
    
            // setImageFormatGL
            //
            // Description:
            //      Set OpenGL internal image-format description string.
            //
            // Parameters:
            //      sImageFormatGL - description string.
            //
            // Returns:
            //      None
            //
            void
    setImageFormatGL(std::string sImageFormatGL);
    
            // setShader
            //
            // Description:
            //      Set shader description string.
            //
            // Parameters:
            //      sShader - description string.
            //
            // Returns:
            //      None
            //
            void
    setShader(std::string sShader);    
         
            // setFramesPerSecond
            //
            // Description:
            //      Set the current frames-per-second.
            //
            // Parameters
            //      nFPS - the frames per second
            //
            // Returns:
            //      None
            //
            void
    setFramesPerSecond(double nFPS);

            // setRenderTime
            //
            // Description:
            //      Set the current frames-per-second.
            //
            // Parameters
            //      nTimeFrame - the time to render this frame
            //
            // Returns:
            //      None
            //
            void
    setRenderTime(double nTimeFrame);

            // setBytesDownloaded
            //
            // Description:
            //      Set the number of bytes sent to the GPU.
            //          This should be the number of bytes sent down for
            //      the last recent frame.
            //
            // Parameters:
            //      nDownloadedBytes - the number of bytes downloaded to the
            //                         GPU for the most recent frame.
            //
            // Returns:
            //      None
            //
            void
    setBytesDownloaded(unsigned int nDownloadedBytes);
    
            // setBytesRead
            //
            // Description:
            //      Set the number of bytes read from the GPU.
            //          This should be the number of bytes read-back from
            //      the GPU after compositing the most recent frame.
            //
            // Parameters:
            //      nReadBytes - the number of bytes read back from the GPU
            //                   for the most recent frame.
            //
            // Returns:
            //      None
            //
            void
    setBytesRead(unsigned int nBytesRead);
    
   
private:
    //
    // Priate methods
    //
    
            // drawPerformanceInfo
            //
            // Description:
            //      Draw a string with the performance numbers on the screen.
            //
            // Parameters:
            //      nPositionX - the x-position of the string's starting point.
            //      nPositionY - the y-position of the string's starting point.
            //
            // Returns:
            //      None    
            void
    drawPerformanceInformation(unsigned int nPositionX, unsigned int nPositionY)
            const;
            
            void
    drawString(unsigned int nPositionX, unsigned int nPositionY, std::string sString)
            const;
            
            void
    drawStringBox(unsigned int nPositionX, unsigned int nPositionY, 
                unsigned int nWidth, unsigned int nHeight, std::string sString)
            const;
            
            unsigned int
    textHeight()
            const;
            
            unsigned int 
    textBoxHeight()
            const;
            
            unsigned int
    maxStringLength(std::string aStrings[] )
            const;
            
            unsigned int 
    stringLength(const std::string & rString)
            const;

private:
    //
    // Private data
    //
   
    std::string  _aInfoItem[N_INFO_ITEMS];
    std::string  _aInfoValue[N_INFO_ITEMS];
    unsigned int _nMaxItemString;
        
    double _nFPS;
    double _nDownloadRate;
    double _nReadbackRate;
    
    unsigned int _nWidth;
    unsigned int _nHeight;
};

#endif // APPLICATION_INFO_H
