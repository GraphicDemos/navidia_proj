/*********************************************************************NVMH2****
Copyright (C) 1999, 2000, 2001 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

******************************************************************************/

#include "nvObjReader.h"
#include "nvMeshIO.h"
#include "nvObjIO.h"
#include "nvMeshPreprocessor.h"
#include "NVMeshMender.h"
#include <math.h>


int nvObjReader::ReadFile(const char* fileName, unsigned int flags)

{
	nvObjIO* loader = new nvObjIO();
	Reset();
	
	loader->ReadFile(fileName);
	unsigned int numTriangles = loader->GetNumTriangles();
	if (numTriangles == 0)
		return false;
	std::vector<nvTriInfo> triInfos;
	loader->GetTriangleIndices(m_triangleIndices);
	loader->GetTriInfo(triInfos);
	

	std::vector<float> temp1;
	size_t temp2, temp3;
	loader->GetAttributeValues(nvMeshAttribute::VERTEX_POSITION,temp1,temp2,temp3);
	if (!loader->GetAttributesAndValues(m_attributes,m_attributeArrays,m_attributeSizes,m_attributeCounts))
		return false;

	nvMeshPreprocessor* mpp = new nvMeshPreprocessor();
	bool normals = false;
	bool texcoords = false;
	mpp->SetAttributeData(nvMeshAttribute::TRIANGLE_INDICES,&m_triangleIndices[0],sizeof(int)*3,numTriangles);
	for (unsigned int i=0;i<m_attributes.size();++i)
	{
		if (!strcmp(m_attributes[i],nvMeshAttribute::VERTEX_NORMAL))
			normals = true;
		else if (!strcmp(m_attributes[i],nvMeshAttribute::TEX_COORD0))
			texcoords = true;
		mpp->SetAttributeData(m_attributes[i],&((m_attributeArrays[i])[0]),m_attributeSizes[i]/sizeof(float),m_attributeCounts[i]);
	}
	
	mpp->SetTriInfo(triInfos);
	/*
	std::vector<const char*> groupTypes;
	loader->GetGroupTypes(groupTypes);
	for (i=0;i<groupTypes.size();++i)
	{
		std::vector<const char*> groupNames;
		if (loader->GetGroups(groupTypes[i],groupNames))
		{
			//size_t attSize = loader->GetAttributeSize(attributes[i]);
			for (int j=0;j<groupNames.size();++j)
			{
				std::vector<int> groupTriangleIndices;
				std::vector<int> groupAttributeIndices;
				std::vector<int> indexedTriangles;
				bool attributePresent;
				if (loader->GetGroupInfo(groupTypes[i],groupNames[j],groupTriangleIndices,attributePresent,
					groupAttributeIndices,indexedTriangles))
				{
					mpp->SetGroupInfo(groupTypes[i],groupNames[j],groupTriangleIndices,groupAttributeIndices,
						indexedTriangles);
				}
			}
		}
	}
	*/

	mpp->GetNewNumTriangles();
	mpp->GetNewTriangleIndices(m_newTriangleIndices);
	
	//int newNumVertices = mpp->GetNewNumElements(nvMeshAttribute::VERTEX_POSITION);
	mpp->GetNewAttributeArray(nvMeshAttribute::VERTEX_POSITION,m_newVertexPositions);
	m_nameToVectorMap[nvMeshAttribute::VERTEX_POSITION] = m_newVertexPositions;
	if (normals) {
		mpp->GetNewAttributeArray(nvMeshAttribute::VERTEX_NORMAL,m_newVertexNormals);
		m_nameToVectorMap[nvMeshAttribute::VERTEX_NORMAL] = m_newVertexNormals;
	}
	if (texcoords)
	{
		mpp->GetNewAttributeArray(nvMeshAttribute::TEX_COORD0,m_newTexCoords);
		m_nameToVectorMap[nvMeshAttribute::TEX_COORD0] = m_newTexCoords;
	}

	delete mpp;
	delete loader;
	m_fileReadOkay = true;
	return 1;
}

static void normalize( float v[3] )
{
    float len = (float) sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if ( len == 0.0 )
        {
        fprintf( stderr, "." );
        v[0] = 0.0;
        v[1] = 0.0;
        v[2] = 1.0;
        return;
        }
    len = 1.0f / len;
    v[0] = v[0]*len;
    v[1] = v[1]*len;
    v[2] = v[2]*len;
}

int nvObjReader::ReadAndMungeFile(const char* fileName, unsigned int flags,
								  unsigned int& numVertices,
								  std::vector<int>& indices, 
								  std::vector<float>& vertexPositions,
								  std::vector<float>& vertexNormals,
								  std::vector<float>& vertexTangents,
								  std::vector<float>& vertexBinormals,
								  std::vector<float>& vertexTexCoords)
{
	ReadFile(fileName,0);

	std::vector<int> triIndices;
	if (!GetTriangleIndices(triIndices))
		return 0;

	std::vector<float> vpos;
	if (!GetAttribute(nvMeshAttribute::VERTEX_POSITION,vpos))
		return 0;

	std::vector<float> vnor;
	bool normals = false;
	if (GetAttribute(nvMeshAttribute::VERTEX_NORMAL,vnor))
		normals = true;

	std::vector<float> vtex;
	bool textures = false;
	if (GetAttribute(nvMeshAttribute::TEX_COORD0,vtex))
		textures = true;

    numVertices   = vpos.size()/3;
    //int nindices = triIndices.size();

    std::vector<NVMeshMender::VertexAttribute> inputAtts;
	std::vector<NVMeshMender::VertexAttribute> outputAtts;

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
	texCoordAtt.floatVector_ = vtex;

	std::vector<float> vtan;
    NVMeshMender::VertexAttribute tgtSpaceAtt;
	tgtSpaceAtt.Name_ = "tangent";
    tgtSpaceAtt.floatVector_ = vtan;

    std::vector<float> vbin;
    NVMeshMender::VertexAttribute binSpaceAtt;
	binSpaceAtt.Name_ = "binormal";
    binSpaceAtt.floatVector_ = vbin;
	
	inputAtts.push_back(posAtt);
	inputAtts.push_back(triIndAtt);
	if (normals)
		inputAtts.push_back(norAtt);
	if (textures)
		inputAtts.push_back(texCoordAtt);
	
	outputAtts.push_back(posAtt);
	outputAtts.push_back(triIndAtt);
	outputAtts.push_back(norAtt);
	outputAtts.push_back(texCoordAtt);
	outputAtts.push_back(tgtSpaceAtt);
	outputAtts.push_back(binSpaceAtt);
	
	NVMeshMender mger;
	mger.Munge(inputAtts,outputAtts);

    
    numVertices = outputAtts[ 0 ].floatVector_.size() / 3;

	vertexPositions = outputAtts[0].floatVector_;
	indices = outputAtts[1].intVector_;
	vertexNormals = outputAtts[2].floatVector_;
	vertexTangents = outputAtts[4].floatVector_;
	vertexBinormals = outputAtts[5].floatVector_;


    // copy over texture coordinates skipping every 3rd float since (s,t,1)
    // is returned (for generated texture coordinates anyways)
    for ( unsigned int t = 0; t < 3*numVertices; t++ )
    {
        if ( t%3 == 2 ) continue;
        vertexTexCoords.push_back(outputAtts[3].floatVector_[t]);
    }
	
	// normalize vectors
    for (unsigned int ti = 0; ti < numVertices; ti++ )
    {
        normalize( &(vertexTangents[3*ti]) );
        normalize( &(vertexBinormals[3*ti]) );
        normalize( &(vertexNormals[3*ti]) );
    }
	return 1;
}

nvObjReader::~nvObjReader()
{
	Reset();
}

void nvObjReader::Reset()
{
	m_attributes.clear();
	m_attributeArrays.clear();
	m_attributeSizes.clear();
	m_attributeCounts.clear();
	m_triangleIndices.clear();
	m_nameToVectorMap.clear();
	m_fileReadOkay = false;
}

bool nvObjReader::GetTriangleIndices(std::vector<int>& triangleIndices)
{
	if (m_fileReadOkay)
	{
		triangleIndices = m_newTriangleIndices;
		return true;
	}
	else
		return false;
}

bool nvObjReader::GetAttribute(const char* attributeName, std::vector<float>& attributeValues)
{
    if (m_fileReadOkay)
    {
        std::map<const char*, std::vector<float> >::iterator iter = m_nameToVectorMap.begin();
        while(iter != m_nameToVectorMap.end())
        {
        	if (strcmp(attributeName, (*iter).first) == 0)
        	{
        	    attributeValues = (*iter).second;
        	    return true;
        	}
	    
        	iter++;
        }	
    }
    return false;
}

