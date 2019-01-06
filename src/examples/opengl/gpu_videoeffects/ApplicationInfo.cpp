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
//      Copyright (C) 2004 by NVIDIA Corporation. All rights reserved.
//
// ----------------------------------------------------------------------------


//
// Include
//

#include "ApplicationInfo.h"

#include <sstream>
#include <iomanip>

#include <GL/glut.h>
#include <GL/gl.h>


//
// Defines
//

#define TEXT_TYPE GLUT_BITMAP_HELVETICA_12
#define N_TEXT_HEIGHT 12
#define N_TEXT_SEPERATOR_TOP 1
#define N_TEXT_SEPERATOR_BOTTOM 5
#define N_TEXT_SEPERATOR_LEFT 3
#define N_TEXT_SEPERATOR_RIGHT 3


// ----------------------------------------------------------------------------
// ApplicationInfo class
//
    //
    // Construction and destruction
    //
    
        // Default constructor
        //
ApplicationInfo::ApplicationInfo(): _nWidth(0)
                                  , _nHeight(0)
{
    _aInfoItem[N_ITEM_IMAGE_SOURE]          = "Image Source";
    _aInfoItem[N_ITEM_IMAGE_SINK]           = "Image Sink";
    _aInfoItem[N_ITEM_IMAGE_FORMAT]         = "Image Format";
    _aInfoItem[N_ITEM_IMAGE_FORMAT_GL]      = "GL Internal";
    _aInfoItem[N_ITEM_SHADER]               = "Shader";
    _aInfoItem[N_ITEM_FRAMES_PER_SECOND]    = "FPS";
    _aInfoItem[N_ITEM_TRANSFER_DOWNLOAD]    = "Download Rate";
    _aInfoItem[N_ITEM_TRANSFER_READBACK]    = "Readback Rate";
    _aInfoItem[N_ITEM_TRANSFER_TOTAL]       = "Total Transfer";
    
    _nMaxItemString = maxStringLength(_aInfoItem);

    _nHeight = textBoxHeight() * N_INFO_ITEMS + 1;
}

        // Destructor
        //
ApplicationInfo::~ApplicationInfo()
{
    ;
}

    
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
ApplicationInfo::width()
        const
{
    return _nWidth;
}
        
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
ApplicationInfo::height()
        const
{
    return _nHeight;
}
            
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
ApplicationInfo::render(unsigned int nPositionX, unsigned int nPositionY)
{
    std::ostringstream oStringBuffer;
    
    oStringBuffer << std::showpoint << std::setprecision(5) << std::setw(8) << _nDownloadRate+_nReadbackRate;
    _aInfoValue[N_ITEM_TRANSFER_TOTAL] = oStringBuffer.str();
    
    nPositionY += textBoxHeight() * (N_INFO_ITEMS-1);
    unsigned int nMaxValueString = maxStringLength(_aInfoValue);
 
    _nWidth = _nMaxItemString + nMaxValueString;
 
    float aTextColor[3] = {1.0f, 1.0f, 1.0f};
    glColor3fv(aTextColor);
    for (int iLine = 0; iLine < N_INFO_ITEMS; ++iLine)
    {
        drawStringBox(nPositionX, nPositionY, _nMaxItemString, textBoxHeight(), _aInfoItem[iLine]);
        drawStringBox(nPositionX + _nMaxItemString, nPositionY, 
                    nMaxValueString, textBoxHeight(), 
                    _aInfoValue[iLine]);
        nPositionY -= textBoxHeight();
    }
}

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
ApplicationInfo::setImageSource(std::string sImageSource)
{
    _aInfoValue[N_ITEM_IMAGE_SOURE] = sImageSource;
}

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
ApplicationInfo::setImageSink(std::string sImageSink)
{
    _aInfoValue[N_ITEM_IMAGE_SINK] = sImageSink;
}

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
ApplicationInfo::setImageFormat(std::string sImageFormat)
{
    _aInfoValue[N_ITEM_IMAGE_FORMAT] = sImageFormat;
}

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
ApplicationInfo::setImageFormatGL(std::string sImageFormatGL)
{
    _aInfoValue[N_ITEM_IMAGE_FORMAT_GL] = sImageFormatGL;

}
    
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
ApplicationInfo::setShader(std::string sShader)
{
    _aInfoValue[N_ITEM_SHADER] = sShader;
}
         
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
ApplicationInfo::setFramesPerSecond(double nFPS)
{
    _nFPS = nFPS;
    
    std::ostringstream oStringBuffer;
    
    oStringBuffer << std::showpoint << std::setprecision(4) << std::setw(8) << nFPS;
    _aInfoValue[N_ITEM_FRAMES_PER_SECOND] = oStringBuffer.str();
}

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
ApplicationInfo::setRenderTime(double nTimeFrame)
{
    std::ostringstream oStringBuffer;
    
    oStringBuffer << std::showpoint << std::setprecision(4) << std::setw(8) << _nFPS;
    oStringBuffer << " - " << std::showpoint << std::setprecision(4) << std::setw(12) << nTimeFrame;
    _aInfoValue[N_ITEM_FRAMES_PER_SECOND] = oStringBuffer.str();
}

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
ApplicationInfo::setBytesDownloaded(unsigned int nDownloadedBytes)
{
    std::ostringstream oStringBuffer;
    
    _nDownloadRate = _nFPS * nDownloadedBytes / (1024*1024); // MB/s

    oStringBuffer << std::showpoint << std::setprecision(5) << std::setw(8) << _nDownloadRate;
    _aInfoValue[N_ITEM_TRANSFER_DOWNLOAD] = oStringBuffer.str();
}

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
ApplicationInfo::setBytesRead(unsigned int nReadBytes)
{
    std::ostringstream oStringBuffer;
    
    _nReadbackRate = _nFPS * nReadBytes / (1024*1024); // MB/s

    oStringBuffer << std::showpoint << std::setprecision(5) << std::setw(8) << _nReadbackRate;
    _aInfoValue[N_ITEM_TRANSFER_READBACK] = oStringBuffer.str();
}


    //
    // Private methods
    //
    
        void
ApplicationInfo::drawString(unsigned int nPositionX, unsigned int nPositionY, std::string sString)
        const
{
    glRasterPos2i(nPositionX, nPositionY);
    for (unsigned int iChar = 0; iChar < sString.length(); ++iChar)
        glutBitmapCharacter(TEXT_TYPE, sString[iChar]);
}

        void
ApplicationInfo::drawStringBox(unsigned int nPositionX, unsigned int nPositionY, 
                unsigned int nWidth, unsigned int nHeight, std::string sString)
        const
{
    glBegin(GL_LINE_LOOP);
        glVertex2f(static_cast<float>(nPositionX), static_cast<float>(nPositionY));
        glVertex2f(static_cast<float>(nPositionX + nWidth), static_cast<float>(nPositionY));
        glVertex2f(static_cast<float>(nPositionX + nWidth), static_cast<float>(nPositionY + nHeight));
        glVertex2f(static_cast<float>(nPositionX), static_cast<float>(nPositionY + nHeight));
    glEnd();
    
    drawString(nPositionX + N_TEXT_SEPERATOR_LEFT, nPositionY + N_TEXT_SEPERATOR_BOTTOM, sString);
}


        unsigned int
ApplicationInfo::textHeight()
        const
{
    return N_TEXT_HEIGHT;
}

        unsigned int 
ApplicationInfo::textBoxHeight()
        const
{
    return N_TEXT_HEIGHT + N_TEXT_SEPERATOR_TOP + N_TEXT_SEPERATOR_BOTTOM;
}
        
        unsigned int
ApplicationInfo::maxStringLength(std::string aStrings[])
        const
{
    unsigned int nMaxLength = 0;
    
    for (int iItem = 0; iItem < N_INFO_ITEMS; ++iItem)
    {
        unsigned int nStringLength = stringLength(aStrings[iItem]);
    
        if (nStringLength > nMaxLength)
            nMaxLength = nStringLength;
    }
    
    return nMaxLength + N_TEXT_SEPERATOR_RIGHT + N_TEXT_SEPERATOR_LEFT;
    
}   
        
        unsigned int 
ApplicationInfo::stringLength(const std::string & rString)
        const
{
    unsigned int nLength = 0;
    
    for (unsigned int iChar = 0; iChar < rString.length(); ++iChar)
        nLength += glutBitmapWidth(TEXT_TYPE, rString[iChar]);
        
    return nLength;
}
            

