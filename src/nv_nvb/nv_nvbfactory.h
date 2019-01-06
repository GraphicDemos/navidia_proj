/*********************************************************************NVMH1****
File:
nv_nvbfactory.h

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef _nv_nvbfactory_h_
#define _nv_nvbfactory_h_

#include "NVBBreaker.h"


extern long gNVB_options;

struct vert_opt;

struct vert_opt
{
    vert_opt() { num_tmaps = 0; v = vec3_null; n = vec3_null; c = vec4_null; t = vec2_null; smg_id = NV_BAD_IDX; face_idx = NV_BAD_IDX; };
    vert_opt(const vert_opt & face);
    ~vert_opt();

    unsigned int    smg_id;
    unsigned int    face_idx;
    vec3            v;
    vec2            t;
    vec3            n;
    vec4            c;
    vec4            weights;
    nv_idx          bones[4];
    vec3            v_offset[4];

    unsigned int    num_tmaps;
    vec3 *          tmaps;
    vert_opt & operator= (const vert_opt & face);
};

typedef std::map<unsigned int,vert_opt> FaceMapType;
typedef FaceMapType::value_type         FaceMapPair;
typedef FaceMapType::iterator           FaceMapIt;

// keep track of duplicated vertices to limit vertex duplication
// when smoothing group defers...
struct idxvert_opt
{
    unsigned int    new_idx;
    vert_opt        face;
};
typedef std::multimap<unsigned int, idxvert_opt>    FaceMMapType;
typedef FaceMMapType::value_type                    FaceMMapPair;
typedef FaceMMapType::iterator                      FaceMMapIt;

typedef struct _mesh
{
    unsigned int    count;
    FaceMapType     face_map;
    FaceMMapType    face_mmap;
} mesh_opt;

typedef std::list<nv_idx>                   IdxType;

typedef std::map<unsigned int,IdxType*>     TriMapType;
typedef TriMapType::value_type              TriMapPair;
typedef TriMapType::iterator                TriMapIt;

typedef std::map<unsigned int,mesh_opt*>    MatFaceMapType;
typedef MatFaceMapType::value_type          MatFaceMapPair;
typedef MatFaceMapType::iterator            MatFaceMapIt;

typedef struct _segmesh
{
    TriMapType      tri_map;
    MatFaceMapType  matface_map;
} segmesh_opt;

typedef std::map<nv_node*,segmesh_opt*>     MeshMapType;
typedef MeshMapType::value_type             MeshMapPair;
typedef MeshMapType::iterator               MeshMapIt;
typedef MeshMapType::const_iterator         MeshMapCIt;

class NVBFactory : public NVBBreaker
{
	public:
	// Constructor/Destructor
										NVBFactory();
	virtual								~NVBFactory();

	virtual			bool				NewScene				(const NVBSceneInfo& scene);
	virtual			bool				NewCamera				(const NVBCameraInfo& camera);
	virtual			bool				NewLight				(const NVBLightInfo& light);
	virtual			bool				NewMaterial				(const NVBMaterialInfo& material);
	virtual			bool				NewTexture				(const NVBTextureInfo& texture);
	virtual			bool				NewMesh					(const NVBMeshInfo& mesh);
	virtual			bool				NewShape				(const NVBShapeInfo& shape);
	virtual			bool				NewHelper				(const NVBHelperInfo& helper);
	virtual			bool				NewController			(const NVBControllerInfo& controller);
	virtual			bool				NewMotion				(const NVBMotionInfo& motion);
        virtual			bool                            EndImport();
    
	virtual			bool				NVBImportError			(const char* errortext, udword errorcode);
	virtual			void				NVBLog					(TLogLevel level, char *fmt, ...);

    bool                                AddMeshModel            (nv_model * model,const NVBMeshInfo& mesh, bool skin = false);
    nv_node *                           SetNode                 (nv_node * node, const NVBBaseInfo & info);
    void                                SetScene                (nv_scene  * scene);

	virtual			void				SetLogCallback(TloggingCB cbfn, unsigned long userparam=0);

	private:

    nv_scene      * _scene;
    unsigned int    _num_new_maps;
    unsigned int    _num_maps;
    
    typedef std::multimap<unsigned int,nv_node*>  IDNodeMapType;
    typedef IDNodeMapType::value_type             IDNodePair;
    typedef IDNodeMapType::iterator               IDNodeMapIt;

    IDNodeMapType   _idnodemap;
    MeshMapType     _skinmeshmap;
    MeshMapType     _meshmap;
	
	// Logging callback. _loguserparam is when the callback need to have an id to know that it comes from the factory
	unsigned long	_loguserparam;
	TloggingCB		_logcb;
};

#endif // _nv_nvbfactory_h_
