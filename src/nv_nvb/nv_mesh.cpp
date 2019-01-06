/*********************************************************************NVMH1****
File:
nv_mesh.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    
******************************************************************************/

#include "stdafx.h"

#include <vector>
#include <set>

using namespace std;

nv_mesh::nv_mesh()
{
    material_id = NV_BAD_IDX;
    num_vertices = 0;
    vertices = 0;
    normals = 0;
    colors = 0;

    skin = false;
    weights = 0;        // vertex weights
    bone_idxs = 0;      // 4 bones per vertex.

    num_texcoord_sets = 0;
    texcoord_sets = 0;
    num_faces = 0;
    faces_idx = 0;
    // bounding box information...
    aabb_min = vec3( FLT_MAX, FLT_MAX, FLT_MAX);
    aabb_max = vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX);
}

nv_mesh::~nv_mesh()
{
    if (num_vertices)
    {
        delete [] vertices;
        num_vertices = 0;
        vertices = 0;
    }

    if (normals)
    {
        delete [] normals;
        normals = 0;
    }

    if (colors)
    {
        delete [] colors;
        colors = 0;
    }

    if (num_texcoord_sets)
    {
        delete [] texcoord_sets;
        texcoord_sets = 0;
        num_texcoord_sets = 0;
    }

    if (num_faces)
    {
        delete [] faces_idx;
        num_faces = 0;
    }

    if (weights)
    {
        delete [] weights;
        weights = 0;
    }

    if (bone_idxs)
    {
        delete [] bone_idxs;
        bone_idxs = 0;
    }
}


        /// Write a mesh to an nv_output_stream.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const nv_mesh & rMesh)
{
    rOutputStream << rMesh.material_id;

                                // Geometry
    rOutputStream << rMesh.num_vertices;

    unsigned int iVertex;
    for (iVertex = 0; iVertex < rMesh.num_vertices; ++iVertex)
    {
        rOutputStream << rMesh.vertices[iVertex];
        rOutputStream << rMesh.normals[iVertex];
        rOutputStream << rMesh.colors[iVertex];
    }

                                // Skinning
    rOutputStream << rMesh.skin;

    if (rMesh.skin)
    {
        for (iVertex = 0; iVertex < rMesh.num_vertices; ++iVertex)
        {
            rOutputStream << rMesh.weights[iVertex];
            rOutputStream << rMesh.bone_idxs[4*iVertex + 0] << rMesh.bone_idxs[4*iVertex + 1]
                          << rMesh.bone_idxs[4*iVertex + 2] << rMesh.bone_idxs[4*iVertex + 3]; 
        }
    }

                                // Texture coordinate sets
    rOutputStream << rMesh.num_texcoord_sets;
    for (unsigned int iTexCoordSet = 0; iTexCoordSet < rMesh.num_texcoord_sets; ++iTexCoordSet)
    {
                                // texcoord_set doesn't know the length of it's arrays,
                                // so we have to pass it here.
        rMesh.texcoord_sets[iTexCoordSet].num_coords = rMesh.num_vertices;
        rOutputStream << rMesh.texcoord_sets[iTexCoordSet];
    }
                                // Face information
    rOutputStream << rMesh.num_faces;
    for (iVertex = 0; iVertex < rMesh.num_faces*3; ++iVertex)
        rOutputStream << rMesh.faces_idx[iVertex];
                                // Attributes
    rOutputStream << rMesh.mesh_attr;

                                // Bounding box 
    rOutputStream << rMesh.aabb_min << rMesh.aabb_max;

    return rOutputStream;
}

        /// Read a mesh from an nv_input_stream.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_mesh & rMesh)
{
    rInputStream >> rMesh.material_id;

                                // Geometry
    rInputStream >> rMesh.num_vertices;

    if (0 == rMesh.num_vertices)
    {
        rMesh.vertices = 0;
        rMesh.normals  = 0;
        rMesh.colors   = 0;
    }
    else
    {
        rMesh.vertices = new vec3[rMesh.num_vertices];
        rMesh.normals  = new vec3[rMesh.num_vertices];
        rMesh.colors   = new vec4[rMesh.num_vertices];
    }

    unsigned int iVertex;
    for (iVertex = 0; iVertex < rMesh.num_vertices; ++iVertex)
    {
        rInputStream >> rMesh.vertices[iVertex];
        rInputStream >> rMesh.normals[iVertex];
        rInputStream >> rMesh.colors[iVertex];
    }

                                // Skinning
    rInputStream >> rMesh.skin;

    if (rMesh.skin)
    {
        rMesh.weights   = new vec4[rMesh.num_vertices];
        rMesh.bone_idxs = new nv_idx[4 * rMesh.num_vertices];

        for (iVertex = 0; iVertex < rMesh.num_vertices; ++iVertex)
        {
            rInputStream >> rMesh.weights[iVertex];
            rInputStream >> rMesh.bone_idxs[4*iVertex + 0] >> rMesh.bone_idxs[4*iVertex + 1]
                         >> rMesh.bone_idxs[4*iVertex + 2] >> rMesh.bone_idxs[4*iVertex + 3]; 
        }
    }
    else
    {
        rMesh.weights   = 0;
        rMesh.bone_idxs = 0;
    }

                                // Texture coordinate sets
    rInputStream >> rMesh.num_texcoord_sets;

    if (0 == rMesh.num_texcoord_sets)
        rMesh.texcoord_sets = 0;
    else
        rMesh.texcoord_sets = new nv_texcoord_set[rMesh.num_texcoord_sets];

    for (unsigned int iTexCoordSet = 0; iTexCoordSet < rMesh.num_texcoord_sets; ++iTexCoordSet)
    {
        rInputStream >> rMesh.texcoord_sets[iTexCoordSet];
    }

                                // Face information
    rInputStream >> rMesh.num_faces;

    if (0 == rMesh.num_faces)
        rMesh.faces_idx = 0;
    else
        rMesh.faces_idx = new nv_idx[3*rMesh.num_faces];

    for (iVertex = 0; iVertex < rMesh.num_faces*3; ++iVertex)
        rInputStream >> rMesh.faces_idx[iVertex];

                                // Attributes
    rInputStream >> rMesh.mesh_attr;

                                // Bounding box 
    rInputStream >> rMesh.aabb_min >> rMesh.aabb_max;

    return rInputStream;
}

//
// Public methods
//

        // calculateTangentSpaces
        //
        // Description:
        //      Calculates tangent-space information for a UV set.
        //          The tangen-space information consists of the
        //      per-vertex tangent and binormal vectors stored with
        //      each UV set.
        //
        // Parameters:
        //      iUVSet - UV set index.
        //
        // Returns:
        //      true - on success,
        //      flase - otherwise.
        //
        bool
nv_mesh::calculateTangentSpaces(unsigned int iUVSet)
{
    // Calculate neighbor information for every vertex,
    // consisting of an array with all adjacent vertices.

    vector<set<nv_idx> > aAdjacentVertices(num_vertices);

    for (unsigned int iTriangle = 0; iTriangle < num_faces; ++iTriangle)
    {
        aAdjacentVertices[faces_idx[3*iTriangle + 0]].insert(faces_idx[3*iTriangle + 1]);
        aAdjacentVertices[faces_idx[3*iTriangle + 0]].insert(faces_idx[3*iTriangle + 2]);

        aAdjacentVertices[faces_idx[3*iTriangle + 1]].insert(faces_idx[3*iTriangle + 2]);
        aAdjacentVertices[faces_idx[3*iTriangle + 1]].insert(faces_idx[3*iTriangle + 0]);

        aAdjacentVertices[faces_idx[3*iTriangle + 2]].insert(faces_idx[3*iTriangle + 0]);
        aAdjacentVertices[faces_idx[3*iTriangle + 2]].insert(faces_idx[3*iTriangle + 1]);
    }

    // Calculate a deltaU for each vertex and use it as weight
    // for each vector from the center vertex to the neighboring
    // vertex.
    for (unsigned int iVertex = 0; iVertex < num_vertices; ++iVertex)
    {
        for (unsigned int iUVSet = 0; iUVSet < num_texcoord_sets; ++iUVSet)
        {
            texcoord_sets[iUVSet].tangents[iVertex] = vec3(0.0f, 0.0f, 0.0f);

            for (set<nv_idx>::const_iterator iAdjacent = aAdjacentVertices[iVertex].begin();
                 iAdjacent != aAdjacentVertices[iVertex].end();
                 ++iAdjacent)
            {
                float nDeltaU = texcoord_sets[iUVSet].texcoords[2*(*iAdjacent)] 
                              - texcoord_sets[iUVSet].texcoords[2*iVertex];
                vec3  vDeltaP = vertices[*iAdjacent] - vertices[iVertex];

                texcoord_sets[iUVSet].tangents[iVertex] += (nDeltaU / dot(vDeltaP, vDeltaP)) * vDeltaP;
            }
                                // Calculate the binormal as the cross-product of the tangent
                                // and the normal.
            cross(texcoord_sets[iUVSet].binormals[iVertex], 
                  normals[iVertex], 
                  texcoord_sets[iUVSet].tangents[iVertex]);
                                // If degenerate case we try alternative tangents, 
                                // until we're successful.
            float nLength = nv_norm(texcoord_sets[iUVSet].binormals[iVertex]);
            while (nLength == 0.0f)
            {
                texcoord_sets[iUVSet].tangents[iVertex] += vec3(0.1f, -0.2f, 0.3f);
                cross(texcoord_sets[iUVSet].binormals[iVertex], 
                      normals[iVertex], 
                      texcoord_sets[iUVSet].tangents[iVertex]);
                nLength = nv_norm(texcoord_sets[iUVSet].binormals[iVertex]);
            }
                                // Normalize the binormal.
            scale(texcoord_sets[iUVSet].binormals[iVertex], 1.0f/nLength);
                                // Recalculate the tangent as the cross-product of the binormal and
                                // the normal.
            cross(texcoord_sets[iUVSet].tangents[iVertex], 
                  texcoord_sets[iUVSet].binormals[iVertex],
                  normals[iVertex]);
        }
    }
    
    return true;
}
