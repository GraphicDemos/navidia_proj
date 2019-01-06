///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains code to import NVB files.
 *  \file       NVBBreaker.h
 *  \author     Pierre Terdiman
 *  \date       April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Usage:
  1) Use NVBBreaker as a base class and override all app-dependent methods. They'll get called during the import.
  2) Call the Import(filename) method.

  That's it!
  ...poorly designed, but does the job...
*/

#include "NVBLoader.h"
#include "NVBString.h"
#include "nv_attribute.h"
#include "NVBFormat.h"
#include "NVBCustomArray.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __NVBBREAKER_H__
#define __NVBBREAKER_H__

    
    enum NVBError
    {
        NVB_ERROR_OK                = 0,
        NVB_ERROR_FILENOTFOUND      = 1,
        NVB_ERROR_OUTOFMEMORY       = 2,
        NVB_ERROR_INVALIDFILE       = 3,
        NVB_ERROR_CANTDEPACK        = 4,
        NVB_ERROR_UNKNOWNVERSION    = 5,
        NVB_ERROR_CHUNKNOTFOUND     = 6,
        NVB_ERROR_OBSOLETEFILE      = 7,
        NVB_ERROR_FORCEDWORD        = 0x7fffffff
    };

    enum NVB_GUID_CHUNKS
    {
        NVB_MAIN_CHUNK              = 0,
        NVB_MESH_CHUNK              = 1,
        NVB_CAMS_CHUNK              = 2,
        NVB_LIGHTS_CHUNK            = 3,
        NVB_SHAPES_CHUNK            = 4,
        NVB_HELPERS_CHUNK           = 5,
        NVB_TEXTURES_CHUNK          = 6,
        NVB_MATERIALS_CHUNK         = 7,
        NVB_CONTROLLERS_CHUNK       = 8,
        NVB_MOVERS_CHUNK            = 9,
        NVB_GUID_FORCEDWORD         = 0x7fffffff
    };
    
    
    class NVBCORE_API NVBComponents
    {
        public:
        // Constructor/Destructor
                                NVBComponents();
        virtual                 ~NVBComponents();
        // NVB file components
                udword          mNbMeshes;          //!< Number of geomobjects (meshes) found
                udword          mNbDerived;         //!< Number of derived objects found (actually number of skins)
                udword          mNbCameras;         //!< Number of cameras found
                udword          mNbLights;          //!< Number of lights found
                udword          mNbShapes;          //!< Number of shapes found
                udword          mNbHelpers;         //!< Number of helpers found
                udword          mNbControllers;     //!< Number of controllers found
                udword          mNbMaterials;       //!< Number of materials found
                udword          mNbTextures;        //!< Number of textures found
                udword          mNbUnknowns;        //!< Number of unknown nodes found
                udword          mNbInvalidNodes;    //!< Number of invalid nodes found
    };

    class NVBCORE_API NVBSceneInfo : public NVBComponents
    {
        public:
        // Constructor/Destructor
                                NVBSceneInfo();
        virtual                 ~NVBSceneInfo();
        // Time-related info
                udword          mFirstFrame;        //!< Timeline's first frame number
                udword          mLastFrame;         //!< Timeline's last frame number
                udword          mFrameRate;         //!< Global frame rate
                udword          mDeltaTime;         //!< Ticks per frame
        // Environment
                Point           mBackColor;         //!< Background color
                Point           mAmbientColor;      //!< Global ambient color
        // Scene info
                String          mSceneInfo;         //!< The scene information string
    };

    class NVBCORE_API NVBBaseInfo
    {
        public:
        enum ObjDescType
        {
            NODEDESC_NULL        = 0x00000000,
            NODEDESC_ATTRIBUTES  = 0x00000001
        };
        // Constructor/Destructor
                                NVBBaseInfo();
        virtual                 ~NVBBaseInfo();

        // Database information
                String          mName;              //!< Object's name
                sdword          mID;                //!< Object's ID
                sdword          mParentID;          //!< Parent's ID
                sdword          mLinkID;            //!< Link ID
                bool            mGroup;             //!< true if the object belongs to a group
        // Position/Rotation/Scale = PRS
                PRS             mPrs;
        // Pivot
                Point           mPivotPos;          //!< Global position
                Quat            mPivotRot;          //!< Global rotation
        // Rendering information
                udword          mWireColor;         //!< The wireframe color
                bool            mLocalPRS;          //!< true for local PRS
                bool            mD3DCompliant;      //!< true if converted to D3D frame
        // User properties
                String          mUserProps;         //!< The user-defined properties
                udword          mBaseFlags;         //!< Door to extensions
                nv_attribute    mAttr;              //!< Abstract Attributes

                bool            Import(CustomArray& importer);
                bool            ImportAttributes(CustomArray& importer);
    };

    // Camera types from MAX.
    enum CType
    {
        CTYPE_FREE          = 0,    //!< Free camera
        CTYPE_TARGET        = 1,    //!< Target camera
        CTYPE_PARALLEL      = 2,    //!< Parallel camera
        CTYPE_FORCE_DWORD   = 0x7fffffff
    };

    // FOV types
    enum FOVType
    {
        FOV_HORIZONTAL      = 0,    //!< Horizontal FOV
        FOV_VERTICAL        = 1,    //!< Vertical FOV
        FOV_DIAGONAL        = 2,    //!< Diagonal FOV
        FOV_FORCE_DWORD = 0x7fffffff
    };

    class NVBCORE_API NVBCameraInfo : public NVBBaseInfo
    {
        public:
        // Constructor/Destructor
                                NVBCameraInfo();
        virtual                 ~NVBCameraInfo();

        // Camera parameters
                CType           mType;                  //!< Camera type
                bool            mOrthoCam;              //!< Camera type: ortographic (true) or perspective (false)
                float           mFOV;                   //!< Field-Of-View (degrees) or Width for ortho cams        [Animatable]
                FOVType         mFOVType;               //!< FOV Type
                float           mNearClip;              //!< Near/hither clip                                       [Animatable]
                float           mFarClip;               //!< Far/yon clip                                           [Animatable]
                float           mTDist;                 //!< Distance to target                                     [Animatable]
                long            mHLineDisplay;          //!< Horizon Line Display
                float           mEnvNearRange;          //!< Environment near range                                 [Animatable]
                float           mEnvFarRange;           //!< Environment far range                                  [Animatable]
                long            mEnvDisplay;            //!< Environment display
                long            mManualClip;            //!< Manual clip
    };

    // Light types from MAX. Usual rendering APIs only keep Point, Spot and Directional lights.
    enum LType
    {
        LTYPE_OMNI          = 0,    //!< Omnidirectional        (PointLight)
        LTYPE_TSPOT         = 1,    //!< Targeted               (SpotLight)
        LTYPE_DIR           = 2,    //!< Directional            (DirectionalLight)
        LTYPE_FSPOT         = 3,    //!< Free                   (SpotLight)
        LTYPE_TDIR          = 4,    //!< Targeted directional   (DirectionalLight)
        LTYPE_FORCE_DWORD   = 0x7fffffff
    };
    // NB: mIsSpot and mIsDir are dumped from MAX, but they may not match D3D's definitions...

    // Spotlight shape.
    enum SpotShp
    {
        SPOTSHP_RECT        = 0,    //!< Rectangle.
        SPOTSHP_CIRCLE      = 1,    //!< Circle.
        SPOTSHP_FORCE_DWORD = 0x7fffffff
    };

    // Decay type
    enum DecayType
    {
        DECAYTYPE_NONE      = 0,
        DECAYTYPE_INV       = 1,
        DECAYTYPE_INVSQ     = 2
    };

    class NVBCORE_API NVBLightInfo : public NVBBaseInfo
    {
        public:
        // Constructor/Destructor
                                NVBLightInfo();
        virtual                 ~NVBLightInfo();

        // Light parameters
                LType           mLightType;                     //!< Light's type
                bool            mIsSpot;                        //!< Is the light a spotlight?
                bool            mIsDir;                         //!< Is the light a directional?
                Point           mColor;                         //!< Light's color                                          [Animatable]
                float           mIntensity;                     //!< Light's intensity                                      [Animatable]
                float           mContrast;                      //!< Light's contrast                                       [Animatable]
                float           mDiffuseSoft;                   //!< Light's diffuse soft                                   [Animatable]
                bool            mLightUsed;                     //!< Is the light used?
                bool            mAffectDiffuse;                 //!< Does the light affect diffuse?
                bool            mAffectSpecular;                //!< Does the light affect specular?
                bool            mUseAttenNear;                  //
                bool            mAttenNearDisplay;              //
                bool            mUseAtten;                      //!< Is attenuation used?
                bool            mShowAtten;                     //
                float           mNearAttenStart;                //!< Near atten start                                       [Animatable]
                float           mNearAttenEnd;                  //!< Near atten end                                         [Animatable]
                float           mAttenStart;                    //!< Atten start                                            [Animatable]
                float           mAttenEnd;                      //!< Atten end (use that as a range for non-dir lights)     [Animatable]
                char            mDecayType;                     //!< Light's decay type
                float           mHotSpot;                       //!< Light's hotspot                                        [Animatable]
                float           mFallsize;                      //!< Light's falloff                                        [Animatable]
                float           mAspect;                        //!< Light's aspect                                         [Animatable]
                SpotShp         mSpotShape;                     //!< Light's spot shape
                long            mOvershoot;                     //!< Light's overshoot
                bool            mConeDisplay;                   //
                float           mTDist;                         //!< Distance to target                                     [Animatable]
                long            mShadowType;                    //!< Light's shadow type
                long            mAbsMapBias;                    //!< Light's absolute map bias
                float           mRayBias;                       //!< Raytrace bias                                          [Animatable]
                float           mMapBias;                       //!< Map bias                                               [Animatable]
                float           mMapRange;                      //!< Map range                                              [Animatable]
                long            mMapSize;                       //!< Map size                                               [Animatable]
                bool            mCastShadows;                   //!< Cast shadows
                float           mShadowDensity;                 //!< Shadow density                                         [Animatable]
                Point           mShadowColor;                   //!< Shadow color                                           [Animatable]
                bool            mLightAffectsShadow;            //!< Light affects shadow or not
    };

    // Transparency types from MAX 3.1. ("Advanced Transparency" in the Material Editor)
    enum TranspaType
    {
        TRANSPA_SUBTRACTIVE = 0,
        TRANSPA_ADDITIVE    = 1,
        TRANSPA_FILTER      = 2,
        TRANSPA_FORCE_DWORD = 0x7fffffff
    };

    // Shading mode
    enum ShadingMode
    {
        SHADING_BLINN       = 0,
        SHADING_PHONG       = 1,
        SHADING_METAL       = 2,
        SHADING_ANISO       = 3,
        SHADING_FORCE_DWORD = 0x7fffffff
    };

    //! A possible texture matrix.
    struct NVBCORE_API TextureMatrix
    {
        TextureMatrix()
        {
            ZeroMemory(m, 3*4*sizeof(float));
            m[0][0] = m[1][1] = m[2][2] = 1.0f;
        }
        //! Warning: that's a direct dump from MAX (untransformed)
        float m[4][3];
    };

    //! Crop values. Useful for T-pages. Warning: equations to use them are weird.
    struct NVBCORE_API TextureCrop
    {
        TextureCrop() : mOffsetU(0.0f), mOffsetV(0.0f), mScaleU(1.0f), mScaleV(1.0f)    {}

        float   mOffsetU;   //!< Offset for U
        float   mOffsetV;   //!< Offset for V
        float   mScaleU;    //!< Scale for U
        float   mScaleV;    //!< Scale for V
    };

    class NVBCORE_API NVBMaterialInfo
    {
        public:
        enum MatDescType
        {
            MATDESC_NULL        = 0x00000000,
            MATDESC_ATTRIBUTES  = 0x00000001
        };
        // Constructor/Destructor
                                NVBMaterialInfo();
        virtual                 ~NVBMaterialInfo();

        // Database information
                String          mName;                              //!< Material name
                sdword          mID;                                //!< Material ID
        // Texture IDs
                sdword          mAmbientMapID;                      //!< Ambient texture map (seems not to exist anymore in MAX 3)
                sdword          mDiffuseMapID;                      //!< Diffuse texture map
                sdword          mSpecularMapID;                     //!< Specular texture map
                sdword          mShininessMapID;                    //!< Shininess texture map
                sdword          mShiningStrengthMapID;              //!< Shining Strength texture map
                sdword          mSelfIllumMapID;                    //!< Self Illum texture map
                sdword          mOpacityMapID;                      //!< Opacity texture map
                sdword          mFilterMapID;                       //!< Filter texture map
                sdword          mBumpMapID;                         //!< Bump texture map
                sdword          mReflectionMapID;                   //!< Reflection texture map
                sdword          mRefractionMapID;                   //!< Refraction texture map
                sdword          mDisplacementMapID;                 //!< Displacement texture map
        // Amounts
                float           mAmbientCoeff;                      //!< Ambient texture %                                  [Animatable]
                float           mDiffuseCoeff;                      //!< Diffuse tetxure %                                  [Animatable]
                float           mSpecularCoeff;                     //!< Specular texture %                                 [Animatable]
                float           mShininessCoeff;                    //!< Shininess texture %                                [Animatable]
                float           mShiningStrengthCoeff;              //!< Shining Strength texture %                         [Animatable]
                float           mSelfIllumCoeff;                    //!< Self Illum texture %                               [Animatable]
                float           mOpacityCoeff;                      //!< Opacity texture %                                  [Animatable]
                float           mFilterCoeff;                       //!< Filter texture %                                   [Animatable]
                float           mBumpCoeff;                         //!< Bump texture %                                     [Animatable]
                float           mReflectionCoeff;                   //!< Reflection texture %                               [Animatable]
                float           mRefractionCoeff;                   //!< Refraction texture %                               [Animatable]
                float           mDisplacementCoeff;                 //!< Displacement texture %                             [Animatable]
        // Colors
                Point           mMtlAmbientColor;                   //!< Ambient Color                                      [Animatable]
                Point           mMtlDiffuseColor;                   //!< Diffuse Color                                      [Animatable]
                Point           mMtlSpecularColor;                  //!< Specular Color                                     [Animatable]
                Point           mMtlFilterColor;                    //!< Filter Color                                       [Animatable]
        // Static properties
                ShadingMode     mShading;                           //!< Material Shading
                bool            mSoften;                            //!< MaterialSoften
                bool            mFaceMap;                           //!< MaterialFaceMap
                bool            mTwoSided;                          //!< true for two-sided materials
                bool            mWire;                              //!< true for wireframe mode
                bool            mWireUnits;                         //!< Wire Units
                bool            mFalloffOut;                        //!< MaterialFalloffOut
                TranspaType     mTransparencyType;                  //!< MaterialTransparencyType
        // Dynamic properties
                float           mShininess;                         //!< MaterialShininess                                  [Animatable]
                float           mShiningStrength;                   //!< MaterialShiningStrength                            [Animatable]
                float           mSelfIllum;                         //!< MaterialSelfIllum                                  [Animatable]
                float           mOpacity;                           //!< MaterialOpacity                                    [Animatable]
                float           mOpaFalloff;                        //!< MaterialOpacityFalloff                             [Animatable]
                float           mWireSize;                          //!< MaterialWireSize                                   [Animatable]
                float           mIOR;                               //!< MaterialIOR                                        [Animatable]
        // Dynamic properties
                float           mBounce;                            //!< Bounce                                             [Animatable]
                float           mStaticFriction;                    //!< Static Friction                                    [Animatable]
                float           mSlidingFriction;                   //!< Sliding Friction                                   [Animatable]
        // Crop values & texture matrix (for diffuse map)
        // Valid if mDiffuseMapID!=INVALID_ID
                TextureCrop     mCValues;                           //!< Cropping values
                TextureMatrix   mTMtx;                              //!< Texture matrix
        // Custom Attributes    
                udword          mMatFlags;                          //!< Door to extension
                nv_attribute    mAttr;                              //!< Abstract Attributes
    };

    class NVBCORE_API NVBTextureInfo
    {
        public:
        // Constructor/Destructor
                                NVBTextureInfo();
        virtual                 ~NVBTextureInfo();

        // Database information
                String          mName;                          //!< Texture name & path
                sdword          mID;                            //!< Texture ID
        // Crop values & texture matrix
                TextureCrop     mCValues;                       //!< Cropping values
                TextureMatrix   mTMtx;                          //!< Texture matrix
                udword          mChannel;                       //!< Texture Channel
        // Bitmap data
                udword          mWidth;                         //!< Texture's width
                udword          mHeight;                        //!< Texture's height
                ubyte*          mBitmap;                        //!< Texture in R,G,B,A order. (always contains Alpha)
                bool            mHasAlpha;                      //!< True => Alpha is valid.
                bool            mIsBitmapIncluded;              //!< True if previous fields are valid, else must be read from disk
    };

    struct NVBCORE_API FaceProperties
    {
        udword  MatID;      //!< Material ID
        udword  Smg;        //!< Smoothing groups
        ubyte   EdgeVis;    //!< Edge visibility code
    };

    struct NVBCORE_API BasicBone
    {
        udword  CSID;       //!< Character Studio ID
        udword  pCSID;      //!< Parent's CSID
    };

    class NVBCORE_API NVBNativeMeshInfo
    {
        public:
        enum MatDescType
        {
            MESHDESC_NULL        = 0x00000000,
            MESHDESC_ATTRIBUTES  = 0x00000001
        };
        // Constructor/Destructor
                                NVBNativeMeshInfo();
        virtual                 ~NVBNativeMeshInfo();

        // Useful figures
                udword          mNbFaces;                   //!< Number of faces in MAX native data
                udword          mNbVerts;                   //!< Number of Vertices.
                udword          mNbTVerts;                  //!< Number of texture-Vertices (mapping coordinates)
                udword          mNbCVerts;                  //!< Number of vertex-colors
                udword          mNbTMaps;                   //!< Number of parameterizations
                udword*         mNbTMapVerts;               //!< Number of vertices per parameterization
        // NVB Flags
                udword          mFlags;                     //!< See NVB flags
        // Topologies
                udword*         mFaces;                     //!< List of faces
                udword*         mTFaces;                    //!< List of texture-faces
                udword*         mCFaces;                    //!< List of color-faces
                udword**        mTMapFaces;                 //!< List of parameterization faces
                FaceProperties* mFaceProperties;            //!< List of face properties
        // Geometries
                Point**         mTMaps;                     //!< Array of lists of texture vertices
                Point*          mVerts;                     //!< List of Vertices (null for skins, use offset vectors)
                Point*          mTVerts;                    //!< List of texture-Vertices
                Point*          mCVerts;                    //!< List of vertex-colors
        // Skin-related information
                udword*         mBonesNb;                   //!< Number of bones for each vertex, or null if one bone/vertex. (only for skins)
                udword*         mBonesID;                   //!< IDs of driving bones. (only for skins)
                udword*         mBonesLocalID;              //!< Local IDs of driving bones. (only for skins)
                Point*          mOffsetVectors;             //!< Character Studio's offset vectors. (only for skins)
                float*          mWeights;                   //!< Bones weights when there's more than one bone/vertex, else null. (only for skins)
        // Skeleton-related information
                udword          mNbBones;                   //!< Number of bones in the skeleton below (only for skins)
                BasicBone*      mSkeleton;                  //!< Skeletal information (only for skins)
        // Possible convex hull
                udword          mNbHullVerts;               //!< Number of Vertices in the convex hull
                udword          mNbHullFaces;               //!< Number of faces in the convex hull
                Point*          mHullVerts;                 //!< Convex hull Vertices
                udword*         mHullFaces;                 //!< Convex hull faces
        // Possible bounding sphere
                Point           mBSCenter;                  //!< Bounding sphere center
                float           mBSRadius;                  //!< Bounding sphere radius
        // Possible volume integrals
                Point           mCOM;                       //!< Center Of Mass
                float           mMass;                      //!< Total mass
                float           mInertiaTensor[3][3];       //!< Inertia tensor (mass matrix) relative to the origin
                float           mCOMInertiaTensor[3][3];    //!< Inertia tensor (mass matrix) relative to the COM
        // Winding order
                bool            mParity;                    //!< Faces are CW or CCW.
        // Custom Attributes
                udword          mMeshFlags;                 //!< Door to extensions
                nv_attribute    mMeshAttr;                  //!< Abstract Mesh Attributes
    };

    class NVBCORE_API NVBMeshInfo : public NVBBaseInfo, public NVBNativeMeshInfo
    {
        public:
        // Constructor/Destructor
                                NVBMeshInfo();
        virtual                 ~NVBMeshInfo();

        // Parameters
                bool            mIsCollapsed;       //!< true => mesh comes from a collapsed modifier stack
                bool            mIsSkeleton;        //!< true => mesh is a BIPED part (i.e. a bone)
                bool            mIsInstance;        //!< true => mesh is an instance from another mesh
                bool            mIsTarget;          //!< true => mesh is a target node (camera, spot or directional target)
                bool            mIsConvertible;     //!< false => mesh can't be converted to triangles (and the native format is unsupported)
                bool            mIsSkin;            //!< true => the mesh is a PHYSIQUE skin
                bool            mCastShadows;       //!< true => a shadow volume can be built from the mesh

        // Biped-related information (valid if mIsSkeleton==true)
                udword          mCharID;            //!< Owner character's ID
                udword          mCSID;              //!< CSID code.
        // Mesh data
                MBResult*       mCleanMesh;         //!< Mesh after consolidation
        // Lightmapper data
                udword          mNbColors;          //!< Number of computed colors
                Point*          mColors;            //!< Computed colors
    };

    class NVBCORE_API NVBShapeInfo : public NVBBaseInfo
    {
        public:
        // Constructor/Destructor
                                NVBShapeInfo();
        virtual                 ~NVBShapeInfo();

        // Shape parameters
                udword          mNbLines;               //!< Number of polylines
                udword*         mNbVerts;               //!< Number of Vertices for each polyline
                bool*           mClosed;                //!< Closed/open polylines
                Point*          mVerts;                 //!< List of Vertices
                udword          mTotalNbVerts;          //!< Total number of Vertices in the list (=sum of all mNbVerts)
                sdword          mMatID;                 //!< Shape's material ID
    };

    //! Supported helper types from MAX.
    enum HType
    {
        HTYPE_DUMMY             = 0,                            //!< Dummy object
        HTYPE_GIZMO_BOX         = 1,                            //!< Box gizmo
        HTYPE_GIZMO_CYLINDER    = 2,                            //!< Cylinder gizmo
        HTYPE_GIZMO_SPHERE      = 3,                            //!< Sphere gizmo
        HTYPE_UNDEFINED         = 0xffffffff,
    };

    class NVBCORE_API NVBHelperInfo : public NVBBaseInfo
    {
        public:
        // Constructor/Destructor
                                NVBHelperInfo();
        virtual                 ~NVBHelperInfo();

        // Helper parameters
                HType           mHelperType;        //!< Type of helper
                bool            mIsGroupHead;       //!< True for group heads
                float           mLength;            //!< For BoxGizmo
                float           mWidth;             //!< For BoxGizmo
                float           mHeight;            //!< For BoxGizmo/CylinderGizmo
                float           mRadius;            //!< For CylinderGizmo/SphereGizmo
                bool            mHemi;              //!< For SphereGizmo
    };

    class NVBCORE_API NVBControllerInfo
    {
        public:
        // Constructor/Destructor
                                NVBControllerInfo();
        virtual                 ~NVBControllerInfo();

                String          mField;                 //!< Controlled field
                sdword          mObjectID;              //!< Controller's ID
                sdword          mOwnerID;               //!< Owner object's ID
                NVBObjType      mOwnerType;             //!< Owner object's type

                NVBCtrlType     mCtrlType;              //!< Controlled type (float, quat, etc)
                NVBCtrlMode     mCtrlMode;              //!< Controller mode (sampling, keyframes, etc)

        // Sampling-related values
                udword          mNbSamples;             //!< Number of samples
                udword          mSamplingRate;          //!< Sampling rate
                void*           mSamples;               //!< Sampled data
                udword          mNbVertices;            //!< Number of Vertices for morph controllers
    };

    class NVBCORE_API NVBMotionInfo
    {
        public:
        // Constructor/Destructor
                                NVBMotionInfo()     {}
        virtual                 ~NVBMotionInfo()    {}

                String          mName;              //!< Motion's name
                String          mType;              //!< Motion's type
                udword          mCharID;            //!< Owner's character ID
                udword          mNbBones;           //!< Number of bones involved
                udword          mNbVBones;          //!< Number of virtual bones involved
                void*           mData;              //!< Motion data
                bool            mLocalPRS;          //!< True for relative PRS
                bool            mD3DCompliant;      //!< True if converted to D3D frame
    };

    typedef enum                
    {
        LOG_MSG=0, 
        LOG_WARN=1, 
        LOG_ERR=2 
    } TLogLevel;
    typedef         void                (*TloggingCB)(const char *str, TLogLevel level, unsigned long userparam);

    class NVBCORE_API NVBBreaker : public NVBComponents
    {
        public:
        // Constructor / Destructor
                                NVBBreaker();
        virtual                 ~NVBBreaker();

        // Import method
        virtual bool            Import(const String& filename);
        


        // Application-dependent import methods, called by the importer
        virtual bool            NewScene                (const NVBSceneInfo& scene)             = 0;
        virtual bool            NewCamera               (const NVBCameraInfo& camera)           = 0;
        virtual bool            NewLight                (const NVBLightInfo& light)             = 0;
        virtual bool            NewMaterial             (const NVBMaterialInfo& material)       = 0;
        virtual bool            NewTexture              (const NVBTextureInfo& texture)         = 0;
        virtual bool            NewMesh                 (const NVBMeshInfo& mesh)               = 0;
        virtual bool            NewShape                (const NVBShapeInfo& shape)             = 0;
        virtual bool            NewHelper               (const NVBHelperInfo& helper)           = 0;
        virtual bool            NewController           (const NVBControllerInfo& controller)   = 0;
        virtual bool            NewMotion               (const NVBMotionInfo& motion)           = 0;

        // optional end of processing notification
        virtual bool            EndImport();

        // Application-dependent error handler
        virtual bool            NVBImportError          (const char* errortext, udword errorcode)   = 0;

        // Application-dependent log method
        virtual void            NVBLog                  (TLogLevel level, char *fmt, ...)       = 0;

        virtual void            SetLogCallback(TloggingCB cbfn, unsigned long userparam=0)      = 0;

        // Free used memory
                void            ReleaseRam();
        protected:
                NVBFileType     mFileType;
                CustomArray*    mImportArray;
        private:
                bool            Import();

                bool            ImportCameras           (CustomArray& importer);
                bool            ImportLights            (CustomArray& importer);
                bool            ImportMaterials         (CustomArray& importer);
                bool            ImportAttributes        (nv_attribute & attr,CustomArray& importer);
                bool            ImportTextures          (CustomArray& importer);
                bool            ImportMeshAttributes    (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportMeshes            (CustomArray& importer);
                bool            ImportShapes            (CustomArray& importer);
                bool            ImportHelpers           (CustomArray& importer);
                bool            ImportControllers       (CustomArray& importer);
                bool            ImportMotion            (CustomArray& importer);

                bool            ImportVertices          (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportSkinData          (NVBMeshInfo& curmesh, CustomArray& importer, udword version);
                bool            ImportTextureVertices   (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportVertexColors      (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportFaces             (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportExtraStuff        (NVBMeshInfo& curmesh, CustomArray& importer);
                bool            ImportConsolidated      (NVBMeshInfo& curmesh, CustomArray& importer, udword version);
                bool            ImportLightingData      (NVBMeshInfo& curmesh, CustomArray& importer);

                bool            ImportCroppingValues    (TextureCrop& cvalues, TextureMatrix& tmtx, CustomArray& importer);
                int             GUIDtoIndex(NVB_GUID *guidID);
    };

#endif // __NVBBREAKER_H__

