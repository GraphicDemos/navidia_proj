//------------------------------------------------------------------------------
// File : ReactionDiffusion.hpp
//------------------------------------------------------------------------------
// Copyright 2002 Mark J. Harris and
//						The University of North Carolina at Chapel Hill
//------------------------------------------------------------------------------
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
#ifndef __REACTION_DIFFUSION_HPP__
#define __REACTION_DIFFUSION_HPP__

#include <cg/cgGL.h>
#include <shared/geep/streamop.h>
#include <shared/geep/streamopGL.h>
#include <shared/geep/streamopCgGL.h>

using namespace geep;

class PBuffer;

class ReactionDiffusion
{
public:
  ReactionDiffusion(unsigned int iResX, unsigned int iResY);
  ~ReactionDiffusion();

  void Initialize(CGcontext cgContext);
  void Update();

  void Reset();
  void Shutdown();

  void  SetK(float rK)            { _rK = rK;   }
  void  SetF(float rF)            { _rF = rF;   }
  void  SetDiffusionU(float rDU)  { _rDiffusionU = rDU; }
  void  SetDiffusionV(float rDV)  { _rDiffusionV = rDV; }
  float GetK() const              { return _rK; }
  float GetF() const              { return _rF; }
  float GetDiffusionU() const     { return _rDiffusionU; }
  float GetDiffusionV() const     { return _rDiffusionV; }
  
  void         GetResolution(int &x, int &y) const { x = _iResX; y = _iResY; }
  unsigned int GetTexture() const { return _iConcentrationTextureID; }
  unsigned int GetDisplayTexture() const { return _iDisplayTextureID; }
  unsigned int GetNormalMap() const { return _iNormalMapID; }

  void TogglePause()              { _bPaused = !_bPaused; }
  void SingleStep()               { _bSingleStep = true;  }

  void Display();
  void DisplayWithBump(float lightPos[3]);

  void GenerateDisplayMaps(); // normal and 8-bit display texture
  
private:
  typedef StreamOp< NoopOutputPolicy, NoopStatePolicy,
                    NoopVertexPipePolicy, GenericCgGLFragmentProgram, 
                    SingleTextureGLComputePolicy, NoopUpdatePolicy >
  DiseaseStreamOp;

  DiseaseStreamOp _updateRD;
  DiseaseStreamOp _genNormalMap;
  DiseaseStreamOp _display;

  unsigned int  _iResX;
  unsigned int  _iResY;

  float         _rPixelSizeX;
  float         _rPixelSizeY;

  // constants for the ReactionDiffusion simulation.
  float         _rK;          //     = 0.0575f;
  float         _rF;          //     = 0.025f;
  float         _rDiffusionU; //     = 0.0004f;
  float         _rDiffusionV; //     = 0.0002f
  
  unsigned int  _iConcentrationTextureID;
  unsigned int  _iDisplayTextureID;
  unsigned int  _iNormalMapID;

  PBuffer *_pRT;
 
  bool          _bPaused;
  bool          _bSingleStep;

  CGcontext     _cgContext;
};

#endif //__REACTION_DIFFUSION_HPP__
