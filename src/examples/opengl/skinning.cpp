/*********************************************************************NVMH1****
File:
vtxprg_skin.cpp

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#if defined(WIN32)
# include <windows.h>
# pragma warning (disable:4786) // Disable the STL debug information warnings
#elif defined(UNIX)
# include <GL/glx.h>
#endif

#include <float.h>
#define GLH_EXT_SINGLE_FILE

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <shared/read_text_file.h>
#include <shared/timer.h>
#include <shared/quitapp.h>
#include <nv_util.h>
#include <nv_unzip.h>

#include "skinning.h"

using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

ase::model * model;
bool        paused;
bool        dl;
vector<int> ttime;
int         min_vtxskin = 0;
int         max_vtxskin = 4;
bool        ref_pose = false;
glh::matrix4f id;

float factor[4];
float color[4];
float lightPos[4] = { 1.f, 0.f, 0.f, 1.0f, };

#define EPS                     0.00001
#define MATRIX_BIN_MAX_SIZE     29

GLhandleARB programObject;
GLint boneMatricesParam = -1;
GLint colorParam = -1;
GLint lightPosParam = -1;

GLint positionAttrib = -1;
GLint normalAttrib = -1;
GLint weightAttrib = -1;
GLint indexAttrib = -1;
GLint numBonesAttrib = -1;

vtxprg_geom_array vp_geom_array;

typedef std::vector<ase::geomobj*>   bone_ref_stack;

// frame per second counter
timer fps(10);

enum
{
    M_PAUSE,
    M_PLUS,
    M_MINUS,
    M_DISPLAY_LIST,
    M_TOGGLE_WIREFRAME,
    M_TOGGLE_ANIMATION,
    M_INC_MAX_VTX,
    M_DEC_MAX_VTX,
    M_INC_MIN_VTX,
    M_DEC_MIN_VTX,
};

// glut-ish callbacks
void display();
void idle();
void key(unsigned char k, int x, int y);
void menu(int item);

// my functions
void init_opengl();

void GLErrorReport()
{
    GLuint errnum;
    const char *errstr;
    while ((errnum = glGetError())) 
    {
        errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
        printf(errstr);
    }
    return;
}

// This routine determine if the mat_stack stack has enough room
// available to merge with bone_tms and still be small enough to
// fit MATRIX_BIN_MAX_SIZE matrices
bool is_fitting(bone_ref_stack & stack, bone_ref_stack & bone_refs)
{
    // the obvious case...
    if (stack.size() + bone_refs.size() <= MATRIX_BIN_MAX_SIZE)
        return true;
    // a bit less...
    int count = 0;
    bone_ref_stack::iterator it = bone_refs.begin();
    // for all the incoming bones, we test if they already
    // belong to stack. if so, we increment count to keep track
    // of how many are already store in track
    while (it != bone_refs.end())
    {
        bool  found = false;
        bone_ref_stack::iterator stack_it = stack.begin();
        while (stack_it != stack.end() && found == false)
        {
            if (*stack_it == *it)
            {
                count++;
                break;
            }
            ++stack_it;
        }
        ++it;
    }
    // then we can test if we would fit, knowing that count matrices
    // of bone_tms are already stored in stack
    if (stack.size() + bone_refs.size() - count <= MATRIX_BIN_MAX_SIZE)
        return true;
    // then we can't fit them all...
    return false;
}

// This routine tests if a pointer to a matrix is already stored
// in a given stack and returns its index if found. If not found
// it pushes it back onto the stack and return the index at which
// it has been stored
int add_mat_idx(bone_ref_stack & stack, ase::geomobj * boneref)
{
    int idx = -1;
    int count = 0;
    bone_ref_stack::iterator it = stack.begin();
    while (it!= stack.end() && idx == -1)
    {
        if (boneref == *it)
            idx = count;
        else
        {
            ++count;
            ++it;
        }
    }
    
    if (it == stack.end())
    {
        idx = stack.size();
        stack.push_back(boneref);
    }
    return idx;
}

int has_mat_idx(bone_ref_stack & stack, ase::geomobj * boneref)
{
    int idx = -1;
    int count = 0;
    bone_ref_stack::iterator it = stack.begin();
    while (it!= stack.end() && idx == -1)
    {
        if (boneref == *it)
            idx = count;
        else
        {
            ++count;
            ++it;
        }
    }
    
    return (idx == -1) ? 0 : 1;
}

void convert_local_to_world(ase::model * m)
{
    unsigned int i;
    int i_times_3;
    ase::geomobj * geom = NULL;
    ase::geom_it it = m->geom.begin();
    while (it != m->geom.end())
    {
        geom = *it;
        glh::matrix4f mat(geom->tm);
        for (i = 0; i < geom->numv; ++i)
        {
            i_times_3 = i * 3;
            glh::vec3f vec(&geom->v[i_times_3]);
            mat.mult_matrix_vec(vec);
            geom->v[i_times_3] = vec[0];
            geom->v[i_times_3 + 1] = vec[1];
            geom->v[i_times_3 + 2] = vec[2];
        }
        for (i = 0; i < geom->numn; ++i)
        {
            i_times_3 = i * 3;
            glh::vec3f vec(&geom->n[i_times_3]);
            mat.mult_matrix_dir(vec);
            geom->n[i_times_3] = vec[0];
            geom->n[i_times_3 + 1] = vec[1];
            geom->n[i_times_3 + 2] = vec[2];
        }
        ++it;
    }
}

void load_skin()
{
    int cur_bin;
    int i,i_times_3;
    int j,j_times_3;
    int k,l;
    int v_idx,v_idx_times_4,v_idx_times_3;
    int n_idx;
    int t_idx;
    float * bv;
    int * normals_idx;
    int * texcoords_idx;
    unsigned int size;
    unsigned char * buf;

    // Load the .ASE files from ZIP volume
    buf = unzip::open("skin_models.zip","harley-run.ase", &size);
    if (buf == NULL)
        quitapp(-1);

    // Parse the ASE buffer
    model = ase::load((char *)buf,size, 0.01);
    delete [] buf;

    // get the normals in world space to compute the offset
    convert_local_to_world(model );
    // Process the ase::model
    ase::geom_it it = model->geom.begin();
    while (it != model->geom.end())
    {
        ase::geomobj * geom = *it;
        if (geom->numbv)
        {
            vtxprg_geom * vp_geom = new vtxprg_geom;

            // set the pointer to the array display list objects to 0
            vp_geom->dl[0] = 0;
            vp_geom->dl[1] = 0;
            // copy the name of the object
            vp_geom->name = geom->name;
            // copy the number of vertices
            vp_geom->numvertices = geom->numv;

            vp_geom->v = new float[vp_geom->numvertices * 3];
            memcpy(vp_geom->v, geom->v, sizeof(float) * vp_geom->numvertices * 3);

            vp_geom->n = new float[vp_geom->numvertices * 3];
            memset(vp_geom->n, 0, sizeof(float) * vp_geom->numvertices * 3);

            vp_geom->w = new float[vp_geom->numvertices * 4];
            memset(vp_geom->w, 0, sizeof(float) * vp_geom->numvertices * 4);

            // allocate v_numbones to hold the number of bone reference per vertex
            // and memset it to 0
            vp_geom->v_numbones = new unsigned int[vp_geom->numvertices];
            memset(vp_geom->v_numbones,0,sizeof(int) * vp_geom->numvertices);

            // allocate the 4 v_offset that will hold the vertices defined in each
            // bone's coordinate space. We memset them all to 0
            vp_geom->v_offset[0] = new float[vp_geom->numvertices * 3];
            vp_geom->v_offset[1] = new float[vp_geom->numvertices * 3];
            vp_geom->v_offset[2] = new float[vp_geom->numvertices * 3];
            vp_geom->v_offset[3] = new float[vp_geom->numvertices * 3];

            memset(vp_geom->v_offset[0], 0, sizeof(float) * vp_geom->numvertices * 3);
            memset(vp_geom->v_offset[1], 0, sizeof(float) * vp_geom->numvertices * 3);
            memset(vp_geom->v_offset[2], 0, sizeof(float) * vp_geom->numvertices * 3);
            memset(vp_geom->v_offset[3], 0, sizeof(float) * vp_geom->numvertices * 3);

            // allocate the 4 n_offset that will hold the normals defined in each
            // bone's coordinate space. We memset them all to 0
            vp_geom->n_offset[0] = new float[vp_geom->numvertices * 4];
            vp_geom->n_offset[1] = new float[vp_geom->numvertices * 4];
            vp_geom->n_offset[2] = new float[vp_geom->numvertices * 4];
            vp_geom->n_offset[3] = new float[vp_geom->numvertices * 4];

            memset(vp_geom->n_offset[0], 0, sizeof(float) * vp_geom->numvertices * 4);
            memset(vp_geom->n_offset[1], 0, sizeof(float) * vp_geom->numvertices * 4);
            memset(vp_geom->n_offset[2], 0, sizeof(float) * vp_geom->numvertices * 4);
            memset(vp_geom->n_offset[3], 0, sizeof(float) * vp_geom->numvertices * 4);

            // copy the material index
            vp_geom->matidx = geom->matidx;
            // copy the number of texture coordinates
            vp_geom->numt = geom->numt;
            // if we have any texture coordinates
            if (vp_geom->numt)
            {
                // This means we can have tangent data, in case we want to do some per pixel shaders.
                // In the case we want to do per pixel shading, we will use n_offset to store the binormal
                // instead of the normal. Then we will finally cross Tangent and Binormal to get the normal
                // in the vertex program

                // allocate the 4 t_offset that will hold the tangents defined in each
                // bone's coordinate space. We memset them all to 0
                vp_geom->t_offset[0] = new float[vp_geom->numvertices * 4];
                vp_geom->t_offset[1] = new float[vp_geom->numvertices * 4];
                vp_geom->t_offset[2] = new float[vp_geom->numvertices * 4];
                vp_geom->t_offset[3] = new float[vp_geom->numvertices * 4];

                memset(vp_geom->t_offset[0], 0, sizeof(float) * vp_geom->numvertices * 4);
                memset(vp_geom->t_offset[1], 0, sizeof(float) * vp_geom->numvertices * 4);
                memset(vp_geom->t_offset[2], 0, sizeof(float) * vp_geom->numvertices * 4);
                memset(vp_geom->t_offset[3], 0, sizeof(float) * vp_geom->numvertices * 4);
            }

            // allocate an array of indices into the array of normals
            normals_idx = new int[vp_geom->numvertices];
            if (geom->numt)
                // allocate an array of indices into the array of texture coords
                texcoords_idx = new int[vp_geom->numvertices];

            bone_ref_stack    main_stack;

            // allocate an array of float that will hold the matrices indices per vertex
            // and memset it to 0
            vp_geom->v_matidx = new float[vp_geom->numvertices * 4];
            memset(vp_geom->v_matidx,0,sizeof(float) * vp_geom->numvertices * 4);
            // allocate a temporary array of float that will hold the matrix indices per vertex
            // and memset it to -1
            int * tmp_v_matidx = new int[vp_geom->numvertices * 4];
            vp_geom->matbin_idx = new int[vp_geom->numvertices];
            memset(vp_geom->matbin_idx,-1,sizeof(int) * vp_geom->numvertices);
            
            // we loop through all the *blended* vertices, i.e. vertices that are skinned
            for (i = 0; i < (int)geom->numbv; ++i)
            {
                v_idx = geom->vbv[i];
                v_idx_times_4 = v_idx * 4;
                v_idx_times_3 = v_idx * 3;
                // get a reference to the vertex bone reference counter (initialized to zero)
                unsigned int & v_offset_idx = vp_geom->v_numbones[v_idx];
                // get the vertex offset and its weight
                bv = &geom->bv[ i * 4];
                // check if this is not some fifth bone reference...
                if (v_offset_idx < 4)
                {
                    // copy the vertex offset and its weight
                    // note: weights are stored in the w component of the normal offset
                    vp_geom->v_offset[v_offset_idx][v_idx_times_3] = bv[0];
                    vp_geom->v_offset[v_offset_idx][v_idx_times_3 + 1] = bv[1];
                    vp_geom->v_offset[v_offset_idx][v_idx_times_3 + 2] = bv[2];
                    vp_geom->n_offset[v_offset_idx][v_idx_times_4 + 3] = bv[3];

                    // copy the weights only
                    vp_geom->w[v_idx_times_4 + v_offset_idx] = bv[3];
                
                    // add to the main stack the matrix (bone) this vertex offset is referring to and
                    // store its index in the stack at the vertex matrix index attribute
                    tmp_v_matidx[v_idx_times_4 + v_offset_idx] = add_mat_idx(main_stack, geom->bgeomref[i]);
                    // increment the number of bone references
                    v_offset_idx++;
                }
                else // if we have more than 4 matrices, we renormalize the weights and
                     // just forget about the rest!
                {
                    float sum = 0.0f;
                    // Sometimes a vertex weight is reference multiple times a given bone. In this case,
                    // lets add up the weight.
                    if (has_mat_idx(main_stack,geom->bgeomref[i]) == 0)
                    {
                        // sum up the weights
                        for (j = 0; j < 4; ++j)
                            sum += vp_geom->n_offset[j][v_idx_times_4 + 3];
                        // renormalize the weights
                        // note: one could actually look at the all the weight and pick
                        // the one that are higher since the skinning will more influenced
                        // by bones more heavily weighted
                        for (j = 0; j < 4; ++j)
                        {
                            vp_geom->n_offset[j][v_idx_times_4 + 3] /= sum;
                            vp_geom->w[v_idx_times_4 + j] /= sum;
                        }
                    }
                    else
                    {
                        int idx = add_mat_idx(main_stack, geom->bgeomref[i]);
                        for (unsigned int jj = 0; jj < v_offset_idx; ++jj)
                            if (tmp_v_matidx[v_idx_times_4 + jj] == idx)
                            {
                                vp_geom->w[v_idx_times_4 + jj] += bv[3];
                            }
                    }
                }
            }

            // worker stack of matrix stacks used to store matrix indices.
            // the size of the stack gives the number of matrix bins needed
            // to render the mesh model
            std::vector<bone_ref_stack>  bone_ref_sstack;
            // for all the faces in the model
            for (i = 0; i < (int)geom->numf; ++i)
            {
                i_times_3 = i * 3;
                // worker matrix stack used to accumulate and test if we don't 
                // overflow the current constant table area reserved to store
                // the bone matrices
                bone_ref_stack bone_refs;
                // lets loop through all the vertices
                for (j = 0; j < 3; ++j)
                {
                    // get the vertex index
                    v_idx = geom->f[i_times_3 + j];
                    // for all the bone references...
                    for (k = 0; k < (int)vp_geom->v_numbones[v_idx]; ++k)
                    {
                        // we stack the pointers to the bones
                        add_mat_idx(bone_refs,main_stack[tmp_v_matidx[v_idx * 4+ k]]);
                    }
                }

                // then we determine how many matrix bins are needed
                // for this model
                cur_bin = -1;
                bool fit = false;
                // we will add bins until bone_ref_sstack has enough bins
                // to the number of matrices in bone_tms
                while (fit == false)
                {
                    cur_bin++;
                    if (cur_bin >= (int)bone_ref_sstack.size() )
                    {
                        bone_ref_stack stack;
                        bone_ref_sstack.push_back(stack);
                    }
                    fit = is_fitting(bone_ref_sstack[cur_bin], bone_refs);
                }

                // now that we are sure to have a bin that has enough
                // room to receive the new matrices, we can merge
                bone_ref_stack::iterator bit = bone_refs.begin();
                while (bit != bone_refs.end())
                {
                    add_mat_idx(bone_ref_sstack[cur_bin], *bit);
                    ++bit;
                }
                assert(bone_ref_sstack[cur_bin].size() <= MATRIX_BIN_MAX_SIZE);

                // lets loop through all the vertices
                for (j = 0; j < 3; ++j)
                {
                    // get the vertex index
                    v_idx = geom->f[i_times_3 + j];
                    // get the normal index
                    n_idx = geom->fvn[i_times_3 + j];

                    // we want to make sure that this particular vertex is used by one and only one bin.
                    // Therefore, matbin_idx[v_idx] has to be unitialized or pointing to the same bin. 
                    if (vp_geom->matbin_idx[v_idx] == -1 || vp_geom->matbin_idx[v_idx] == cur_bin)
                        vp_geom->matbin_idx[v_idx] = cur_bin;
                    else
                    {
                        // otherwise...
                        v_idx_times_4 = v_idx * 4;
                        v_idx_times_3 = v_idx * 3;
                        // We have to insert a new vertex
                        // We have to duplicate the vertex that fucks everything up.
                        // Basically, we have to reallocate everything to grow up by one
                        // and duplicate the data and reassign properly
                        float * ftemp;
                        for (k = 0; k < 4; ++k)
                        {
                            ftemp = new float[(vp_geom->numvertices + 1) * 3];
                            memcpy(ftemp, vp_geom->v_offset[k], sizeof(float) * vp_geom->numvertices * 3);
                            delete [] vp_geom->v_offset[k];
                            vp_geom->v_offset[k] = ftemp;
                            vp_geom->v_offset[k][vp_geom->numvertices * 3] = vp_geom->v_offset[k][v_idx_times_3];
                            vp_geom->v_offset[k][vp_geom->numvertices * 3 + 1] = vp_geom->v_offset[k][v_idx_times_3 + 1];
                            vp_geom->v_offset[k][vp_geom->numvertices * 3 + 2] = vp_geom->v_offset[k][v_idx_times_3 + 2];
                        }
                        if (vp_geom->numt)
                        {
                            for (k = 0; k < 4; ++k)
                            {
                                ftemp = new float[(vp_geom->numvertices + 1) * 4];
                                delete [] vp_geom->t_offset[k];
                                vp_geom->t_offset[k] = ftemp;
                                memset(vp_geom->t_offset[k], 0, sizeof(float) * (vp_geom->numvertices + 1) * 4);
                            }
                        }
                        for (k = 0; k < 4; ++k)
                        {
                            ftemp = new float[(vp_geom->numvertices + 1) * 4];
                            memcpy(ftemp, vp_geom->n_offset[k], sizeof(float) * vp_geom->numvertices * 4);
                            delete [] vp_geom->n_offset[k];
                            vp_geom->n_offset[k] = ftemp;
                            vp_geom->n_offset[k][vp_geom->numvertices * 4 + 3] = ftemp[v_idx_times_4 + 3];
                        }
                        // Reallocate the matbin_idx
                        int * temp = new int[vp_geom->numvertices + 1];
                        memcpy(temp,vp_geom->matbin_idx,sizeof(int) * vp_geom->numvertices);
                        delete [] vp_geom->matbin_idx;
                        vp_geom->matbin_idx = temp;
                        vp_geom->matbin_idx[vp_geom->numvertices] = cur_bin;
                        // Reallocate the num_bones
                        unsigned int * utemp = new unsigned int[vp_geom->numvertices + 1];
                        memcpy(utemp,vp_geom->v_numbones,sizeof(unsigned int) * vp_geom->numvertices);
                        delete [] vp_geom->v_numbones;
                        vp_geom->v_numbones = utemp;
                        vp_geom->v_numbones[vp_geom->numvertices] = vp_geom->v_numbones[v_idx];
                        // Reallocate the v_matidx
                        temp = new int[(vp_geom->numvertices + 1) * 4];
                        memcpy(temp,tmp_v_matidx,sizeof(int) * vp_geom->numvertices * 4);
                        delete [] tmp_v_matidx;
                        tmp_v_matidx = temp;
                        tmp_v_matidx[vp_geom->numvertices * 4] = tmp_v_matidx[v_idx_times_4];
                        tmp_v_matidx[vp_geom->numvertices * 4 + 1] = tmp_v_matidx[v_idx_times_4 + 1];
                        tmp_v_matidx[vp_geom->numvertices * 4 + 2] = tmp_v_matidx[v_idx_times_4 + 2];
                        tmp_v_matidx[vp_geom->numvertices * 4 + 3] = tmp_v_matidx[v_idx_times_4 + 3];
                        delete [] vp_geom->v_matidx;
                        vp_geom->v_matidx = new float[(vp_geom->numvertices + 1 )* 4];
                        memset(vp_geom->v_matidx,0,sizeof(float) * (vp_geom->numvertices + 1 ) * 4);
                        // Reallocate the normals
                        int * itemp = new int[vp_geom->numvertices + 1];
                        delete [] normals_idx;
                        normals_idx = itemp;

                        // update the vertices
                        ftemp = new float[(vp_geom->numvertices + 1 )* 3];
                        memcpy(ftemp, vp_geom->v, vp_geom->numvertices * 3 * sizeof(float));
                        delete [] vp_geom->v;
                        vp_geom->v = ftemp;
                        vp_geom->v[vp_geom->numvertices * 3] = vp_geom->v[v_idx_times_3];
                        vp_geom->v[vp_geom->numvertices * 3 + 1] = vp_geom->v[v_idx_times_3 + 1];
                        vp_geom->v[vp_geom->numvertices * 3 + 2] = vp_geom->v[v_idx_times_3 + 2];

                        // update the normals
                        ftemp = new float[(vp_geom->numvertices + 1 )* 3];
                        memcpy(ftemp, vp_geom->n, vp_geom->numvertices * 3 * sizeof(float));
                        delete [] vp_geom->n;
                        vp_geom->n = ftemp;

                        // update the weights
                        ftemp = new float[(vp_geom->numvertices + 1 )* 4];
                        memcpy(ftemp, vp_geom->w, vp_geom->numvertices * 4 * sizeof(float));
                        delete [] vp_geom->w;
                        vp_geom->w = ftemp;
                        vp_geom->w[vp_geom->numvertices * 4] = vp_geom->w[v_idx_times_4];
                        vp_geom->w[vp_geom->numvertices * 4 + 1] = vp_geom->w[v_idx_times_4 + 1];
                        vp_geom->w[vp_geom->numvertices * 4 + 2] = vp_geom->w[v_idx_times_4 + 2];
                        vp_geom->w[vp_geom->numvertices * 4 + 3] = vp_geom->w[v_idx_times_4 + 3];

                        // Reallocate the texture coords
                        if (geom->numt)
                        {
                            itemp = new int[vp_geom->numvertices + 1];
                            delete [] texcoords_idx;
                            texcoords_idx = itemp;
                        }
                        // update the face vertex index
                        geom->f[i_times_3 + j] = vp_geom->numvertices;
                        ftemp = new float[(geom->numn + 1) * 3];
                        memcpy(ftemp,geom->n,sizeof(float) * geom->numn * 3);
                        delete [] geom->n;
                        geom->n = ftemp;
                        geom->n[geom->numn * 3] = geom->n[n_idx * 3];
                        geom->n[geom->numn * 3 + 1] = geom->n[n_idx * 3 + 1];
                        geom->n[geom->numn * 3 + 2] = geom->n[n_idx * 3 + 2];
                        geom->fvn[i_times_3 + j] = geom->numn;
                        geom->numn++;
                        // update the number of vertices
                        vp_geom->numvertices++;
                        // roll back a little...
                        --i;
                        break;
                    }
                }
            }

            // let get the number of bins...
            unsigned int numbin = bone_ref_sstack.size();
            // temporary array to keep track of the number of faces
            // using a particular bin
            int * inc = new int[numbin];
            memset(inc,0,sizeof(int) * numbin);

            // copy the numbins to vp_geom
            vp_geom->numbins = numbin;
            // allocate numbin arrays of vertex indices to store the faces
            vp_geom->faces = new int*[numbin];
            // allocate an array of numbin integer to store the number of faces
            // per bin
            vp_geom->num_faces = new int[numbin];
            // allocate numbin arrays of vertex indices to store the faces per bins
            for (i = 0; i < (int)numbin; ++i)
                vp_geom->faces[i] = new int[geom->numf * 3];

            // for all the faces...
            for (i = 0; i < (int)geom->numf; ++i)
            {
                i_times_3 = i * 3;
                assert( vp_geom->matbin_idx[geom->f[i_times_3 + 1]] == vp_geom->matbin_idx[geom->f[i_times_3 + 0]]);
                assert( vp_geom->matbin_idx[geom->f[i_times_3 + 2]] == vp_geom->matbin_idx[geom->f[i_times_3 + 1]]);
                // get the bin of the current face (all the vertices share the same bin at this point)
                cur_bin = vp_geom->matbin_idx[geom->f[i_times_3]];
                // for all the vertices...
                for (j = 0; j < 3; ++j)
                {
                    v_idx = geom->f[i_times_3 + j];
                    n_idx = geom->fvn[i_times_3 + j];
                    if (geom->numt)
                        t_idx = geom->tf[i_times_3 + j];
                    v_idx_times_4 = v_idx * 4;
                    // for all the bone references...
                    for (k = 0; k < (int)vp_geom->v_numbones[v_idx]; ++k)
                    {
                        // we compute the corresponding index in the constant table and store it
                        // in vp_geom->v_matidx
                        vp_geom->v_matidx[v_idx_times_4 + k] = 
                            add_mat_idx(bone_ref_sstack[cur_bin], main_stack[int(tmp_v_matidx[v_idx_times_4 + k])]) * 3;
                        assert(bone_ref_sstack[cur_bin].size() <= MATRIX_BIN_MAX_SIZE);
                    }
                    // now we store the vertex index for this face vertex of this bin
                    vp_geom->faces[cur_bin][inc[cur_bin] * 3 + j] = v_idx;
                    // temporary keep track of corresponding indices to normals
                    // and texture coords
                    normals_idx[v_idx] = n_idx;
                    if (geom->numt)
                        texcoords_idx[v_idx] = t_idx;
                }
                // increment the face counter for this bin
                inc[cur_bin]++;
            }

            // array of int to keep track of the exact number of
            // matrices in each bin
            vp_geom->num_bones_ref = new int[numbin];
            // array of pointers to pointers to matrices to keep track of the
            // transforms at all time
            vp_geom->bones_ref = new ase::geomobj**[numbin];
            // loop through all the bins
            for (i = 0; i < (int)numbin; ++i)
            {
                // get the number of matrices
                int size = bone_ref_sstack[i].size();
                assert(size <= MATRIX_BIN_MAX_SIZE && size >= 0);

                // copy the number of faces to be drawn with this particular bin
                vp_geom->num_faces[i] = inc[i];
                // intialize the counter to used to increment
                // the indexation of the matrix pointer storage
                int count = -1;
                // allocate the array of pointers to matrices
                // by the number of matrices in the current bin
                vp_geom->bones_ref[i] = new ase::geomobj*[size];
                // store the number of matrices in this bin
                vp_geom->num_bones_ref[i] = size;
                // iterate and copy the pointers directly
                bone_ref_stack::iterator it = bone_ref_sstack[i].begin();
                while (it!= bone_ref_sstack[i].end())
                {
                    vp_geom->bones_ref[i][++count] = *it;
                    ++it;
                }
            }
            // if we have texture coords, lets allocate an array to hold them
            if (vp_geom->numt)
                vp_geom->t = new float[vp_geom->numvertices * 3];
            // lets loop through all the bins to compute the normal offsets and
            // copy the texture coordinates
            // for each bin...
            for (i = 0; i< vp_geom->numbins; ++i)
            {
                // for each face...
                for (j = 0; j < vp_geom->num_faces[i]; ++j)
                {
                    j_times_3 = j * 3;
                    // for each vertex...
                    for (k = 0; k < 3; ++k)
                    {
                        // grab the various indices
                        v_idx = vp_geom->faces[i][j_times_3 + k];
                        v_idx_times_4 = v_idx * 4;
                        n_idx = normals_idx[v_idx];
                        // we copy the texcoords if available
                        if (geom->numt)
                        {
                            t_idx = texcoords_idx[v_idx];
                            vp_geom->t[v_idx * 3] = geom->t[t_idx * 3];
                            vp_geom->t[v_idx * 3 + 1] = geom->t[t_idx * 3 + 1];
                            vp_geom->t[v_idx * 3 + 2] = geom->t[t_idx * 3 + 2];
                        }
                        // based on the number of bone reference, we need to compute each
                        // normal offset in the coordinate space of each bone
                        int numbones = vp_geom->v_numbones[v_idx];
                        for (l = 0; l < numbones; ++l)
                        {
                            glh::vec3f norm(&geom->n[n_idx * 3]);
                            memcpy(&vp_geom->n[v_idx * 3], &geom->n[n_idx * 3], sizeof(float) * 3);

                            glh::matrix4f mat(vp_geom->bones_ref[i][(int)vp_geom->v_matidx[v_idx_times_4 + l] / 3]->tm);
                            mat = mat.inverse();
                            mat.mult_matrix_dir(norm);

                            vp_geom->n_offset[l][v_idx_times_4]     += norm[0];
                            vp_geom->n_offset[l][v_idx_times_4 + 1] += norm[1];
                            vp_geom->n_offset[l][v_idx_times_4 + 2] += norm[2];
                        }
                        // and we make sure the extra normals are zero-ed out
                        for (; l < 4; ++l)
                        {
                            vp_geom->n_offset[l][v_idx_times_4]     = 0.0f;
                            vp_geom->n_offset[l][v_idx_times_4 + 1] = 0.0f;
                            vp_geom->n_offset[l][v_idx_times_4 + 2] = 0.0f;
                        }
                    }
                }
            }
            // We need to normalize the normals
            for (i = 0; i< vp_geom->numbins; ++i)
            {
                for (j = 0; j < vp_geom->numvertices; ++j)
                {
                    v_idx_times_4 = j * 4;
                    float norm;
                    for (l = 0; l < 4; ++l)
                    {
                        norm =          vp_geom->n_offset[l][v_idx_times_4]*vp_geom->n_offset[l][v_idx_times_4]+
                                        vp_geom->n_offset[l][v_idx_times_4 + 1]*vp_geom->n_offset[l][v_idx_times_4 + 1]+
                                        vp_geom->n_offset[l][v_idx_times_4 + 2]*vp_geom->n_offset[l][v_idx_times_4 + 2];
                        if (norm > EPS)
                        {
                            norm = 1.0f / sqrt(norm);
                            vp_geom->n_offset[l][v_idx_times_4]*= norm;
                            vp_geom->n_offset[l][v_idx_times_4 + 1]*= norm;
                            vp_geom->n_offset[l][v_idx_times_4 + 2]*= norm;
                        }
                    }
                }
            }
            // lets allocate the arrays to hold the number of faces referencing vertices that need
            // one, two, three or four bones to be drawn properly, per bin.
            for (i = 0; i < 4; ++i)
            {
                vp_geom->num_bonesperface[i] = new int[vp_geom->numbins];
                memset(vp_geom->num_bonesperface[i],0,sizeof(int) * vp_geom->numbins);
            }
            // Next we loop through all the bins and all the faces to collect what is maximum
            // number of bone reference needed per vertex. This is vital to determine what vertex
            // program is needed to be bound in order to draw properly a particular face
            for (i = 0; i< vp_geom->numbins; ++i)
            {
                for (j = 0; j < vp_geom->num_faces[i]; ++j)
                {
                    int max = 1;
                    int numbones[3];
                    // for each vertex we get the maximum num of bone reference
                    for (k = 0; k < 3; ++k)
                    {
                        v_idx = vp_geom->faces[i][j * 3 + k];
                        numbones[k] = vp_geom->v_numbones[v_idx];
                        if (numbones[k] > max)
                            max = numbones[k];
                    }
                    // and now we increment the number of face using this particular
                    // vertex program
                    vp_geom->num_bonesperface[max - 1][i]++;
                }
            }

            // Next we allocate the arrays of vertex indices used to define the faces
            // on a per bin basis...
            for (j = 0; j < 4; ++j)
                vp_geom->vidx_boneperfaces[j] = new int*[vp_geom->numbins];
            for (i = 0; i< vp_geom->numbins; ++i)
            {
                for (j = 0; j < 4; ++j)
                    vp_geom->vidx_boneperfaces[j][i] = new int[vp_geom->num_bonesperface[j][i] * 3];
            }
            // reset the number of faces counter to be used again below...
            memset(vp_geom->num_bonesperface[0],0,sizeof(int) * vp_geom->numbins);
            memset(vp_geom->num_bonesperface[1],0,sizeof(int) * vp_geom->numbins);
            memset(vp_geom->num_bonesperface[2],0,sizeof(int) * vp_geom->numbins);
            memset(vp_geom->num_bonesperface[3],0,sizeof(int) * vp_geom->numbins);

            // Next we just have to copy the vertex indices for each face
            // and put them into the corresponding vertex program array.
            for (i = 0; i< vp_geom->numbins; ++i)
            {
                for (j = 0; j < vp_geom->num_faces[i]; ++j)
                {
                    int max = 1;
                    int numbones[3];
                    // find out what is the maximum number of bone reference...
                    for (k = 0; k < 3; ++k)
                    {
                        v_idx = vp_geom->faces[i][j * 3 + k];
                        numbones[k] = vp_geom->v_numbones[v_idx];
                        if (numbones[k] > max)
                            max = numbones[k];
                    }
                    // copy the indices
                    for (k = 0; k < 3; ++k)
                        vp_geom->vidx_boneperfaces[max - 1][i][vp_geom->num_bonesperface[max - 1][i] * 3 + k] = vp_geom->faces[i][j * 3 + k];
                    vp_geom->num_bonesperface[max - 1][i]++;
                }
            }
            // delete our worker arrays
            delete [] inc;
            delete [] tmp_v_matidx;
            delete [] normals_idx;
            if (geom->numt)
                delete [] texcoords_idx;
            // allocate array for display lists objects
            vp_geom->dl[0] = new unsigned int[4 * vp_geom->numbins];
            vp_geom->dl[1] = new unsigned int[4 * vp_geom->numbins];
            memset(vp_geom->dl[0], 0, 4 * vp_geom->numbins * sizeof(unsigned int));
            memset(vp_geom->dl[1], 0, 4 * vp_geom->numbins * sizeof(unsigned int));
            vp_geom_array.push_back(vp_geom);
            }
        ++it;
    }
}


void update_geom(ase::model * m, ase::geomobj * geom, int & time, ase::geomobj * root, bool offset)
{
    if (geom->anim)
    {
        ase::animdata * anim = geom->anim;
        if (anim->numpos)
        {
            if (anim->numpos <= time)
                time = 0;
            geom->tm[12] = anim->pos[time * 3];
            geom->tm[13] = anim->pos[time * 3 + 1];
            geom->tm[14] = anim->pos[time * 3 + 2];
    
       
            if (geom->parent)
            {
                /*
                geom->tm[12] += geom->rtm[12];
                geom->tm[13] += geom->rtm[13];
                geom->tm[14] += geom->rtm[14];
                */
            }
            else
            {
                geom->tm[12] = anim->pos[0];
                geom->tm[13] = anim->pos[1];
                geom->tm[14] = anim->pos[2];
            }
        }
        else
        {
            geom->tm[12] = geom->rtm[12];
            geom->tm[13] = geom->rtm[13];
            geom->tm[14] = geom->rtm[14];
        }

        if (anim->numrot)
        {
            if (anim->numrot <= time)
                time = 0;

            glh::quaternionf q(&anim->rot[time * 4]);
            glh::matrix4f mat;
            q.get_value(mat);

            geom->tm[0] = mat(0,0);
            geom->tm[1] = mat(1,0);
            geom->tm[2] = mat(2,0);
            geom->tm[4] = mat(0,1);
            geom->tm[5] = mat(1,1);
            geom->tm[6] = mat(2,1);
            geom->tm[8] = mat(0,2);
            geom->tm[9] = mat(1,2);
            geom->tm[10] = mat(2,2);
        }
        else
        {
            geom->tm[0] = geom->rtm[0];
            geom->tm[1] = geom->rtm[1];
            geom->tm[2] = geom->rtm[2];
            geom->tm[4] = geom->rtm[4];
            geom->tm[5] = geom->rtm[5];
            geom->tm[6] = geom->rtm[6];
            geom->tm[8] = geom->rtm[8];
            geom->tm[9] = geom->rtm[9];
            geom->tm[10] = geom->rtm[10];
        }
        if (geom->parent)
        {
            glh::matrix4f matparent(geom->parent->tm);
            glh::matrix4f mat(geom->tm);
            mat = matparent * mat;
            memcpy(geom->tm,mat.get_value(),sizeof(float) * 16);
        }
    }
    else
    {
        if (geom->parent)
        {
            glh::matrix4f matparent(geom->parent->tm);
            glh::matrix4f mat(geom->rtm);
            mat = matparent * mat;
            memcpy(geom->tm,mat.get_value(),sizeof(float) * 16);
        }
    }


    ase::geom_it it = geom->children.begin();
    while (it != geom->children.end())
    {
        update_geom(m,*it,time,root,offset);
        ++it;
    }

    if (offset)
        return;

    // needed for non offset based skinning
    if (geom->parent)
    {
        glh::matrix4f mat(geom->tm);
        mat = mat * geom->bone_offset_tm;
        memcpy(geom->tm,mat.get_value(),sizeof(float) * 16);
    }
}

void update_anim(ase::model * m, int & time, bool  offset)
{
    ase::geomobj * root = ase::get_geomobj(m,"\"Bip01 Pelvis\"");
    ase::geom_it it = m->root_geom.begin();
    while (it != m->root_geom.end())
    {
        update_geom(m, *it, time, root, offset);
        ++it;
    }
    m->time = time;
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("GLSL Skinning Demo");

    load_skin();
    init_opengl();

    glut_helpers_initialize();

    cb.keyboard_function = key;
    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    object.configure_buttons(1);

    object.dolly.dolly[2] = -1.1;
    object.pan.pan[0] = -0.0;
    object.pan.pan[1] = -0.0;
    object.trackball.r[0] = -0.528517;
    object.trackball.r[1] = -0.471187;
    object.trackball.r[2] = -0.434137;
    object.trackball.r[3] = 0.556936;

    object.trackball.centroid[0] = 0.003839;
    object.trackball.centroid[1] = 0.052791;
    object.trackball.centroid[2] = 0.371201;

    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glut_add_interactor(&reshaper);

    glutCreateMenu(menu);
    glutAddMenuEntry("Increase jester [+]", M_PLUS);
    glutAddMenuEntry("Decrease jester [-]", M_MINUS);
    glutAddMenuEntry("Toggle Display List usage [d]", M_DISPLAY_LIST);
    glutAddMenuEntry("Toggle Wireframe [w]", M_TOGGLE_WIREFRAME);
    glutAddMenuEntry("Toggle Animation [SPACEBAR]", M_TOGGLE_ANIMATION);
    glutAddMenuEntry("Increase maximal bone skinning processing [M]", M_INC_MAX_VTX);
    glutAddMenuEntry("Decrease maximal bone skinning processing [m]", M_DEC_MAX_VTX);
    glutAddMenuEntry("Increase minimal bone skinning processing [N]", M_INC_MIN_VTX);
    glutAddMenuEntry("Decrease minimal bone skinning processing [n]", M_DEC_MIN_VTX);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();

    delete model;
    return 0;
}

void menu(int item)
{
    switch (item) 
    {
    case M_PLUS:
        key('+', 0, 0);
        break;
    case M_MINUS:
        key('-', 0, 0);
        break;
    case M_PAUSE:
        key(' ', 0, 0);
        break;
    case M_DISPLAY_LIST:
        key('d', 0, 0);
        break;
    case M_TOGGLE_WIREFRAME:
        key('w', 0, 0);
        break;
    case M_TOGGLE_ANIMATION:
        key(' ', 0, 0);
        break;
    case M_INC_MAX_VTX:
        key('M', 0, 0);
        break;
    case M_DEC_MAX_VTX:
        key('m', 0, 0);
        break;
    case M_INC_MIN_VTX:
        key('N', 0, 0);
        break;
    case M_DEC_MIN_VTX:
        key('n', 0, 0);
        break;
    }
    glutPostRedisplay();
}

void cleanExit(int exitval)
{
    if (programObject)
        glDeleteObjectARB(programObject);

    if(exitval == 0) { exit(0); }
    else { quitapp(exitval); }
}

void printInfoLog(GLhandleARB object)
{
    int maxLength = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

    char *infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);

    printf("%s\n", infoLog);
}

void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType)
{
    assert(programObject != 0);
    assert(shaderSource != 0);
    assert(shaderType != 0);

    GLhandleARB object = glCreateShaderObjectARB(shaderType);
    assert(object != 0);

    GLint length = (GLint)strlen(shaderSource);
    glShaderSourceARB(object, 1, &shaderSource, &length);

    // compile vertex shader object
    glCompileShaderARB(object);

    // check if shader compiled
    GLint compiled = 0;
    glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (!compiled)
    {
        printInfoLog(object);
        cleanExit(-1);
    }

    // attach vertex shader to program object
    glAttachObjectARB(programObject, object);

    // delete vertex object, no longer needed
    glDeleteObjectARB(object);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void init_opengl()
{
    ttime.push_back(0);
    paused = false;
    dl = true;

    if (!glh_init_extensions("GL_VERSION_1_2 GL_ARB_shader_objects GL_ARB_vertex_program GL_ARB_vertex_shader"))
    {
        cerr << "Necessary extensions were not supported:" << endl
             << glh_get_unsupported_extensions() << endl;
        quitapp( -1 );
    }

    factor[0] = 2.0f;
    factor[1] = 1.0f;
    factor[2] = 0.0f;
    factor[3] = 0.0f;
    
    color[0] = 0.9f;
    color[1] = 0.9f;
    color[2] = 0.9f;
    color[3] = 0.0f;
    
    data_path path;
    path.path.push_back(".");
    path.path.push_back("../../../MEDIA/programs");
    path.path.push_back("../../../../MEDIA/programs");

    programObject = glCreateProgramObjectARB();

    string filename = path.get_file("glsl_skinning/skinning.glsl");
    if (filename == "")
    {
        printf("Unable to load skinning.glsl, exiting...\n");
        cleanExit(-1);
    }

    GLcharARB *shaderData = read_text_file(filename.c_str());
    addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB);

    delete [] shaderData;

    // bind position attribute to location 0
    glBindAttribLocationARB(programObject, 0, "position");

    glLinkProgramARB(programObject);

    GLint linked = false;
    glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked)
    {
        printInfoLog(programObject);
        cout << "Shaders failed to link, exiting..." << endl;
        cleanExit(-1);
    }

    glValidateProgramARB(programObject);

    GLint validated = false;
    glGetObjectParameterivARB(programObject, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
    if (!validated)
    {
        printInfoLog(programObject);
        cout << "Shaders failed to validate, exiting..." << endl;
        cleanExit(-1);
    }

    // get attribute locations
    positionAttrib = glGetAttribLocationARB(programObject, "position");
    assert(positionAttrib >= 0);

    normalAttrib = glGetAttribLocationARB(programObject, "normal");
    assert(normalAttrib >= 0);

    weightAttrib = glGetAttribLocationARB(programObject, "weight");
    assert(weightAttrib >= 0);

    indexAttrib = glGetAttribLocationARB(programObject, "index");
    assert(indexAttrib >= 0);

    numBonesAttrib = glGetAttribLocationARB(programObject, "numBones");
    assert(numBonesAttrib >= 0);

    // get uniform locations
    colorParam = glGetUniformLocationARB(programObject, "color");
    assert(colorParam >= 0);

    lightPosParam = glGetUniformLocationARB(programObject, "lightPos");
    assert(lightPosParam >= 0);

    boneMatricesParam = glGetUniformLocationARB(programObject, "boneMatrices[0]");
    assert(boneMatricesParam >= 0);

    glUseProgramObjectARB(programObject);
    glUniform4fvARB(lightPosParam, 1, lightPos);

    glUseProgramObjectARB(0);

    GLErrorReport();

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void key(unsigned char k, int x, int y)
{
    static GLenum e = GL_FILL;

    if (k==27 || k=='q') 
        cleanExit(0);

    if (k == ' ')
        paused = !paused;
    if (k == 'd' || k == 'D')
        dl = !dl;
    if (k == '+')
        ttime.push_back(0);
    if (k == '-')
        if (ttime.size())
            ttime.pop_back();
    if (k == 'm')
        max_vtxskin--;
    if (k == 'M')
        max_vtxskin++;
    if (k == 'n')
        min_vtxskin--;
    if (k == 'N')
        min_vtxskin++;

    int inc = 0;
    if (k == '>')
        inc = 1;
    if (k == '<')
        inc = -1;
    
    for (unsigned int i = 0; i < ttime.size(); ++i)
    {
        if (paused)
            ttime[i] += inc;
        if (ttime[i] < 0)
            ttime[i] = 0;
    }

    if (max_vtxskin > 4)
        max_vtxskin = 4;
    if (max_vtxskin < 0)
        max_vtxskin = 0;
    if (min_vtxskin > max_vtxskin)
        min_vtxskin = max_vtxskin;
    if (min_vtxskin < 0)
        min_vtxskin = 0;

    if (k == 'w' || k == 'W')
    {
        e  = (e == GL_FILL) ? GL_LINE : GL_FILL;
        glPolygonMode(GL_FRONT,e);
    }
}

void idle()
{
    glutPostRedisplay();
}

void set_material(ase::model * m, vtxprg_geom * geom, ase::matobj * mat)
{
    glUniform4fvARB(colorParam, 1, mat->diffuse);
}

void render_geometry(vtxprg_geom * geom)
{
    int j,i,k,l;
    int v_idx,v_idx_times_4,v_idx_times_3;

    // we loop through all the bins
    for (i = 0; i< geom->numbins; ++i)
    {
        for (l = min_vtxskin; l < max_vtxskin; l++)
        {
            // for each bin, we look if there is any set of faces that are using
            // 1,2,3,4 bone references per vertex to be drawn
            if (geom->num_bonesperface[l][i])
            {
                // we pick the corresponding vertex program, i.e. number of bone references per vertex
                glVertexAttrib1fARB(numBonesAttrib, l+1);

                if (geom->matidx >= 0)
                {
                    ase::matobj * mat = model->mat[geom->matidx];
                    set_material(model,geom,mat);
                }

                // we load the bone transforms in the constant table
                for (j = 0; j < geom->num_bones_ref[i];++j)
                {
                    float * mat = geom->bones_ref[i][j]->tm;

                    GLint currentMatrix = boneMatricesParam + j;
                    glUniformMatrix4fvARB(currentMatrix, 1, GL_FALSE, mat);
                }

                // if we have a list of display lists
                if (dl)
                {
                    // test if the display list for the current bin, current bone reference number exists
                    if (geom->dl[1][i * 4 + l])
                    {
                        // lets draw it
                        glCallList(geom->dl[1][i * 4 + l]);
                        continue;
                    }
                    else
                    {
                        // lets create it
                        geom->dl[1][i * 4 + l] = glGenLists(1);
                        glNewList(geom->dl[1][i * 4 + l], GL_COMPILE);
                    }
                }

                // lets draw
                glBegin(GL_TRIANGLES);
                // for all the faces...
                for (j = 0; j < geom->num_bonesperface[l][i]; ++j)
                {
                    // for all the vertex indices defining the face triangle
                    for (k = 0; k < 3; ++k)
                    {
                        // get the vertex index
                        v_idx = geom->vidx_boneperfaces[l][i][j * 3 + k];

                        // premultiply few things...
                        v_idx_times_4 = v_idx * 4;
                        v_idx_times_3 = v_idx * 3;

                        float index[4];
                        for (int ind = 0; ind < 4; ind++)
                            index[ind] = geom->v_matidx[v_idx_times_4+ind]/3.0f;

                        glVertexAttrib3fvARB(normalAttrib, &geom->n[v_idx_times_3]);    // normal
                        glVertexAttrib4fvARB(weightAttrib, &geom->w[v_idx_times_4]);    // weights
                        glVertexAttrib4fvARB(indexAttrib, &index[0]);                   // indices
                        glVertexAttrib3fvARB(positionAttrib, &geom->v[v_idx_times_3]);
                    }
                }
                glEnd();

                if (dl)
                    glEndList();
            }
        }
    }
}

void render(ase::model * model)
{
    vtxprg_geom_it it = vp_geom_array.begin();
    while (it != vp_geom_array.end())
    {
        render_geometry(*it);
        ++it;
    }
}

void display()
{
    static char buf[32];
    static char txt[256];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    camera.apply_inverse_transform();
    object.apply_transform();

    glUseProgramObjectARB(programObject);

    for (unsigned int i = 0; i < ttime.size(); ++i)
    {
        glPushMatrix();
        glTranslatef(-.3 * (i % 10),.5 * (i / 10), 0);
        
        update_anim(model, ttime[i], false);
        if (paused == false)
            ttime[i]++;

        render(model);
        glPopMatrix();
    }
    glPopMatrix();

    glUseProgramObjectARB(0);

    GLErrorReport();

    glutSwapBuffers();

    fps.frame();
    if (fps.timing_updated())
        sprintf(buf,"FPS: %f", fps.get_fps());

    sprintf(txt, "Display List: %s - Bone processing (%d to %d) - %s", dl ? "on" : "off", min_vtxskin, max_vtxskin, buf);
    glutSetWindowTitle(txt);
}
