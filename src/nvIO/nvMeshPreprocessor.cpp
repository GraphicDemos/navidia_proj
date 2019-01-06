/*********************************************************************NVMH2****
Copyright (C) 1999, 2000, 2001 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

******************************************************************************/

#include "nvMeshPreprocessor.h"
#include <assert.h>
#include "nvMeshIO.h"

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif



typedef struct
{
	int data[3];
} TriInt;

nvMeshPreprocessor::~nvMeshPreprocessor()
{
	tnameToSmoothingGroupMap::iterator itrGroupMap = m_nameToSmoothingGroupMap.begin();
	while (itrGroupMap != m_nameToSmoothingGroupMap.end())
	{
		delete itrGroupMap->second;
		itrGroupMap++;
	}
}

nvMeshPreprocessor::nvMeshPreprocessor() : 
	m_done(false), m_numVertices(0), m_vertexPositions(0), m_numNormals(0), 
	m_vertexNormals(0),m_numTexCoords(0),m_texCoordSize(2),m_allNormalsDefined(true),
	m_allTexcoordsDefined(true)
{
}

int nvMeshPreprocessor::SetAttributeData(const char* attribute, const void* data, 
							const size_t& attributeSize, const size_t& numElements)
{
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_POSITION))
	{
		m_numVertices = numElements;
		m_vertexPositions = (float*)data; // Need to do something about this!
		return 1;
	}
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_NORMAL))
	{
		m_numNormals = numElements;
		m_vertexNormals = (float*)data;
		return 1;
	}
	if (!strcmp(attribute,nvMeshAttribute::TEX_COORD0))
	{
		m_numTexCoords = numElements;
		m_texCoords = (float*)data;
		m_texCoordSize = attributeSize;
	}
	if (!strcmp(attribute,nvMeshAttribute::TRIANGLE_INDICES))
	{
		m_numTriangles = numElements;
		m_triangleIndices = (int*)data;
		return 1;
	}

	return 0;
}
 
int nvMeshPreprocessor::SetGroupInfo(const char* groupType, const char* groupName, 
	const std::vector<int>& groupTriangleIndices, const std::vector<int>& groupAttributeIndices,
	const std::vector<int>& indexedTriangles)
{
	if (!strcmp(groupType,nvMeshAttribute::SMOOTHING_GROUP))
	{
		if (m_numVertices == 0 || m_vertexPositions == 0)
			return 0;

		// If attributes are not present that's okay, but if they are then the two arrays should match:
		if (groupTriangleIndices.size() != groupAttributeIndices.size() && groupAttributeIndices.size() != 0)
			return 0;

		// Empty groups are not allowed.
		if (groupTriangleIndices.size() == 0)
			return 0;

		// Create a new group and stick it in the map for smoothing groups.
		nvPreprocessorGroup* g = new nvPreprocessorGroup();
		g->m_name = groupName;
		g->m_numTriangles = groupTriangleIndices.size();
		g->m_triangleIndices = groupTriangleIndices;
		g->m_dataIndices = groupAttributeIndices;
		g->m_indexedTriangles = indexedTriangles;
		m_nameToSmoothingGroupMap[std::string(groupName)] = g;
	}
	else if (!strcmp(groupType,nvMeshAttribute::TEXCOORD_GROUP))
	{
		if (m_numVertices == 0 || m_vertexPositions == 0)
			return 0;
		if (groupTriangleIndices.size() != groupAttributeIndices.size() && groupAttributeIndices.size() != 0)
			return 0;
		if (groupTriangleIndices.size() == 0)
			return 0;

		// Create a new group and stick in the map for texcoord groups.
		nvPreprocessorGroup* g = new nvPreprocessorGroup();
		g->m_name = groupName;
		g->m_numTriangles = groupTriangleIndices.size();
		g->m_triangleIndices = groupTriangleIndices;
		g->m_dataIndices = groupAttributeIndices;
		g->m_indexedTriangles = indexedTriangles;
		m_nameToTexCoordGroupMap[std::string(groupName)] = g;
	}
	
	return 0;
}

size_t nvMeshPreprocessor::GetNewNumElements(const char* attribute)
{
	if (!m_done)
	{
		PreprocessTriangles();
		//ProcessSmoothingGroups();
		//ProcessTexCoordGroups();
		m_done = true;
	}
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_POSITION))
		return m_newNumVertices;
	if (!strcmp(attribute,nvMeshAttribute::TRIANGLE_INDICES))
		return m_newTriangleIndices.size()/3;
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_NORMAL))
		return m_newNumVertices;
	if (!strcmp(attribute,nvMeshAttribute::TEX_COORD0))
		return m_newNumVertices;
	return 0;
}

size_t nvMeshPreprocessor::GetNewNumTriangles()
{
	if (!m_done)
	{
		PreprocessTriangles();
		//ProcessSmoothingGroups();
		//ProcessTexCoordGroups();
		m_done = true;
	}
	return m_newTriangleIndices.size() / 3;
}


int nvMeshPreprocessor::GetNewAttributeArray(const char* attribute, std::vector<float>& newArray)
{
	if (!m_done)
	{
		PreprocessTriangles();
		//ProcessSmoothingGroups();
		//ProcessTexCoordGroups();
		m_done = true;
	}
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_POSITION))
	{
		newArray = m_newVertexPositions;
		return 1;
	}
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_NORMAL))
	{
		if (m_allNormalsDefined)
			newArray = m_newVertexNormals;
		return 1;
	}
	if (!strcmp(attribute,nvMeshAttribute::TEX_COORD0))
	{
		if (m_allTexcoordsDefined)
			newArray = m_newTexCoords;
		return 1;
	}
	return 0;
}

void nvMeshPreprocessor::GetNewTriangleIndices(std::vector<int>& triangleIndices)
{
	triangleIndices = m_newTriangleIndices;
}

void nvMeshPreprocessor::SetTriInfo(const std::vector<nvTriInfo>& triInfo)
{
	m_triInfo = triInfo;
}

void nvMeshPreprocessor::ProcessSmoothingGroups()
{
	// Go over each group. For each vertex, we need to know how many groups it participates in. 
	// If vertex is in n groups, we will create n-1 new vertices. Each group needs to be informed of
	// the change by updating its triangle indices. If normal data exists, we need to assign the appropriate
	// normal data also to the new vertices. 

	m_newNumVertices = m_numVertices;
	std::map<std::string, nvPreprocessorGroup*>::iterator iter;
	// The following keeps track of which indices are already 'used' and by which group.
	std::map<unsigned int, nvPreprocessorGroup*> vertexIndexMap;
	m_newVertexPositions.clear();
	m_newVertexNormals.clear();
	for (unsigned int i=0;i<m_numVertices*3;++i)
		m_newVertexPositions.push_back(m_vertexPositions[i]);
	bool normalsAvailable;
	if (m_numNormals > 0)
		normalsAvailable = true;
	else
		normalsAvailable = false;
	std::map<unsigned int, TriFloat> newVertexIndexToNormalMap;

	for (iter=m_nameToSmoothingGroupMap.begin();iter!=m_nameToSmoothingGroupMap.end();++iter)
	{
		nvPreprocessorGroup* g = (*iter).second;
		std::vector<int> triIndices = g->m_triangleIndices;
		std::vector<int> indexedTriangles = g->m_indexedTriangles;
		assert(indexedTriangles.size()*3 == triIndices.size());
		// This keeps track of the new index assigned to a vertex. Only those
		// vertices already in vertexIndexMap will be put here:
		std::map<unsigned int, unsigned int> vertexIndexToNewIndexMap;

		std::map<unsigned int, nvPreprocessorGroup*>::iterator iter2;
		for (unsigned int i=0;i<triIndices.size();++i)
		{
			if (triIndices[i] >= (int)m_numVertices)
				assert(false); // sanity check.
			iter2 = vertexIndexMap.find(triIndices[i]);
			if (iter2 == vertexIndexMap.end())
			{
				vertexIndexMap[triIndices[i]] = g; // first ocurrence ofthis vertex.
				// save its associated normal.
				if (normalsAvailable) {
					assert(g->m_dataIndices[i] < (int)m_numNormals); // sanity check.
					TriFloat tf;
					for (int j=0;j<3;++j)
						tf.data[j] = m_vertexNormals[3*(g->m_dataIndices[i])+j];
					newVertexIndexToNormalMap[triIndices[i]] = tf;
					m_newVertexIndexToOldMap[triIndices[i]] = triIndices[i];
					m_newVertexIndexToTriangleIndexMap[triIndices[i]] = indexedTriangles[i/3];
				}
			}
			else {
				// vertex index already used by some group - check if it's this group.
				if (vertexIndexMap[triIndices[i]] != g) {
					// some other group used the index, check if we've mapped to a new index
					// already:
					std::map<unsigned int, unsigned int>::iterator iter3 = 
						vertexIndexToNewIndexMap.find(triIndices[i]);
					unsigned int newIndex;
					if (iter3 == vertexIndexToNewIndexMap.end())
					{
						// not mapped, assign new index.
						newIndex = m_newNumVertices;
						vertexIndexToNewIndexMap[triIndices[i]] = newIndex;
						m_newNumVertices++;
						// Copy over the old vertex position into new vertex position
						for (int j=0;j<3;++j)
							m_newVertexPositions.push_back(m_vertexPositions[3*triIndices[i]+j]);
						

						// Copy over the old normal data into new normal data:
						if (normalsAvailable)
						{
							TriFloat tf;
							for (int j=0;j<3;++j)
								tf.data[j] = m_vertexNormals[3*(g->m_dataIndices[i])+j];
							newVertexIndexToNormalMap[newIndex] = tf;
							
						}
						

						m_newVertexIndexToOldMap[newIndex] = triIndices[i];
						m_newVertexIndexToTriangleIndexMap[newIndex] = indexedTriangles[i/3];
					}
					else
						newIndex = vertexIndexToNewIndexMap[triIndices[i]];
					// Replace in triangleIndices:
					g->m_triangleIndices[i] = newIndex;
		
				}
				// else do nothing since it's the right index.
			}
		}
	}
	if (normalsAvailable)
	{
		// put all the normals in the newnormals array:
		for (unsigned int i=0;i<m_newNumVertices;++i)
		{
			// Find the appropriate trifloat: 
			std::map<unsigned int, TriFloat>::iterator iter2;
			iter2 = newVertexIndexToNormalMap.find(i);
			if (iter2 == newVertexIndexToNormalMap.end())
			{
				// this means that this is a vertex which is declared but not used in any of the faces.
				// Give it a normal of (0,0,0). 
				for (int j=0;j<3;++j)
					m_newVertexNormals.push_back(0.0f);
			}
			else {
				TriFloat tf = (*iter2).second;
				for (int j=0;j<3;++j)
					m_newVertexNormals.push_back(tf.data[j]);
			}
			

		}
	}
	// Write out the new triangle indices:
	m_newTriangleIndices.clear();
	for (iter=m_nameToSmoothingGroupMap.begin();iter!=m_nameToSmoothingGroupMap.end();++iter)
	{
		nvPreprocessorGroup* g = (*iter).second;
		for (unsigned int j=0;j<g->m_triangleIndices.size();++j)
			m_newTriangleIndices.push_back(g->m_triangleIndices[j]);
	}
}



struct TriIntCompare
{
	bool operator() (const TriInt& t1, const TriInt& t2) const
	{
		for (int i=0;i<3;i++)
		{
			if (t1.data[i] < t2.data[i])
				return true;
			else if (t1.data[i] > t2.data[i])
				return false;
		}
		return false;
	}
};

typedef std::set<TriInt, TriIntCompare> TriIntSet;
typedef std::map<TriInt, std::vector<nvTriInfo*>, TriIntCompare> GroupIndicesMap;

void UpdateSet(int index, nvTriInfo* triInfo, std::map<int, TriIntSet>& vmap, 
			   std::map<int, GroupIndicesMap >& vgindicesMap, TriInt& newTriInt)
{
	std::map<int, TriIntSet>::iterator iter = vmap.find(index);
	
	newTriInt.data[0] = triInfo->m_geometryGroupIndex;
	newTriInt.data[1] = triInfo->m_smoothingGroupIndex;
	newTriInt.data[2] = triInfo->m_texCoordGroupIndex;
	if (iter == vmap.end())
	{
		TriIntSet newSet;
		newSet.insert(newTriInt);
		vmap[index] = newSet;
	}
	else
	{
		vmap[index].insert(newTriInt);
	}
	std::map<int, GroupIndicesMap >::iterator iter2;
	iter2 = vgindicesMap.find(index);
	if (iter2 == vgindicesMap.end())
	{
		GroupIndicesMap newMap;
		newMap[newTriInt].push_back(triInfo);
		vgindicesMap[index] = newMap;
	}
	else
	{
		GroupIndicesMap::iterator iter3 = vgindicesMap[index].find(newTriInt);
		if (iter3 == vgindicesMap[index].end())
			vgindicesMap[index][newTriInt] = std::vector<nvTriInfo*>();
		vgindicesMap[index][newTriInt].push_back(triInfo);
	}
}

// Update j'th index in the map to be 'newIndex'.
void UpdateTriInfoIndices(const int& posIndex, const int& newIndex, nvTriInfo* triInfo, std::map<nvTriInfo*, TriInt>& triMap)
{
	int j = -1;
	for (int i=0;i<3;++i)
	{
		if (triInfo->m_positionIndices[i] == posIndex)
		{ 
			j = i;
			break;
		}
	}
	assert(j >= 0 && j < 3);
	std::map<nvTriInfo*, TriInt>::iterator iter = triMap.find(triInfo);
	if (iter == triMap.end())
	{
		// Create a new TriInt with same indices as triInfo
		TriInt newTriInt;
		for (int i=0;i<3;++i)
			newTriInt.data[i] = triInfo->m_positionIndices[i];
		// update j'th index
		newTriInt.data[j] = newIndex;
		triMap[triInfo] = newTriInt;
	}
	else
	{
		triMap[triInfo].data[j] = newIndex;
	}
}

/*
// For each vertex (as given by its position) create a map from vector to its associated triInfo's.
void nvMeshPreprocessor::PreprocessTriangles()
{
	std::map<int, TriIntSet > vertexToGroupsMap; // for each vertex, the set of group triples that it's in.
	std::map<int, TriIntSet >::iterator iter;

	std::map<TriInt, std::vector<int>, TriIntCompare > groupsToTriInfoMap; // for each group triple, the tri's that have that
		// group triple.
	
	std::map<int, GroupIndicesMap > vertexToGroupIndicesMap;
	std::map<nvTriInfo*, TriInt> triInfoToNewIndicesMap; // map from a triInfo to its new position indices.
	
	bool normalsAvailable = false;
	//if (m_numNormals > 0)
	//	normalsAvailable = true;
	bool texcoordsAvailable = false;
	if (m_numTexCoords > 0)
		texcoordsAvailable = true;
	// copy all the old vertex positions into the new vertex positions:

	m_newVertexPositions.clear();
	m_newVertexNormals.clear();
	m_newTexCoords.clear();

	//for (int i=0;i<m_numVertices*3;++i)
	//	m_newVertexPositions.push_back(m_vertexPositions[i]);

	//m_newNumVertices = m_newVertexPositions.size();

	//std::map<TriIntSet,
	m_newNumVertices = 0;
	int i;
	for (i=0;i<m_triInfo.size();++i)
	{
		for (int j=0;j<3;++j)
		{
			// For each vertex figure out which groups it lies in:
			int posIndex = m_triInfo[i].m_positionIndices[j];
			TriInt newTriInt;
			UpdateSet(posIndex,&(m_triInfo[i]),vertexToGroupsMap,vertexToGroupIndicesMap,newTriInt);
			// Insert into groupToTriInt:
			groupsToTriInfoMap[newTriInt].push_back(i);
			
		}
	}
	// Go over each vertex and figure out how it should be split, and update the various triInfo's.
	int j;
	for (i=0;i<m_numVertices;++i)
	{
		iter = vertexToGroupsMap.find(i);
		int posIndex = (*iter).first;
		if (iter == vertexToGroupsMap.end())
		{
			// vertex was not in any group. we will let it pass, its normal will be set to (0,0,0)
			// as well as its tex coords:
			for (j=0;j<3;++j)
				m_newVertexPositions.push_back(m_vertexPositions[3*i+j]);
			for (j=0;j<3;++j)
				m_newVertexNormals.push_back(0.0f);
			for (j=0;j<m_texCoordSize;++j)
				m_newTexCoords.push_back(0.0f);
			m_normalDefined.push_back(false);
			m_texcoordDefined.push_back(false);
			continue;
		}
	
		TriIntSet tset = (*iter).second;
		TriIntSet::iterator iter2;
		// We will create a vertex for each unique triple that a vertex is in. This vertex will have
		// its associated position, normal (if available) and texcoords (if available) also saved into 
		// the normals and positions  array.
		
		for (iter2=tset.begin();iter2!=tset.end();++iter2)
		{
			TriInt group = *iter2;
			// Get vertex position from its position index:
			TriFloat* pos = (TriFloat*)(&m_vertexPositions[3*posIndex]);
			
				// Add to new positions:		
				for (j=0;j<3;++j)
					m_newVertexPositions.push_back(pos->data[j]);
			

			// Get the groupIndices map for this vertex:
			GroupIndicesMap giMap = vertexToGroupIndicesMap[posIndex];
			if (giMap.size() == 0) {
				assert(false); // should not happen
			}
			// Get the triInfo's for this vertex (for this group)
			GroupIndicesMap::iterator iter3 = giMap.find(group);
			if (iter3 == giMap.end())
			{
				assert(false);
			}
			else {
				std::vector<nvTriInfo*> tinfos = (*iter3).second;
				assert(tinfos.size() > 0);
				nvTriInfo* tinfo = tinfos[0];
				int thisIndex = -1;
				// Get the index (0,1,2) for this vertex in this triInfo:
				for (j=0;j<3;++j)
				{
					if (tinfo->m_positionIndices[j] == posIndex)
					{
						thisIndex = j;
						break;
					}
				}
				if (thisIndex == -1)
				{
					assert(false);					
				}
				else {
					// Get the index for the normal for this vertex for this group (should be same for all
					// triInfos in this group so we can get it from the first triInfo in this group):
					int normalIndex = tinfo->m_normalIndices[thisIndex];
					// Check whether the normalIndex is -1, if so then this particular vertex does
					// not have an associated normal. It will be set to all 0's.
					
					// Copy the normal into new normals:
					if (normalsAvailable)
					{
						if (normalIndex < 0)
						{
							m_allNormalsDefined = false;
							m_normalDefined.push_back(false);
						}
						else 
						{
							for (j=0;j<3;++j)
								m_newVertexNormals.push_back(m_vertexNormals[3*normalIndex+j]);
						}
					}
					int texcoordIndex = tinfo->m_textureIndices[thisIndex];
					// copy the texcoords into new texcoords
					if (texcoordsAvailable)
					{
						if (texcoordIndex < 0)
						{
							m_allTexcoordsDefined = false;
							m_texcoordDefined.push_back(false);
						}
						else
						{
							for (j=0;j<3;++j)
								m_newTexCoords.push_back(m_texCoords[3*texcoordIndex+j]);
						}
					}
					
				}
			
				// Need to update the triInfo's for this vertex, for this group:			
				for (j=0;j<tinfos.size();++j)
				{
					UpdateTriInfoIndices(posIndex,m_newNumVertices,tinfos[j],triInfoToNewIndicesMap);
				}
			}
			m_newNumVertices++;
			
		}
		
	}

	// Go over each triInfo and update its position indices.


	// Write out the new triangle indices:
	m_newTriangleIndices.clear();
	for (i=0;i<m_triInfo.size();++i)
	{
		std::map<nvTriInfo*, TriInt>::iterator iter = triInfoToNewIndicesMap.find(&(m_triInfo[i]));
		if (iter == triInfoToNewIndicesMap.end())
		{
			assert(false);
			continue;
		}
		else {
			TriInt triInt = (*iter).second;
			for (int j=0;j<3;++j)
			{
				m_newTriangleIndices.push_back(triInt.data[j]);
				assert(triInt.data[j] < m_newNumVertices);
			}
		}
		
	}

}
*/

bool Equal(const TriInt& t1, const TriInt& t2)
{
	for (int i=0;i<3;++i)
	{
		if (t1.data[i] != t2.data[i])
			return false;
	}
	return true;
}

void GetGroup(nvTriInfo* triInfo, TriInt& newTriInt)
{
	newTriInt.data[0] = triInfo->m_geometryGroupIndex;
	newTriInt.data[1] = triInfo->m_smoothingGroupIndex;
	newTriInt.data[2] = triInfo->m_texCoordGroupIndex;
}

void nvMeshPreprocessor::UpdateNormals(int j, int newIndex, nvTriInfo* tinfo)
{
	bool normalsAvailable = false;
	if (m_numNormals > 0)
		normalsAvailable = true;

	int normalIndex = tinfo->m_normalIndices[j];
	if (normalsAvailable)
	{
		if (normalIndex < 0)
		{
			m_allNormalsDefined = false;

		}
		else 
		{
			TriFloat tf;
			for (int j=0;j<3;++j)
				tf.data[j] = m_vertexNormals[3*normalIndex+j];
			m_newVertexIndexToNormalMap[newIndex] = tf;
		}
	}
	else 
		m_allNormalsDefined = false;
}

void nvMeshPreprocessor::UpdateTexCoords0(int j, int newIndex, nvTriInfo* triInfo)
{
	bool texcoordsAvailable = false;
	if (m_numTexCoords > 0)
		texcoordsAvailable = true;

	int texcoordIndex = triInfo->m_textureIndices[j];
	if (texcoordsAvailable)
	{
		if (texcoordIndex < 0)
		{
			m_allTexcoordsDefined = false;

		}
		else 
		{
			TriFloat tf;
			for (unsigned int j=0;j<m_texCoordSize;++j)
				tf.data[j] = m_texCoords[m_texCoordSize*texcoordIndex+j];
			m_newVertexIndexToTexCoord0Map[newIndex] = tf;
		}
	}
	else
		m_allTexcoordsDefined = false;
}

void nvMeshPreprocessor::CheckTexCoords()
{
	unsigned int i;

	// Copy all old vertex positions into the new vertex positions
	for (i=0;i<m_numVertices*3;++i)
		m_newVertexPositions.push_back(m_vertexPositions[i]);
	
	m_newNumVertices = m_newVertexPositions.size()/3;

	int newIndex;

	std::map<int, std::set<int> > vertexToTexCoordSetMap;
	std::map<int, std::set<int> >::iterator iter;
	for (i=0;i<m_triInfo.size();++i)
	{
		nvTriInfo tinfo = m_triInfo[i];
		for (int j=0;j<3;++j)
		{
			int posindex = tinfo.m_positionIndices[j];
			int texindex = tinfo.m_textureIndices[j];
			iter = vertexToTexCoordSetMap.find(posindex);
			if (iter == vertexToTexCoordSetMap.end())
			{
				std::set<int> newSet;
				newSet.insert(texindex);
				vertexToTexCoordSetMap[posindex] = newSet;
			}
			else
			{
				// Check if the texindex is the same:
				std::set<int>::iterator iter2 = (*iter).second.find(texindex);
				if (iter2 == (*iter).second.end())
				{
					// new texindex, so need to create a new vertex:
					newIndex = m_newNumVertices;
					m_newNumVertices++;
					// Copy over the old vertex position into new vertex position
					for (int k=0;k<3;++k)
						m_newVertexPositions.push_back(m_vertexPositions[3*posindex+k]);
					// Update the index of the vertex in this triInfo
					m_triInfo[i].m_positionIndices[j] = newIndex;
				}
			}
		}
	}
	for (iter=vertexToTexCoordSetMap.begin();iter!=vertexToTexCoordSetMap.end();++iter)
	{
		if ((*iter).second.size() > 1)
			printf("duplicate\n");
	}
}

// For each vertex (as given by its position) create a map from vector to its associated triInfo's.
void nvMeshPreprocessor::PreprocessTriangles()
{

	CheckTexCoords();
	std::map<TriInt, std::vector<int>, TriIntCompare > groupsToTriInfoMap; // for each group triple, the tri's that have that
		// group triple.
	
	m_newVertexIndexToNormalMap.clear();
	m_newVertexIndexToTexCoord0Map.clear();

	bool normalsAvailable = false;
	if (m_numNormals > 0)
		normalsAvailable = true;
	bool texcoordsAvailable = false;
	if (m_numTexCoords > 0)
		texcoordsAvailable = true;
	// copy all the old vertex positions into the new vertex positions:

	//m_newVertexPositions.clear();
	m_newVertexNormals.clear();
	m_newTexCoords.clear();

	unsigned int i;
	//for (i=0;i<m_numVertices*3;++i)
	//	m_newVertexPositions.push_back(m_vertexPositions[i]);

	m_newNumVertices = m_newVertexPositions.size()/3;

	
	//m_newNumVertices = 0;
	std::map<int, TriInt> vertexIndexMap;
	for (i=0;i<m_triInfo.size();++i)
	{
		TriInt newTriInt;
		GetGroup(&(m_triInfo[i]),newTriInt);
		groupsToTriInfoMap[newTriInt].push_back(i);
	}
	
	// Go over each group and figure out the vertices that need to be split:
	int j;
	std::map<TriInt, std::vector<int>, TriIntCompare >::iterator iter;
	std::map<int, TriInt>::iterator iter3;
	for (iter=groupsToTriInfoMap.begin();iter!=groupsToTriInfoMap.end();++iter)
	{
		TriInt group = (*iter).first;
		std::vector<int> groupTriInfos = (*iter).second;
		std::map<int, int> vertexIndexToNewIndexMap;
		// Go over each triangle corresponding to this triple.
		for (i=0;i<groupTriInfos.size();++i)
		{
			nvTriInfo* tinfo = &(m_triInfo[groupTriInfos[i]]);
			for (j=0;j<3;++j)
			{
				int posIndex = tinfo->m_positionIndices[j];
				iter3 = vertexIndexMap.find(posIndex);
				if (iter3 == vertexIndexMap.end())
				{
					vertexIndexMap[posIndex] = group; // first occurrence ofthis vertex.
					// save its associated data.
					UpdateNormals(j,posIndex,tinfo);
					UpdateTexCoords0(j,posIndex,tinfo);
				}
				else
				{
					// vertex index already used by some group - check if it's this group.
					if (!Equal(vertexIndexMap[posIndex],group)) 
					{
						// some other group used the index, check if we've mapped to a new index
						// already:
						std::map<int, int>::iterator iter4 = vertexIndexToNewIndexMap.find(posIndex);
						int newIndex;
						if (iter4 == vertexIndexToNewIndexMap.end())
						{
							// not mapped, assign new index.
							newIndex = m_newNumVertices;
							vertexIndexToNewIndexMap[posIndex] = newIndex;
							m_newNumVertices++;
							// Copy over the old vertex position into new vertex position
							for (int k=0;k<3;++k)
								m_newVertexPositions.push_back(m_vertexPositions[3*posIndex+k]);
						
							UpdateNormals(j,newIndex,tinfo);
							UpdateTexCoords0(j,newIndex,tinfo);
							m_newVertexIndexToOldMap[newIndex] = posIndex;
						}
						else
							newIndex = vertexIndexToNewIndexMap[posIndex];
						// Replace in triangleIndices:
						tinfo->m_positionIndices[j] = newIndex;
		
					}
					// else the index is used by this group, so do nothing since we have the right index.
					
				}
				
			}
		}
	}
	
	if (normalsAvailable)
	{
		// put all the normals in the newnormals array:
		for (unsigned int i=0;i<m_newNumVertices;++i)
		{
			// Find the appropriate trifloat: 
			std::map<int, TriFloat>::iterator iter2;
			iter2 = m_newVertexIndexToNormalMap.find(i);
			if (iter2 == m_newVertexIndexToNormalMap.end())
			{
				// this means that this is a vertex which is declared but not used in any of the faces.
				// Give it a normal of (0,0,0). 
				for (int j=0;j<3;++j)
					m_newVertexNormals.push_back(0.0f);
			}
			else {
				TriFloat tf = (*iter2).second;
				for (int j=0;j<3;++j)
					m_newVertexNormals.push_back(tf.data[j]);
			}
			

		}
	}

	if (texcoordsAvailable)
	{
		// put all the texcoords in the newtexcoords array:
		for (unsigned int i=0;i<m_newNumVertices;++i)
		{
			// Find the appropriate trifloat: 
			std::map<int, TriFloat>::iterator iter2;
			iter2 = m_newVertexIndexToTexCoord0Map.find(i);
			if (iter2 == m_newVertexIndexToTexCoord0Map.end())
			{
				// this means that this is a vertex which is declared but not used in any of the faces.
				// Give it a texcoord of (0,0,0). 
				for (unsigned int j=0;j<3;++j)
					m_newTexCoords.push_back(0.0f);
			}
			else {
				TriFloat tf = (*iter2).second;
				for (int j=0;j<3;++j)
					m_newTexCoords.push_back(tf.data[j]);
			}
		}
	}
	
	// Write out the new triangle indices:
	m_newTriangleIndices.clear();
	for (iter=groupsToTriInfoMap.begin();iter!=groupsToTriInfoMap.end();++iter)
	{
		//TriInt group = (*iter2).first;
		std::vector<int> groupTriInfos = (*iter).second;
		for (unsigned int i=0;i<groupTriInfos.size();++i)
		{
			nvTriInfo* tinfo = &(m_triInfo[groupTriInfos[i]]);
			for (j=0;j<3;++j)
				m_newTriangleIndices.push_back(tinfo->m_positionIndices[j]);
		}
	}

}

