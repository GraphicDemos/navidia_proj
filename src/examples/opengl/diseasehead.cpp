#include <stdio.h>
#include <memory.h>
#include <Cg/cgGL.h>
#include <string>
#include "diseasehead.h"
#include <vector>
#include <iostream>

#include <glh/glh_extensions.h>
#include <nv_png.h>
#include <shared/objload.h>
#include <shared/normalMapUtilsGL.h>
#include <shared/data_path.h>
#include <shared/quitapp.h>
#include <shared/load_cubemap.h>

using namespace std;
using namespace glh;

Head::Head(bool managed)
:   _rBumpScale(1.0f),
    _rDiseaseBumpScale(1.0f),
    _rDiseaseMapScale(4.0f),
    _iNumVertices(0), 
    _iNumIndices(0), 
    _managed(managed),
    _cgContext(0)
{
    _rLightPos[0] = _rLightPos[1] = _rLightPos[2] = 10;
}

Head::~Head()
{
    if (_managed)
        Destroy();
}

void Head::Initialize(CGcontext context)
{
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    _programsPath = media.get_path("programs/gpgpu_disease/disease.cg");
    if (_programsPath.empty())
    {
        printf("Unable to locate Cg programs, exiting...\n");
        quitapp(1);
    }

    // Load the model data
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

    glGenTextures(1, &_iCubeTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _iCubeTextureID);
    load_png_cubemap("cube_face_%s.png", true);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _vertexProgram.InitializeVP(_cgContext, _programsPath + "/disease.cg", "vertexProgram");

    _blinnFragmentProgram.InitializeFP(_cgContext, _programsPath + "/disease.cg", "blinnSkin");
    _blinnFragmentProgram.SetTextureParameter("normalTexture", _iNormalTextureID);
    _blinnFragmentProgram.SetTextureParameter("diffuseTexture", _iDiffuseTextureID);
    _blinnFragmentProgram.SetTextureParameter("glossyTexture", _iGlossyTextureID);

    _crawlFragmentProgram.InitializeFP(_cgContext, _programsPath + "/disease.cg", "crawlSkin");
    _crawlFragmentProgram.SetTextureParameter("normalTexture", _iNormalTextureID);
    _crawlFragmentProgram.SetTextureParameter("diffuseTexture", _iDiffuseTextureID);
    _crawlFragmentProgram.SetTextureParameter("glossyTexture", _iGlossyTextureID);
   
    _diseaseFragmentProgram.InitializeFP(_cgContext, _programsPath + "/disease.cg", "diseaseSkin");
    _diseaseFragmentProgram.SetTextureParameter("normalTexture", _iNormalTextureID);
    _diseaseFragmentProgram.SetTextureParameter("diffuseTexture", _iDiffuseTextureID);
    _diseaseFragmentProgram.SetTextureParameter("glossyTexture", _iGlossyTextureID);

    _chromeFragmentProgram.InitializeFP(_cgContext, _programsPath + "/disease.cg", "chromeSkin");
    _chromeFragmentProgram.SetTextureParameter("normalTexture", _iNormalTextureID);
    _chromeFragmentProgram.SetTextureParameter("diffuseTexture", _iDiffuseTextureID);
    _chromeFragmentProgram.SetTextureParameter("glossyTexture", _iGlossyTextureID);
}

void Head::Destroy()
{
    glDeleteTextures(1, &_iDiffuseTextureID);
    glDeleteTextures(1, &_iGlossyTextureID);
    glDeleteTextures(1, &_iNormalTextureID);

    glDeleteBuffers(HEAD_NUM_VBOS, _iVBOs);
}

void Head::SetDiseaseTextures(unsigned int iDiseaseTexID, 
                              unsigned int iNormalTexID,
                              int iResX, int iResY)
{ 
    _iDiseaseTextureID = iDiseaseTexID; 
    _iDiseaseResX = iResX;
    _iDiseaseResY = iResY;
    _iDiseaseNormalTextureID = iNormalTexID;
}

void Head::SetDiseaseColor(float color[3])
{
    _diseaseFragmentProgram.SetFragmentParameter3fv("diseaseColor", color);
}

void Head::Display(DisplayMode eMode)
{
    glColor3f(1, 1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GenericCgGLFragmentProgram *pFP = 0;
    switch(eMode)
    {
    case HEAD_DISPLAY_BLINN:
        pFP = &_blinnFragmentProgram;
        break;
    case HEAD_DISPLAY_CRAWL:
        pFP = &_crawlFragmentProgram;
        _crawlFragmentProgram.SetTextureParameter("diseaseNormalTexture", 
                                                  _iDiseaseNormalTextureID);
        break;
    case HEAD_DISPLAY_DISEASE:
        pFP = &_diseaseFragmentProgram;
        _diseaseFragmentProgram.SetTextureParameter("diseaseNormalTexture", 
                                                    _iDiseaseNormalTextureID);
        _diseaseFragmentProgram.SetTextureParameter("diseaseTexture", 
                                                    _iDiseaseTextureID);
        break;
    case HEAD_DISPLAY_CHROME:
        pFP = &_chromeFragmentProgram;
        _chromeFragmentProgram.SetTextureParameter("diseaseNormalTexture", 
                                                   _iDiseaseNormalTextureID);
        _chromeFragmentProgram.SetTextureParameter("diseaseTexture", 
                                                   _iDiseaseTextureID);
        _chromeFragmentProgram.SetTextureParameter("envTexture", 
                                                   _iCubeTextureID);
        break;
    default:
        assert(pFP);
        break;
    }
    pFP->SetFragmentParameter3fv("eyeSpaceLightPosition", _rLightPos);
    pFP->SetState();

    _vertexProgram.SetStateMatrixParameter("modelViewProj", 
                                           CG_GL_MODELVIEW_PROJECTION_MATRIX, 
                                           CG_GL_MATRIX_IDENTITY);
    _vertexProgram.SetStateMatrixParameter("modelView", 
                                           CG_GL_MODELVIEW_MATRIX, 
                                           CG_GL_MATRIX_IDENTITY);
    _vertexProgram.SetVertexParameter1f("bumpScale", _rBumpScale);
    _vertexProgram.SetVertexParameter1f("diseaseBumpScale", _rDiseaseBumpScale);
    _vertexProgram.SetVertexParameter1f("diseaseMapScale", _rDiseaseMapScale);
    _vertexProgram.SetState();

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_POSITION]);
    _vertexProgram.EnableClientState("oPosition");
    _vertexProgram.SetParameterPointer("oPosition", 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TEXTURE_COORD_0]);
    _vertexProgram.EnableClientState("texcoord0");
    _vertexProgram.SetParameterPointer("texcoord0", 2, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_NORMAL]);
    _vertexProgram.EnableClientState("normal");
    _vertexProgram.SetParameterPointer("normal", 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_BINORMAL]);
    _vertexProgram.EnableClientState("binormal");
    _vertexProgram.SetParameterPointer("binormal", 3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, _iVBOs[HEAD_TANGENT]);
    _vertexProgram.EnableClientState("tangent");
    _vertexProgram.SetParameterPointer("tangent", 3, GL_FLOAT, 0, 0);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _iVBOs[HEAD_INDEX]);
    glDrawElements( GL_TRIANGLES, _iNumIndices, GL_UNSIGNED_INT, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    // Disable vertex program state
    _vertexProgram.DisableClientState("oPosition");
    _vertexProgram.DisableClientState("texcoord0");
    _vertexProgram.DisableClientState("normal");
    _vertexProgram.DisableClientState("binormal");
    _vertexProgram.DisableClientState("tangent");

    _vertexProgram.ResetState();
    pFP->ResetState();

    glPopMatrix();
}