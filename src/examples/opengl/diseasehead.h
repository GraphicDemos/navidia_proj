#include <Cg/cgGL.h>
#include <shared/geep/streamopCgGL.h>

using namespace geep;

class Head
{
public:
    enum DisplayMode
    {
        HEAD_DISPLAY_BLINN,
        HEAD_DISPLAY_CRAWL,
        HEAD_DISPLAY_DISEASE,
        HEAD_DISPLAY_CHROME,
        HEAD_DISPLAY_NUM_MODES
    };

    Head(bool managed = false);
    ~Head();

    void  Initialize(CGcontext context);
    void  Destroy();

    void  Display(DisplayMode eMode = HEAD_DISPLAY_BLINN);

    float GetBumpScale() const   { return _rBumpScale; }
    void  SetBumpScale(float rS) { _rBumpScale = rS;   }

    float GetDiseaseBumpScale() const    { return _rDiseaseBumpScale; }
    void  SetDiseaseBumpScale(float rS)  { _rDiseaseBumpScale = rS;   }

    float GetDiseaseMapScale() const     { return _rDiseaseMapScale;  }
    void  SetDiseaseMapScale(float rS)   { _rDiseaseMapScale = rS;    }

    void  SetDiseaseColor(float color[3]);

    void  SetDiseaseTextures(unsigned int iDiseaseTexID, 
                             unsigned int iNormalTexID,
                             int iResX, int iResY);

    void  SetLightPosition(float pos[3]) { memcpy(_rLightPos, pos, 3 * sizeof(float)); }

private:

    void  LoadCubeMap();

    float         _rBumpScale;
    float         _rDiseaseBumpScale;
    float         _rDiseaseMapScale;
    float         _rLightPos[3];

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
    GLuint        _iDiseaseTextureID;
    GLuint        _iDiseaseDisplayTextureID;
    GLuint        _iDiseaseNormalTextureID;
    GLuint        _iCubeTextureID;  

    int           _iDiseaseResX;
    int           _iDiseaseResY;

    bool          _managed;
    
    // CG context, programs, profiles, and parameters
    CGcontext     _cgContext;

    CGprofile     _vertexProfile;
    CGprofile     _fragmentProfile;

    GenericCgGLVertexProgram   _vertexProgram;
    GenericCgGLFragmentProgram _blinnFragmentProgram;
    GenericCgGLFragmentProgram _crawlFragmentProgram;
    GenericCgGLFragmentProgram _diseaseFragmentProgram;
    GenericCgGLFragmentProgram _chromeFragmentProgram;
   
    std::string   _programsPath;
};
