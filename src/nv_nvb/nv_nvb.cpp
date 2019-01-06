/*********************************************************************NVMH1****
File:
nv_nvb.cpp

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
    Defines the entry point for the DLL application.
    
******************************************************************************/

#include "stdafx.h"
#include <nv_nvbloader/NVBLoader.h>
#include "nv_nvbfactory.h"
#include <NVBFile.h>

NVBFactory  TheFactory;

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

bool NVBLoad(const char * file, nv_scene * scene, long options )
{
    // Find the file somewhere in registered paths
    String sFilename;

    // Check the file exists
    if(!FindFile(file, &sFilename))
    {
        std::cerr << ("File not found.", NVB_ERROR_FILENOTFOUND);
        return false;
    }

    nv_file oFile;

    oFile.open(sFilename);

    char * sFileHeader = new char[5];
    sFileHeader[4] = '\0';
    oFile.read(sFileHeader, 4);

    if (!strcmp(sFileHeader, "NVBP") || !strcmp(sFileHeader, "NVB!"))
    {
        oFile.close();

        gNVB_options = options;
        TheFactory.SetScene(scene);

        return TheFactory.Import(file);
    }
    else if (!strcmp(sFileHeader, "NVB2"))
    {
        unsigned int nMajorVersion;
        unsigned int nMinorVersion;

        oFile >> nMajorVersion >> nMinorVersion;

        oFile >> *scene;

        oFile.close();

        if (NVB_LHS == options)
            changeHandedness(scene);        
    }
    else
    {
        std::cerr << "Invalid file format" << std::endl;

        return false;
    }

    return true;
}

void NVBSetLogCallback(void* cbfn, unsigned long userparam)
{
	TheFactory.SetLogCallback((TloggingCB)cbfn, userparam);
}


        // changeHandedness
        //
        // Description:
        //      Converte scene data's handedness.
        //          Depending on what graphics library is used it
        //      is more convenient to have all sence data specified with
        //      respect to a righthanded coordinate system, as opposed to
        //      left-handed coordinate system.
        //          This is a helper method that converts the handedness of
        //      a scene in place.
        // 
        // Parameters:
        //      pScene - pointer to the nv_scene to be changed.
        //
        // Returns:
        //      true  - on success,
        //      flase - otherwise.
        //
        bool
changeHandedness(nv_scene * pScene)
{
    for (unsigned int iNode = 0; iNode < pScene->num_nodes; ++iNode)
    {
        nv_node * pNode = pScene->nodes[iNode];
        switch(pNode->get_type())
        {
            case nv_node::ANONYMOUS:
            {
                if (!changeHandedness(pNode))
                    return false;
            }
            break;

            case nv_node::GEOMETRY:
            {
                if (!changeHandedness(pNode))
                    return false;
                if (!changeHandedness(static_cast<nv_model *>(pNode)))
                    return false;
            }
            break;

            case nv_node::LIGHT:
            {
                if (!changeHandedness(pNode))
                    return false;
                if (!changeHandedness(static_cast<nv_light *>(pNode)))
                    return false;
            }
            break;
            case nv_node::CAMERA:
            {
                if (!changeHandedness(pNode))
                    return false;
                if (!changeHandedness(static_cast<nv_camera *>(pNode)))
                    return false;
            }
            break;
        }
    }

    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an nv_node's handedness.
        //          Nodes contain a transform matrix that needs to
        //      be changed depending on the handedness of the coordinate
        //      system.
        //
        // Parameters:
        //      pNode - pointer to the nv_node to be changed.
        //
        // Returns:
        //      true  - on success,
        //      false - otherwise.
        //
        bool
changeHandedness(nv_node * pNode)
{
    if (!changeHandedness(&pNode->xform))
        return false;
    if (!changeHandedness(&pNode->anim))
        return false;

    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an nv_model's handedness.
        //          All positions and normals switch handedness.
        //
        // Parameters:
        //      pModel - pointer to the nv_model node to be changed.
        //
        // Returns:
        //      true  - on success,
        //      false - otherwise.
        //
        bool
changeHandedness(nv_model * pModel)
{
    for (unsigned int iMesh = 0; iMesh < pModel->num_meshes; ++iMesh)
        changeHandedness(&pModel->meshes[iMesh]);

    pModel->aabb_min.z *= -1.0f;
    pModel->aabb_max.z *= -1.0f;
    
    return true;;
}

        // changeHandedness
        //
        // Description:
        //      Convert an nv_mesh's handedness.
        //          All positions and normals switch handedness.
        //
        // Parameters:
        //      pMesh - pointer to the nv_mdesh to be changed.
        //
        // Returns:
        //      true  - on success,
        //      false - otherwise.
        //
        bool
changeHandedness(nv_mesh * pMesh)
{
    for (unsigned int iVertex = 0; iVertex < pMesh->num_vertices; ++iVertex)
    {
        pMesh->vertices[iVertex].z *= -1.0f;
        pMesh->normals [iVertex].x *= 1.0f;
        pMesh->normals [iVertex].y *= 1.0f;
        pMesh->normals [iVertex].z *= -1.0f;
    }
    
    pMesh->aabb_min.z *= -1.0f;
    pMesh->aabb_max.z *= -1.0f;

    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an nv_light's handedness.
        //          Directional lights store the light direction
        //      as a vector.
        //
        // Parameters:
        //      pLight - pointer to the nv_light to be changed.
        //
        // Returns:
        //      true  - on success,
        //      flase - otherwise.
        //
        bool
changeHandedness(nv_light * pLight)
{
    pLight->direction.z *= -1.0f;

    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an nv_camera's handedness.
        //
        // Parameters:
        //      pCamera - pointer to the nv_camera to be changed.
        //
        // Returns:
        //      true  - on success,
        //      flase - otherwise.
        //
        bool
changeHandedness(nv_camera * pCamera)
{
    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an animation's handedness.
        //
        // Parameters:
        //      pAnimation - pointer to the nv_animation to be changed.
        //
        // Returns:
        //      true  - on success,
        //      flase - otherwise.
        //
        bool
changeHandedness(nv_animation * pAnimation)
{

    for (unsigned int iKey = 0; iKey < pAnimation->num_keys; ++iKey)
    {
        if (pAnimation->rot != 0)
            changeHandedness(&pAnimation->rot[iKey]);
        if (pAnimation->pos != 0)
            pAnimation->pos[  iKey].z *= -1.0f;
        if (pAnimation->scale != 0)
            pAnimation->scale[iKey].z *= -1.0f;
    }

    return true;
}

        // changeHandedness
        //
        // Description:
        //      Convert an mat4's handedness.
        //
        // Parameters:
        //      pMatrix - pointer to the mat4 to be changed.
        //
        // Returns:
        //      true  - on success,
        //      false - otherwise.
        //
        bool
changeHandedness(mat4 * pMatrix)
{
    mat4 mMirrorZ = mat4_id;
    mMirrorZ.a22 = -1.0f;
    mat4 tmp;

    mult(tmp, *pMatrix, mMirrorZ);
    mult(*pMatrix, mMirrorZ, tmp);

    return true;
}


        // changeHandedness
        //
        // Description:
        //      Convert a quat's handedness.
        //
        // Parameters:
        //      pQuaternion - pointer to the quat to be changed.
        //
        // Returns:
        //      true  - on success,
        //      flase - otherwise.
        //
        bool
changeHandedness(quat * pQuaternion)
{
                                // A change of handedness inverts all rotations.
                                // The inverse of a unit quaternion is its conjugate.
                                // Thus the ceapest way to invert the effect of this
                                // unit-quaternion's rotation is to conjugate the
                                // quaternion.
    // conj(*pQuaternion, *pQuaternion);

    pQuaternion->z *= -1.0f;
    pQuaternion->w *= -1.0f;

    return true;
}
