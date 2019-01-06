
/*********************************************************************NVMH2****
Copyright (C) 1999, 2000, 2001 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments: 
	Wrapper class for nvObjIO. Provides a single entry point, so that you don't
	have to do the preprocessing to figure out smoothing groups etc.
  

******************************************************************************/

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif

#include <vector>
#include <map>
#include "nvMeshIO.h"
class NVIO_API nvObjReader
{
public:
	virtual ~nvObjReader();
	// Flags are not defined yet, but will be things like compute normals blah...
	virtual int ReadFile(const char* fileName, unsigned int flags);
	virtual int ReadAndMungeFile(const char* fileName, unsigned int flags,
		unsigned int& numVertices,
		std::vector<int>& indices, 
		std::vector<float>& vertexPositions,
		std::vector<float>& vertexNormals,
		std::vector<float>& vertexTangents,
		std::vector<float>& vertexBinormals,
		std::vector<float>& vertexTexCoords);
	
	// Since STL vectors are packed you can use &triangleIndices[0] directly as an int*
	virtual bool GetTriangleIndices(std::vector<int>& triangleIndices);
	// Similarly for ith attribute, &attributeValues[i][0] is a float* that can be used directly.
	virtual bool GetAttribute(const char* attributeName, std::vector<float>& attributeValues);
	

private:
	void Reset();
private:
	std::vector<const char*> m_attributes;
	std::vector<std::vector<float> > m_attributeArrays;
	std::vector<size_t> m_attributeSizes;
	std::vector<size_t> m_attributeCounts;
	std::vector<int> m_triangleIndices;

	std::vector<int> m_newTriangleIndices;
	std::vector<float> m_newVertexPositions;
	std::vector<float> m_newVertexNormals;
	std::vector<float> m_newTexCoords;
	bool m_fileReadOkay;
	std::map<const char*, std::vector<float> > m_nameToVectorMap;
};

