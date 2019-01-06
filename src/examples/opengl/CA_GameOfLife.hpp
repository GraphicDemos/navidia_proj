#ifndef __CA_GAMEOFLIFE_HPP__
#define __CA_GAMEOFLIFE_HPP__

// forward declaration so we don't have to include the file here
class PBuffer;

class CA_GameOfLife
{
public:
	CA_GameOfLife();
	~CA_GameOfLife();

    void Initialize(const char *pRulesFilename, const char *pInitialMapFilename);

    void Tick();
    void Display();
    
    void SingleStep()                               { _bSingleStep  = true;         }
    void EnableAnimation(bool bEnable)              { _bAnimate     = bEnable;      }
    void Reset()                                    { _bReset       = true;         }

    void EnableAlphaTest(bool bEnable)              { _bAlphaTest   = bEnable;      }
    void SetAlphaReference(float rAlphaRef)         { _rAlphaRef    = rAlphaRef;    }
    
    unsigned int GetOutputTextureID()               { return _pDynamicTextureIDs[CA_TEXTURE_TARGET_MAP];    }
    unsigned int GetIntermediateTextureID()         { return _pDynamicTextureIDs[CA_TEXTURE_INTERMEDIATE];  }

    enum RenderMode
    {
        CA_FULLSCREEN_NEIGHBOR_CALC,
        CA_FULLSCREEN_FINALOUT,
        CA_TILED_THREE_WINDOWS,
        CA_DO_NOT_RENDER
    };

    void SetRenderMode(RenderMode eMode)            { _eRenderMode  = eMode;        }
    void EnableWireframe(bool bEnable)              { _bWireframe   = bEnable;      }
    void EnableBorderWrapping(bool bEnable)         { _bWrap        = bEnable;      }

private:

    enum StaticTextureNames
    {
        CA_TEXTURE_RULES,
        CA_TEXTURE_INITIAL_MAP,
        CA_NUM_STATIC_TEXTURES
    };

    enum DynamicTextureNames
    {
        CA_TEXTURE_INTERMEDIATE,
        CA_TEXTURE_TARGET_MAP,
        CA_NUM_DYNAMIC_TEXTURES
    };
    
    enum ListNames
    {
        CA_REGCOMBINER_EQ_WEIGHT_COMBINE,
        CA_REGCOMBINER_TEXELS_TO_BLUE_WITH_BIAS,
        CA_TEXSHADER_DEPENDENT_GB,
        CA_DRAW_SCREEN_QUAD,
        CA_NUM_LISTS
    };

    unsigned int    _pRulesDimensions[2];       // the dimensions of the rules map.
    unsigned int    _pInitialMapDimensions[2];  // the dimensions of the initial map.
    
    GLuint          _pStaticTextureIDs[CA_NUM_STATIC_TEXTURES];     
    GLuint          _pDynamicTextureIDs[CA_NUM_DYNAMIC_TEXTURES];

    GLuint          _iCurrentSourceID;          // the current "input" texture to an update step.
    
    GLuint          _pDisplayListIDs[CA_NUM_LISTS];
    
    GLuint          _iVertexProgramID;          // one vertex shader is used to choose the texcoord offset

    unsigned int    _iFlipState;                // used to flip target texture configurations.

    bool            _bWrap;                     // CA can either wrap its borders, or clamp (clamp by default)  
    bool            _bReset;                    // are we resetting this frame? (user hit reset).
    bool            _bSingleStep;               // animation step on keypress.
    bool            _bAnimate;                  // continuous animation.
    bool            _bSlow;                     // run slow.
    bool            _bWireframe;                // render in wireframe mode
    bool            _bOccasionalExcitation;     // used to keep life exciting
    bool            _bAlphaTest;                // get big performance gains using alpha test.
    
    float           _rAlphaRef;                 // value to use for reference in alpha testing.

    unsigned int    _iSlowDelay;                // amount to delay when running slow.
    unsigned int    _iSkipInterval;             // frames to skip simulation.
    unsigned int    _iExcitationInterval;       // frames between excitations

    PBuffer         *_pPixelBuffer;             // pbuffer for rendering.

    RenderMode      _eRenderMode; 

    // private methods
    void _DoSingleTimeStep3Pass();
    void _LoadRulesAndInitialMap(const char *pRulesFilename, const char *pInitialMapFilename);
    void _CreateAndWriteUVOffsets(unsigned int width, unsigned int height);

    enum ConstantMemoryLocations
    {
        CV_WORLDVIEWPROJ_0 = 0,
        CV_WORLDVIEWPROJ_1 = 1,
        CV_WORLDVIEWPROJ_2 = 2,
        CV_WORLDVIEWPROJ_3 = 3,

        CV_UV_OFFSET_TO_USE = 4,

        CV_UV_T0_NO_OFFSET   = 8,
        CV_UV_T0_TYPE1       = 9,
        CV_UV_T0_TYPE2      = 10,
        CV_UV_T0_TYPE3      = 11,
        CV_UV_T0_TYPE4      = 12,

        CV_UV_T1_NO_OFFSET  = 13,
        CV_UV_T1_TYPE1      = 14,
        CV_UV_T1_TYPE2      = 15,
        CV_UV_T1_TYPE3      = 16,
        CV_UV_T1_TYPE4      = 17,

        CV_UV_T2_NO_OFFSET  = 18,
        CV_UV_T2_TYPE1      = 19,
        CV_UV_T2_TYPE2      = 20,
        CV_UV_T2_TYPE3      = 21,
        CV_UV_T2_TYPE4      = 22,

        CV_UV_T3_NO_OFFSET  = 23,
        CV_UV_T3_TYPE1      = 24,
        CV_UV_T3_TYPE2      = 25,
        CV_UV_T3_TYPE3      = 26,
        CV_UV_T3_TYPE4      = 27,
    };
    
};


#endif //__CA_GAMEOFLIFE_HPP__
