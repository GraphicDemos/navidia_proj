/*********************************************************************NVMH2****
Copyright (C) 1999, 2000, 2001 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

******************************************************************************/

// Interface for mesh loaders. See below for design issues etc. 

#ifndef __NV_MESH_IO_H
#define __NV_MESH_IO_H

#ifdef _WIN32
#pragma warning (disable : 4786)
//#pragma comment (lib, "nv_math.lib")
#endif

#include <stdio.h>
#include <vector>

// Function linkage
#if NVIO_SHARED

#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined __MINGW32__
#  ifdef NVIO_EXPORTS
#    define NVIO_API __declspec(dllexport)
#  else
#    define NVIO_API __declspec(dllimport)
#  endif
#endif //
#else
#  define NVIO_API
#endif //NVIO_SHARED


 

namespace nvMeshAttribute
{
	enum {
		eVertexPosition = 0,
		eVertexNormal,
		eTriangleIndices,
		eTexCoord0
	};

	static const char* TRIANGLE_INDICES = "TRIANGLE_INDICES";

	// Per-vertex attributes
	static const char* VERTEX_POSITION = "VERTEX_POSITION";
	static const char* VERTEX_NORMAL = "VERTEX_NORMAL";
	static const char* TEX_COORD0 = "TEX_COORD0";

	// Group types
	static const char* SMOOTHING_GROUP = "SMOOTHING_GROUP";
	static const char* TEXCOORD_GROUP = "TEXCOORD_GROUP";
	
}

struct NVIO_API nvTriInfo
{
	int index;
	int m_geometryGroupIndex;
	int m_positionIndices[3];
	int m_smoothingGroupIndex;
	int m_normalIndices[3];
	int m_texCoordGroupIndex;
	int m_textureIndices[3];
};

struct NVIO_API  nvMeshIO
{
	virtual ~nvMeshIO() {};

	// File extensions recognized by loader.
	virtual const std::vector<const char*>& GetFileExtensions() const = 0;

	// Set the current file to be read. This should be set first before making any
	// subsequent calls. Returns 0 if it could not open the file.
	virtual int ReadFile(const char* fileName) = 0;

	/***** Basic Mesh Info *****/
	
	// Get the number of triangles in base mesh:
	virtual size_t GetNumTriangles() = 0;
	// Get the triangle indices: 
	virtual void GetTriangleIndices(std::vector<int>& triIndices) = 0; 

	/***** Mesh Per-Vertex Attributes *****/
	// Returns the list of attributes (as strings) that are provided in the current file. Returns 0
	// if it did not have a file to read.
	virtual int GetAttributes(std::vector<const char*>& attributes) = 0;
	
	virtual int GetAttributesAndValues(std::vector<const char*>& attributesRead, 
		std::vector<std::vector<float> >& attributeValueArrays, std::vector<size_t>& attributeSizes,
		std::vector<size_t>& attributeCounts) = 0;

	virtual int GetAttributeValues(const char* attribute, std::vector<float>& attributeValues, 
		size_t& attributeSize, size_t& attributeCount) = 0;

	
	/***** Group Info *****/
	// Get group 'types', e.g. SMOOTHING_GROUP, TEXCOORD_GROUP, ...
	virtual void GetGroupTypes(std::vector<const char*>& groupTypes) = 0;
	// Get the 'names' of groups for a given attribute:
	virtual int GetGroups(const char* groupType, std::vector<const char*>& groupNames) = 0;
	// Get Group Info for given attribute, groupName,On return,triangleIndices will return the set of triangles which 
	// form the group, attributePresent indicates whether attribute was actually present in the file (e.g. a file
	// could define texture coordinate groups but not provide the coordinates themselves), If attributePresent is true,
	// the vector triangleAttributeIndices will be filled with the indices for each triangle - these index into
	// the vector returned by GetAttributeValues(attribute) below.
	virtual int GetGroupInfo(const char* attribute, const char* groupName, 
		std::vector<int>& triangleIndices, bool& attributePresent,
		std::vector<int>& triangleAttributeIndices, std::vector<int>& indexedTriangles) = 0;

};

/*
	A mesh MUST have i) vertex positions and ii) triangle indices. 
	It is assumed that the mesh will be triangulated by the loader for faces with > 3 sides (or in the case of
	surfaces)
	
	Groups: For a given attribute, the faces are partitioned into groups. It is assumed that these groups are
	DISJOINT. Thus, e.g., you cannot assign two diffuse maps, or two smoothing groups which overlap. Each vertex
	attribute must have a unique value. 
	
*/

#endif

