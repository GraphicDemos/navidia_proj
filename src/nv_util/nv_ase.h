/*********************************************************************NVMH1****
File:
nv_ase.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef __nv_ase_h__
#define __nv_ase_h__

#include <vector>
#include <map>
#ifdef _WIN32
# include <windows.h>
#endif
#include "nv_util.h"
#include <GL/gl.h>

namespace ase
{
    // forward declaration
    struct texdata;
    struct mapobj;
    struct matobj;
    struct geomobj;

    // type definition
    typedef std::vector<mapobj*>    map_array;
    typedef map_array::iterator     map_it;
    
    typedef std::vector<matobj*>    mat_array;
    typedef mat_array::iterator     mat_it;

    typedef std::vector<geomobj*>   geom_array;
    typedef geom_array::iterator    geom_it;

    typedef std::map<char*,texdata*> tex_map;
    typedef tex_map::iterator       tex_it;
    typedef tex_map::const_iterator tex_cit;
    typedef tex_map::value_type     tex_pair;

    // structure definition
    struct NVUTIL_CLASS texdata
    {
        texdata() : name(0), width(0), height(0), components(0), pixels(0) {}
        ~texdata();

        char *          name;
        int             width;
        int             height;
        int             components;
        unsigned char * pixels;

        // GL stuff - may want to remove that stuff?
        GLuint          textid;
        GLuint          pixelfmt;
    };

    struct NVUTIL_CLASS animdata
    {
        animdata() : numpos(0), numrot(0) {}
        ~animdata();

        // The anim structure strips out key information. We assume that the 
        // data has been sampled at constant time interval

        float *         pos;    // positional data
        int             numpos;
        float *         rot;    // rotational data (quaternion)
        int             numrot;
    };

    struct NVUTIL_CLASS mapobj
    {
        mapobj() : name(0), classname(0), bitmap(0), map_type(0), bitmap_filter(0) {};
        ~mapobj();

        char *          name;
        char *          classname;
        unsigned int    subno;
        float           amount;
        char *          bitmap;
        char *          map_type;
        float           u_offset;
        float           v_offset;
        float           u_tiling;
        float           v_tiling;
        float           angle;
        float           blur;
        float           blur_offset;
        float           noise_amt;
        float           noise_size;
        unsigned int    noise_level;
        float           noise_phase;
        char*           bitmap_filter;

        map_array       map_generic;
	};

    struct NVUTIL_CLASS matobj
    {
        matobj() : name(0), classname(0), shader(0), twosided(false), falloff(false), xp_type(0) {};
        ~matobj();

        char *          name;
		char *          classname;
        float           ambient[3];
        float           diffuse[4];
        float           specular[4];
        float           shine;
        float           shinestrength;
        float           transparency;
        float           wiresize;
        char *          shader;
        float           xp_falloff;
        float           selfillum;
        bool            twosided;
        bool            falloff;
        char *          xp_type;
        
        mat_array       submat;

        map_array       map_ambient;
        map_array       map_generic;
        map_array       map_diffuse;
        map_array       map_specular;
        map_array       map_bump;
        map_array       map_selfillum;
        map_array       map_reflect;
        map_array       map_shine;
        map_array       map_shinestrength;
        map_array       map_opacity;
        map_array       map_refract;
        map_array       map_filtercolor;
    };

     
     //  in the the vec array
    struct NVUTIL_CLASS patch
    {
        int numedges;       // 4 for quad, 3 for tri
        int edge[4];        // 4 indices to reference edges
        int interior[4];    // 4 indices to reference inner control vectors
        int smg;            // smoothing group idx
        int mtlid;
    };

    struct NVUTIL_CLASS geomobj
    {
        geomobj();
        ~geomobj();

        typedef enum {
            polygonal,
            patched
        } geomtype;

        geomtype        type;   // tells if the geomobj is polygonal or patched

        char *          name;   // object name
        geomobj *       parent; // ref to the parent geomobj
        geom_array      children;

        float           bone_offset_tm[16]; // bone offset transform
        float           tm[16]; // world transform
        float           rtm[16]; // 4x4 transform from the parent

        // polygonal
        float *         v;      // vertices
        float *         n;      // normals
        float *         t;      // texcoords
        float *         c;      // vertex colors
        unsigned int    numn;   // number of vertex normals
        unsigned int    numt;   // number of texcoords
        unsigned int    numv;   // number of vertices
        unsigned int    numc;   // number of vertices color
        unsigned int *  fvn;    // indices for the vertex normals indexed by face index
        
        unsigned int *  fsubmat; // submaterial per face
        unsigned int *  f;      // faces
        unsigned int *  tf;     // texture faces
        float *         fn;     // face normals
        unsigned int *  fmapv;  // face map vertices
        unsigned int    numf;   // number of faces
        unsigned int    numtf;  // number of texture faces
        unsigned int *  smg;    // smoothing group

        // patches
        int             numverts;
        float *         verts;
        int             numvecs;
        float *         vecs;
        int             numedges;
        int *           edges;
        int             numpatches;
        patch *         patches;     
                                     

        int             numtvchannels; // normally 2, where channel 0 is vertex colors 
                                     // and channel 1 is texcoords
        int *           numtvverts;  // number of tvverts per channel
        float **        tvverts;     // array of tvverts per channel
        int **          tvpatches;   // array of indices to tvverts per channel 
                                     // (defining a tvpatch (dim = 4))
        
        // animation
        animdata *      anim;   // ref to the animation data

        // material
        int             matidx; // index in the material array

        // vertex weighting data - may have to be reordered
        unsigned int    numbv;      // number of blended vertices
        float *         bv;         // blended vertices {x,y,z,w} where w is the weight
        float **        bmatref;    // per blended vertex geom obj matrix tm reference
        unsigned int *  vbv;        // vertex indices
        geomobj **      bgeomref;   // per blended vertex geom obj reference
    };

    struct NVUTIL_CLASS model
    {
        model() : name(0), time(0) {}
        ~model();

        char *      name;
        geom_array  root_geom;
        geom_array  geom;
        mat_array   mat;
        tex_map     tex;
        int         time;
    };

	NVUTIL_API extern model * load(const char * filename, float scale);
	NVUTIL_API extern model * load(const char * buf, unsigned int size, float scale);
	NVUTIL_API extern geomobj * get_geomobj(model * m, const char * name);
	NVUTIL_API extern bool load_tex( model * m);
}


#endif /* __nv_ase_h__ */
