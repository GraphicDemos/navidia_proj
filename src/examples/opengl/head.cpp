#if defined(UNIX)
#define GL_GLEXT_PROTOTYPES
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>

#include <stdio.h>
#include <memory.h>
#include <Cg/cgGL.h>
#include <string>
#include "head.h"
#include <vector>
#include <iostream>

#include <nv_png.h>
#include <shared/objload.h>
#include <shared/normalMapUtilsGL.h>
#include <shared/data_path.h>
#include <shared/quitapp.h>

using namespace std;
using namespace glh;

Head::Head(bool managed)
: _rBumpScale(1.0f),
_bMipMap(false),
_iMaxMipLevel(8),
_bHalfPrecision(true),
_iNumVertices(0), 
_iNumIndices(0), 
_managed(managed),
_cgContext(0),
_vertexProgram(0),
_normFragmentProgram(0),
_bumpScaleParam(0),
_positionParam(0),
_texcoordParam(0),
_tangentParam(0),
_binormalParam(0),
_normalParam(0),
_normDiffuseMapParam(0),
_normGlossMapParam(0),
_normNormalMapParam(0),
_normEyeSpaceLightPosParam(0)
{
    _rLightPos[0] = _rLightPos[1] = _rLightPos[2] = 10;
}

Head::~Head()
{
    if (_managed)
        Destroy();
}

bool Head::Initialize(CGcontext context)
{
    if (GL_TRUE != glh_init_extension("GL_ARB_vertex_buffer_object"))
    {
        fprintf(stderr, "Error: ARB_vertex_buffer_object Extension not found.");
        return false;
    }

    if (CG_VERSION_NUM < 1200)
    {
        fprintf(stderr, "Normalization Heuristics requires Cg 1.2 or higher.");
        return false;
    }
    
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    _programsPath = media.get_path("programs/normalization_heuristics/norm_vertex.cg");
    if (_programsPath.empty())
    {
        printf("Unable to locate Cg programs, exiting...\n");
        quitapp(1);
    }

    // Setup CG programs
    _cgContext = context;

    _vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    if (CG_PROFILE_UNKNOWN == _vertexProfile)
    {
        printf("Vertex programming extensions (GL_ARB_vertex_program or "
               "GL_NV_vertex_program extension not supported, exiting...\n");
        quitapp(1);
    }

    _fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    if (CG_PROFILE_UNKNOWN == _fragmentProfile)
    {
        printf("Fragment programming extensions (GL_ARB_fragment_program or "
               "GL_NV_fragment_program) not supported, exiting...\n");
        quitapp(1);
    }

    _vertexProgram = cgCreateProgramFromFile(_cgContext, CG_SOURCE, 
        (_programsPath + "/norm_vertex.cg").c_str(),
        _vertexProfile, NULL, NULL);

    cgGLLoadProgram(_vertexProgram);

    _bumpScaleParam = cgGetNamedParameter(_vertexProgram, "bumpScale");
    _positionParam  = cgGetNamedParameter(_vertexProgram, "oPosition");  
    _texcoordParam  = cgGetNamedParameter(_vertexProgram, "texcoord0");  
    _tangentParam   = cgGetNamedParameter(_vertexProgram, "tangent");
    _binormalParam  = cgGetNamedParameter(_vertexProgram, "binormal");
    _normalParam    = cgGetNamedParameter(_vertexProgram, "normal");

    if (!_bumpScaleParam || !_positionParam || !_texcoordParam || 
        !_tangentParam || !_binormalParam || !_normalParam)
    {
        printf("Unable to retrieve vertex program parameters, exiting...\n");
        quitapp(1);
    }

    ResetFragmentProgram(false, false, false, false, false, false);

    // Load the textures.
    array2<vec3ub> img;
    int iWidth, iHeight;

    // textures
    fprintf( stderr, "Loading diffuse map." );
    read_png_rgb("Walker_Head-1024DF-7.png", img);
    iWidth = img.get_width();
    iHeight = img.get_height();

    glGenTextures(1, &_iDiffuseTextureID);
    glBindTexture(GL_TEXTURE_2D, _iDiffuseTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, 
        GL_RGB, GL_UNSIGNED_BYTE, img.get_pointer());
    fprintf( stderr, "." );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    fprintf( stderr, ".\n" );

    // Oiliness texture
    fprintf( stderr, "Loading gloss map." );

    read_png_rgb("Walker_Head-1024SS-2.png", img);
    iWidth = img.get_width();
    iHeight = img.get_height();

    glGenTextures(1, &_iGlossyTextureID);
    glBindTexture(GL_TEXTURE_2D, _iGlossyTextureID);
    fprintf( stderr, "." );

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, 
        GL_RGB, GL_UNSIGNED_BYTE, img.get_pointer());
    fprintf( stderr, "." );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    fprintf( stderr, ".\n" );

    // Normal map texture
    fprintf( stderr, "Loading normal map." );
    array2<unsigned char> bpimg;
    read_png_grey("Walker_Head-1024BP-2.png", bpimg);
    iWidth = img.get_width();
    iHeight = img.get_height();

    float *pNormalTex = new float[3 * iWidth * iHeight];
    float scale[3] = {1, 1, 0.15f};
    glGenTextures(1, &_iNormalTextureID);
    glBindTexture(GL_TEXTURE_2D, _iNormalTextureID);
    fprintf( stderr, "." );	

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    BumpmapToNormalmap((unsigned char*)bpimg.get_pointer(), pNormalTex, scale, iWidth, iHeight);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SIGNED_RGB_NV, iWidth, iHeight, 0, 
        GL_RGB, GL_FLOAT, pNormalTex);

    delete [] pNormalTex;
    fprintf( stderr, ".\n" );

    // Create the normalization cube map
    glGenTextures(1, &_iNormCubeMapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, _iNormCubeMapTextureID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Make the normalization cube map levels (it is mipmapped).
    int iMaxLevel = 8;
    int iLevel = 0;
    int iRes = 256;
    for (iLevel = 0; iLevel <= iMaxLevel; iLevel++)
    {
        NormalizationCubeMapGL(iLevel, GL_SIGNED_RGB_NV, iRes, GL_RGB, false);  
        iRes /= 2;
    }

    fprintf( stderr, ".\n" );

    // Create the normalization cube map
    glGenTextures(2, _iHiLoNormCubeMapTextureID);  
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, _iHiLoNormCubeMapTextureID[0]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    iRes = 256;
    iLevel = 0;
    for (iLevel = 0; iLevel <= iMaxLevel; iLevel++)
    {
        NormalizationCubeMapGL(iLevel, GL_SIGNED_HILO16_NV, iRes, GL_HILO_NV, false);  
        iRes /= 2;
    }

    fprintf( stderr, ".\n" );

    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, _iHiLoNormCubeMapTextureID[1]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    iRes = 256;
    iLevel = 0;
    for (iLevel = 0; iLevel <= iMaxLevel; iLevel++)
    {
        NormalizationCubeMapGL(iLevel, GL_SIGNED_HILO16_NV, iRes, GL_HILO_NV, 
            false, true);  
        iRes /= 2;
    }

    fprintf( stderr, ".\n" );

    unsigned int *pIndices;
    float *pPositions, *pNormals, *pTangents, *pBinormals, *pTexCoords;
    // Load the model data
    printf("Loading Model");
    LoadObjModel(media.get_file("models/BURKE_TestHead.obj").c_str(), 
        _iNumVertices, _iNumIndices, pIndices, pPositions, pNormals, 
        pTangents, pBinormals, pTexCoords);
    printf("\n");

    // Setup VBO
    glGenBuffersARB(HEAD_NUM_VBOS, _iVBOs);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_INDEX]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, _iNumIndices * sizeof(unsigned int), 
        pIndices, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_POSITION]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (_iNumVertices * 3) * sizeof(float), 
        pPositions, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_NORMAL]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (_iNumVertices * 3) * sizeof(float), 
        pNormals, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TANGENT]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (_iNumVertices * 3) * sizeof(float), 
        pTangents, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_BINORMAL]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (_iNumVertices * 3) * sizeof(float), 
        pBinormals, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TEXTURE_COORD_0]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (_iNumVertices * 2) * sizeof(float), 
        pTexCoords, GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    delete [] pIndices;
    delete [] pPositions;
    delete [] pNormals;
    delete [] pTangents;
    delete [] pBinormals;
    delete [] pTexCoords;

    return true;
}

void Head::Destroy()
{
    glDeleteTextures(1, &_iDiffuseTextureID);
    glDeleteTextures(1, &_iGlossyTextureID);
    glDeleteTextures(1, &_iNormalTextureID);

    // destroy the program objects
    cgDestroyProgram(_vertexProgram);
    cgDestroyProgram(_normFragmentProgram);

    glDeleteBuffers(HEAD_NUM_VBOS, _iVBOs);
}

void Head::ResetFragmentProgram(bool bV, bool bL, bool bH, bool bN, bool bHilo, bool bOpt)
{
    vector<const char*> args;
    args.clear();
    if (_bHalfPrecision) args.push_back("-DUSE_HALF");
    args.push_back(0);
  
    if (_normFragmentProgram) cgDestroyProgram(_normFragmentProgram);
    
    cgSetAutoCompile(_cgContext, CG_COMPILE_MANUAL);
    cgGLSetManageTextureParameters(_cgContext, CG_TRUE);

    _normFragmentProgram = cgCreateProgramFromFile(_cgContext,
        CG_SOURCE, 
        (_programsPath + "/norm.cg").c_str(),
        _fragmentProfile, 
        NULL, &args[0]);
    
    _stdNorm = _cubeNorm = 0;
    if (!_stdNorm && !(bV && bL && bH && bN))
    {
        _stdNorm = cgCreateParameter(_cgContext, 
            cgGetNamedUserType(_normFragmentProgram, "StdNormalizer"));
    }

    string cubeType = bHilo ? "HiloCubeNormalizer" : "CubeNormalizer";
        
    if (bV || bL || bH || bN)
    {
        if (_cubeNorm)
            cgDestroyParameter(_cubeNorm);
        _cubeNorm = cgCreateParameter(_cgContext, 
            cgGetNamedUserType(_normFragmentProgram, cubeType.c_str()));
    }

    bool bCubes[4]; 
    bCubes[0] = bV; bCubes[1] = bL; bCubes[2] = bH; bCubes[3] = bN;

    for (int i = 0; i < 4; ++i)
    {       
        char name[32];
        sprintf(name, "normalizer[%d]", i);
        CGparameter normIface = cgGetNamedParameter(_normFragmentProgram, name);
        cgConnectParameter(bCubes[i] ? _cubeNorm : _stdNorm, normIface);
    }

    cgCompileProgram(_normFragmentProgram);

    cgGLLoadProgram(_normFragmentProgram);

    _normNormCubeMapParam = 0;
    _normHiLoNormCubeMapParamXY = _normHiLoNormCubeMapParamZ = 0;

    if (_cubeNorm && !bHilo)
    {
        _normNormCubeMapParam = cgGetNamedStructParameter(_cubeNorm, "normCube");
        cgGLSetTextureParameter(_normNormCubeMapParam, _iNormCubeMapTextureID);
    }
    else if (_cubeNorm)
    {
        _normHiLoNormCubeMapParamXY = cgGetNamedStructParameter(_cubeNorm, "normCubeXY");
        cgGLSetTextureParameter(_normHiLoNormCubeMapParamXY, _iHiLoNormCubeMapTextureID[0]);
        _normHiLoNormCubeMapParamZ = cgGetNamedStructParameter(_cubeNorm, "normCubeZ"); 
        cgGLSetTextureParameter(_normHiLoNormCubeMapParamZ, _iHiLoNormCubeMapTextureID[1]);
    }

    _normEyeSpaceLightPosParam = cgGetNamedParameter(_normFragmentProgram, 
                                                     "eyeSpaceLightPosition");
    _normDiffuseMapParam = cgGetNamedParameter(_normFragmentProgram, 
                                               "diffuseTexture");
    _normGlossMapParam = cgGetNamedParameter(_normFragmentProgram, 
                                             "glossyTexture");
    _normNormalMapParam = cgGetNamedParameter(_normFragmentProgram, 
                                              "normalTexture");

    cgGLSetTextureParameter(_normDiffuseMapParam, _iDiffuseTextureID);
    cgGLSetTextureParameter(_normGlossMapParam, _iGlossyTextureID);
    cgGLSetTextureParameter(_normNormalMapParam, _iNormalTextureID);
}

void Head::Display(DisplayMode eMode)
{
    glColor3f(1, 1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    cgGLEnableProfile(_fragmentProfile);

    if (1)//HEAD_DISPLAY_STD == eMode)
    {   
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, _iNormCubeMapTextureID);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, 
            _bMipMap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAX_LOD, _iMaxMipLevel);

        cgGLBindProgram(_normFragmentProgram);

        cgGLSetParameter3fv(_normEyeSpaceLightPosParam, _rLightPos);

        cgGLSetTextureParameter(_normDiffuseMapParam, _iDiffuseTextureID);
        cgGLSetTextureParameter(_normGlossMapParam, _iGlossyTextureID);
        cgGLSetTextureParameter(_normNormalMapParam, _iNormalTextureID);
    }

    // Setup vertex program and draw
    cgGLBindProgram(_vertexProgram);
    cgGLEnableProfile(_vertexProfile);

    CGparameter param;
    param = cgGetNamedParameter(_vertexProgram, "modelViewProj");
    cgGLSetStateMatrixParameter(param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    param = cgGetNamedParameter(_vertexProgram, "modelView");
    cgGLSetStateMatrixParameter(param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);

    cgGLSetParameter1f(_bumpScaleParam, _rBumpScale);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_POSITION]);
    cgGLEnableClientState(_positionParam);
    cgGLSetParameterPointer(_positionParam, 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_NORMAL]);
    cgGLEnableClientState(_normalParam);
    cgGLSetParameterPointer(_normalParam, 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TANGENT]);
    cgGLEnableClientState(_tangentParam);
    cgGLSetParameterPointer(_tangentParam, 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_BINORMAL]);
    cgGLEnableClientState(_binormalParam);
    cgGLSetParameterPointer(_binormalParam, 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TEXTURE_COORD_0]);
    cgGLEnableClientState(_texcoordParam);
    cgGLSetParameterPointer(_texcoordParam, 2, GL_FLOAT, 0, 0);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _iVBOs[HEAD_INDEX]);
    glDrawElements(GL_TRIANGLES, _iNumIndices, GL_UNSIGNED_INT, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    // Disable vertex program state
    cgGLDisableClientState(_positionParam);
    cgGLDisableClientState(_texcoordParam);
    cgGLDisableClientState(_normalParam);
    cgGLDisableClientState(_binormalParam);
    cgGLDisableClientState(_tangentParam);

    cgGLDisableProfile(_vertexProfile);

    cgGLDisableProfile(_fragmentProfile);

    glPopMatrix();
}

void Head::PrintCurrentFragmentProgram(DisplayMode eMode)
{
    printf("%s\n", cgGetProgramString(_normFragmentProgram, 
           CG_COMPILED_PROGRAM));
}
