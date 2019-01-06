#ifndef __NORMALMAPUTILSGL_H__
#define __NORMALMAPUTILSGL_H__

#include "normalMapUtils.h"
#include <GL/glext.h>

inline int formatToComponents(GLenum format)
{
    switch(format) 
    {
    case GL_LUMINANCE:
    case GL_INTENSITY:
    case GL_ALPHA:
        return 1;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_HILO_NV:
        return 2;
        break;
    case GL_RGB:
        return 3;
        break;
    case GL_RGBA:
    case GL_ABGR_EXT:
    default:
        return 4;
        break;
    }
}

inline void NormalizationCubeMapGL(int    level, 
                                   GLenum internalFormat, 
                                   int    size, 
                                   GLenum format, 
                                   bool   scaleAndBias = true, 
                                   bool   hiloZ        = false)
{
    const GLenum faceIDs[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,   // -,-,+
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,   // +,-,-
                                GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,   // +,+,+
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,   // +,-,-
                                GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,   // +,-,+
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB};  // -,-,-

    float *data = new float[size * size * formatToComponents(format)];

    for (int face = 0; face < 6; ++face)
    {
        
        MakeNormalizationCubeMapFace((CubeFace)face, data, size, 
                                     formatToComponents(format), 
                                     scaleAndBias, hiloZ);
		glTexImage2D(faceIDs[face], level, internalFormat, 
                     size, size, 0, format, GL_FLOAT, data);
    
    }
    delete [] data;
}

#endif //__NORMALMAPUTILSGL_H__
