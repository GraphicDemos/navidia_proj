///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  This file contains all NVB format-related constants.
 *  \file       NVBFormat.h
 *  \author     Kenneth Hurley
 *  \date       April, 1, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __NVBFORMAT_H__
#define __NVBFORMAT_H__

#include <windows.h>
#ifdef WIN32
typedef GUID NVB_GUID;
#else
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} NVB_GUID;
#endif
    // chunk GUIDS
    // {D543B753-5A58-4da0-819B-38574E51E1D7}
    static const NVB_GUID guidMain = 
    { 0xd543b753, 0x5a58, 0x4da0, { 0x81, 0x9b, 0x38, 0x57, 0x4e, 0x51, 0xe1, 0xd7 } };
    // {6C61F765-9B85-4ead-8326-2B687BB48F3F}
    static const NVB_GUID guidMesh = 
    { 0x6c61f765, 0x9b85, 0x4ead, { 0x83, 0x26, 0x2b, 0x68, 0x7b, 0xb4, 0x8f, 0x3f } };
    // {50E53BDB-F91E-4ce8-9C56-2F42A9942EC6}
    static const NVB_GUID guidCams = 
    { 0x50e53bdb, 0xf91e, 0x4ce8, { 0x9c, 0x56, 0x2f, 0x42, 0xa9, 0x94, 0x2e, 0xc6 } };
    // {91904FC6-54C1-4057-81AB-2FFA42483523}
    static const NVB_GUID guidLights = 
    { 0x91904fc6, 0x54c1, 0x4057, { 0x81, 0xab, 0x2f, 0xfa, 0x42, 0x48, 0x35, 0x23 } };
    // {9A3C20F8-F992-4910-8D9B-F60BF2666EEA}
    static const NVB_GUID guidShapes = 
    { 0x9a3c20f8, 0xf992, 0x4910, { 0x8d, 0x9b, 0xf6, 0xb, 0xf2, 0x66, 0x6e, 0xea } };
    // {A9DEAADE-970B-4639-BF81-7153467233A3}
    static const NVB_GUID guidHelpers = 
    { 0xa9deaade, 0x970b, 0x4639, { 0xbf, 0x81, 0x71, 0x53, 0x46, 0x72, 0x33, 0xa3 } };
    // {F63BD6CF-440B-48a8-92EF-51CFF01DFE9C}
    static const NVB_GUID guidTextures = 
    { 0xf63bd6cf, 0x440b, 0x48a8, { 0x92, 0xef, 0x51, 0xcf, 0xf0, 0x1d, 0xfe, 0x9c } };
    // {DAF937F2-6E9B-4993-9B10-77D4FBCF8C3C}
    static const NVB_GUID guidMaterials = 
    { 0xdaf937f2, 0x6e9b, 0x4993, { 0x9b, 0x10, 0x77, 0xd4, 0xfb, 0xcf, 0x8c, 0x3c } };
    // {E2D32CC8-03F5-4a00-B30C-C8DE0F78FE65}
    static const NVB_GUID guidControllers = 
    { 0xe2d32cc8, 0x3f5, 0x4a00, { 0xb3, 0xc, 0xc8, 0xde, 0xf, 0x78, 0xfe, 0x65 } };
    // {169D9C8F-83BE-4b9f-AD58-4D60B8752BA4}
    static const NVB_GUID guidMovers = 
    { 0x169d9c8f, 0x83be, 0x4b9f, { 0xad, 0x58, 0x4d, 0x60, 0xb8, 0x75, 0x2b, 0xa4 } };
    
    // Chunk versions
    #define CHUNK_MAIN_VER          2   
    #define CHUNK_MESH_VER          7
    #define CHUNK_CAMS_VER          2
    #define CHUNK_LITE_VER          3
    #define CHUNK_SHAP_VER          2
    #define CHUNK_HELP_VER          2
    #define CHUNK_TEXM_VER          2
    #define CHUNK_MATL_VER          2
    #define CHUNK_CTRL_VER          2
    #define CHUNK_MOVE_VER          4

    // Mesh flags.
    #define NVB_VFACE               (1<<0)              //!< Mesh has vertex-faces.
    #define NVB_TFACE               (1<<1)              //!< Mesh has texture-faces.
    #define NVB_CFACE               (1<<2)              //!< Mesh has color-faces.
    #define NVB_UVW                 (1<<3)              //!< UVW's are exported
    #define NVB_WDISCARDED          (1<<4)              //!< W is discarded
    #define NVB_VERTEXCOLORS        (1<<5)              //!< Vertex colors are exported
    #define NVB_ONEBONEPERVERTEX    (1<<6)              //!< Simple skin with one driving bone/vertex
    #define NVB_CONVEXHULL          (1<<7)              //!< The convex hull has been exported
    #define NVB_BOUNDINGSPHERE      (1<<8)              //!< The bounding sphere has been exported
    #define NVB_INERTIATENSOR       (1<<9)              //!< The inertia tensor has been exported
    #define NVB_QUANTIZEDVERTICES   (1<<10)             //!< Vertices have been quantized
    #define NVB_WORDFACES           (1<<11)             //!< Vertex references within faces are stored as words instead of dwords
    #define NVB_COMPRESSED          (1<<12)             //!< Mesh has been saved in a compression-friendly way
    #define NVB_EDGEVIS             (1<<13)             //!< Edge visibility has been exported

    #define NVB_CONSOLIDATION       (1<<16)             //!< Mesh has been consolidated
    #define NVB_FACENORMALS         (1<<17)             //!< Export normals to faces
    #define NVB_VERTEXNORMALS       (1<<18)             //!< Export normals to Vertices
    #define NVB_NORMALINFO          (1<<19)             //!< Export NormalInfo

    // Scene flags
    enum NVBFileType
    {
        NVB_FILE_SCENE              = 0x00001000,       //!< Complete 3D scene
        NVB_FILE_MOTION             = 0x00001100,       //!< Motion file
        NVB_FILE_FORCE_DWORD        = 0x7fffffff
    };

    // Object types
    enum NVBObjType
    {
        NVB_OBJ_UNDEFINED           = 0,                //!< Undefined object
        NVB_OBJ_CAMERA              = 1,                //!< A camera
        NVB_OBJ_LIGHT               = 2,                //!< A light
        NVB_OBJ_MESH                = 3,                //!< A mesh
        NVB_OBJ_BPATCH              = 4,                //!< A b-patch
        NVB_OBJ_CONTROLLER          = 5,                //!< A controller
        NVB_OBJ_HELPER              = 6,                //!< A helper
        NVB_OBJ_MATERIAL            = 7,                //!< A material
        NVB_OBJ_TEXTURE             = 8,                //!< A texture
        NVB_OBJ_MOTION              = 9,                //!< A character motion
        NVB_OBJ_SHAPE               = 10,               //!< A shape
        NVB_OBJ_FORCE_DWORD         = 0x7fffffff
    };

    // Controllers flags
    enum NVBCtrlType
    {
        NVB_CTRL_NONE               = 0,                //!< No controller
        NVB_CTRL_FLOAT              = 1,                //!< Float controller
        NVB_CTRL_VECTOR             = 2,                //!< Vector controller
        NVB_CTRL_QUAT               = 3,                //!< Quaternion controller
        NVB_CTRL_SCALE              = 4,                //!< Scale controller
        NVB_CTRL_PR                 = 5,                //!< PR controller
        NVB_CTRL_PRS                = 6,                //!< PRS controller
        NVB_CTRL_VERTEXCLOUD        = 7,                //!< Morph controller
        NVB_CTYPE_FORCE_DWORD       = 0x7fffffff
    };

    enum NVBCtrlMode
    {
        NVB_CTRL_SAMPLES            = 1,                //!< Samples
        NVB_CTRL_KEYFRAMES          = 2,                //!< Keyframes
        NVB_CTRL_PROCEDURAL         = 3,                //!< Procedural
        NVB_CMODE_FORCE_DWORD       = 0x7fffffff
    };

    // Compression flags
    enum NVBCompression
    {
        NVB_COMPRESSION_NONE        = 0,                //!< Not compressed
        NVB_COMPRESSION_ZLIB        = 1,                //!< Compressed with ZLib
        NVB_COMPRESSION_BZIP2       = 2,                //!< Compressed with BZip2  
        NVB_COMPRESSION_CUSTOM      = 3,                //!< Custom compression
        NVB_COMPRESSION_FORCE_DWORD = 0x7fffffff
    };

#endif // __NVBFORMAT_H__
