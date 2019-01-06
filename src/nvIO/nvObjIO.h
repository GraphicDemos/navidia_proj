/*********************************************************************NVMH2****
Copyright (C) 1999, 2000, 2001 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments: 
	See at end of file for how smoothing groups etc. are handled.

	Current limitations:
	1.	Only the first group in a group statement is processed. The rest are ignored.
		Thus a face can be only 1 smoothing group.
	2.  Only 2d texture coordinates are supported. 1D and 3D left for future.
	3.  Negative referencing of vertex indices is not supported.
	
  

******************************************************************************/

#ifndef __NV_OBJ_IO_H
#define __NV_OBJ_IO_H

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif

#include "nvMeshIO.h"

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif

#include <map>
#include <vector>
#include <string>

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif

struct NVIO_API nvObjGroup
{
	nvObjGroup() {
		m_numTriangles = 0;
	}
	std::string m_name;
	int m_id;
	unsigned int m_numTriangles;
	std::vector<int> m_triangleIndices; // Triangles in this group. These are the indices into the 
		// vertex position indices.
	std::vector<int> m_dataIndices; // Indices into the data associated with this group. For smoothing
		// groups, e.g., this will be normals.
	std::vector<int> m_indexedTriangles; // m_indexedTriangles[i] returns the 'name' or 'index' of a triangle
		// in the list of all triangles.
	
	
};

class NVIO_API nvObjIO : public nvMeshIO
{
public:
	nvObjIO();
	virtual ~nvObjIO();
	virtual const std::vector<const char*>& GetFileExtensions() const;
	virtual int ReadFile(const char* fileName);
	
	virtual int GetAttributes(std::vector<const char*>& attributes);
	
	virtual size_t GetNumTriangles();
	virtual void GetTriangleIndices(std::vector<int>& triIndices); 
	
	virtual void GetGroupTypes(std::vector<const char*>& groupTypes);
	virtual int GetGroups(const char* groupType, std::vector<const char*>& groupNames);

	// For given group type and name, get info: trianglePositionIndices are the indices into 
	// the vertex position array, triangleAttributeIndices are the indices into the actual attribute
	// array (e.g. for smoothing groups this is normals, for texcoord groups this is texcoords),
	// indexedTriangles is the list of 'named' or indexed triangles that are in this group, size of
	// indexedTriangles is therefore 1/3 of the size of trianglePositionIndices and of triangleAttributeIndices.
	virtual int GetGroupInfo(const char* groupType, const char* groupName, 
		std::vector<int>& trianglePositionIndices, bool& attributePresent,
		std::vector<int>& triangleAttributeIndices, std::vector<int>& indexedTriangles);

	virtual int GetAttributesAndValues(std::vector<const char*>& attributesRead, 
		std::vector<std::vector<float> >& attributeValueArrays, std::vector<size_t>& attributeSizes,
		std::vector<size_t>& attributeCounts);
	virtual int GetAttributeValues(const char* attribute, std::vector<float>& attributeValues, 
		size_t& attributeSize, size_t& attributeCount);	

	virtual int GetTriInfo(std::vector<nvTriInfo>& triInfo);
private: // methods
	void Reset();
	int FirstPass();
	int SecondPass();
	
	bool ParseFace();
	int GetSmoothingGroups(std::vector<const char*>& groupNames);
	int GetSmoothingGroupInfo(const char* groupName, std::vector<int>& triangleIndices,
		bool& attributePresent, std::vector<int>& normalIndices, std::vector<int>& indexedTriangles);
	int GetTexCoordGroups(std::vector<const char*>& groupNames);
	int GetTexCoordGroupInfo(const char* groupName, std::vector<int>& triangleIndices,
		bool& attributePresent, std::vector<int>& texCoordIndices, std::vector<int>& indexedTriangles);

	int CheckAttributes(const std::vector<const char*>& attributes);
	
		
	void ReadSmoothingGroup(const char* buf);
	nvObjGroup* FindOrCreateSmoothingGroup(const char* group);

	nvObjGroup* FindOrCreateTexCoordGroup(const char* group);

	void UpdateGroups(int vertexIndex, int texCoordIndex, int normalIndex);
	void UpdateGroups(int numTriangles);
	void DetermineTexCoordSize(char* buf);
	void UpdateTriInfo(int i, int vertexIndex, int texCoordIndex, int normalIndex, nvTriInfo& triInfo);
	
private:
	bool m_dirty; // Flag to indicate whether current file was changed.
	FILE* m_fp;	  // Current file.
	bool m_fileRead;
	std::vector<const char*> m_fileExtensions;
	std::vector<const char*> m_attributes;			// Attributes found in current file.

	unsigned int m_numVertices;
	unsigned int m_numNormals;
	unsigned int m_numTexCoords;	
	unsigned int m_numTriangles;
	unsigned int m_texCoordSize;
	unsigned int m_curNumTexCoords; // used in second pass for negative offsets.

	std::vector<float> m_vertexPositions2;
	std::vector<float> m_vertexNormals2;
	std::vector<int> m_triangleIndices;
	std::vector<int> m_texCoordIndices2;
	std::vector<float> m_texCoords2;
	std::vector<int> m_normalIndices2;

	//float* m_vertexPositions;
	//float* m_vertexNormals;
	//int* m_triangleIndices;		// Deliberately not unsigned, because obj can have negative indices.
	//float* m_texCoords;

	typedef std::map<std::string, nvObjGroup*> tnameToSmoothingGroupMap;
	typedef std::map<std::string, nvObjGroup*> tnameToTexCoordGroupMap;

	// Smoothing groups.
	nvObjGroup* m_curSmoothingGroup;
	tnameToSmoothingGroupMap m_nameToSmoothingGroupMap;
	std::vector<nvObjGroup*> m_smoothingGroups; // Array of smoothing groups 

	// Tex coord groups
	nvObjGroup* m_curTexCoordGroup;
	tnameToTexCoordGroupMap m_nameToTexCoordGroupMap;
	std::vector<nvObjGroup*> m_texCoordGroups;

	std::vector<nvTriInfo> m_triInfos;

};

/* 
	Smoothing Groups:
		Syntax: s <int> OR s off
		If the vn token is not used, then this token can be used to specify a group of faces for normal
		calculation. To fit the API, we process smoothing groups by splitting vertices in multiple smoothing
		groups into multiple vertices, one for each smoothing group that it participates in. Note that this loses 
		topological information but is a straightforward way of processing them while keeping the API simple. 
		(Alternately we could retain topological information, but we would have to change the API in nvMeshIO.)
		Smoothing group 0 is considered the default smoothing group.

    Texture Coordinate Groups:
		Obj does not directly support texture coordinate groups. The texture coordinates assigned to a vertex
		and directly defined in terms of 'texture vertices', which have the syntax: vt <float> <float>
		The _texture images_ that a texture is mapped to are given by the material groups. Example, diffuse map,
		is set per material. In this loader, we keep the coordinates separate from the 'maps'. Thus, there is only
		ONE texture coordinate group in general, unless texture coordinates are not specified at all from 
		some 'geometry' groups. In this case we create additional groups where the data is not present.

*/

#endif
