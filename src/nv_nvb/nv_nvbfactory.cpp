/*********************************************************************NVMH1****
File:
nv_nvbfactory.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#include "stdafx.h"
#include <nvIO/NVMeshMender.h>

long gNVB_options = 0;

struct bookeep_tex
{
    bookeep_tex() { new_channel = NV_BAD_IDX; };
    nv_idx  base_channel;
    mat4    tex_mat;
    vec2    uv_scale;
    vec2    uv_offset;
    nv_idx  new_channel;
};

bookeep_tex* bk_texs = 0;

typedef std::list<bookeep_tex>  coord_type;
typedef coord_type::iterator    coord_it;

bool operator==(const bookeep_tex & a, const bookeep_tex & b)
{
    if (memcmp(&a,&b,sizeof(bookeep_tex)) == 0)
        return true;
    return false;
}

vert_opt::vert_opt(const vert_opt & face)
{
    num_tmaps = 0; 
    *this = face;
}

vert_opt::~vert_opt()
{
    if (num_tmaps)
    {
        delete [] tmaps;
        num_tmaps = 0;
        tmaps = 0;
    }
}

vert_opt & vert_opt::operator= (const vert_opt & face)
{
    face_idx = face.face_idx;
    smg_id = face.smg_id;
    v = face.v;
    t = face.t;
    n = face.n;
    c = face.c;
    weights = face.weights;
    bones[0] = face.bones[0];
    bones[1] = face.bones[1];
    bones[2] = face.bones[2];
    bones[3] = face.bones[3];
    v_offset[0] = face.v_offset[0];
    v_offset[1] = face.v_offset[1];
    v_offset[2] = face.v_offset[2];
    v_offset[3] = face.v_offset[3];
    if(num_tmaps)
        delete [] tmaps;
    num_tmaps = face.num_tmaps;
    if(num_tmaps)
        tmaps = new vec3[num_tmaps];
    for (unsigned int i = 0; i < num_tmaps; ++i)
    {
        tmaps[i] = face.tmaps[i];
    }
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This factory is used to make the render manager independent from NVB files. This is an interface between the NVB breaker and the app.
 *	Theoretically the factory is called by the NVB breaker, fills some app-dependent creation structure, then call the generic object creation
 *	methods from the app. The other way would have been to support NVB structures natively in the app, which is possible but not generic enough
 *	and probably not user-friendly (some may want to use the render manager with their own data format). This design is cleaner since it ensures
 *	the app HASA NVB breaker, whereas the older design ensures the app ISA NVB breaker.
 *
 *	\class		NVBFactory
 *	\author		Pierre Terdiman
 *	\version	1.0
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBFactory::NVBFactory()
{
    _scene = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NVBFactory::~NVBFactory()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Overriden error handler.
 *	\param		errortext	[in] text message
 *	\param		errorcode	[in] error code
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NVBImportError(const char* errortext, udword errorcode)
{
	NVBLog(LOG_ERR, "%s\n", errortext);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Overriden log handler.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NVBFactory::NVBLog(TLogLevel level, char *fmt, ...)
{
	static char buff[1024];

	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va); 

	if(_logcb) 
	{
		_logcb(buff, level, _loguserparam);
	}
	else
	{
		switch(level)
		{
		case LOG_MSG:
			printf("LOG: ");
			break;
		case LOG_WARN:
			printf("LOG WARNING : ");
			break;
		case LOG_ERR:
			printf("LOG ERROR : ");
			break;
		}
		printf((const char*)buff);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new scene.
 *	\param		scene		[in] the new scene info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewScene(const NVBSceneInfo& scene)
{
	NVBLog(LOG_MSG, "Importing new scene...\n");

    // models allocation
    unsigned int num_nodes = scene.mNbMeshes + scene.mNbDerived + scene.mNbLights + scene.mNbCameras + scene.mNbHelpers;
    if (num_nodes)
        _scene->nodes = new nv_node*[num_nodes];
    _scene->num_nodes = 0;
    // textures allocation
    if (scene.mNbTextures)
        _scene->textures = new nv_texture[scene.mNbTextures];
    _scene->num_textures = 0;
    // material allocation
    if (scene.mNbMaterials)
        _scene->materials = new nv_material[scene.mNbMaterials];
    _scene->num_materials = 0;

    // book keeping for materials...
    if (scene.mNbTextures)
        bk_texs = new bookeep_tex[scene.mNbTextures];

    _scene->ambient.x = scene.mAmbientColor.x;
    _scene->ambient.y = scene.mAmbientColor.y;
    _scene->ambient.z = scene.mAmbientColor.z;
    _scene->ambient.w = nv_one;

    _num_new_maps = 0;
    _num_maps = scene.mNbTextures;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to assign BaseInfo properties to a node.
 *	\param		node		[out] the node to be set
*	\param		info		[in] the BaseInfo to be used to set the node
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
nv_node * NVBFactory::SetNode(nv_node * node, const NVBBaseInfo & info)
{
    _scene->nodes[_scene->num_nodes] = node;

    node->name = new char[strlen(info.mName) + 1 ];
    strcpy(node->name, info.mName);

    _idnodemap.insert(IDNodePair(info.mID,node));

    node->parent = nv_idx(info.mParentID);
    node->target = nv_idx(info.mLinkID);
    
    // xforms...
	vec3 pos( info.mPrs.mPos.x, info.mPrs.mPos.z, nv_zero);
	quat rot( info.mPrs.mRot.p.x, info.mPrs.mRot.p.z, nv_zero, nv_zero);
	if (gNVB_options & NVB_RHS)
	{
		pos.z = -info.mPrs.mPos.y;
		rot.z = -info.mPrs.mRot.p.y;
		rot.w = info.mPrs.mRot.w;
	}
	else if(gNVB_options & NVB_LHS)
	{
		pos.z = info.mPrs.mPos.y;
		rot.z = info.mPrs.mRot.p.y;
		rot.w = -info.mPrs.mRot.w;
	}
    quat rot_prime;
    conj(rot_prime, rot);
	node->xform.set_rot(rot_prime);
	node->xform.set_translation(pos);
    _scene->num_nodes++;

    node->attr = info.mAttr;
    return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new camera.
 *	\param		camera		[in] the new camera info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewCamera(const NVBCameraInfo& camera)
{
	NVBLog(LOG_MSG, "Importing new camera...\n");
    
    nv_camera* cam = reinterpret_cast<nv_camera*>(SetNode(nv_factory::get_factory()->new_camera(), camera));
    
    cam->fov = camera.mFOV;
    cam->focal_length = camera.mTDist; // not totally sure!
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new controller.
 *	\param		controller		[in] the new controller info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewController(const NVBControllerInfo& controller)
{
    unsigned int i;
    nv_node * node = NULL;

	NVBLog(LOG_MSG, "Importing new controller: ");

    IDNodeMapIt it = _idnodemap.find(controller.mOwnerID);
    if (it != _idnodemap.end())
        node = (*it).second;
    else
        return false;

    nv_animation & anim = node->anim;
    if (controller.mCtrlMode == NVB_CTRL_SAMPLES &&
        (controller.mCtrlType == NVB_CTRL_PR || controller.mCtrlType == NVB_CTRL_PRS))
    {
        NVBLog(LOG_MSG, "%s\n",node->name);
        anim.num_keys = controller.mNbSamples;
        anim.freq = static_cast<nv_scalar>(controller.mSamplingRate);
        anim.pos = new vec3[anim.num_keys];
        anim.rot = new quat[anim.num_keys];
        
        _scene->num_keys = anim.num_keys;
        
        if (controller.mCtrlType == NVB_CTRL_PRS)
            anim.scale = new vec3[anim.num_keys];

        float* data = (float*)controller.mSamples;

        for (i = 0; i < anim.num_keys; ++i)
        {
            quat rot;
            anim.pos[i].x = *data++;
            if (gNVB_options & NVB_RHS)
                anim.pos[i].z = -*data++;
            else if (gNVB_options & NVB_LHS)
                anim.pos[i].z = *data++;

			anim.pos[i].y = *data++;
            rot.x = *data++;
            if (gNVB_options & NVB_RHS)
			{
                rot.z = -*data++;
				rot.y = *data++;
				rot.w = *data++;
			}
            else if (gNVB_options & NVB_LHS)
			{
                rot.z = *data++;
				rot.y = *data++;
				rot.w = -*data++;
			}
            
            conj(anim.rot[i],rot);

            if (controller.mCtrlType == NVB_CTRL_PRS)
            {
                anim.scale[i].x = *data++;
                if (gNVB_options & NVB_RHS)
                    anim.scale[i].z = -*data++;
                else if (gNVB_options & NVB_LHS)
                    anim.scale[i].z = *data++;
                anim.scale[i].y = *data++;
            }
        }
    }
    else
        NVBLog(LOG_WARN, "<ignored>\n");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new helper.
 *	\param		helper		[in] the new helper info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewHelper(const NVBHelperInfo& helper)
{
	NVBLog(LOG_MSG, "Importing new");
    if (_scene && helper.mHelperType == HTYPE_DUMMY)
    {
        nv_node* node = SetNode(nv_factory::get_factory()->new_node(), helper);

        NVBLog(LOG_MSG, " (helper) bone: %s\n", node->name);
        assert(helper.mID != -1);
        return true;
    }
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new light.
 *	\param		light		[in] the new light info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewLight(const NVBLightInfo& light)
{
	NVBLog(LOG_MSG, "Importing new light...\n");

    nv_light* l = reinterpret_cast<nv_light*>(SetNode(nv_factory::get_factory()->new_light(), light));

	if (light.mAffectDiffuse)
	{
		l->color.x = light.mColor.x;
		l->color.y = light.mColor.y;
		l->color.z = light.mColor.z;
		l->color.w = nv_one;
	}
	else
	{
		l->color = vec4_null;
	}
    
	// Set the specular to be the same as the diffuse, if affecting
    if (light.mAffectSpecular)
		l->specular = l->color;
	else 
		l->specular = vec4_null;

    l->ambient = vec4_null;

    l->specular_exp = light.mIntensity;
    // parsing the type...
    // TODO: interpret the fucking params so that they
    // make sense to nv_light...
    switch (light.mLightType)
    {
        default:
        case LTYPE_OMNI:
            l->light = nv_light::POINT;
            break;
        case LTYPE_DIR:
            l->light = nv_light::DIRECTIONAL;
            break;
        case LTYPE_FSPOT:
        case LTYPE_TSPOT:
            l->light = nv_light::SPOT;
            l->phi = light.mFallsize;
            l->theta = light.mHotSpot;
            break;
    };


	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new material.
 *	\param		material		[in] the new material info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewMaterial(const NVBMaterialInfo& material)
{
	NVBLog(LOG_MSG, "Importing new material...\n");

    unsigned int i;
    nv_material & mat = _scene->materials[_scene->num_materials];

    mat.name = new char[strlen(material.mName.Get()) + 1];
    strcpy(mat.name, material.mName.Get());

    mat.id = material.mID;

    // textures...
    std::vector<nv_idx> tex_idx;

    if (material.mAmbientMapID != -1)
    {
        tex_idx.push_back(material.mAmbientMapID);
        _scene->textures[material.mAmbientMapID].type = nv_texture::AMBIENT;
    }

    if (material.mDiffuseMapID != -1)
    {
        tex_idx.push_back(material.mDiffuseMapID);
        _scene->textures[material.mDiffuseMapID].type = nv_texture::DIFFUSE;
    }
        
    if (material.mSpecularMapID != -1)
    {
        tex_idx.push_back(material.mSpecularMapID);
        _scene->textures[material.mSpecularMapID].type = nv_texture::SPECULAR;
    }
        
    if (material.mShiningStrengthMapID != -1)
    {
        tex_idx.push_back(material.mShiningStrengthMapID);
        _scene->textures[material.mShiningStrengthMapID].type = nv_texture::SPECULAR_POWER;
    }
        
    if (material.mBumpMapID != -1)
    {
        tex_idx.push_back(material.mBumpMapID);
        _scene->textures[material.mBumpMapID].type = nv_texture::BUMP;

        nv_idx new_idx = _num_new_maps + _num_maps;
        nv_texture * textures = new nv_texture[new_idx + 1];
        bookeep_tex * bk_textures = new bookeep_tex[new_idx + 1];
        for (i = 0; i < new_idx; ++i)
        {
            textures[i] = _scene->textures[i];
            bk_textures[i] = bk_texs[i];
        }

        tex_idx.push_back(new_idx);

        textures[new_idx] = textures[material.mBumpMapID];
        bk_textures[new_idx] = bk_texs[material.mBumpMapID];
        
        std::string normal_map = textures[new_idx].name;
        std::string::size_type pos = normal_map.rfind(".");
        normal_map.insert(pos,"_normal");
        delete [] textures[new_idx].name;
        textures[new_idx].name = new char[strlen(normal_map.c_str()) + 1];
        strcpy(textures[new_idx].name, normal_map.c_str());
        textures[new_idx].type = nv_texture::NORMAL;
        if (_num_maps + _num_new_maps)
        {
            delete [] _scene->textures;
            delete [] bk_texs;
        }
            
        _scene->textures = textures;
        _scene->num_textures = new_idx + 1;
        // reflect the addition to the book_tex
        bk_texs = bk_textures;
        _num_new_maps++;        
    }
        
    if (material.mSelfIllumMapID != -1)
    {
        tex_idx.push_back(material.mSelfIllumMapID);
        _scene->textures[material.mSelfIllumMapID].type = nv_texture::SELF_ILLUMATION;
    }

    if (material.mOpacityMapID != -1)
    {
        tex_idx.push_back(material.mOpacityMapID);
        _scene->textures[material.mOpacityMapID].type = nv_texture::OPACITY;
    }

    if (material.mReflectionMapID != -1)
    {
        tex_idx.push_back(material.mReflectionMapID);
        _scene->textures[material.mReflectionMapID].type = nv_texture::REFLECTION;
    }

    if (material.mRefractionMapID != -1)
    {
        tex_idx.push_back(material.mRefractionMapID);
        _scene->textures[material.mRefractionMapID].type = nv_texture::REFRACTION;
    }

    mat.num_textures = tex_idx.size();
    mat.tex_channel = new nv_idx[mat.num_textures];
    memset(mat.tex_channel, NV_BAD_IDX, mat.num_textures * sizeof(nv_idx));
    mat.textures = new nv_idx[mat.num_textures];
    for (i = 0; i < mat.num_textures; ++i)
        mat.textures[i] = tex_idx[i];

    // material colors...
    mat.ambient[0] = material.mMtlAmbientColor.x;
    mat.ambient[1] = material.mMtlAmbientColor.y;
    mat.ambient[2] = material.mMtlAmbientColor.z;
    mat.ambient[3] = nv_one;

    mat.diffuse[0] = material.mMtlDiffuseColor.x;
    mat.diffuse[1] = material.mMtlDiffuseColor.y;
    mat.diffuse[2] = material.mMtlDiffuseColor.z;
    mat.diffuse[3] = nv_one;

    mat.specular[0] = material.mMtlSpecularColor.x;
    mat.specular[1] = material.mMtlSpecularColor.y;
    mat.specular[2] = material.mMtlSpecularColor.z;
    mat.specular[3] = nv_one;

    mat.emission[0] = nv_zero;
    mat.emission[1] = nv_zero;
    mat.emission[2] = nv_zero;
    mat.emission[3] = nv_one;

    mat.shininess = material.mShininess;

    mat.transparent = material.mOpacity;

    // transparency
    if (mat.transparent < nv_one)
        mat.diffuse[3] = mat.transparent;

    // fog properties...
    mat.fog = false;
    mat.fog_color = vec4_one;
    _scene->num_materials++;

    mat.attr = material.mAttr;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new mesh.
 *  \param      model       [out] skinned model node
 *	\param		mesh		[in] the new mesh info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct w_offset
{
    vec3        offset;
    nv_scalar   weight;
    nv_idx      bone_id;
};

bool heavier(const w_offset & a, const w_offset & b)
{
    if (a.weight > b.weight )
        return true;
    return false;
}

static bool is_matching(const vert_opt & a, const vert_opt & b)
{
    unsigned int j;
    bool tex_test = true;
    bool map_test = true;
    bool col_test = true;
    bool smg_test = true;

    if (a.smg_id != b.smg_id)
        smg_test = false;

    if (a.t != b.t)
        tex_test = false;

    if (a.c != b.c)
        col_test = false;

    if (a.num_tmaps != b.num_tmaps)
        map_test = false;
    else
    {
        for (j = 0; j < a.num_tmaps && map_test == true; ++j)
        {
            if (a.tmaps[j] != b.tmaps[j])
                map_test = false;
        }
    }
    return tex_test && map_test && col_test && smg_test;
}


bool NVBFactory::AddMeshModel(nv_model * model,const NVBMeshInfo& mesh, bool skin)
{
    unsigned int i,j,k;
    mesh_opt * m_opt;
 
    MeshMapIt it;
    if (skin)
    {
        it = _skinmeshmap.find(reinterpret_cast<nv_node*>(model));
        assert(it == _skinmeshmap.end());
    }
    else
    {
        it = _meshmap.find(reinterpret_cast<nv_node*>(model));
        assert(it == _meshmap.end());
    }

    segmesh_opt * opt = new segmesh_opt;
    if (skin)
        _skinmeshmap.insert(MeshMapPair(model,opt));
    else
        _meshmap.insert(MeshMapPair(model,opt));
    TriMapType      & tri_map = opt->tri_map;
    MatFaceMapType  & matface_map = opt->matface_map;

    unsigned int max_face_idx = 0;
    for (i =0; i < mesh.mNbFaces*3; ++i)
    {
        unsigned int face_idx = mesh.mFaces[i];
        if (max_face_idx < face_idx)
            max_face_idx = face_idx;
    }

    for (i =0; i < mesh.mNbFaces*3; ++i)
    {
        unsigned int mat_id = mesh.mFaceProperties[i / 3].MatID;
        unsigned int smg_id = mesh.mFaceProperties[i / 3].Smg;

        // lets sort by material and find the corresponding mesh_opt
        MatFaceMapIt it_matfacemap = matface_map.find(mat_id);
        if (it_matfacemap == matface_map.end())
        {
            m_opt = new mesh_opt;
            m_opt->count = 0;
            matface_map.insert(MatFaceMapPair(mat_id,m_opt));
        }
        else
            m_opt = (*it_matfacemap).second;

        unsigned int idx;
        unsigned int ori_face_idx = mesh.mFaces[i];
        unsigned int face_idx = ori_face_idx;
        FaceMapIt it_face_map = m_opt->face_map.find(face_idx);
        vert_opt face;
        bool create_face = false;

        // build the face as expected
        face.smg_id = smg_id;
        if (mesh.mNbTVerts)
        {
            idx = mesh.mTFaces[i];
            if (idx != NV_BAD_IDX)
            {
                face.t.x = mesh.mTVerts[ idx ].x;
                face.t.y = mesh.mTVerts[ idx ].y;
            }
        }
        if (mesh.mNbCVerts)
        {
            idx = mesh.mCFaces[i];
            face.c.x = mesh.mCVerts[ idx ].x;
            face.c.y = mesh.mCVerts[ idx ].y;
            face.c.z = mesh.mCVerts[ idx ].z;
			face.c.w = nv_one;
        }
		else
			face.c = vec4_one;
        face.num_tmaps = mesh.mNbTMaps;
        if (face.num_tmaps)
        {
            face.tmaps = new vec3[face.num_tmaps];
            for (j = 0; j < face.num_tmaps; ++j)
            {
                if (mesh.mNbTMapVerts[j])
                {
                    idx = mesh.mTMapFaces[j][i];
                    if (nv_idx(idx) != NV_BAD_IDX)
                    {
                        face.tmaps[j].x = mesh.mTMaps[j][idx].x;
                        face.tmaps[j].y = mesh.mTMaps[j][idx].y;
                        face.tmaps[j].z = mesh.mTMaps[j][idx].z;
                    }
                    else
                        face.tmaps[j] = vec3_null;
                }
            }
        }
        if (it_face_map == m_opt->face_map.end())
        {
            create_face = true;
        }
        else
        {
            if (is_matching((*it_face_map).second,face) == false)
            {
                std::pair<FaceMMapIt,FaceMMapIt> pair_mmap = m_opt->face_mmap.equal_range(ori_face_idx);
                FaceMMapIt it_face_mmap = pair_mmap.first;
                bool found = false;
                while (it_face_mmap != pair_mmap.second && found == false)
                {
                    idxvert_opt & idxface = (*it_face_mmap).second;
                    if (is_matching(idxface.face,face))
                    {
                        face_idx = idxface.new_idx;
                        found = true;
                    }
                    ++it_face_mmap;
                }
                if (found == false)
                {
                    create_face = true;
                    ++max_face_idx;
                    face_idx = max_face_idx;
                }
            }
        }
            
        if (create_face)
        {
            if (skin)
            {
                if (mesh.mBonesNb)
                {
                    unsigned int sum = 0;

                    for (k = 0; k < ori_face_idx; ++k)
                        sum += mesh.mBonesNb[k];

                    std::vector<w_offset>   w_offsets;
                    for (k = 0; k < mesh.mBonesNb[ori_face_idx];++k)
                    {
                        if (mesh.mWeights[sum + k] > nv_eps)
                        {
                            w_offset w;

                            w.offset.x = mesh.mOffsetVectors[sum + k].x;
                            w.offset.y = mesh.mOffsetVectors[sum + k].z;
                            if (gNVB_options & NVB_RHS)
                                w.offset.z = -mesh.mOffsetVectors[sum + k].y;
                            else if (gNVB_options & NVB_LHS)
                                w.offset.z = mesh.mOffsetVectors[sum + k].y;

                            w.weight = mesh.mWeights[sum + k];
                            w.bone_id = mesh.mBonesLocalID[sum + k];
                            w_offsets.push_back(w);
                        }
                    }

                    std::sort(w_offsets.begin(), w_offsets.end(), heavier);

                    for (k = 0; k < w_offsets.size() && k < 4; ++k)
                    {
                        w_offset & w = w_offsets[k];
                        face.v_offset[k] = w.offset;
                        face.weights[k] = w.weight;
                        face.bones[k] = w.bone_id;
                    }
                    if (w_offsets.size() > 4)
                        NVBLog(LOG_ERR, "Too many bone refs %d\n",mesh.mBonesNb[ori_face_idx]);
                    if (w_offsets.size() == 0)
                        NVBLog(LOG_ERR, "Missing bone refs %d\n",mesh.mBonesNb[ori_face_idx]);
                }
                else
                {
                    face.v_offset[0].x = mesh.mOffsetVectors[ori_face_idx].x;
                    face.v_offset[0].y = mesh.mOffsetVectors[ori_face_idx].z;
                    if (gNVB_options & NVB_RHS)
                        face.v_offset[0].z = -mesh.mOffsetVectors[ori_face_idx].y;
                    else if (gNVB_options & NVB_LHS)
                        face.v_offset[0].z = mesh.mOffsetVectors[ori_face_idx].y;
                    face.bones[0] = mesh.mBonesLocalID[ori_face_idx];
                    face.weights[0] = nv_one;
                    k = 1;
                }
            
                for (; k < 4; ++k)
                {
                    face.weights[k] = nv_zero;
                    face.bones[k] = NV_BAD_IDX;
                }

                // check for valid weights...
                nv_scalar w_sum = nv_zero;
                for (k = 0; k < 4; ++k)
                {
                    w_sum += face.weights[k];
                }
                if ((w_sum < nv_one - nv_eps) || (w_sum > nv_one + nv_eps))
                {
                    NVBLog(LOG_WARN, "unnormalized weigths - renormalizing...\n");
                    for (k = 0; k < 4; ++k)
                        face.weights[k] /= w_sum;
                }
            }
            else
            {
                face.v.x = mesh.mVerts[ ori_face_idx ].x;
                face.v.y = mesh.mVerts[ ori_face_idx ].z;
                if (gNVB_options & NVB_RHS)
                    face.v.z = -mesh.mVerts[ ori_face_idx ].y;
                else if (gNVB_options & NVB_LHS)
                    face.v.z = mesh.mVerts[ ori_face_idx ].y;
            }

            // add the vertex and store its new idx
            face.face_idx = m_opt->count;
            m_opt->face_map.insert(FaceMapPair(face_idx,face));
            if (ori_face_idx != face_idx)
            {
                // store the newly created and duplicated independently
                idxvert_opt idxface;
                idxface.face = face;
                idxface.new_idx = face_idx;
                m_opt->face_mmap.insert(FaceMMapPair(ori_face_idx,idxface));
            }
            m_opt->count++;
        }
        // add the face indices...
        TriMapIt it = tri_map.find(mat_id);
        if (it != tri_map.end())
        {
            (*it).second->push_back(face_idx);
        }
        else
        {
            IdxType * idx_type = new IdxType;
            idx_type->push_back(face_idx);
            tri_map.insert(TriMapPair(mat_id,idx_type));
        }
    }

    if (skin)
        return true;

    //compute normals...
    vec3 face_n;
    vec3 edge0;
    vec3 edge1;

    for (i =0; i < mesh.mNbFaces; ++i)
    {
        unsigned int mat_id = mesh.mFaceProperties[i].MatID;
        unsigned int smg_id = mesh.mFaceProperties[i].Smg;

        // lets sort by material and find the corresponding mesh_opt
        MatFaceMapIt it_matfacemap = matface_map.find(mat_id);
        assert(it_matfacemap != matface_map.end());
        m_opt = (*it_matfacemap).second;
        unsigned int face0_idx = mesh.mFaces[i*3];
        unsigned int face1_idx = mesh.mFaces[i*3 + 1];
        unsigned int face2_idx = mesh.mFaces[i*3 + 2];

        FaceMapIt it_face0_map = m_opt->face_map.find(face0_idx);
        assert(it_face0_map != m_opt->face_map.end());

        FaceMapIt it_face1_map = m_opt->face_map.find(face1_idx);
        assert(it_face1_map != m_opt->face_map.end());

        FaceMapIt it_face2_map = m_opt->face_map.find(face2_idx);
        assert(it_face2_map != m_opt->face_map.end());

        vert_opt & face0 = (*it_face0_map).second;
        vert_opt & face1 = (*it_face1_map).second;
        vert_opt & face2 = (*it_face2_map).second;

        sub(edge0, face1.v, face0.v);
        sub(edge1, face2.v, face0.v);
        cross(face_n, edge0, edge1);

        nv_scalar weight = nv_norm(face_n);
        if (weight > nv_eps)
            scale(face_n,nv_one / weight);
        else
            scale(face_n,nv_scalar(0.001));
        normalize(face_n);
        
        if(gNVB_options & NVB_LHS)
            face_n = -face_n;


        if (face0.smg_id == smg_id)
            face0.n += face_n;
        if (face1.smg_id == smg_id)
            face1.n += face_n;
        if (face2.smg_id == smg_id)
            face2.n += face_n;

        std::pair<FaceMMapIt,FaceMMapIt> pair_mmap = m_opt->face_mmap.equal_range(face0_idx);
        FaceMMapIt it_face_mmap = pair_mmap.first;
        while (it_face_mmap != pair_mmap.second)
        {
            idxvert_opt & idxface = (*it_face_mmap).second;
            if (idxface.face.smg_id == smg_id)
            {
                FaceMapIt it_face_map = m_opt->face_map.find(idxface.new_idx);
                vert_opt & face = (*it_face_map).second;
                face.n += face_n;
            }
            ++it_face_mmap;
        }

        pair_mmap = m_opt->face_mmap.equal_range(face1_idx);
        it_face_mmap = pair_mmap.first;
        while (it_face_mmap != pair_mmap.second)
        {
            idxvert_opt & idxface = (*it_face_mmap).second;
            if (idxface.face.smg_id == smg_id)
            {
                FaceMapIt it_face_map = m_opt->face_map.find(idxface.new_idx);
                vert_opt & face = (*it_face_map).second;
                face.n += face_n;
            }
            ++it_face_mmap;
        }

        pair_mmap = m_opt->face_mmap.equal_range(face2_idx);
        it_face_mmap = pair_mmap.first;
        while (it_face_mmap != pair_mmap.second)
        {
            idxvert_opt & idxface = (*it_face_mmap).second;
            if (idxface.face.smg_id == smg_id)
            {
                FaceMapIt it_face_map = m_opt->face_map.find(idxface.new_idx);
                vert_opt & face = (*it_face_map).second;
                face.n += face_n;
            }
            ++it_face_mmap;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new mesh.
 *	\param		mesh		[in] the new mesh info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewMesh(const NVBMeshInfo& mesh)
{
	NVBLog(LOG_MSG, "Importing new ");

    if (_scene && mesh.mID != -1)
    {
        if (mesh.mIsTarget)
        {
            nv_node * node = SetNode(nv_factory::get_factory()->new_node(), mesh);
            NVBLog(LOG_MSG, "<anonymous> node: %s\n", node->name);
            assert(mesh.mID != -1);
            return true;
        }

        if (mesh.mIsSkeleton)
        {
            nv_node * node = SetNode(nv_factory::get_factory()->new_node(), mesh);
            NVBLog(LOG_MSG, "bone: %s\n", node->name);
            assert(mesh.mID != -1);
            return true;
        }

        nv_model* model = reinterpret_cast<nv_model*>(SetNode(nv_factory::get_factory()->new_model(), mesh));

        if (mesh.mIsSkin)
        {
            NVBLog(LOG_MSG, "skin: %s\n", model->name);
            return AddMeshModel(model, mesh, true);
        }
        
        NVBLog(LOG_MSG, "mesh: %s\n", model->name);
        return AddMeshModel(model, mesh);
    }
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new shape.
 *	\param		shape		[in] the new shape info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewShape(const NVBShapeInfo& shape)
{
	NVBLog(LOG_MSG, "Importing new shape...\n");
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to create a new motion.
 *	\param		motion		[in] the new motion info structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewMotion(const NVBMotionInfo& motion)
{
	NVBLog(LOG_MSG, "Importing new motion...\n");
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to retrieve the texture information from a NVB info structure.
 *	\param		texture		[in] the imported texture info structure.
 *	\return		Self-Reference.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NVBFactory::NewTexture(const NVBTextureInfo& texture)
{
	NVBLog(LOG_MSG, "Importing new texture...\n");
    
    nv_texture & tex =  _scene->textures[_scene->num_textures];

    bookeep_tex & bk_tex = bk_texs[_scene->num_textures];

    tex.name = new char[strlen(texture.mName.Get()) + 1];
    strcpy(tex.name, texture.mName.Get());

    tex.tex_mat.a00 = texture.mTMtx.m[0][0];
    tex.tex_mat.a01 = texture.mTMtx.m[1][0];
    tex.tex_mat.a02 = texture.mTMtx.m[2][0];
    tex.tex_mat.a03 = texture.mTMtx.m[3][0];
    tex.tex_mat.a10 = texture.mTMtx.m[0][1];
    tex.tex_mat.a11 = texture.mTMtx.m[1][1];
    tex.tex_mat.a12 = texture.mTMtx.m[2][1];
    tex.tex_mat.a13 = texture.mTMtx.m[3][1];
    tex.tex_mat.a20 = texture.mTMtx.m[0][2];
    tex.tex_mat.a21 = texture.mTMtx.m[1][2];
    tex.tex_mat.a22 = texture.mTMtx.m[2][2];
    tex.tex_mat.a23 = texture.mTMtx.m[3][2];

    tex.tex_mat.a30 = nv_zero;
    tex.tex_mat.a31 = nv_zero;
    tex.tex_mat.a32 = nv_zero;
    tex.tex_mat.a33 = nv_one;

    // book keeping matrices and uv xform to be
    // used to process the texture coordinates at
    // the end of the file parsing...
    bk_tex.tex_mat.a00 = texture.mTMtx.m[0][0];
    bk_tex.tex_mat.a01 = texture.mTMtx.m[1][0];
    bk_tex.tex_mat.a02 = texture.mTMtx.m[2][0];
    bk_tex.tex_mat.a03 = texture.mTMtx.m[3][0];

    bk_tex.tex_mat.a10 = texture.mTMtx.m[0][1];
    bk_tex.tex_mat.a11 = texture.mTMtx.m[1][1];
    bk_tex.tex_mat.a12 = texture.mTMtx.m[2][1];
    bk_tex.tex_mat.a13 = texture.mTMtx.m[3][1];

    bk_tex.tex_mat.a20 = texture.mTMtx.m[0][2];
    bk_tex.tex_mat.a21 = texture.mTMtx.m[1][2];
    bk_tex.tex_mat.a22 = texture.mTMtx.m[2][2];
    bk_tex.tex_mat.a23 = texture.mTMtx.m[3][2];

    bk_tex.tex_mat.a30 = nv_zero;
    bk_tex.tex_mat.a31 = nv_zero;
    bk_tex.tex_mat.a32 = nv_zero;
    bk_tex.tex_mat.a33 = nv_one;

    bk_tex.uv_scale = vec2(texture.mCValues.mScaleU, texture.mCValues.mScaleV);
    bk_tex.uv_offset = vec2(texture.mCValues.mOffsetU, texture.mCValues.mOffsetV);

    bk_tex.base_channel = texture.mChannel;


    _scene->num_textures++;
	return true;
}

void NVBFactory::SetScene( nv_scene  * scene)
{
    _scene = scene;
}

bool NVBFactory::EndImport()
{
    unsigned int i,j,k,l;
    nv_idx idx;
    unsigned int * faces;
    vec3 tmp;
    vec3 uv = vec3_null;


    // remap bone/mesh references...
    NVBLog(LOG_MSG, "Remap bone/mesh references...\n");
    for (k = 0; k < _scene->num_nodes; ++k)
    {
        nv_node * node = _scene->nodes[k];
        MeshMapIt it_skin = _skinmeshmap.find(node);
        if (it_skin != _skinmeshmap.end())
        {
            nv_model * model = reinterpret_cast<nv_model *>(node);

            TriMapType      & tri_map = (*it_skin).second->tri_map;
            MatFaceMapType  & matface_map = (*it_skin).second->matface_map;
            mesh_opt *        m_opt;

            TriMapIt it = tri_map.begin();
            model->num_meshes = tri_map.size();
            model->meshes = new nv_mesh[model->num_meshes];
            int count = 0;

            while (it != tri_map.end())
            {
                nv_mesh & msh = model->meshes[count];

                msh.num_faces = (*it).second->size() / 3;
                msh.material_id = (*it).first;
                msh.faces_idx = new nv_idx[msh.num_faces * 3];

                for (i = 0; i < msh.num_faces * 3; ++i,(*it).second->pop_front())
                    msh.faces_idx[i] = (*it).second->front();
                    

                MatFaceMapIt it_mapfacemap = matface_map.find((*it).first);
                assert(it_mapfacemap != matface_map.end());
                m_opt = (*it_mapfacemap).second;

                msh.skin = true;
                msh.num_vertices = m_opt->face_map.size();
                msh.vertices = new vec3[msh.num_vertices];
                msh.normals = new vec3[msh.num_vertices];
				msh.colors = new vec4[msh.num_vertices];

                msh.weights = new vec4[msh.num_vertices];
                msh.bone_idxs = new nv_idx[msh.num_vertices * 4];

                unsigned int texdim = 0;
                bool * faceidx_cache = new bool[msh.num_vertices];
                memset(faceidx_cache, 0, msh.num_vertices * sizeof(bool));
                bool alloc_texture = false;
                for (i = 0; i < msh.num_faces * 3; ++i)
                {
                    unsigned int face_idx = msh.faces_idx[i];
                    FaceMapIt it_face_map = m_opt->face_map.find(face_idx);
                    assert(it_face_map != m_opt->face_map.end());
                    vert_opt face = (*it_face_map).second;

                    msh.faces_idx[i] = face.face_idx;

                    if (faceidx_cache[face.face_idx] == false)
                    {
                        faceidx_cache[face.face_idx] = true;
                        face.v = vec3_null;
                        // for some reason we are missing consistent bone refs...lets fix this
                        for (j = 0; j < 4; ++j)
                        {
                            if (face.bones[j] == NV_BAD_IDX)
                                continue;
                            IDNodeMapIt it = _idnodemap.find(face.bones[j]);
                            if (it == _idnodemap.end())
                                face.bones[j] = NV_BAD_IDX;
                        }
                        nv_scalar weight_fix = nv_zero;
                        for (j = 0; j < 4; ++j)
                        {
                            if (face.bones[j] == NV_BAD_IDX)
                                continue;
                            weight_fix += face.weights[j];
                        }
                        if (weight_fix < nv_eps)
                            weight_fix = nv_one;
                        weight_fix = nv_one / weight_fix;
                        for (j = 0; j < 4; ++j)
                        {
                            if (face.bones[j] == NV_BAD_IDX)
                                continue;
                            face.weights[j] *= weight_fix;
                        }
                        // end of fix

                        for (j = 0; j < 4; ++j)
                        {
                            if (face.bones[j] == NV_BAD_IDX)
                                continue;
                            IDNodeMapIt it = _idnodemap.find(face.bones[j]);
                            assert(it != _idnodemap.end());
                            nv_idx bone_idx = _scene->find_node_idx((*it).second);
                            assert(bone_idx != NV_BAD_IDX);
                            nv_node * bone_ref = _scene->nodes[bone_idx];

                            mult_pos( tmp, bone_ref->xform, face.v_offset[j]);
                            scale(tmp, face.weights[j]);
                            face.v += tmp;
                        }

                        msh.vertices[face.face_idx] = face.v;
						msh.colors[face.face_idx] = face.c;
                        msh.weights[face.face_idx] = face.weights;

                        msh.bone_idxs[face.face_idx * 4] = face.bones[0];
                        msh.bone_idxs[face.face_idx * 4 + 1] = face.bones[1];
                        msh.bone_idxs[face.face_idx * 4 + 2] = face.bones[2];
                        msh.bone_idxs[face.face_idx * 4 + 3] = face.bones[3];
                        if (face.num_tmaps && alloc_texture == false)
                        {
                            alloc_texture = true;
                            texdim = 2;
                            msh.num_texcoord_sets = face.num_tmaps;
                            msh.texcoord_sets = new nv_texcoord_set[msh.num_texcoord_sets];
                            for (j = 0;j < face.num_tmaps; ++j)
                            {
                                msh.texcoord_sets[j].dim = texdim;
                                msh.texcoord_sets[j].texcoords = new nv_scalar[msh.num_vertices * texdim];
                            }
                        }
                        for (j = 0;j < face.num_tmaps; ++j)
                        {
                            msh.texcoord_sets[j].texcoords[face.face_idx*texdim] = face.tmaps[j].x;
                            msh.texcoord_sets[j].texcoords[face.face_idx*texdim+1] = face.tmaps[j].y;
                        }
                        // aabb min...
                        if (face.v.x < model->aabb_min.x)
                            model->aabb_min.x = face.v.x;
                        if (face.v.y < model->aabb_min.y)
                            model->aabb_min.y = face.v.y;
                        if (face.v.z < model->aabb_min.z)
                            model->aabb_min.z = face.v.z;
                        // aabb max...
                        if (face.v.x > model->aabb_max.x)
                            model->aabb_max.x = face.v.x;
                        if (face.v.y > model->aabb_max.y)
                            model->aabb_max.y = face.v.y;
                        if (face.v.z > model->aabb_max.z)
                            model->aabb_max.z = face.v.z;
                    }
                }
                delete [] faceidx_cache;
                ++count;
                ++it;

                vec3 v0(vec3_null);
                vec3 v1(vec3_null);
                vec3 v2(vec3_null);
                vec3 face_n;
                vec3 edge0;
                vec3 edge1;
                memset(msh.normals,0,msh.num_vertices * sizeof(vec3));
                for (i = 0; i < msh.num_faces; ++i)
                {
                    unsigned int idx0 = msh.faces_idx[i*3];
                    unsigned int idx1 = msh.faces_idx[i*3+1];
                    unsigned int idx2 = msh.faces_idx[i*3+2];

                    v0 = msh.vertices[idx0];

                    v1 = msh.vertices[idx1];
                    sub(edge0, v1, v0);
                    v2 = msh.vertices[idx2];
                    sub(edge1, v2, v0);

                    cross(face_n, edge0, edge1);

                    nv_scalar weight = nv_norm(face_n);
                    if (weight > nv_eps)
                        scale(face_n,nv_one / weight);
                    else
                        scale(face_n,nv_scalar(0.001));

                    if(gNVB_options & NVB_LHS)
                        face_n = -face_n;

                    msh.normals[idx0] += face_n;
                    msh.normals[idx1] += face_n;
                    msh.normals[idx2] += face_n;

                }
                for (i = 0; i < msh.num_vertices; ++i)
                {
                    normalize(msh.normals[i]);
                }
            }
        }
        MeshMapIt it_mesh = _meshmap.find(node);
        if (it_mesh != _meshmap.end())
        {
            nv_model * model = reinterpret_cast<nv_model *>(node);

            TriMapType      & tri_map = (*it_mesh).second->tri_map;
            MatFaceMapType  & matface_map = (*it_mesh).second->matface_map;
            mesh_opt *        m_opt;

            TriMapIt it = tri_map.begin();
            model->num_meshes = tri_map.size();
            model->meshes = new nv_mesh[model->num_meshes];
            int count = 0;
            while (it != tri_map.end())
            {
                nv_mesh & msh = model->meshes[count];

                msh.num_faces = (*it).second->size() / 3;
                msh.material_id = (*it).first;
                msh.faces_idx = new nv_idx[msh.num_faces * 3];

                for (i = 0; i < msh.num_faces * 3; ++i,(*it).second->pop_front())
                    msh.faces_idx[i] = (*it).second->front();

                MatFaceMapIt it_mapfacemap = matface_map.find((*it).first);
                assert(it_mapfacemap != matface_map.end());
                m_opt = (*it_mapfacemap).second;
                msh.skin = false;
                msh.num_vertices = m_opt->face_map.size();
                msh.vertices = new vec3[msh.num_vertices];
                msh.normals = new vec3[msh.num_vertices];
				msh.colors = new vec4[msh.num_vertices];

                unsigned int texdim = 0;
                bool * faceidx_cache = new bool[msh.num_vertices];
                memset(faceidx_cache, 0, msh.num_vertices * sizeof(bool));
                bool alloc_texture = false;
                for (i = 0; i < msh.num_faces * 3; ++i)
                {
                    unsigned int face_idx = msh.faces_idx[i];
                    FaceMapIt it_face_map = m_opt->face_map.find(face_idx);
                    assert(it_face_map != m_opt->face_map.end());
                    vert_opt face = (*it_face_map).second;

                    msh.faces_idx[i] = face.face_idx;

                    if (faceidx_cache[face.face_idx] == false)
                    {
                        faceidx_cache[face.face_idx] = true;
                        msh.vertices[face.face_idx] = face.v;
						msh.colors[face.face_idx] = face.c;

                        normalize(face.n);

                        msh.normals[face.face_idx] = face.n;
                        if (face.num_tmaps && alloc_texture == false)
                        {
                            alloc_texture = true;
                            texdim = 2;
                            msh.num_texcoord_sets = face.num_tmaps;
                            msh.texcoord_sets = new nv_texcoord_set[msh.num_texcoord_sets];
                            for (j = 0;j < face.num_tmaps; ++j)
                            {
                                msh.texcoord_sets[j].dim = texdim;
                                msh.texcoord_sets[j].texcoords = new nv_scalar[msh.num_vertices * texdim];
                            }
                        }
                        for (j = 0;j < face.num_tmaps; ++j)
                        {
                            msh.texcoord_sets[j].texcoords[face.face_idx*texdim] = face.tmaps[j].x;
                            msh.texcoord_sets[j].texcoords[face.face_idx*texdim+1] = face.tmaps[j].y;
                        }
                        vec3 tmp;
                        mult_pos(tmp,model->xform,face.v);
                        // model bounding box
                        // aabb min...
                        if (tmp.x < model->aabb_min.x)
                            model->aabb_min.x = tmp.x;
                        if (tmp.y < model->aabb_min.y)
                            model->aabb_min.y = tmp.y;
                        if (tmp.z < model->aabb_min.z)
                            model->aabb_min.z = tmp.z;
                        // aabb max...
                        if (tmp.x > model->aabb_max.x)
                            model->aabb_max.x = tmp.x;
                        if (tmp.y > model->aabb_max.y)
                            model->aabb_max.y = tmp.y;
                        if (tmp.z > model->aabb_max.z)
                            model->aabb_max.z = tmp.z;
                        // mesh bounding box
                        // aabb min...
                        if (tmp.x < msh.aabb_min.x)
                            msh.aabb_min.x = tmp.x;
                        if (tmp.y < msh.aabb_min.y)
                            msh.aabb_min.y = tmp.y;
                        if (tmp.z < msh.aabb_min.z)
                            msh.aabb_min.z = tmp.z;
                        // aabb max...
                        if (tmp.x > msh.aabb_max.x)
                            msh.aabb_max.x = tmp.x;
                        if (tmp.y > msh.aabb_max.y)
                            msh.aabb_max.y = tmp.y;
                        if (tmp.z > msh.aabb_max.z)
                            msh.aabb_max.z = tmp.z;
                    }
                }
                delete [] faceidx_cache;
                ++count;
                ++it;
            }
        }
    }


    NVBLog(LOG_MSG, "Process AABBs...\n");
    NVBLog(LOG_MSG, "Process texture coordinates...\n");
    for (i = 0; i < _scene->num_nodes; ++i)
    {
        nv_node * node = _scene->nodes[i];
        if ( node->get_type() == nv_node::GEOMETRY )
        {
            nv_model * model = reinterpret_cast<nv_model *>(node);

            for ( j = 0; j < model->num_meshes; ++j)
            {
                nv_mesh & mesh = model->meshes[j];
 
                if (mesh.num_texcoord_sets)
                {
                    faces = mesh.faces_idx;

                    nv_material & mat = _scene->materials[mesh.material_id];
                    // count the number of textures coords set to compute...
                    coord_type coord_list;
                    coord_it it;
                    unsigned int num_texchannel = 0;
                    
                    for (k = 0; k < mat.num_textures; ++k)
                    {
                        nv_idx tex = mat.textures[k];
                        it = std::find(coord_list.begin(), coord_list.end(), bk_texs[tex]);
                        if (it == coord_list.end())
                        {
                            // ensures that there is a 1:1 relationship
                            // between textures and their references
                            if (bk_texs[tex].new_channel != NV_BAD_IDX)
                            {
                                assert(bk_texs[tex].new_channel == num_texchannel);
                                assert(mat.tex_channel[k] == num_texchannel);
                            }
                                
                            bk_texs[tex].new_channel = num_texchannel;
                            mat.tex_channel[k] = num_texchannel;
                            coord_list.push_back(bk_texs[tex]);
                            ++num_texchannel;
                        }
                        else
                            mat.tex_channel[k] = (*it).new_channel;

                    }
                    nv_texcoord_set * texcoord_sets = new nv_texcoord_set[num_texchannel];
                    for (k = 0; k < num_texchannel; ++k)
                    {
                        texcoord_sets[k].dim = 2;
                        texcoord_sets[k].texcoords = new nv_scalar[2 * mesh.num_vertices];
                    }

                    for (k = 0; k < mesh.num_vertices; ++k)
                    {
                        idx = k * 2;
                        it = coord_list.begin();
                        for (l = 0; l < num_texchannel; ++l)
                        {
                            bookeep_tex & bk_tex = *it;
                            uv.x = mesh.texcoord_sets[bk_tex.base_channel].texcoords[idx];
                            uv.y = mesh.texcoord_sets[bk_tex.base_channel].texcoords[idx + 1];

                            mult_pos( tmp, bk_tex.tex_mat, uv );
                            texcoord_sets[l].texcoords[idx] = bk_tex.uv_scale.x * tmp.x + bk_tex.uv_offset.x;
                            texcoord_sets[l].texcoords[idx + 1] = bk_tex.uv_scale.y * tmp.y + bk_tex.uv_offset.y;
                            ++it;
                        }
						
                    }
					if (num_texchannel)
					{
						delete [] mesh.texcoord_sets;
						mesh.texcoord_sets = texcoord_sets;
						mesh.num_texcoord_sets = num_texchannel;
					}
                }
            }
        }
    }
    if (_scene->num_textures)
        delete [] bk_texs;

    NVBLog(LOG_MSG, "Construct node hierarchy...\n");
    // child to parent relationship...
    for (i = 0; i < _scene->num_nodes; ++i)
    {
        nv_node * node = _scene->nodes[i];
        IDNodeMapIt it = _idnodemap.find((unsigned int)(node->parent));
        if (it != _idnodemap.end())
            node->parent = _scene->find_node_idx((*it).second);
        else
            node->parent = NV_BAD_IDX;
        it = _idnodemap.find((unsigned int)(node->target));
        if (it != _idnodemap.end())
            node->target = _scene->find_node_idx((*it).second);
        else
            node->target = NV_BAD_IDX;
    }
    // parent to children relationship...
    // 0(N^2)... I know...
    std::vector<nv_idx> child_ids;
    NVMeshMender aMender;

    for (i = 0; i < _scene->num_nodes; ++i)
    {
        child_ids.clear();
        
        nv_node * node = _scene->nodes[i];
        for (j = 0; j < _scene->num_nodes; ++j)
        {
            if ( nv_idx(i) == _scene->nodes[j]->parent && i != j)
                child_ids.push_back(j);
        }
        node->num_children = child_ids.size();
        if (node->num_children)
        {
            node->children = new nv_idx[node->num_children];
            for (j = 0; j < node->num_children; ++j)
                node->children[j] = child_ids[j];
        }

        if (node->get_type() == nv_node::GEOMETRY )
        {
            nv_model * model = reinterpret_cast<nv_model*>(node);
            // reassign skin bone refs...
            for (j = 0; j < model->num_meshes; ++j)
            {
                nv_mesh & mesh = model->meshes[j];
                if (mesh.skin)
                {
                    for (k = 0; k < mesh.num_vertices * 4; ++k)
                    {
                        if (mesh.bone_idxs[k] != NV_BAD_IDX)
                        {
                            IDNodeMapIt it = _idnodemap.find(mesh.bone_idxs[k]);
                            if (it != _idnodemap.end())
                                mesh.bone_idxs[k] = _scene->find_node_idx((*it).second);
                            else
                                NVBLog(LOG_ERR, "bone ref not found!\n");
                        }
                    }
                }
            }
            // compute the aabb for skin meshes...
            for (j = 0; j < model->num_meshes; ++j)
            {
                std::vector<float> vpos;
                std::vector<int>   triIndices;
                std::vector<float> vnor; 
                std::vector<float> texCoords; 

                nv_mesh & mesh = model->meshes[j];
                for (k = 0; k < mesh.num_vertices; ++k)
                {
                    vec3 v(mesh.vertices[k]);
                    vec3 n(mesh.normals[k]);

                    vpos.push_back(v.x);
                    vpos.push_back(v.y);
                    vpos.push_back(v.z);

                    vnor.push_back(n.x);
                    vnor.push_back(n.y);
                    vnor.push_back(n.z);
                }
                for (k = 0; k < mesh.num_faces * 3; ++k)
                {
                    triIndices.push_back(mesh.faces_idx[k]);
                }

                for (k = 0; k < mesh.num_texcoord_sets; ++k)
                {
                    texCoords.clear();

                    for (l = 0; l < mesh.num_vertices; ++l)
                    {
                        texCoords.push_back(mesh.texcoord_sets[k].texcoords[l*2]);
                        texCoords.push_back(mesh.texcoord_sets[k].texcoords[l*2+1]);
                        texCoords.push_back(nv_zero);
                    }

                    std::vector<NVMeshMender::VertexAttribute> inputAtts;// What you have
                    std::vector<NVMeshMender::VertexAttribute> outputAtts;// What you want.

                    NVMeshMender::VertexAttribute posAtt;
                    posAtt.Name_ = "position";
                    posAtt.floatVector_ = vpos;

                    NVMeshMender::VertexAttribute triIndAtt;
                    triIndAtt.Name_ = "indices";
                    triIndAtt.intVector_ = triIndices;

                    NVMeshMender::VertexAttribute norAtt;
                    norAtt.Name_ = "normal";
                    norAtt.floatVector_ = vnor;

                    NVMeshMender::VertexAttribute texCoordAtt;
                    texCoordAtt.Name_ = "tex0";
                    texCoordAtt.floatVector_ = texCoords;

                    NVMeshMender::VertexAttribute tgtSpaceAtt;
                    tgtSpaceAtt.Name_ = "tangent";

                    NVMeshMender::VertexAttribute binormalAtt;
                    binormalAtt.Name_ = "binormal";

                    inputAtts.push_back(posAtt);
                    inputAtts.push_back(triIndAtt);
                    inputAtts.push_back(norAtt);
                    inputAtts.push_back(texCoordAtt);

                    outputAtts.push_back(posAtt);
                    outputAtts.push_back(triIndAtt);
                    outputAtts.push_back(norAtt);
                    outputAtts.push_back(texCoordAtt);
                    outputAtts.push_back(tgtSpaceAtt);
                    outputAtts.push_back(binormalAtt);

                    bool bSuccess = aMender.Munge( 
                                 inputAtts,                     // these are my positions & indices
                                 outputAtts,                    // these are the outputs I requested, plus extra stuff generated on my behalf
                                 nv_two_pi / ( 6.0f ),                     // tangent space smooth angle
                                 NULL,                          // no Texture matrix applied to my tex0 coords
                                 NVMeshMender::DontFixTangents,            // fix degenerate bases & texture mirroring
                                 NVMeshMender::DontFixCylindricalTexGen,    // handle cylidrically mapped textures via vertex duplication
                                 NVMeshMender::DontWeightNormalsByFaceSize // weight vertex normals by the triangle's size
                                 );

                    if (bSuccess)
                    {
                        mesh.texcoord_sets[k].binormals = new vec3[mesh.num_vertices];
                        mesh.texcoord_sets[k].tangents = new vec3[mesh.num_vertices];

                        for (l = 0; l < mesh.num_vertices; ++l)
                        {
                            mesh.texcoord_sets[k].tangents[l].x = outputAtts[4].floatVector_[l * 3];
                            mesh.texcoord_sets[k].tangents[l].y = outputAtts[4].floatVector_[l * 3 + 1];
                            mesh.texcoord_sets[k].tangents[l].z = outputAtts[4].floatVector_[l * 3 + 2];

                            mesh.texcoord_sets[k].binormals[l].x = outputAtts[5].floatVector_[l * 3];
                            mesh.texcoord_sets[k].binormals[l].y = outputAtts[5].floatVector_[l * 3 + 1];
                            mesh.texcoord_sets[k].binormals[l].z = outputAtts[5].floatVector_[l * 3 + 2];
                        }
                    }
                }
            }

            // for bones...
            if (model->num_meshes == 0)
            {
                // aabb min...
                if (model->xform.x < model->aabb_min.x)
                    model->aabb_min.x = model->xform.x;
                if (model->xform.y < model->aabb_min.y)
                    model->aabb_min.y = model->xform.y;
                if (model->xform.z < model->aabb_min.z)
                    model->aabb_min.z = model->xform.z;
                // aabb max...
                if (model->xform.x > model->aabb_max.x)
                    model->aabb_max.x = model->xform.x;
                if (model->xform.y > model->aabb_max.y)
                    model->aabb_max.y = model->xform.y;
                if (model->xform.z > model->aabb_max.z)
                    model->aabb_max.z = model->xform.z;
            }

            // scene bounding box computation...
            // computated as the union of the bounding
            // boxes of all the models
            if (_scene->aabb_min.x > model->aabb_min.x )
                _scene->aabb_min.x = model->aabb_min.x;
            if (_scene->aabb_min.y > model->aabb_min.y )
                _scene->aabb_min.y = model->aabb_min.y;
            if (_scene->aabb_min.z > model->aabb_min.z )
                _scene->aabb_min.z = model->aabb_min.z;

            if (_scene->aabb_max.x < model->aabb_max.x )
                _scene->aabb_max.x = model->aabb_max.x;
            if (_scene->aabb_max.y < model->aabb_max.y )
                _scene->aabb_max.y = model->aabb_max.y;
            if (_scene->aabb_max.z < model->aabb_max.z )
                _scene->aabb_max.z = model->aabb_max.z;
            // update the model aabb bbox only...
            if (_scene->models_aabb_min.x > model->aabb_min.x )
                _scene->models_aabb_min.x = model->aabb_min.x;
            if (_scene->models_aabb_min.y > model->aabb_min.y )
                _scene->models_aabb_min.y = model->aabb_min.y;
            if (_scene->models_aabb_min.z > model->aabb_min.z )
                _scene->models_aabb_min.z = model->aabb_min.z;

            if (_scene->models_aabb_max.x < model->aabb_max.x )
                _scene->models_aabb_max.x = model->aabb_max.x;
            if (_scene->models_aabb_max.y < model->aabb_max.y )
                _scene->models_aabb_max.y = model->aabb_max.y;
            if (_scene->models_aabb_max.z < model->aabb_max.z )
                _scene->models_aabb_max.z = model->aabb_max.z;
        }
        else
        {
            // scene bounding box computation...
            // computated as the union of the bounding
            // boxes of all the dummies, lights, bones...
            if (_scene->aabb_min.x > node->xform.x )
                _scene->aabb_min.x = node->xform.x;
            if (_scene->aabb_min.y > node->xform.y )
                _scene->aabb_min.y = node->xform.y;
            if (_scene->aabb_min.z > node->xform.z )
                _scene->aabb_min.z = node->xform.z;

            if (_scene->aabb_max.x < node->xform.x )
                _scene->aabb_max.x = node->xform.x;
            if (_scene->aabb_max.y < node->xform.y )
                _scene->aabb_max.y = node->xform.y;
            if (_scene->aabb_max.z < node->xform.z )
                _scene->aabb_max.z = node->xform.z;

        }
    }
    // Clean up
    MeshMapIt it = _meshmap.begin();
    while (it != _meshmap.end())
    {
        TriMapType      & tri_map = (*it).second->tri_map;
        MatFaceMapType  & matface_map = (*it).second->matface_map;
        
        MatFaceMapIt it_matfacemap = matface_map.begin();
        while (it_matfacemap != matface_map.end())
        {
            delete (*it_matfacemap).second;
            ++it_matfacemap;
        }

        TriMapIt it_trimap = tri_map.begin();
        while(it_trimap != tri_map.end())
        {
            delete (*it_trimap).second;
            ++it_trimap;
        }

        delete (*it).second;
        ++it;
    }
    it = _skinmeshmap.begin();
    while (it != _skinmeshmap.end())
    {
        TriMapType      & tri_map = (*it).second->tri_map;
        MatFaceMapType  & matface_map = (*it).second->matface_map;

        MatFaceMapIt it_matfacemap = matface_map.begin();
        while (it_matfacemap != matface_map.end())
        {
            delete (*it_matfacemap).second;
            ++it_matfacemap;
        }

        TriMapIt it_trimap = tri_map.begin();
        while(it_trimap != tri_map.end())
        {
            delete (*it_trimap).second;
            ++it_trimap;
        }

        delete (*it).second;
        ++it;
    }
    _meshmap.clear();
    _idnodemap.clear();
	_skinmeshmap.clear();

    return true;
}

void NVBFactory::SetLogCallback(TloggingCB cbfn, unsigned long userparam)
{
	_logcb = cbfn;
	_loguserparam = userparam;
}
