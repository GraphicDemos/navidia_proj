///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains code to import NVB files.
 *  \file       NVBBreaker.cpp
 *  \author     Pierre Terdiman
 *  \date       April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "NVBBreaker.h"
#include "NVBFile.h"
#include "NVBBuffer.h"

#ifndef NV_BAD_IDX
#define NV_BAD_IDX      0xFFFFFFFF
#endif

#define NVB_CHECKALLOC(x)   { if(!x) { NVBImportError("Out of memory.", NVB_ERROR_OUTOFMEMORY); return false;} }
#define NVB_CHECKVERSION(v)                                                                                                                     \
        {       if(Version>v)   { NVBImportError("Obsolete code! Get the latest NVB reader.", NVB_ERROR_UNKNOWNVERSION);    return false;   }   \
        else    if(Version<v)   NVBImportError("Found obsolete chunk. Please resave the file.", NVB_ERROR_OBSOLETEFILE);                    }

static bool gHasPivot = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBComponents::NVBComponents()
{
    mNbMeshes       = 0;
    mNbDerived      = 0;
    mNbCameras      = 0;
    mNbLights       = 0;
    mNbShapes       = 0;
    mNbHelpers      = 0;
    mNbControllers  = 0;
    mNbMaterials    = 0;
    mNbTextures     = 0;
    mNbUnknowns     = 0;
    mNbInvalidNodes = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBComponents::~NVBComponents()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBSceneInfo::NVBSceneInfo() : mFirstFrame(0), mLastFrame(0), mFrameRate(0), mDeltaTime(0)
{
    mBackColor      .Zero();
    mAmbientColor   .Zero();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBSceneInfo::~NVBSceneInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBBaseInfo::NVBBaseInfo() : mID(INVALID_ID), mParentID(INVALID_ID), mLinkID(INVALID_ID)
{
    mPrs.Reset();
    mWireColor      = 0x7fffffff;
    mLocalPRS       = false;
    mD3DCompliant   = false;
    mGroup          = false;
    mBaseFlags      = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBBaseInfo::~NVBBaseInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBCameraInfo::NVBCameraInfo()
{
    mType           = CTYPE_TARGET;
    mOrthoCam       = false;
    mFOV            = 0.0f;
    mFOVType        = FOV_HORIZONTAL;
    mNearClip       = 0.0f;
    mFarClip        = 0.0f;
    mTDist          = 0.0f;
    mHLineDisplay   = 0;
    mEnvNearRange   = 0.0f;
    mEnvFarRange    = 0.0f;
    mEnvDisplay     = 0;
    mManualClip     = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBCameraInfo::~NVBCameraInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBLightInfo::NVBLightInfo()
{
    // ...
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBLightInfo::~NVBLightInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBMaterialInfo::NVBMaterialInfo()
{
    mID                     = INVALID_ID;

    mAmbientMapID           = INVALID_ID;
    mDiffuseMapID           = INVALID_ID;
    mSpecularMapID          = INVALID_ID;
    mShininessMapID         = INVALID_ID;
    mShiningStrengthMapID   = INVALID_ID;
    mSelfIllumMapID         = INVALID_ID;
    mOpacityMapID           = INVALID_ID;
    mFilterMapID            = INVALID_ID;
    mBumpMapID              = INVALID_ID;
    mReflectionMapID        = INVALID_ID;
    mRefractionMapID        = INVALID_ID;
    mDisplacementMapID      = INVALID_ID;

    mAmbientCoeff           = 1.0f;
    mDiffuseCoeff           = 1.0f;
    mSpecularCoeff          = 1.0f;
    mShininessCoeff         = 1.0f;
    mShiningStrengthCoeff   = 1.0f;
    mSelfIllumCoeff         = 1.0f;
    mOpacityCoeff           = 1.0f;
    mFilterCoeff            = 1.0f;
    mBumpCoeff              = 1.0f;
    mReflectionCoeff        = 1.0f;
    mRefractionCoeff        = 1.0f;
    mDisplacementCoeff      = 1.0f;

    mMtlAmbientColor        .Zero();
    mMtlDiffuseColor        .Zero();
    mMtlSpecularColor       .Zero();
    mMtlFilterColor         .Zero();

    mShading                = SHADING_FORCE_DWORD;
    mSoften                 = false;
    mFaceMap                = false;
    mTwoSided               = false;
    mWire                   = false;
    mWireUnits              = false;
    mFalloffOut             = false;
    mTransparencyType       = TRANSPA_FORCE_DWORD;

    mShininess              = 0.0f;
    mShiningStrength        = 0.0f;
    mSelfIllum              = 0.0f;
    mOpacity                = 1.0f;
    mOpaFalloff             = 0.0f;
    mWireSize               = 0.0f;
    mIOR                    = 0.0f;

    mBounce                 = 0.0f;
    mStaticFriction         = 0.0f;
    mSlidingFriction        = 0.0f;

    mMatFlags               = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBMaterialInfo::~NVBMaterialInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBTextureInfo::NVBTextureInfo()
{
    mID                 = INVALID_ID;
    mWidth              = 0;
    mHeight             = 0;
    mBitmap             = null;
    mHasAlpha           = false;
    mIsBitmapIncluded   = false;
    mChannel            = NV_BAD_IDX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBTextureInfo::~NVBTextureInfo()
{
    DELETEARRAY(mBitmap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBNativeMeshInfo::NVBNativeMeshInfo()
{
    mNbTMaps        = 0;
    mNbTMapVerts    = null;
    mTMapFaces      = null;
    mTMaps          = null;
    
    mNbFaces        = 0;
    mNbVerts        = 0;
    mNbTVerts       = 0;
    mNbCVerts       = 0;
    mFlags          = 0;
    mFaces          = null;
    mTFaces         = null;
    mCFaces         = null;
    mFaceProperties = null;
    mVerts          = null;
    mTVerts         = null;
    mCVerts         = null;
    mParity         = false;

    mBonesNb        = null;
    mBonesID        = null;
    mBonesLocalID   = null;
    mOffsetVectors  = null;
    mWeights        = null;
    mSkeleton       = null;
    mNbBones        = 0;

    mNbHullVerts    = 0;
    mNbHullFaces    = 0;
    mHullVerts      = null;
    mHullFaces      = null;

    mBSCenter.Zero();
    mBSRadius       = 0.0f;

    mCOM.Zero();
    mMass           = 0.0f;
    ZeroMemory(mInertiaTensor, 9*sizeof(float));
    ZeroMemory(mCOMInertiaTensor, 9*sizeof(float));
    mMeshFlags      = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBNativeMeshInfo::~NVBNativeMeshInfo()
{
    for (unsigned int i = 0 ;i < mNbTMaps; ++i)
    {
        delete [] mTMapFaces[i];
        delete [] mTMaps[i];
    }

    if (mNbTMaps)
    {
        delete [] mTMapFaces;
        delete [] mNbTMapVerts;
        delete [] mTMaps;
        mNbTMaps = 0;
    }

    DELETEARRAY(mHullFaces);
    DELETEARRAY(mHullVerts);

    DELETEARRAY(mSkeleton);
    DELETEARRAY(mWeights);
    DELETEARRAY(mOffsetVectors);
    DELETEARRAY(mBonesID);
    DELETEARRAY(mBonesLocalID);
    DELETEARRAY(mBonesNb);

    DELETEARRAY(mFaces);
    DELETEARRAY(mTFaces);
    DELETEARRAY(mCFaces);
    DELETEARRAY(mFaceProperties);
    DELETEARRAY(mVerts);
    DELETEARRAY(mTVerts);
    DELETEARRAY(mCVerts);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBMeshInfo::NVBMeshInfo()
{
    mIsCollapsed    = false;
    mIsSkeleton     = false;
    mIsInstance     = false;
    mIsTarget       = false;
    mIsConvertible  = false;
    mIsSkin         = false;
    mCastShadows    = false;

    mCharID         = INVALID_ID;
    mCSID           = INVALID_ID;

    mCleanMesh      = null;

    mNbColors       = 0;
    mColors         = null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBMeshInfo::~NVBMeshInfo()
{
    DELETESINGLE(mCleanMesh);
    DELETEARRAY(mColors);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBShapeInfo::NVBShapeInfo() : mNbLines(0), mNbVerts(null), mClosed(null), mVerts(null), mTotalNbVerts(0), mMatID(INVALID_ID)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBShapeInfo::~NVBShapeInfo()
{
    DELETEARRAY(mVerts);
    DELETEARRAY(mClosed);
    DELETEARRAY(mNbVerts);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBHelperInfo::NVBHelperInfo() : mIsGroupHead(false)
{
    mHelperType     = HTYPE_DUMMY;
    mIsGroupHead    = false;
    mLength         = 0.0f;
    mWidth          = 0.0f;
    mHeight         = 0.0f;
    mRadius         = 0.0f;
    mHemi           = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBHelperInfo::~NVBHelperInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBControllerInfo::NVBControllerInfo()
{
    mObjectID       = INVALID_ID;
    mOwnerID        = INVALID_ID;
    mOwnerType      = NVB_OBJ_UNDEFINED;

    mCtrlType       = NVB_CTRL_NONE;
    mCtrlMode       = NVB_CTRL_SAMPLES;

    mNbSamples      = 0;
    mSamplingRate   = 0;
    mSamples        = null;
    mNbVertices     = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBControllerInfo::~NVBControllerInfo()
{
    if (mSamples)
    {
        delete [] (float*)mSamples;
        mSamples = null;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBBreaker::NVBBreaker() : mFileType(NVB_FILE_FORCE_DWORD), mImportArray(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBBreaker::~NVBBreaker()
{
    ReleaseRam();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to free used memory.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NVBBreaker::ReleaseRam()
{
    DELETESINGLE(mImportArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import a NVB scene.
 *  \param      filename        [in] the scene's filename
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::Import(const String& filename)
{
    // Find the file somewhere in registered paths
    String FoundFile;

    // Check the file exists
    if(!FindFile(filename, &FoundFile))
    {
        NVBImportError("File not found.", NVB_ERROR_FILENOTFOUND);
        return false;
    }

    // Release possibly already existing array
    DELETESINGLE(mImportArray);

    // Create a new array
    mImportArray = new CustomArray((const char *)FoundFile);
    NVB_CHECKALLOC(mImportArray);

    // Parse the array
    return Import();
}

int NVBBreaker::GUIDtoIndex(NVB_GUID *guidID)
{
    if (!memcmp(guidID, &guidMain, sizeof(NVB_GUID)))
    {
        return NVB_MAIN_CHUNK;
    }

    if (!memcmp(guidID, &guidMesh, sizeof(NVB_GUID)))
    {
        return NVB_MESH_CHUNK;
    }

    if (!memcmp(guidID, &guidCams, sizeof(NVB_GUID)))
    {
        return NVB_CAMS_CHUNK;
    }
    
    if (!memcmp(guidID, &guidLights, sizeof(NVB_GUID)))
    {
        return NVB_LIGHTS_CHUNK;
    }
    
    if (!memcmp(guidID, &guidShapes, sizeof(NVB_GUID)))
    {
        return NVB_SHAPES_CHUNK;
    }

    if (!memcmp(guidID, &guidHelpers, sizeof(NVB_GUID)))
    {
        return NVB_HELPERS_CHUNK;
    }
    
    if (!memcmp(guidID, &guidTextures, sizeof(NVB_GUID)))
    {
        return NVB_TEXTURES_CHUNK;
    }
    
    if (!memcmp(guidID, &guidMaterials, sizeof(NVB_GUID)))
    {
        return NVB_MATERIALS_CHUNK;
    }
    
    if (!memcmp(guidID, &guidControllers, sizeof(NVB_GUID)))
    {
        return NVB_CONTROLLERS_CHUNK;
    }
    
    if (!memcmp(guidID, &guidMovers, sizeof(NVB_GUID)))
    {
        return NVB_MOVERS_CHUNK;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import a NVB scene.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void swap_endian(unsigned short *val)
    {
#ifdef MACOS
        //unsigned short *ival = (unsigned short *)val;
  
        *val =  ((*val >>  8) & 0x000000ff) |
                ((*val <<  8) & 0x0000ff00);
#endif
    }

    inline void swap_endian(unsigned long *val)
    {
#ifdef MACOS
        //unsigned short *ival = (unsigned short *)val;
  
        *val =  ((*val >> 24) & 0x000000ff) |
                ((*val >>  8) & 0x0000ff00) |
                ((*val <<  8) & 0x00ff0000) |
                ((*val << 24) & 0xff000000);
#endif
    }
    
bool NVBBreaker::Import()
{
    // Check format signature
    ubyte b1 = mImportArray->GetByte();
    ubyte b2 = mImportArray->GetByte();
    ubyte b3 = mImportArray->GetByte();

    // The three first bytes must be "NVB"
    if(b1!='N' || b2!='V' || b3!='B')
    {
        NVBImportError("Invalid NVB file.", NVB_ERROR_INVALIDFILE);
        return false;
    }

    // The fourth byte can be '!' for normal files and 'P' for packed files.
    ubyte b4 = mImportArray->GetByte();
    if(b4=='P')
    {
#ifdef SUPPORT_COMPRESSION  // uncomment and use ZLib / BZip2
        // This is a packed NVB file. NVB files are packed with ZLib or BZip2 only, so that anyone can read them back.
        udword Compression  = mImportArray->GetDword(); // Compression scheme
        udword OriginalSize = mImportArray->GetDword(); // Size of the forthcoming depacked buffer
        udword PackedSize   = mImportArray->GetDword(); // Size of the packed buffer

        // Get some bytes for the depacked buffer
        ubyte* Depacked = new ubyte[OriginalSize];
        NVB_CHECKALLOC(Depacked);

        if(Compression==NVB_COMPRESSION_ZLIB)
        {
            // Use ZLib to depack the file
            int ErrorCode = uncompress((Bytef*)Depacked, (uLongf*)&OriginalSize, (Bytef*)mImportArray->GetAddress(), PackedSize);
            if(ErrorCode!=Z_OK)
            {
                NVBImportError("Depacking failed.", NVB_ERROR_CANTDEPACK);
                return false;
            }
        }
        else if(Compression==NVB_COMPRESSION_BZIP2)
        {
            // Use BZip2 to depack the file
            int ErrorCode = BZ2_bzBuffToBuffDecompress((char*)Depacked, &OriginalSize, (char*)mImportArray->GetAddress(), PackedSize, 0, 0);
            if(ErrorCode!=BZ_OK)
            {
                NVBImportError("Depacking failed.", NVB_ERROR_CANTDEPACK);
                return false;
            }
        }
        else
        {
            NVBImportError("Depacking failed.", NVB_ERROR_CANTDEPACK);
            return false;
        }

        // Release packed buffer
        DELETESINGLE(mImportArray);

        // Create a new array filled with depacked data
        mImportArray = new CustomArray(OriginalSize, Depacked);
        NVB_CHECKALLOC(mImportArray);

        // Release depacked buffer
        DELETEARRAY(Depacked);

        // And wrap it up
        return Import();
#else
        ASSERT(!"Not compiled with compression support!\n");
#endif
    }
    else if(b4=='!')
    {
        // This is a valid NVB file. Get the type.
        mFileType = (NVBFileType)mImportArray->GetDword();
    }
    else
    {
        // Uh... unknown header... corrupted file ?
        NVBImportError("Invalid NVB file.", NVB_ERROR_INVALIDFILE);
        return false;
    }

    NVB_GUID *curGUID;
    int guidIndex;
    // Fill a scene structure
    NVBSceneInfo    SceneInfo;
    udword Size;
    udword Version;

    while ((curGUID = (NVB_GUID *)mImportArray->GetData(sizeof(NVB_GUID))) != NULL)
    {
        swap_endian(&curGUID->Data1);
        swap_endian(&curGUID->Data2);
        swap_endian(&curGUID->Data3);
        guidIndex = GUIDtoIndex(curGUID);
        switch (guidIndex)
        {
        case NVB_MAIN_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            // Get chunk version
            Version = mImportArray->GetDword();
            NVB_CHECKVERSION(CHUNK_MAIN_VER);

            // Ugly but....
            if (Version>=2)
                gHasPivot = true;
            else
                gHasPivot = false;

            // Time-related info
            SceneInfo.mFirstFrame       = mImportArray->GetDword();
            SceneInfo.mLastFrame        = mImportArray->GetDword();
            SceneInfo.mFrameRate        = mImportArray->GetDword();
            SceneInfo.mDeltaTime        = mImportArray->GetDword();
            // Background color
            SceneInfo.mBackColor.x      = mImportArray->GetFloat();
            SceneInfo.mBackColor.y      = mImportArray->GetFloat();
            SceneInfo.mBackColor.z      = mImportArray->GetFloat();
            // Global ambient color
            SceneInfo.mAmbientColor.x   = mImportArray->GetFloat();
            SceneInfo.mAmbientColor.y   = mImportArray->GetFloat();
            SceneInfo.mAmbientColor.z   = mImportArray->GetFloat();

            // Scene info
            if (Version>=2)
                SceneInfo.mSceneInfo.Set((const char*)mImportArray->GetString());

            // Get number of expected elements
            mNbMeshes       = SceneInfo.mNbMeshes       = mImportArray->GetDword();
            mNbDerived      = SceneInfo.mNbDerived      = mImportArray->GetDword();
            mNbCameras      = SceneInfo.mNbCameras      = mImportArray->GetDword();
            mNbLights       = SceneInfo.mNbLights       = mImportArray->GetDword();
            mNbShapes       = SceneInfo.mNbShapes       = mImportArray->GetDword();
            mNbHelpers      = SceneInfo.mNbHelpers      = mImportArray->GetDword();
            mNbControllers  = SceneInfo.mNbControllers  = mImportArray->GetDword();
            mNbMaterials    = SceneInfo.mNbMaterials    = mImportArray->GetDword();
            mNbTextures     = SceneInfo.mNbTextures     = mImportArray->GetDword();
            mNbUnknowns     = SceneInfo.mNbUnknowns     = mImportArray->GetDword();
            mNbInvalidNodes = SceneInfo.mNbInvalidNodes = mImportArray->GetDword();

            // Call the app
            NewScene(SceneInfo);
            break;
        case NVB_MOVERS_CHUNK:
            // Get size of chunk version
            Size    = mImportArray->GetDword();
            // Import everything
            ImportMotion(*mImportArray);
            break;
        case NVB_CAMS_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportCameras(*mImportArray))   return false;
            break;
        case NVB_LIGHTS_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportLights        (*mImportArray))    return false;
            break;
        case NVB_TEXTURES_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportTextures      (*mImportArray))    return false;
            break;
        case NVB_MATERIALS_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportMaterials     (*mImportArray))    return false;
            break;
        case NVB_MESH_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportMeshes        (*mImportArray))    return false;
            break;
        case NVB_SHAPES_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportShapes        (*mImportArray))    return false;
            break;
        case NVB_HELPERS_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportHelpers       (*mImportArray))    return false;
            break;
        case NVB_CONTROLLERS_CHUNK:
            // Get size of chunk version
            Size = mImportArray->GetDword();
            if(!ImportControllers   (*mImportArray))    return false;
            break;
        default:
            NVBLog(LOG_WARN, "Unknown GUID chunk, skipping\n");
            // Get size of chunk version
            Size = mImportArray->GetDword();
            // skip chunk
            if (mImportArray->GetData(Size) == NULL)
            {
                NVBImportError("Invalid Chunk found.", NVB_ERROR_INVALIDFILE);
                return false;
            }
        }
    }
        
    // Release array
    DELETESINGLE(mImportArray);

    EndImport();

    return true;
}

bool NVBBreaker::EndImport()
{
    // don't do anything by default...
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import a BIPED motion
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportMotion(CustomArray& importer)
{
    // Get version number back
    udword Version  = importer.GetDword();
    NVB_CHECKVERSION(CHUNK_MOVE_VER);

    // Log message
    NVBLog(LOG_MSG, "Importing BIPED motion...\n");

    // Fill a motion structure
    NVBMotionInfo Mot;

    Mot.mCharID     = importer.GetDword();                      // LinkID
    Mot.mNbBones    = importer.GetDword();                      // Nb bones
    Mot.mNbVBones   = importer.GetDword();                      // Nb virtual bones
    // Since version 2
    if(Version>=2)  Mot.mLocalPRS = importer.GetByte()!=0;      // Local/global PRS
    else            Mot.mLocalPRS = true;                       // Arbitrary
    // Since version 4
    if(Version>=4)  Mot.mD3DCompliant = importer.GetByte()!=0;  // D3D compatible
    else            Mot.mD3DCompliant = false;
    // Since version 3
    if(Version>=3)  Mot.mName.Set((const char*)importer.GetString());       // The motion's name
    else            Mot.mName.Set((const char*)"MotionName");
    if(Version>=3)  Mot.mType.Set((const char*)importer.GetString());       // The motion's type
    else            Mot.mType.Set((const char*)"MotionType");
    Mot.mData       = importer.GetAddress();                // Motion data

    // Call the app
    NewMotion(Mot);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import basic object information.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBaseInfo::Import(CustomArray& importer)
{
    // Database information
    mName.Set((const char*)importer.GetString());       // The object's name
    mID                 = importer.GetDword();          // The object's ID
    mParentID           = importer.GetDword();          // The parent's ID
    mLinkID             = importer.GetDword();          // The link ID (target nodes, master objects, etc)
    mGroup              = importer.GetByte()!=0;        // true if the object belongs to a group

    // PRS
    mPrs.mPos.x         = importer.GetFloat();          // Position x
    mPrs.mPos.y         = importer.GetFloat();          // Position y
    mPrs.mPos.z         = importer.GetFloat();          // Position z

    mPrs.mRot.p.x       = importer.GetFloat();          // Rotation x
    mPrs.mRot.p.y       = importer.GetFloat();          // Rotation y
    mPrs.mRot.p.z       = importer.GetFloat();          // Rotation z
    mPrs.mRot.w         = importer.GetFloat();          // Rotation w

    mPrs.mScale.x       = importer.GetFloat();          // Scale x
    mPrs.mScale.y       = importer.GetFloat();          // Scale y
    mPrs.mScale.z       = importer.GetFloat();          // Scale z

    // Rendering information
    mWireColor          = importer.GetDword();          // The wireframe color
    mLocalPRS           = importer.GetByte()!=0;        // true for local PRS
    mD3DCompliant       = importer.GetByte()!=0;        // true if converted to D3D frame

    // User-properties
    mUserProps.Set((const char*)importer.GetString());  // The user-defined properties

    // Get pivot
    if(gHasPivot)
    {
        mPivotPos.x     = importer.GetFloat();          // Position x
        mPivotPos.y     = importer.GetFloat();          // Position y
        mPivotPos.z     = importer.GetFloat();          // Position z

        mPivotRot.p.x   = importer.GetFloat();          // Rotation x
        mPivotRot.p.y   = importer.GetFloat();          // Rotation y
        mPivotRot.p.z   = importer.GetFloat();          // Rotation z
        mPivotRot.w     = importer.GetFloat();          // Rotation w
    }
    else
    {
        mPivotPos.Zero();
        mPivotRot.Identity();
    }

    mBaseFlags          = importer.GetDword();              // Attributes
    if (mBaseFlags&NODEDESC_ATTRIBUTES)
    {
        if (ImportAttributes(importer) == false)
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import object custom attributes information.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBaseInfo::ImportAttributes(CustomArray& importer)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the cameras.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportCameras(CustomArray& importer)
{
    // Is there any cameras to import?
    if(mNbCameras)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_CAMS_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d cameras...\n", mNbCameras);

        // Import all cameras
        for(udword n=0;n<mNbCameras;n++)
        {
            // Fill a camera structure
            NVBCameraInfo CurCam;

            // Base info
            CurCam.Import(importer);

            // Get camera information back
            CurCam.mOrthoCam        = importer.GetByte()!=0;    // Camera type: ortographic (true) or perspective (false)
            CurCam.mFOV             = importer.GetFloat();      // Field-Of-View (degrees) or Width for ortho cams
            CurCam.mNearClip        = importer.GetFloat();      // Near/hither clip
            CurCam.mFarClip         = importer.GetFloat();      // Far/yon clip
            CurCam.mTDist           = importer.GetFloat();      // Distance to target
            CurCam.mHLineDisplay    = importer.GetDword();      // Horizon Line Display
            CurCam.mEnvNearRange    = importer.GetFloat();      // Environment near range
            CurCam.mEnvFarRange     = importer.GetFloat();      // Environment far range
            CurCam.mEnvDisplay      = importer.GetDword();      // Environment display
            CurCam.mManualClip      = importer.GetDword();      // Manual clip

            if(Version>=2)
            {
                CurCam.mFOVType     = (FOVType)importer.GetDword();     // FOV type
                CurCam.mType        = (CType)importer.GetDword();       // Camera type
            }

            // Call the app
            NewCamera(CurCam);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the lights.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportLights(CustomArray& importer)
{
    // Is there any lights to import?
    if(mNbLights)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_LITE_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d lights...\n", mNbLights);

        for(udword n=0;n<mNbLights;n++)
        {
            // Fill a light structure
            NVBLightInfo CurLight;

            // Base info
            CurLight.Import(importer);

            // Get light information back
            CurLight.mLightType         = (LType)importer.GetDword();   // Light's type
            CurLight.mIsSpot            = importer.GetByte()!=0;        // Is the light a spotlight?
            CurLight.mIsDir             = importer.GetByte()!=0;        // Is the light a directional?
            CurLight.mColor.x           = importer.GetFloat();          // Light's color
            CurLight.mColor.y           = importer.GetFloat();          //
            CurLight.mColor.z           = importer.GetFloat();          //
            CurLight.mIntensity         = importer.GetFloat();          // Light's intensity
            CurLight.mContrast          = importer.GetFloat();          // Light's contrast
            CurLight.mDiffuseSoft       = importer.GetFloat();          // Light's diffuse soft
            CurLight.mLightUsed         = importer.GetByte()!=0;        // Is the light used?
            CurLight.mAffectDiffuse     = importer.GetByte()!=0;        // Does the light affect diffuse?
            CurLight.mAffectSpecular    = importer.GetByte()!=0;        // Does the light affect specular?
            CurLight.mUseAttenNear      = importer.GetByte()!=0;        //
            CurLight.mAttenNearDisplay  = importer.GetByte()!=0;        //
            CurLight.mUseAtten          = importer.GetByte()!=0;        // Is attenuation used?
            CurLight.mShowAtten         = importer.GetByte()!=0;        //
            CurLight.mNearAttenStart    = importer.GetFloat();          // Near atten start
            CurLight.mNearAttenEnd      = importer.GetFloat();          // Near atten end
            CurLight.mAttenStart        = importer.GetFloat();          // Atten start
            CurLight.mAttenEnd          = importer.GetFloat();          // Atten end (use that as a range for non-dir lights)
            CurLight.mDecayType         = importer.GetByte();           // Light's decay type
            CurLight.mHotSpot           = importer.GetFloat();          // Light's hotspot
            CurLight.mFallsize          = importer.GetFloat();          // Light's falloff
            CurLight.mAspect            = importer.GetFloat();          // Light's aspect
            CurLight.mSpotShape         = (SpotShp)importer.GetDword(); // Light's spot shape
            CurLight.mOvershoot         = importer.GetDword();          // Light's overshoot
            CurLight.mConeDisplay       = importer.GetByte()!=0;        //
            CurLight.mTDist             = importer.GetFloat();          // Distance to target
            CurLight.mShadowType        = importer.GetDword();          // Light's shadow type
            CurLight.mAbsMapBias        = importer.GetDword();          // Light's absolute map bias
            CurLight.mRayBias           = importer.GetFloat();          // Raytrace bias
            CurLight.mMapBias           = importer.GetFloat();          // Map bias
            CurLight.mMapRange          = importer.GetFloat();          // Map range
            CurLight.mMapSize           = importer.GetDword();          // Map size

            if(Version>=2)  CurLight.mCastShadows       = importer.GetByte()!=0;        // Cast shadows
            else            CurLight.mCastShadows       = false;

            if(Version>=3)
            {
                CurLight.mShadowDensity         = importer.GetFloat();      // Shadow density
                CurLight.mShadowColor.x         = importer.GetFloat();      // Shadow color
                CurLight.mShadowColor.y         = importer.GetFloat();      // Shadow color
                CurLight.mShadowColor.z         = importer.GetFloat();      // Shadow color
                CurLight.mLightAffectsShadow    = importer.GetByte()!=0;    // Light affects shadow or not
            }
            else
            {
                CurLight.mShadowDensity         = 0.0f;
                CurLight.mShadowColor.x         = 0.0f;
                CurLight.mShadowColor.y         = 0.0f;
                CurLight.mShadowColor.z         = 0.0f;
                CurLight.mLightAffectsShadow    = false;
            }

            // Call the app
            NewLight(CurLight);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the cropping values & texture matrix.
 *  \param      cvalues     [out] a place to store the cropping values
 *  \param      tmtx        [out] a place to store the texture matrix
 *  \param      importer    [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportCroppingValues(TextureCrop& cvalues, TextureMatrix& tmtx, CustomArray& importer)
{
    // Get cropping values back
    cvalues.mOffsetU    = importer.GetFloat();
    cvalues.mOffsetV    = importer.GetFloat();
    cvalues.mScaleU     = importer.GetFloat();
    cvalues.mScaleV     = importer.GetFloat();

    // Get texture matrix back
    tmtx.m[0][0]        = importer.GetFloat();
    tmtx.m[0][1]        = importer.GetFloat();
    tmtx.m[0][2]        = importer.GetFloat();

    tmtx.m[1][0]        = importer.GetFloat();
    tmtx.m[1][1]        = importer.GetFloat();
    tmtx.m[1][2]        = importer.GetFloat();

    tmtx.m[2][0]        = importer.GetFloat();
    tmtx.m[2][1]        = importer.GetFloat();
    tmtx.m[2][2]        = importer.GetFloat();

    tmtx.m[3][0]        = importer.GetFloat();
    tmtx.m[3][1]        = importer.GetFloat();
    tmtx.m[3][2]        = importer.GetFloat();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the materials attributes.
 *  \param      CurMaterial     [in] the imported array.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportAttributes(nv_attribute & attr,CustomArray& importer)
{
    unsigned int loop = 1;
    unsigned int i;

    unsigned int type = importer.GetDword();
    unsigned int ori_type = type;
    unsigned int size = 0;

    if (type & nv_attribute::NV_ARRAY && type != nv_attribute::NV_UNASSIGNED)
    {
        size = importer.GetDword();
        loop = size;
        type &= ~nv_attribute::NV_ARRAY;
    }

    switch(type)
    {
        case nv_attribute::NV_FLOAT:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((float*)importer.GetData(loop * sizeof(float)),loop);
            }
            else
                attr = importer.GetFloat();
            break;
        case nv_attribute::NV_CHAR:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((char*)importer.GetData(loop * sizeof(char)),loop);
            }
            else
                attr = (char)importer.GetByte();
            break;
        case nv_attribute::NV_UNSIGNED_INT:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((unsigned int*)importer.GetData(loop * sizeof(unsigned int)),loop);
            }
            else
                attr = (unsigned int)importer.GetDword();
            break;
        case nv_attribute::NV_UNSIGNED_CHAR:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((unsigned char*)importer.GetData(loop * sizeof(unsigned char)),loop);
            }
            else
                attr = (unsigned char)importer.GetByte();
            break;
        case nv_attribute::NV_SHORT:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((short*)importer.GetData(loop * sizeof(short)),loop);
            }
            else
                attr = (short)importer.GetWord();
            break;
        case nv_attribute::NV_INT:
            if (loop > 1 || ori_type & nv_attribute::NV_ARRAY)
            {
                attr.array((int*)importer.GetData(loop * sizeof(int)),loop);
            }
            else
                attr = (int)importer.GetDword();
            break;
        default:
            break;
    }

    if (ori_type == nv_attribute::NV_ATTRIBUTE_ARRAY)
    {
        nv_attribute * attr_array = new nv_attribute[loop];

        for (i = 0; i < loop; i++)
            ImportAttributes(attr_array[i], importer);
        attr.array(attr_array,loop);
        delete [] attr_array;
    }

    unsigned int num_attr = importer.GetDword();

    for (i = 0; i < num_attr; ++i)
        ImportAttributes(attr[(const char *)importer.GetString()],importer);

    assert(attr.get_num_attributes() == num_attr);
    assert(attr.get_type() == ori_type);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the materials.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportMaterials(CustomArray& importer)
{
    // Is there any materials to import?
    if(mNbMaterials)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_MATL_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d materials...\n", mNbMaterials);

        // Import all materials
        for(udword n=0;n<mNbMaterials;n++)
        {
            // Fill a material structure
            NVBMaterialInfo CurMaterial;

            // Database information
            CurMaterial.mName.Set((const char*)importer.GetString());   // Material name
            CurMaterial.mID = importer.GetDword();                      // Material ID

            // Texture IDs
            CurMaterial.mAmbientMapID           = importer.GetDword();
            CurMaterial.mDiffuseMapID           = importer.GetDword();
            CurMaterial.mSpecularMapID          = importer.GetDword();
            CurMaterial.mShininessMapID         = importer.GetDword();
            CurMaterial.mShiningStrengthMapID   = importer.GetDword();
            CurMaterial.mSelfIllumMapID         = importer.GetDword();
            CurMaterial.mOpacityMapID           = importer.GetDword();
            CurMaterial.mFilterMapID            = importer.GetDword();
            CurMaterial.mBumpMapID              = importer.GetDword();
            CurMaterial.mReflectionMapID        = importer.GetDword();
            CurMaterial.mRefractionMapID        = importer.GetDword();
            CurMaterial.mDisplacementMapID      = importer.GetDword();

            // Amounts
            CurMaterial.mAmbientCoeff           = importer.GetFloat();
            CurMaterial.mDiffuseCoeff           = importer.GetFloat();
            CurMaterial.mSpecularCoeff          = importer.GetFloat();
            CurMaterial.mShininessCoeff         = importer.GetFloat();
            CurMaterial.mShiningStrengthCoeff   = importer.GetFloat();
            CurMaterial.mSelfIllumCoeff         = importer.GetFloat();
            CurMaterial.mOpacityCoeff           = importer.GetFloat();
            CurMaterial.mFilterCoeff            = importer.GetFloat();
            CurMaterial.mBumpCoeff              = importer.GetFloat();
            CurMaterial.mReflectionCoeff        = importer.GetFloat();
            CurMaterial.mRefractionCoeff        = importer.GetFloat();
            CurMaterial.mDisplacementCoeff      = importer.GetFloat();

            // Colors
            CurMaterial.mMtlAmbientColor.x      = importer.GetFloat();
            CurMaterial.mMtlAmbientColor.y      = importer.GetFloat();
            CurMaterial.mMtlAmbientColor.z      = importer.GetFloat();

            CurMaterial.mMtlDiffuseColor.x      = importer.GetFloat();
            CurMaterial.mMtlDiffuseColor.y      = importer.GetFloat();
            CurMaterial.mMtlDiffuseColor.z      = importer.GetFloat();

            CurMaterial.mMtlSpecularColor.x     = importer.GetFloat();
            CurMaterial.mMtlSpecularColor.y     = importer.GetFloat();
            CurMaterial.mMtlSpecularColor.z     = importer.GetFloat();

            CurMaterial.mMtlFilterColor.x       = importer.GetFloat();
            CurMaterial.mMtlFilterColor.y       = importer.GetFloat();
            CurMaterial.mMtlFilterColor.z       = importer.GetFloat();

            // Static properties
            CurMaterial.mShading                = (ShadingMode)importer.GetDword();
            CurMaterial.mSoften                 = importer.GetByte()!=0;
            CurMaterial.mFaceMap                = importer.GetByte()!=0;
            CurMaterial.mTwoSided               = importer.GetByte()!=0;
            CurMaterial.mWire                   = importer.GetByte()!=0;
            CurMaterial.mWireUnits              = importer.GetByte()!=0;
            CurMaterial.mFalloffOut             = importer.GetByte()!=0;
            CurMaterial.mTransparencyType       = (TranspaType)importer.GetDword();

            // Dynamic properties
            CurMaterial.mShininess              = importer.GetFloat();
            CurMaterial.mShiningStrength        = importer.GetFloat();
            CurMaterial.mSelfIllum              = importer.GetFloat();
            CurMaterial.mOpacity                = importer.GetFloat();
            CurMaterial.mOpaFalloff             = importer.GetFloat();
            CurMaterial.mWireSize               = importer.GetFloat();
            CurMaterial.mIOR                    = importer.GetFloat();

            CurMaterial.mBounce                 = importer.GetFloat();
            CurMaterial.mStaticFriction         = importer.GetFloat();
            CurMaterial.mSlidingFriction        = importer.GetFloat();

            // Cropping values & texture matrix
            if(Version>=2)  
            {
                ImportCroppingValues(CurMaterial.mCValues, CurMaterial.mTMtx, importer);
            }
            // Import custom attributes
            CurMaterial.mMatFlags               = importer.GetDword();    
            if (CurMaterial.mMatFlags&NVBMaterialInfo::MATDESC_ATTRIBUTES)
            {
                if (ImportAttributes(CurMaterial.mAttr,importer) == false)
                    return false;
            }
            // Call the app
            NewMaterial(CurMaterial);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the textures.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportTextures(CustomArray& importer)
{
    // Is there any textures to import?
    if(mNbTextures)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_TEXM_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d textures...\n", mNbTextures);

        // Import all textures
        for(udword n=0;n<mNbTextures;n++)
        {
            // Fill a texture structure
            NVBTextureInfo CurTexture;

            // Database information
            CurTexture.mName.Set((const char*)importer.GetString());    // Texture path
            CurTexture.mID = importer.GetDword();                       // Texture ID

            // Get bitmap data
            ubyte Code = 1; // Default for version <=1
            if(Version>1)   Code = importer.GetByte();
//              CurTexture.mIsBitmapIncluded = Version>1 ? (importer.GetByte()!=0) : true;
            CurTexture.mIsBitmapIncluded = Code!=0;

            if(Code)
            {
                // Get texture information back
                CurTexture.mWidth       = importer.GetDword();
                CurTexture.mHeight      = importer.GetDword();
                CurTexture.mHasAlpha    = importer.GetByte()!=0;

                // Get bytes for a RGBA texture
                CurTexture.mBitmap      = new ubyte[CurTexture.mWidth*CurTexture.mHeight*4];
                NVB_CHECKALLOC(CurTexture.mBitmap);

                if(Code==1)
                {
                    // => RGBA texture
                    for(udword i=0;i<CurTexture.mWidth*CurTexture.mHeight;i++)
                    {
                        CurTexture.mBitmap[i*4+0] = importer.GetByte(); // Red
                        CurTexture.mBitmap[i*4+1] = importer.GetByte(); // Green
                        CurTexture.mBitmap[i*4+2] = importer.GetByte(); // Blue
                        CurTexture.mBitmap[i*4+3] = CurTexture.mHasAlpha ? importer.GetByte() : PIXEL_OPAQUE;
                    }
                }
                else
                {
                    // => Quantized RGB texture
                    ubyte Palette[768];
                    udword i;
                    
                    for(i=0;i<768;i++)   Palette[i] = importer.GetByte();
                    //
                    for(i=0;i<CurTexture.mWidth*CurTexture.mHeight;i++)
                    {
                        ubyte ColorIndex = importer.GetByte();
                        CurTexture.mBitmap[i*4+0] = Palette[ColorIndex*3+0];
                        CurTexture.mBitmap[i*4+1] = Palette[ColorIndex*3+1];
                        CurTexture.mBitmap[i*4+2] = Palette[ColorIndex*3+2];
                        CurTexture.mBitmap[i*4+3] = PIXEL_OPAQUE;
                    }
                }
            }

            // Cropping values & texture matrix
            ImportCroppingValues(CurTexture.mCValues, CurTexture.mTMtx, importer);

            CurTexture.mChannel = importer.GetDword();

            // Call the app
            NewTexture(CurTexture);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the mesh attributes.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportMeshAttributes(NVBMeshInfo& curmesh, CustomArray& importer)
{
    curmesh.mMeshFlags = importer.GetDword();
    if (curmesh.mMeshFlags & NVBMeshInfo::MESHDESC_ATTRIBUTES)
    {
        // load custom attributes
        ImportAttributes(curmesh.mMeshAttr, importer);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the meshes.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportMeshes(CustomArray& importer)
{
    // Is there any meshes to import?
    udword Total = mNbMeshes + mNbDerived;
    if(Total)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_MESH_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d meshes...\n", Total);

        // Import all meshes
        for(udword n=0;n<Total;n++)
        {
            // Fill a mesh structure
            NVBMeshInfo CurMesh;

            // Base info
            CurMesh.Import(importer);

            // Get mesh information back
            CurMesh.mIsCollapsed    = importer.GetByte()!=0;    // true if the object has been collapsed
            CurMesh.mIsSkeleton     = importer.GetByte()!=0;    // true for BIPED parts
            CurMesh.mIsInstance     = importer.GetByte()!=0;    // true for instances
            CurMesh.mIsTarget       = importer.GetByte()!=0;    // true for target objects
            CurMesh.mIsConvertible  = importer.GetByte()!=0;    // true for valid objects
            CurMesh.mIsSkin         = importer.GetByte()!=0;    // true for PHYSIQUE skins
            if(Version>=4)
                CurMesh.mCastShadows= importer.GetByte()!=0;    // true if the mesh can cast its shadow

            // Get skin's character ID
            if(Version>=5 && CurMesh.mIsSkin)
            {
                CurMesh.mCharID     = importer.GetDword();      // the owner's character ID
            }

            // Get BIPED parts information if needed
            if(CurMesh.mIsSkeleton)
            {
                CurMesh.mCharID     = importer.GetDword();      // the owner's character ID
                CurMesh.mCSID       = importer.GetDword();      // the CSID
            }

            // Get data back for non-instance meshes
            if(!CurMesh.mIsInstance)
            {
                CurMesh.mNbFaces    = importer.GetDword();      // Number of faces
                CurMesh.mNbVerts    = importer.GetDword();      // Number of Vertices
                CurMesh.mNbTVerts   = importer.GetDword();      // Number of texture-Vertices
                CurMesh.mNbCVerts   = importer.GetDword();      // Number of vertex-colors
                CurMesh.mFlags      = importer.GetDword();      // Flags
                CurMesh.mParity     = importer.GetByte()!=0;    // Mesh parity
                CurMesh.mNbTMaps    = importer.GetDword();         // Number of surface parameterization

                // Get data for skins / non-skins
                if(!CurMesh.mIsSkin)    ImportVertices          (CurMesh, importer);
                else                    ImportSkinData          (CurMesh, importer, Version);

                // Native texture Vertices
                                        ImportTextureVertices   (CurMesh, importer);

                // Native vertex-colors
                                        ImportVertexColors      (CurMesh, importer);

                // Native faces
                                        ImportFaces             (CurMesh, importer);

                // Extra stuff
                                        ImportExtraStuff        (CurMesh, importer);

                // Consolidated mesh
                                        ImportConsolidated      (CurMesh, importer, Version);

                // Attributes
                                        ImportMeshAttributes    (CurMesh, importer);
            }



            // Lightmapper data
            ImportLightingData(CurMesh, importer);

            // Call the app
            NewMesh(CurMesh);

            // Free consolidation ram
            if(CurMesh.mCleanMesh)
            {
                MBMaterials& Mtl = CurMesh.mCleanMesh->Materials;
                DELETEARRAY(Mtl.MaterialInfo);
                MBGeometry& Geo = CurMesh.mCleanMesh->Geometry;
                DELETEARRAY(Geo.NormalInfo);
                DELETEARRAY(Geo.CVerts);
                DELETEARRAY(Geo.Normals);
                DELETEARRAY(Geo.TVerts);
                DELETEARRAY(Geo.TVertsRefs);
                DELETEARRAY(Geo.Verts);
                DELETEARRAY(Geo.VertsRefs);
                MBTopology& Topo = CurMesh.mCleanMesh->Topology;
                DELETEARRAY(Topo.Normals);
                DELETEARRAY(Topo.VRefs);
                DELETEARRAY(Topo.FacesInSubmesh);
                DELETEARRAY(Topo.SubmeshProperties);

                DELETESINGLE(CurMesh.mCleanMesh);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the Vertices.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportVertices(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Checkings
    if(!curmesh.mNbVerts)   return true;

    // Get some bytes for Vertices
    curmesh.mVerts = new Point[curmesh.mNbVerts];
    NVB_CHECKALLOC(curmesh.mVerts);

    // Get Vertices back
    if(curmesh.mFlags&NVB_QUANTIZEDVERTICES)
    {
        // Get dequantization coeffs
        float DequantCoeffX = importer.GetFloat();
        float DequantCoeffY = importer.GetFloat();
        float DequantCoeffZ = importer.GetFloat();
        // Get quantized Vertices
        for(udword i=0;i<curmesh.mNbVerts;i++)
        {
            sword x = importer.GetWord();
            sword y = importer.GetWord();
            sword z = importer.GetWord();
            // Dequantize
            curmesh.mVerts[i].x = float(x) * DequantCoeffX;
            curmesh.mVerts[i].y = float(y) * DequantCoeffY;
            curmesh.mVerts[i].z = float(z) * DequantCoeffZ;
        }
    }
    else
    {
        // Get Vertices
        for(udword i=0;i<curmesh.mNbVerts;i++)
        {
            float x = importer.GetFloat();
            float y = importer.GetFloat();
            float z = importer.GetFloat();
            curmesh.mVerts[i].x = x;
            curmesh.mVerts[i].y = y;
            curmesh.mVerts[i].z = z;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the skin data.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \param      version         [in] mesh chunk version
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportSkinData(NVBMeshInfo& curmesh, CustomArray& importer, udword version)
{
    // A skin can be a simple or a complex skin. (err, that's *my* wordlist...)
    // - simple: each vertex is linked to a single bone
    // - complex: each vertex is linked to many bones
    if(curmesh.mFlags & NVB_ONEBONEPERVERTEX)
    {
        // The skin has one bone/vertex. Hence:
        // - We have N Vertices and N bones
        // - N offset vectors
        // - N bones ID
        // - mBonesNb remains null
        // - mWeights remains null

        // Get offset vectors back
        curmesh.mOffsetVectors  = new Point[curmesh.mNbVerts];
        NVB_CHECKALLOC(curmesh.mOffsetVectors);
        udword i;
        
        for(i=0;i<curmesh.mNbVerts;i++)
        {
            curmesh.mOffsetVectors[i].x = importer.GetFloat();
            curmesh.mOffsetVectors[i].y = importer.GetFloat();
            curmesh.mOffsetVectors[i].z = importer.GetFloat();

            // Fix what was missing in Flexporter before v1.13
            if(curmesh.mD3DCompliant && version<6)  TSwap(curmesh.mOffsetVectors[i].y, curmesh.mOffsetVectors[i].z);
        }

        // Get bones ID back
        curmesh.mBonesID        = new udword[curmesh.mNbVerts];
        NVB_CHECKALLOC(curmesh.mBonesID);

        for(i=0;i<curmesh.mNbVerts;i++)
        {
            curmesh.mBonesID[i] = importer.GetDword();
        }
        // Get Local ID back
        curmesh.mBonesLocalID       = new udword[curmesh.mNbVerts];
        NVB_CHECKALLOC(curmesh.mBonesLocalID);

        for(i=0;i<curmesh.mNbVerts;i++)
        {
            curmesh.mBonesLocalID[i] = importer.GetDword();
        }
    }
    else
    {
        // The skin has many bones/vertex. Hence:
        // - We have N Vertices and M bones
        // - We have N numbers of bones, stored in mBonesNb
        // - M is the sum of all those number of bones
        // - We have M offset vectors
        // - We have M bones ID
        // - We have M weights stored in mWeights

        // Get number of bones / vertex, compute total number of bones
        curmesh.mBonesNb        = new udword[curmesh.mNbVerts];
        NVB_CHECKALLOC(curmesh.mBonesNb);

        udword Sum=0;
        udword i;
        
        for(i=0;i<curmesh.mNbVerts;i++)
        {
            curmesh.mBonesNb[i] = importer.GetDword();
            Sum+=curmesh.mBonesNb[i];
        }

        // Get bones ID back
        curmesh.mBonesID        = new udword[Sum];
        NVB_CHECKALLOC(curmesh.mBonesID);

        for(i=0;i<Sum;i++)
        {
            curmesh.mBonesID[i] = importer.GetDword();
        }

        // Get Local ID back
        curmesh.mBonesLocalID   = new udword[Sum];
        NVB_CHECKALLOC(curmesh.mBonesLocalID);

        for(i=0;i<Sum;i++)
        {
            curmesh.mBonesLocalID[i] = importer.GetDword();
        }

        // Get weights back
        curmesh.mWeights        = new float[Sum];
        NVB_CHECKALLOC(curmesh.mWeights);

        for(i=0;i<Sum;i++)
        {
            curmesh.mWeights[i] = importer.GetFloat();
        }

        // Get offset vectors back
        curmesh.mOffsetVectors  = new Point[Sum];
        NVB_CHECKALLOC(curmesh.mOffsetVectors);

        for(i=0;i<Sum;i++)
        {
            curmesh.mOffsetVectors[i].x = importer.GetFloat();
            curmesh.mOffsetVectors[i].y = importer.GetFloat();
            curmesh.mOffsetVectors[i].z = importer.GetFloat();

            // Fix what was missing in Flexporter before v1.13
            if(curmesh.mD3DCompliant && version<6)  TSwap(curmesh.mOffsetVectors[i].y, curmesh.mOffsetVectors[i].z);
        }
    }

    // Get skeletal information back. This just gives the skeleton structure in a simple way, so that you
    // can discard the actual BIPED parts and still use the skin (=derived object) alone.
    curmesh.mNbBones = importer.GetDword();

    curmesh.mSkeleton = new BasicBone[curmesh.mNbBones];
    NVB_CHECKALLOC(curmesh.mSkeleton);

    for(udword i=0;i<curmesh.mNbBones;i++)
    {
        curmesh.mSkeleton[i].CSID   = importer.GetDword();  // CSID
        curmesh.mSkeleton[i].pCSID  = importer.GetDword();  // parent's CSID
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the texture-Vertices.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportTextureVertices(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Checkings
    if(!curmesh.mNbTVerts)  return true;

    if(curmesh.mFlags & NVB_UVW)
    {
        // Get some bytes for texture Vertices
        curmesh.mTVerts = new Point[curmesh.mNbTVerts];
        NVB_CHECKALLOC(curmesh.mTVerts);

        // Get texture-Vertices back
        if(curmesh.mFlags&NVB_QUANTIZEDVERTICES)
        {
            // Get dequantization coeffs
            float DequantCoeffX = importer.GetFloat();
            float DequantCoeffY = importer.GetFloat();
            float DequantCoeffZ = (curmesh.mFlags & NVB_WDISCARDED) ? 0.0f : importer.GetFloat();

            // Get quantized Vertices
            for(udword i=0;i<curmesh.mNbTVerts;i++)
            {
                sword x = importer.GetWord();
                sword y = importer.GetWord();
                sword z = (curmesh.mFlags & NVB_WDISCARDED) ? 0 : importer.GetWord();
                // Dequantize
                curmesh.mTVerts[i].x = float(x) * DequantCoeffX;
                curmesh.mTVerts[i].y = float(y) * DequantCoeffY;
                curmesh.mTVerts[i].z = float(z) * DequantCoeffZ;
            }
        }
        else
        {
            // Get texture-Vertices
            for(udword i=0;i<curmesh.mNbTVerts;i++)
            {
                curmesh.mTVerts[i].x = importer.GetFloat();
                curmesh.mTVerts[i].y = importer.GetFloat();
                curmesh.mTVerts[i].z = (curmesh.mFlags & NVB_WDISCARDED) ? 0.0f : importer.GetFloat();
            }
        }

        if (curmesh.mNbTMaps)
        {
            curmesh.mNbTMapVerts = new udword[curmesh.mNbTMaps];
            curmesh.mTMaps = new Point*[curmesh.mNbTMaps];

            for( udword i=0;i < curmesh.mNbTMaps;i++)
            {
                curmesh.mNbTMapVerts[i] = importer.GetDword();
                if (curmesh.mNbTMapVerts[i])
                {
                    curmesh.mTMaps[i] = new Point[curmesh.mNbTMapVerts[i]];

                    if(curmesh.mFlags&NVB_QUANTIZEDVERTICES)
                    {
                        // Get dequantization coeffs
                        float DequantCoeffX = importer.GetFloat();
                        float DequantCoeffY = importer.GetFloat();
                        float DequantCoeffZ = (curmesh.mFlags & NVB_WDISCARDED) ? 0.0f : importer.GetFloat();
                        for ( udword j =0; j <curmesh.mNbTMapVerts[i]; ++j)
                        {
                            sword x = importer.GetWord();
                            sword y = importer.GetWord();
                            sword z = (curmesh.mFlags & NVB_WDISCARDED) ? 0 : importer.GetWord();
                            curmesh.mTMaps[i][j].x = float(x) * DequantCoeffX;
                            curmesh.mTMaps[i][j].y = float(y) * DequantCoeffY;
                            curmesh.mTMaps[i][j].z = float(z) * DequantCoeffZ;
                        }
                    }
                    else
                    {
                        for ( udword j =0; j <curmesh.mNbTMapVerts[i]; ++j)
                        {
                            curmesh.mTMaps[i][j].x = importer.GetFloat();
                            curmesh.mTMaps[i][j].y = importer.GetFloat();
                            curmesh.mTMaps[i][j].z = (curmesh.mFlags & NVB_WDISCARDED) ? 0.0f : importer.GetFloat();
                        }
                    }
                }
                else
                    curmesh.mTMaps[i] = null;
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the vertex-colors.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportVertexColors(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Checkings
    if(!curmesh.mNbCVerts)  return true;

    if(curmesh.mFlags & NVB_VERTEXCOLORS)
    {
        // Get some bytes for vertex-colors
        curmesh.mCVerts = new Point[curmesh.mNbCVerts];
        NVB_CHECKALLOC(curmesh.mCVerts);

        // Get vertex-colors back
        if(curmesh.mFlags&NVB_QUANTIZEDVERTICES)
        {
            // Get dequantization coeffs
            float DequantCoeffX = importer.GetFloat();
            float DequantCoeffY = importer.GetFloat();
            float DequantCoeffZ = importer.GetFloat();

            // Get quantized Vertices
            for(udword i=0;i<curmesh.mNbCVerts;i++)
            {
                sword x = importer.GetWord();
                sword y = importer.GetWord();
                sword z = importer.GetWord();
                // Dequantize
                curmesh.mCVerts[i].x = float(x) * DequantCoeffX;
                curmesh.mCVerts[i].y = float(y) * DequantCoeffY;
                curmesh.mCVerts[i].z = float(z) * DequantCoeffZ;
            }
        }
        else
        {
            // Get vertex-colors
            for(udword i=0;i<curmesh.mNbCVerts;i++)
            {
                curmesh.mCVerts[i].x = importer.GetFloat();
                curmesh.mCVerts[i].y = importer.GetFloat();
                curmesh.mCVerts[i].z = importer.GetFloat();
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the topologies.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportFaces(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Get some bytes for faces
    if(curmesh.mNbVerts)    { curmesh.mFaces = new udword[curmesh.mNbFaces*3];  NVB_CHECKALLOC(curmesh.mFaces); }
    if(curmesh.mNbTVerts)   { curmesh.mTFaces = new udword[curmesh.mNbFaces*3]; NVB_CHECKALLOC(curmesh.mTFaces); }
    if(curmesh.mNbCVerts)   { curmesh.mCFaces = new udword[curmesh.mNbFaces*3]; NVB_CHECKALLOC(curmesh.mCFaces); }

    // Get faces
    if(curmesh.mFaces)
        for(udword j=0;j<curmesh.mNbFaces*3;j++)
            curmesh.mFaces[j] = (curmesh.mFlags&NVB_WORDFACES) ? (udword)importer.GetWord() : importer.GetDword();

    // Get texture faces
    if(curmesh.mTFaces)
    {
        udword j;
        
        for(j=0;j<curmesh.mNbFaces*3;j++)
            curmesh.mTFaces[j] = (curmesh.mFlags&NVB_WORDFACES) ? (udword)importer.GetWord() : importer.GetDword();
        if (curmesh.mNbTMaps)
        {
            curmesh.mTMapFaces = new udword*[curmesh.mNbTMaps];
            for(j=0;j<curmesh.mNbTMaps;j++)
                curmesh.mTMapFaces[j] = new udword[curmesh.mNbFaces * 3];

            for (udword k = 0; k < curmesh.mNbTMaps; ++k)
            {
                for(j=0;j<curmesh.mNbFaces*3;j++)
                {
                    curmesh.mTMapFaces[k][j] = (curmesh.mFlags&NVB_WORDFACES) ? (udword)importer.GetWord() : importer.GetDword();
                }
            }
        }
    }

    // Get color faces
    if(curmesh.mCFaces)
        for(udword j=0;j<curmesh.mNbFaces*3;j++)
            curmesh.mCFaces[j] = (curmesh.mFlags&NVB_WORDFACES) ? (udword)importer.GetWord() : importer.GetDword();

    // Get face properties
    if(curmesh.mNbFaces)
    {
        curmesh.mFaceProperties = new FaceProperties[curmesh.mNbFaces];
        NVB_CHECKALLOC(curmesh.mFaceProperties);
    }

    udword j;
    for(j=0;j<curmesh.mNbFaces;j++) curmesh.mFaceProperties[j].MatID    = importer.GetDword();
    for(j=0;j<curmesh.mNbFaces;j++) curmesh.mFaceProperties[j].Smg      = importer.GetDword();
    for(j=0;j<curmesh.mNbFaces;j++) curmesh.mFaceProperties[j].EdgeVis  = (curmesh.mFlags&NVB_EDGEVIS) ? importer.GetByte() : 0;

    // Undo delta compression
    if(curmesh.mFlags&NVB_COMPRESSED)
    {
        if(curmesh.mFaces)  UnDelta(curmesh.mFaces, curmesh.mNbFaces*3, 4);
        if(curmesh.mTFaces) UnDelta(curmesh.mTFaces, curmesh.mNbFaces*3, 4);
        if(curmesh.mCFaces) UnDelta(curmesh.mCFaces, curmesh.mNbFaces*3, 4);
        if(curmesh.mNbTMaps)
        {
            for(j=0;j<curmesh.mNbTMaps;j++)
                UnDelta(curmesh.mTMapFaces[j], curmesh.mNbFaces*3, 4);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the volume integrals and other extra stuff.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportExtraStuff(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Get convex hull back
    if(curmesh.mFlags & NVB_CONVEXHULL)
    {
        curmesh.mNbHullVerts = importer.GetDword();
        curmesh.mNbHullFaces = importer.GetDword();

        // Get hull Vertices
        curmesh.mHullVerts  = new Point[curmesh.mNbHullVerts];
        NVB_CHECKALLOC(curmesh.mHullVerts);

        udword i;
        
        for(i=0;i<curmesh.mNbHullVerts;i++)
        {
            curmesh.mHullVerts[i].x = importer.GetFloat();
            curmesh.mHullVerts[i].y = importer.GetFloat();
            curmesh.mHullVerts[i].z = importer.GetFloat();
        }

        // Get hull faces
        curmesh.mHullFaces  = new udword[curmesh.mNbHullFaces*3];
        NVB_CHECKALLOC(curmesh.mHullFaces);

        for(i=0;i<curmesh.mNbHullFaces;i++)
        {
            curmesh.mHullFaces[i*3+0] = importer.GetDword();
            curmesh.mHullFaces[i*3+1] = importer.GetDword();
            curmesh.mHullFaces[i*3+2] = importer.GetDword();
        }
    }

    // Get bounding sphere back
    if(curmesh.mFlags & NVB_BOUNDINGSPHERE)
    {
        curmesh.mBSCenter.x = importer.GetFloat();
        curmesh.mBSCenter.y = importer.GetFloat();
        curmesh.mBSCenter.z = importer.GetFloat();
        curmesh.mBSRadius   = importer.GetFloat();
    }

    // Get volume integrals back
    if(curmesh.mFlags & NVB_INERTIATENSOR)
    {
        // Center of mass
        curmesh.mCOM.x = importer.GetFloat();
        curmesh.mCOM.y = importer.GetFloat();
        curmesh.mCOM.z = importer.GetFloat();

        // Mass
        curmesh.mMass = importer.GetFloat();

        // Inertia tensor
        curmesh.mInertiaTensor[0][0] = importer.GetFloat();
        curmesh.mInertiaTensor[0][1] = importer.GetFloat();
        curmesh.mInertiaTensor[0][2] = importer.GetFloat();
        curmesh.mInertiaTensor[1][0] = importer.GetFloat();
        curmesh.mInertiaTensor[1][1] = importer.GetFloat();
        curmesh.mInertiaTensor[1][2] = importer.GetFloat();
        curmesh.mInertiaTensor[2][0] = importer.GetFloat();
        curmesh.mInertiaTensor[2][1] = importer.GetFloat();
        curmesh.mInertiaTensor[2][2] = importer.GetFloat();

        // COM inertia tensor
        curmesh.mCOMInertiaTensor[0][0] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[0][1] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[0][2] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[1][0] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[1][1] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[1][2] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[2][0] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[2][1] = importer.GetFloat();
        curmesh.mCOMInertiaTensor[2][2] = importer.GetFloat();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the consolidation results.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \param      version         [in] mesh chunk version
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportConsolidated(NVBMeshInfo& curmesh, CustomArray& importer, udword version)
{
    // Get consolidated mesh back
    if(curmesh.mFlags & NVB_CONSOLIDATION)
    {
        curmesh.mCleanMesh = new MBResult;
        NVB_CHECKALLOC(curmesh.mCleanMesh);

        // Get topology back
        {
            MBTopology& Topo = curmesh.mCleanMesh->Topology;

            if(version<7)
            {
                Topo.NbFaces        = importer.GetWord();
                Topo.NbSubmeshes    = importer.GetWord();
            }
            else
            {
                Topo.NbFaces        = importer.GetDword();
                Topo.NbSubmeshes    = importer.GetDword();
            }

            // Submeshes
            Topo.SubmeshProperties = new MBSubmesh[Topo.NbSubmeshes];
            NVB_CHECKALLOC(Topo.SubmeshProperties);

            udword i;
            
            for(i=0;i<Topo.NbSubmeshes;i++)
            {
                MBSubmesh* CurSM = &Topo.SubmeshProperties[i];

                CurSM->MatID        = (sdword)importer.GetDword();
                CurSM->SmGrp        = importer.GetDword();
                CurSM->NbFaces      = importer.GetDword();
                CurSM->NbVerts      = importer.GetDword();
                CurSM->NbSubstrips  = importer.GetDword();
            }

            // Connectivity
            Topo.FacesInSubmesh = new udword[Topo.NbSubmeshes];
            NVB_CHECKALLOC(Topo.FacesInSubmesh);

            Topo.VRefs = new udword[Topo.NbFaces*3];
            NVB_CHECKALLOC(Topo.VRefs);

            udword* VRefs = Topo.VRefs;
            for(i=0;i<Topo.NbSubmeshes;i++)
            {
                if(version<7)   Topo.FacesInSubmesh[i] = importer.GetWord();
                else            Topo.FacesInSubmesh[i] = importer.GetDword();

                for(udword j=0;j<Topo.FacesInSubmesh[i];j++)
                {
                    *VRefs++ = importer.GetDword();
                    *VRefs++ = importer.GetDword();
                    *VRefs++ = importer.GetDword();
                }
            }

            // Face normals
            if(curmesh.mFlags & NVB_FACENORMALS)
            {
                Topo.Normals = new float[Topo.NbFaces*3];
                NVB_CHECKALLOC(Topo.Normals);

                for(i=0;i<Topo.NbFaces;i++)
                {
                    Topo.Normals[i*3+0] = importer.GetFloat();
                    Topo.Normals[i*3+1] = importer.GetFloat();
                    Topo.Normals[i*3+2] = importer.GetFloat();
                }
            }
        }

        // Get geometry back
        {
            MBGeometry& Geo = curmesh.mCleanMesh->Geometry;

            if(version<7)
            {
                Geo.NbGeomPts   = importer.GetWord();
                Geo.NbVerts     = importer.GetWord();
                Geo.NbTVerts    = importer.GetWord();
            }
            else
            {
                Geo.NbGeomPts   = importer.GetDword();
                Geo.NbVerts     = importer.GetDword();
                Geo.NbTVerts    = importer.GetDword();
            }

            // Indexed geometry
            Geo.VertsRefs = new udword[Geo.NbVerts];
            NVB_CHECKALLOC(Geo.VertsRefs);

            udword i;
            
            for(i=0;i<Geo.NbVerts;i++)
            {
                Geo.VertsRefs[i] = importer.GetDword();
            }

            // Vertices
            udword _NbVerts = importer.GetDword();
            Geo.Verts = new float[_NbVerts*3];
            NVB_CHECKALLOC(Geo.Verts);

            for(i=0;i<_NbVerts;i++)
            {
                Geo.Verts[i*3+0] = importer.GetFloat();
                Geo.Verts[i*3+1] = importer.GetFloat();
                Geo.Verts[i*3+2] = importer.GetFloat();
            }

            // Indexed UVWs
            if(curmesh.mFlags & NVB_UVW)
            {
                Geo.TVertsRefs = new udword[Geo.NbVerts];
                NVB_CHECKALLOC(Geo.TVertsRefs);

                udword i;
                
                for(i=0;i<Geo.NbVerts;i++)
                {
                    Geo.TVertsRefs[i] = importer.GetDword();
                }

                // UVWs
                udword _NbTVerts = importer.GetDword();
                Geo.TVerts = new float[_NbTVerts*3];
                NVB_CHECKALLOC(Geo.TVerts);

                float* p = Geo.TVerts;
                for(i=0;i<_NbTVerts;i++)
                {
                    *p++ = importer.GetFloat();
                    *p++ = importer.GetFloat();
                    if(!(curmesh.mFlags & NVB_WDISCARDED))  
                    {
                        *p++ = importer.GetFloat();
                    }
                }
            }

            // Normals
            if(curmesh.mFlags & NVB_VERTEXNORMALS)
            {
                udword NbNormals = importer.GetDword();
                Geo.Normals = new float[NbNormals*3];
                NVB_CHECKALLOC(Geo.Normals);

                for(i=0;i<NbNormals;i++)
                {
                    Geo.Normals[i*3+0] = importer.GetFloat();
                    Geo.Normals[i*3+1] = importer.GetFloat();
                    Geo.Normals[i*3+2] = importer.GetFloat();
                }
            }

            // Vertex colors
            if(curmesh.mFlags & NVB_VERTEXCOLORS)
            {
                udword NbVtxColors = importer.GetDword();
                Geo.CVerts = new float[NbVtxColors*3];
                NVB_CHECKALLOC(Geo.CVerts);

                for(udword i=0;i<NbVtxColors;i++)
                {
                    Geo.CVerts[i*3+0] = importer.GetFloat();
                    Geo.CVerts[i*3+1] = importer.GetFloat();
                    Geo.CVerts[i*3+2] = importer.GetFloat();
                }
            }

            // NormalInfo
            if(curmesh.mFlags & NVB_NORMALINFO)
            {
                Geo.NormalInfoSize = importer.GetDword();
                Geo.NormalInfo = new udword[Geo.NormalInfoSize];
                NVB_CHECKALLOC(Geo.NormalInfo);

                for(udword i=0;i<Geo.NormalInfoSize;i++)
                {
                    Geo.NormalInfo[i] = importer.GetDword();
                }
            }
        }

        // Materials
        {
            MBMaterials& Mtl = curmesh.mCleanMesh->Materials;

            Mtl.NbMtls = importer.GetDword();
            Mtl.MaterialInfo    = new MBMatInfo[Mtl.NbMtls];
            NVB_CHECKALLOC(Mtl.MaterialInfo);

            for(udword i=0;i<Mtl.NbMtls;i++)
            {
                MBMatInfo* CurMtl = &Mtl.MaterialInfo[i];

                CurMtl->MatID       = (sdword)importer.GetDword();
                CurMtl->NbFaces     = importer.GetDword();
                CurMtl->NbVerts     = importer.GetDword();
                CurMtl->NbSubmeshes = importer.GetDword();
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the lighting data.
 *  \param      curmesh         [in] current mesh structure
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportLightingData(NVBMeshInfo& curmesh, CustomArray& importer)
{
    // Get number of precomputed colors
    curmesh.mNbColors = importer.GetDword();

    // Get them back if needed
    if(curmesh.mNbColors)
    {
        curmesh.mColors = new Point[curmesh.mNbColors];
        NVB_CHECKALLOC(curmesh.mColors);

        for(udword i=0;i<curmesh.mNbColors;i++)
        {
            curmesh.mColors[i].x = importer.GetFloat();
            curmesh.mColors[i].y = importer.GetFloat();
            curmesh.mColors[i].z = importer.GetFloat();
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the controllers.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportControllers(CustomArray& importer)
{
    // Is there any controllers to import?
    if(mNbControllers)
    {
        // Get version number back
        udword Version      = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_CTRL_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d controllers...\n", mNbControllers);

        // Import all controllers
        for(udword n=0;n<mNbControllers;n++)
        {
            // Fill a controller structure
            NVBControllerInfo CurController;

            CurController.mField.Set((const char*)importer.GetString());    // Get the field back
            CurController.mObjectID = importer.GetDword();                  // Controller's ID
            CurController.mOwnerID  = importer.GetDword();                  // Owner's ID
            if(Version>=2)  CurController.mOwnerType = (NVBObjType)importer.GetDword(); // Owner's type

            CurController.mCtrlType = (NVBCtrlType)importer.GetDword();     // Controller type
            CurController.mCtrlMode = (NVBCtrlMode)importer.GetDword();     // Controller mode

            if(CurController.mCtrlMode==NVB_CTRL_SAMPLES)
            {
                if(CurController.mCtrlType==NVB_CTRL_VERTEXCLOUD)   CurController.mNbVertices = importer.GetDword();

                CurController.mNbSamples    = importer.GetDword();
                CurController.mSamplingRate = importer.GetDword();

                // Use built-in types only
                switch(CurController.mCtrlType)
                {
                    case NVB_CTRL_FLOAT:
                    {
                        float* Samples = new float[CurController.mNbSamples];
                        NVB_CHECKALLOC(Samples);
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            Samples[i]  = importer.GetFloat();
                        }
                        CurController.mSamples = Samples;
                    }
                    break;

                    case NVB_CTRL_VECTOR:
                    {
                        Point* Samples = (Point*)(new float[3*CurController.mNbSamples]);
                        NVB_CHECKALLOC(Samples);
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            Samples[i].x    = importer.GetFloat();
                            Samples[i].y    = importer.GetFloat();
                            Samples[i].z    = importer.GetFloat();
                        }
                        CurController.mSamples = Samples;
                    }
                    break;

                    case NVB_CTRL_QUAT:
                    {
                        Quat* Samples = (Quat*)(new float[4*CurController.mNbSamples]);
                        NVB_CHECKALLOC(Samples);
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            Samples[i].p.x  = importer.GetFloat();
                            Samples[i].p.y  = importer.GetFloat();
                            Samples[i].p.z  = importer.GetFloat();
                            Samples[i].w    = importer.GetFloat();
                        }
                        CurController.mSamples = Samples;
                    }
                    break;

                    case NVB_CTRL_PR:
                    {
                        PR* Samples = (PR*)(new float[7*CurController.mNbSamples]);
                        NVB_CHECKALLOC(Samples);
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            Samples[i].mPos.x   = importer.GetFloat();
                            Samples[i].mPos.y   = importer.GetFloat();
                            Samples[i].mPos.z   = importer.GetFloat();
                            Samples[i].mRot.p.x = importer.GetFloat();
                            Samples[i].mRot.p.y = importer.GetFloat();
                            Samples[i].mRot.p.z = importer.GetFloat();
                            Samples[i].mRot.w   = importer.GetFloat();
                        }
                        CurController.mSamples = Samples;
                    }
                    break;

                    case NVB_CTRL_PRS:
                    {
                        PRS* Samples = (PRS*)(new float[10*CurController.mNbSamples]);
                        NVB_CHECKALLOC(Samples);
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            Samples[i].mPos.x   = importer.GetFloat();
                            Samples[i].mPos.y   = importer.GetFloat();
                            Samples[i].mPos.z   = importer.GetFloat();
                            Samples[i].mRot.p.x = importer.GetFloat();
                            Samples[i].mRot.p.y = importer.GetFloat();
                            Samples[i].mRot.p.z = importer.GetFloat();
                            Samples[i].mRot.w   = importer.GetFloat();
                            Samples[i].mScale.x = importer.GetFloat();
                            Samples[i].mScale.y = importer.GetFloat();
                            Samples[i].mScale.z = importer.GetFloat();
                        }
                        CurController.mSamples = Samples;
                    }
                    break;

                    case NVB_CTRL_VERTEXCLOUD:
                    {
                        float* Samples = new float[3*CurController.mNbSamples*CurController.mNbVertices];
                        NVB_CHECKALLOC(Samples);
                        float* Pool = (float*)Samples;
                        for(udword i=0;i<CurController.mNbSamples;i++)
                        {
                            for(udword j=0;j<CurController.mNbVertices;j++)
                            {
                                *Pool++ = importer.GetFloat();
                                *Pool++ = importer.GetFloat();
                                *Pool++ = importer.GetFloat();
                            }
                        }
                        CurController.mSamples = Samples;
                    }
                    break;
                    case NVB_CTRL_NONE:
                    case NVB_CTRL_SCALE:
                    case NVB_CTYPE_FORCE_DWORD:
                        break;
                }
            }
            else return false;  // only sampling for now.....

            // Call the app
            NewController(CurController);
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the shapes.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportShapes(CustomArray& importer)
{
    // Is there any shapes to import?
    if(mNbShapes)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_SHAP_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d shapes...\n", mNbShapes);

        // Import all shapes
        for(udword n=0;n<mNbShapes;n++)
        {
            // Fill a shape structure
            NVBShapeInfo CurShape;

            // Base info
            CurShape.Import(importer);

            // Get shape information back
            CurShape.mNbLines   = importer.GetDword();  // Number of lines
            if(CHUNK_SHAP_VER>=2)   CurShape.mMatID = importer.GetDword();  // Material ID
            if(CurShape.mNbLines)
            {
                CurShape.mNbVerts = new udword[CurShape.mNbLines];  NVB_CHECKALLOC(CurShape.mNbVerts);
                CurShape.mClosed = new bool[CurShape.mNbLines]; NVB_CHECKALLOC(CurShape.mClosed);

                // Get all polylines
                CustomArray Vertices;
                CurShape.mTotalNbVerts = 0;
                for(udword i=0;i<CurShape.mNbLines;i++)
                {
                    udword NbVerts = importer.GetDword();   // Number of Vertices in current line
                    bool Closed = importer.GetByte()!=0;    // Closed/open status
                    for(udword j=0;j<NbVerts;j++)
                    {
                        float x = importer.GetFloat();
                        float y = importer.GetFloat();
                        float z = importer.GetFloat();
                        Vertices.Store(x).Store(y).Store(z);
                    }
                    CurShape.mNbVerts[i] = NbVerts;
                    CurShape.mClosed[i] = Closed;
                    CurShape.mTotalNbVerts+=NbVerts;
                }
                CurShape.mVerts = new Point[CurShape.mTotalNbVerts];
                NVB_CHECKALLOC(CurShape.mVerts);
                CopyMemory(CurShape.mVerts, Vertices.Collapse(), CurShape.mTotalNbVerts*sizeof(Point));
            }

            // Call the app
            NewShape(CurShape);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  A method to import the helpers.
 *  \param      importer        [in] the imported array.
 *  \return     true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBBreaker::ImportHelpers(CustomArray& importer)
{
    // Is there any helpers to import?
    if(mNbHelpers)
    {
        // Get version number back
        udword Version  = importer.GetDword();
        NVB_CHECKVERSION(CHUNK_HELP_VER);

        // Log message
        NVBLog(LOG_MSG, "Importing %d helpers...\n", mNbHelpers);

        // Import all helpers
        for(udword n=0;n<mNbHelpers;n++)
        {
            // Fill a helper structure
            NVBHelperInfo CurHelper;

            // Base info
            CurHelper.Import(importer);

            // Get helper information back
            if(Version<2)   CurHelper.mIsGroupHead  = importer.GetByte()!=0;
            else
            {
                CurHelper.mHelperType   = (HType)importer.GetDword();
                CurHelper.mIsGroupHead  = importer.GetByte()!=0;
                switch(CurHelper.mHelperType)
                {
                    case HTYPE_GIZMO_BOX:
                    {
                        CurHelper.mLength   = importer.GetFloat();
                        CurHelper.mWidth    = importer.GetFloat();
                        CurHelper.mHeight   = importer.GetFloat();
                    }
                    break;

                    case HTYPE_GIZMO_SPHERE:
                    {
                        CurHelper.mRadius   = importer.GetFloat();
                        CurHelper.mHemi     = importer.GetByte()!=0;
                    }
                    break;

                    case HTYPE_GIZMO_CYLINDER:
                    {
                        CurHelper.mRadius   = importer.GetFloat();
                        CurHelper.mHeight   = importer.GetFloat();
                    }
                    break;
                }
            }

            // Call the app
            NewHelper(CurHelper);
        }
    }
    return true;
}
