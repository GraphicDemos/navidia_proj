/*********************************************************************NVMH1****
File:
vtxprg_skinning.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/

#ifndef __vtxprg_skin_h__
#define __vtxprg_skin_h__

#include <nv_ase.h>

struct vtxprg_geom
{
    std::string     name;           // Convenience for debugging

    int             numvertices;    // number of vertices

                                    // Each vertex offset array 0 - 3 will be copied into
                                    // glTexCoordPointer array 5 - 7

    float *         v;              // array of vertices
    float *         n;              // array of normals
    float *         w;              // array of weights

    float *         v_offset[4];    // arrays of vertex offsets
    float *         n_offset[4];    // arrays of normal offsets, and weights in w

    // Tangent offsets are a little more complicated
    float *         t_offset[4];    // arrays of tangent offsets

    int             numt;           // number of texture coords
    float *         t;              // texture coords array

    int             matidx;         // material index into the ase::model

                                    // We will use VertexPointer to place the indices

    int *           matbin_idx;     // index of which matrix bin to use
    float *         v_matidx;       // Array that contains the index of the matrix data
                                    // in the program data space

                                    // Will be used to load the proper vertex program

    unsigned int *  v_numbones;     // Array that contains the number of bones used
                                    // to blend a vertex

    int *           num_bones_ref;  // number of matrix bins
    ase::geomobj *** bones_ref;      // array of pointers to bones
                                    // fixed to 4 bins (4 * 29 matrices = 116 bones!)

    int *           num_bonesperface[4];    // number of faces having vertices referencing 1..4 bones per bin
    int **          vidx_boneperfaces[4];   // array of vertex indices per bin

    int *           num_faces;      // number of faces per bin
    int **          faces;          // We have to sort faces per bins

    int             numbins;        // number of bins - a bin is a set of transforms (i.e. bones)

    unsigned int  * dl[2];          // display list
};

typedef std::vector<vtxprg_geom*>   vtxprg_geom_array;
typedef vtxprg_geom_array::iterator vtxprg_geom_it;

#endif // __vtxprg_skin_h__
