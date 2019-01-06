#ifndef __NVB_LOADER_H__
#define __NVB_LOADER_H__

#ifdef WIN32
#include <windows.h>                                                // General purpose Win32 functions
#include <windowsx.h>
#endif

// Standard includes
#ifdef _DEBUG
#include <crtdbg.h>                                                 // C runtime debug functions
#endif
#include <stdio.h>                                                  // Standard Input/Output functions
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>                                                 // Assertion functions
#include <stdarg.h>
#include <time.h>
#include <float.h>
#include <math.h>

#include "nv_nvb_loader_decl.h"
#include "NVBMemoryMacros.h"

#ifdef WIN32
#include <shlobj.h>
#include <mmsystem.h>
#include <direct.h>
#include <io.h>
#endif

#ifdef SUPPORT_COMPRESSION
#include "../zlib/zlib.h"
#include "../bzip2/bzlib.h"
#endif 

// Begin the big hook



    // Cosmetic stuff
    #define override(baseclass) virtual

    // Our own inline keyword, so that:
    // - we can switch to __forceinline to check it's really better or not
    // - we can remove __forceinline if the compiler doesn't support it


    #define FUNCTION
    #define EC_OUTOFMEMORY  0
    #define PIXEL_OPAQUE    0xff

    struct Point
    {
        inline_           Point()                                                     {}
        inline_           Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z)   {}
        inline_           Point(float f[3]) : x(f[0]), y(f[1]), z(f[2])               {}
        inline_           Point(const Point& p) : x(p.x), y(p.y), z(p.z)              {}
        inline_           ~Point()                                                    {}

        inline_   void    Zero()  { x=y=z=0.0f; }

        float x,y,z;
    };

    struct Quat
    {
        inline_   Quat&       Identity()      { p.x = p.y = p.z = 0.0f;   w = 1.0f;   return *this;   }

        Point   p;
        float   w;
    };

    class PR
    {
        public:
        inline_                   PR()    {}
        inline_                   ~PR()   {}

                        PR&             Reset()
                                        {
                                            mPos.Zero();
                                            mRot.Identity();
                                            return *this;
                                        }

                        Point           mPos;
                        Quat            mRot;
    };

    class PRS
    {
        public:
        inline_                   PRS()   {}
        inline_                   ~PRS()  {}

                        PRS&            Reset()
                                        {
                                            mPos.Zero();
                                            mRot.Identity();
                                            mScale = Point(1.0f, 1.0f, 1.0f);
                                            return *this;
                                        }

                        Point           mPos;
                        Quat            mRot;
                        Point           mScale;
    };

    #include "NVBTypes.h"
    #include "NVBFPU.h"


//for FLEXPORTER CARBON-COPY (remove MESHMERIZER_API)
    //! Submesh properties.
    struct MESHMERIZER_API MBSubmesh
    {
        sdword          MatID;                  //!< MaterialID for this submesh
        udword          SmGrp;                  //!< Smoothing groups for this submesh
        udword          NbFaces;                //!< Number of faces in this submesh
        udword          NbVerts;                //!< Number of vertices in this submesh
        udword          NbSubstrips;            //!< Number of strips in this submesh
    };

    //! Material properties.
    struct MESHMERIZER_API MBMatInfo
    {
        sdword          MatID;                  //!< This material's ID
        udword          NbFaces;                //!< Number of faces having this material
        udword          NbVerts;                //!< Related number of exported vertices
        udword          NbSubmeshes;            //!< Number of related submeshes
    };

    //! The topology structure.
    struct MESHMERIZER_API MBTopology
    {
                        MBTopology()            { ZeroMemory(this, SIZEOFOBJECT);   }

        udword          NbFaces;                //!< Total number of faces
        udword          NbSubmeshes;            //!< Total number of submeshes (e.g. 6 for the reference cube)
        udword*         VRefs;                  //!< Vertex references (3 refs for each face)
        udword*         FacesInSubmesh;         //!< Number of faces in each submesh
        float*          Normals;                //!< Face normals
        MBSubmesh*      SubmeshProperties;      //!< NbSubmeshes structures
    };

    //! The geometry structure.
    struct MESHMERIZER_API MBGeometry
    {
                        MBGeometry()            { ZeroMemory(this, SIZEOFOBJECT);   }
        // Basic data
        udword          NbGeomPts;              //!< Number of vertices in the original mesh
        udword          NbTVerts;               //!< Number of mapping coordinates in the original mesh
        udword          NbColorVerts;           //!< Number of vertex-colors in the original mesh
        //
        udword          NbVerts;                //!< Number of vertices in the final mesh (some may have been duplicated) = sum of all NbVerts in MBSubmeshes
        // Indnvbs
        udword*         VertsRefs;              //!< Vertex indnvbs (only !=null if mIndexedGeo, else vertices are duplicated). Index in Verts.
        udword*         TVertsRefs;             //!< UVW indnvbs (only !=null if mIndexedUVW, else UVWs are duplicated). Index in TVerts.
        udword*         ColorRefs;              //!< Vertex-color indnvbs (only !=null if mIndexedColors, else colors are duplicated). Index in CVerts.
        // Vertex data
        float*          Verts;                  //!< List of vertices, may be duplicated or not
        float*          TVerts;                 //!< List of UV(W) mappings, may be duplicated or not.
        float*          CVerts;                 //!< List of vertex colors, may be duplicated or not.
        float*          Normals;                //!< Vertex normals. Can't be indexed.
        udword          NormalInfoSize;         //!< Size of the NormalInfo field (in number of udwords, not in bytes)
        udword*         NormalInfo;             //!< Information used to rebuild normals in realtime. See below.
    };

    // More about NormalInfo:
    //
    // NormalInfo contains some extra information used to rebuild vertex-normals in realtime, by averaging
    // a number of face-normals. Each vertex-normal depends on a various number of face-normals. The exact
    // number depends on the mesh topology, but also depends on the smoothing groups.
    //
    // NormalInfo contains data to rebuild one normal/vertex, ie to rebuild NbVerts normals.
    // Format is, for each vertex:
    //      udword      Count               a number of faces
    //      udword      Ref0, Ref1,...      a list of Count face indnvbs
    //
    // To rebuild vertex-normals in realtime you need to:
    // 1) Rebuild all face-normals (which is easy)
    // 2) For each vertex, add Count face-normals according to NormalInfo, then normalize the summed vector.
    //
    // Other techniques exist, of course.

    //! The materials structure.
    struct MESHMERIZER_API MBMaterials
    {
                        MBMaterials() : NbMtls(0), MaterialInfo(null)   {}

        udword          NbMtls;                 //!< Number of materials found.
        MBMatInfo*      MaterialInfo;           //!< One structure for each material.
    };

    //! Result structure.
    struct MESHMERIZER_API MBResult
    {
        MBTopology      Topology;               //!< Result topology.
        MBGeometry      Geometry;               //!< Result geometry
        MBMaterials     Materials;              //!< Result materials
    };

    //
    // Pseudo-code showing how to use the consolidation and the striper result structures:
    // mVB is a DX7 vertex buffer filled thanks to the MBGeometry structure.
    // mResult is the MBResult structure.
    // mStripRuns and mStripLengths are from the STRIPERRESULT structure.
    //
    //  // Get indnvbs
    //  uword* VRefs = mResult->Topology.VRefs;
    //  // Make one API call / material
    //  for(i=0;i<mResult->Materials.NbMtls;i++)
    //  {
    //      // Select correct material
    //      udword MaterialID = mResult->Materials.MaterialInfo[i].MatID;
    //      // Draw mesh
    //      if(mStripRuns)  renderer->DrawIndexedPrimitiveVB(PRIMTYPE_TRISTRIP, mVB, 0, mResult->Geometry.NbVerts, mStripRuns[i], mStripLengths[i]);
    //      else            renderer->DrawIndexedPrimitiveVB(PRIMTYPE_TRILIST, mVB, 0, mResult->Geometry.NbVerts, VRefs, mResult->Materials.MaterialInfo[i].NbFaces*3);
    //      // Update index pointer for trilists
    //      VRefs+=mResult->Materials.MaterialInfo[i].NbFaces*3;
    //  }
    //

//~for FLEXPORTER CARBON-COPY


#endif
