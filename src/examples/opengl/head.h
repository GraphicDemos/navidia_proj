#include <Cg/cgGL.h>

class Head
{
public:
  enum DisplayMode
  {
    HEAD_DISPLAY_STD,
    HEAD_DISPLAY_HILO,
    HEAD_DISPLAY_NUM_MODES
  };

  Head(bool managed = false);
  ~Head();

  bool  Initialize(CGcontext context);
  void  Destroy();

  void  ResetFragmentProgram(bool bV, bool bL, bool bH, bool bN, bool bHilo, bool bOpt);

  void  Display(DisplayMode eMode = HEAD_DISPLAY_STD);

  float GetBumpScale() const  { return _rBumpScale; }
  void  SetBumpScale(float rS) { _rBumpScale = rS;   }

  void  SetLightPosition(float pos[3]) { memcpy(_rLightPos, pos, 3 * sizeof(float)); }

  void  ToggleNormalizationMipMap() { _bMipMap = !_bMipMap; } 
  void  SetMaxMipLevel(int iMaxLevel) { _iMaxMipLevel = iMaxLevel; } 

  void  ToggleHalfPrecision() { _bHalfPrecision = ! _bHalfPrecision; }

  void  PrintCurrentFragmentProgram(DisplayMode eMode);

private:

  float         _rBumpScale;
  float         _rLightPos[3];
  bool          _bMipMap;
  int           _iMaxMipLevel;
  bool          _bHalfPrecision;
  
  // Model Geometry Data
  unsigned int  _iNumVertices;
  unsigned int  _iNumIndices;

  enum VBOIds
  {
      HEAD_INDEX,
      HEAD_POSITION,
      HEAD_NORMAL,
      HEAD_TANGENT,
      HEAD_BINORMAL,
      HEAD_TEXTURE_COORD_0,
      HEAD_NUM_VBOS
  };

  GLuint        _iVBOs[HEAD_NUM_VBOS];

  // Model Texture Data
  GLuint        _iDiffuseTextureID; 
  GLuint        _iGlossyTextureID;
  GLuint        _iNormalTextureID;
  GLuint        _iNormCubeMapTextureID;
  GLuint        _iHiLoNormCubeMapTextureID[2];

  bool          _managed;
  // CG context, programs, profiles, and parameters
  CGcontext     _cgContext;

  CGprofile     _vertexProfile;
  CGprofile     _fragmentProfile;
  
  CGprogram     _vertexProgram;
  CGprogram     _normFragmentProgram;
   
  CGparameter   _stdNorm;
  CGparameter   _cubeNorm;

  CGparameter   _bumpScaleParam;
  
  CGparameter   _positionParam;
  CGparameter   _texcoordParam;
  CGparameter   _tangentParam;
  CGparameter   _binormalParam;
  CGparameter   _normalParam;

  CGparameter   _normDiffuseMapParam;
  CGparameter   _normGlossMapParam;
  CGparameter   _normNormalMapParam;
  CGparameter   _normNormCubeMapParam;
  CGparameter   _normEyeSpaceLightPosParam;

  CGparameter   _normHiLoNormCubeMapParamXY;
  CGparameter   _normHiLoNormCubeMapParamZ;
  
  std::string   _programsPath;
};
