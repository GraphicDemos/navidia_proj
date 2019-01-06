//-----------------------------------------------------------------------------
// File : ReactionDiffusion.cpp
//-----------------------------------------------------------------------------
// Copyright 2002 Mark J. Harris and
//            The University of North Carolina at Chapel Hill
//-----------------------------------------------------------------------------
// Permission to use, copy, modify, distribute and sell this software and its 
// documentation for any purpose is hereby granted without fee, provided that 
// the above copyright notice appear in all copies and that both that copyright 
// notice and this permission notice appear in supporting documentation. 
// Binaries may be compiled with this software without any royalties or 
// restrictions. 
//
// The author(s) and The University of North Carolina at Chapel Hill make no 
// representations about the suitability of this software for any purpose. 
// It is provided "as is" without express or implied warranty.
#include "ReactionDiffusion.h"
#include <shared/pbuffer.h>
#include <gl/glut.h>
#include <cg/cgGL.h>
#include <stdio.h>
#include <assert.h>

#include <shared/data_path.h>

//-----------------------------------------------------------------------------
// Function     	  : ReactionDiffusion::ReactionDiffusion
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::ReactionDiffusion()
* @brief Constructor
*/ 
ReactionDiffusion::ReactionDiffusion(unsigned int iResX, unsigned int iResY)
: _iResX(iResX),
_iResY(iResY),
_rPixelSizeX(0),
_rPixelSizeY(0),
_rK(0.052f),
_rF(0.012f),
_rDiffusionU(0.0004f),
_rDiffusionV(0.0002f),
_iConcentrationTextureID(0),
_pRT(0),
_bPaused(false),
_bSingleStep(false),
_cgContext(0)
{  
}


//-----------------------------------------------------------------------------
// Function     	  : ReactionDiffusion::~ReactionDiffusion
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::~ReactionDiffusion()
* @brief Destructor
*/ 
ReactionDiffusion::~ReactionDiffusion()
{ 
}


//-----------------------------------------------------------------------------
// Function     	  : ReactionDiffusion::Initialize
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::Initialize()
* @brief @todo <WRITE BRIEF ReactionDiffusion::Initialize DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::Initialize FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::Initialize(CGcontext cgContext)
{
    data_path media;
    media.path.push_back(".");
    media.path.push_back("../../../MEDIA");
    media.path.push_back("../../../../MEDIA");
    media.path.push_back("../../../../../../../MEDIA");

    string programsPath = media.get_path("programs/gpgpu_disease/disease.cg");
    if (programsPath.empty())
    {
        printf("Unable to locate Cg programs, exiting...\n");
        return;
    }

    _cgContext = cgContext;

    // Compute the pixel dimensions
    _rPixelSizeX = 1.0 / static_cast<float>(_iResX);
    _rPixelSizeY = 1.0 / static_cast<float>(_iResY);

    _pRT = new PBuffer("rgba float=32");
    if (!_pRT->Initialize(_iResX, _iResY, false, true))
    {
        fprintf(stderr, "ReactionDiffusion::Initialize(): "
                "Could not create offscreen buffer.");
        exit(-1);
    }

    _pRT->Activate();
    {
        glViewport(0, 0, _iResX, _iResY);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(0, 0, 0, 0);
    }
    _pRT->Deactivate();

    glGenTextures(1, &_iConcentrationTextureID);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, _iConcentrationTextureID);

    // Just allocate it, so we can do fast copies later with glCopyTexSubImage2D().
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA32_NV, _iResX, _iResY, 
                 0, GL_RGBA, GL_FLOAT, NULL);

    glGenTextures(1, &_iNormalMapID);
    glBindTexture(GL_TEXTURE_2D, _iNormalMapID);

    // Just allocate it, so we can do fast copies later with glCopyTexSubImage2D().
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _iResX, _iResY, 
                 0, GL_RGBA, GL_FLOAT, NULL);

    glGenTextures(1, &_iDisplayTextureID);
    glBindTexture(GL_TEXTURE_2D, _iDisplayTextureID);

    // Just allocate it, so we can do fast copies later with glCopyTexSubImage2D().
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _iResX * 2, _iResY * 2, 
                 0, GL_RGBA, GL_FLOAT, NULL);


    _updateRD.InitializeFP(_cgContext, programsPath + "/rd.cg", "rd");
    _updateRD.SetTextureParameter("concentration", _iConcentrationTextureID);
    _updateRD.SetTexCoordRect(0, 0, _iResX, _iResY);
    _updateRD.SetStreamRect(0, 0, 1, 1);

    _genNormalMap.InitializeFP(_cgContext, programsPath + "/rd.cg", "genNormalMap");
    _genNormalMap.SetTextureParameter("height", _iConcentrationTextureID);
    _genNormalMap.SetTexCoordRect(0, 0, _iResX, _iResY);
    _genNormalMap.SetStreamRect(0, 0, 1, 1);

    _display.InitializeFP(_cgContext, programsPath + "/rd.cg", "textureRECT");
    _display.SetTextureParameter("tex", _iConcentrationTextureID);
    _display.SetTexCoordRect(0, 0, _iResX, _iResY);
    _display.SetStreamRect(0, 0, 1, 1);

    Reset();
    Reset();
}


//-----------------------------------------------------------------------------
// Function     	  : ReactionDiffusion::Update
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::Update()
* @brief @todo <WRITE BRIEF ReactionDiffusion::Update DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::Update FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::Update()
{
    if (_bPaused && !_bSingleStep)
        return;
    else if (_bSingleStep)
    {
        _bSingleStep = false;
    }

    _pRT->Activate();

    _updateRD.SetFragmentParameter4f("windowDims", _iResX, _iResY, 
        -_rK - _rF + (1 - 655.36f * _rDiffusionV), 0);
    _updateRD.SetFragmentParameter4f("rdParams", 655.36f * _rDiffusionU,
        655.36f * _rDiffusionV, _rK, _rF);

    _updateRD.Compute();
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, _iConcentrationTextureID);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, _iResX, _iResY);

    _pRT->Deactivate();
}


//-----------------------------------------------------------------------------
// Function     	  : ReactionDiffusion::Reset
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::Reset()
* @brief @todo <WRITE BRIEF ReactionDiffusion::Reset DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::Reset FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::Reset()
{
    unsigned int i, j;

    float *pFloatData = new float[4 * _iResX * _iResY];
    unsigned int k = 0;
    for (i = 0; i < _iResX * _iResY; ++i)
    {
        pFloatData[k++] = 1.0;
        pFloatData[k++] = 0.0;
        pFloatData[k++] = 0.0;
        pFloatData[k++] = 0.0;
    }

    for (i = (0.2)*_iResY; i < (0.4)*_iResY; ++i)
    {
        for (j = (0.2)*_iResX; j < (0.4)*_iResX; ++j)
        {
            pFloatData[4 * (i * _iResX + j)    ] = .5;
            pFloatData[4 * (i * _iResX + j) + 1] = .25;
            pFloatData[4 * (i * _iResX + j) + 2] = 0;
            pFloatData[4 * (i * _iResX + j) + 3] = 0;
        }
    }

    // Now perturb the entire grid. Bound the values by [0,1]
    for (k = 0; k < _iResX * _iResY * 4; ++k)
    {
        if ( pFloatData[k] < 1.0 )
        {
            float rRand = .02*rand() / RAND_MAX - .01;
            pFloatData[k] += rRand * pFloatData[k];
        }
    }
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, _iConcentrationTextureID);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, _iResX, _iResY, 
                    GL_RGBA, GL_FLOAT, pFloatData);
        
    delete [] pFloatData;
}


//-----------------------------------------------------------------------------
// Function     	: ReactionDiffusion::Shutdown
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::Shutdown()
* @brief @todo <WRITE BRIEF ReactionDiffusion::Shutdown DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::Shutdown FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::Shutdown()
{
    glDeleteTextures(1, &_iConcentrationTextureID);
    glDeleteTextures(1, &_iNormalMapID);

    if (_pRT)
    {
        delete _pRT;
        _pRT = NULL;
    }
}


//-----------------------------------------------------------------------------
// Function     	: ReactionDiffusion::Display
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::Display()
* @brief @todo <WRITE BRIEF ReactionDiffusion::Display DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::Display FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::Display()
{
    glColor3f(1, 1, 1);
    _display.Compute();
}

//-----------------------------------------------------------------------------
// Function     	: ReactionDiffusion::GenerateDisplayMaps
// Description	    : 
//-----------------------------------------------------------------------------
/**
* @fn ReactionDiffusion::GenerateNormalMap()
* @brief @todo <WRITE BRIEF ReactionDiffusion::GenerateNormalMap DOCUMENTATION>
* 
* @todo <WRITE EXTENDED ReactionDiffusion::GenerateNormalMap FUNCTION DOCUMENTATION>
*/ 
void ReactionDiffusion::GenerateDisplayMaps()
{    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, _iResX, _iResY);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);   

    _genNormalMap.Compute();

    glBindTexture(GL_TEXTURE_2D, _iNormalMapID);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _iResX, _iResY);

    glViewport(0, 0, _iResX * 2, _iResY * 2);
    _display.Compute();

    glBindTexture(GL_TEXTURE_2D, _iDisplayTextureID);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _iResX * 2, _iResY * 2);

    glPopAttrib(); // GL_DEPTH_BUFFER_BIT
    glPopAttrib(); // GL_VIEWPORT_BIT
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
