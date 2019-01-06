
#include "nvObjIO.h"
#include <assert.h>

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif
#include <string>

#ifdef _WIN32
#  pragma warning (disable : 4786)
#endif
#include <sstream>

using namespace std;
#define BUF_SIZE 128

#ifdef _WIN32
#  pragma warning (disable : 4786)
#  pragma warning (disable : 4996)
#endif

// Local object for convenient indexing:
typedef struct { int indices[3]; } TriIndex;

nvObjIO::nvObjIO() //: m_vertexPositions(0), m_triangleIndices(0), m_vertexNormals(0), m_texCoords(0)
{
	m_fileExtensions.clear();
	m_fileExtensions.push_back("obj");
	m_fileExtensions.push_back("OBJ");
	
	m_nameToSmoothingGroupMap.clear();
	Reset();
}

nvObjIO::~nvObjIO()
{
	tnameToSmoothingGroupMap::iterator iter;
	for (iter=m_nameToSmoothingGroupMap.begin();iter != m_nameToSmoothingGroupMap.end();++iter)
	{
		delete (*iter).second;
	}
	tnameToTexCoordGroupMap::iterator iterTexCoord;
	for (iterTexCoord=m_nameToTexCoordGroupMap.begin();iterTexCoord != m_nameToTexCoordGroupMap.end();++iterTexCoord)
	{
		delete (*iterTexCoord).second;
	}
	m_attributes.clear();
	m_nameToSmoothingGroupMap.clear();
	m_nameToTexCoordGroupMap.clear();
}

void nvObjIO::Reset()
{
	m_dirty = true;
	m_fileRead = false;
	m_numVertices = 0;
	m_numNormals = 0;
	m_numTriangles = 0;
	m_numTexCoords = 0;
	m_texCoordSize = 2; // Default is 2, unless FirstPass() determines there are more.
	m_attributes.clear();

	// Clear out the previous smoothing groups if any.
	std::map<std::string, nvObjGroup*>::iterator iter;
	for (iter=m_nameToSmoothingGroupMap.begin();iter != m_nameToSmoothingGroupMap.end();++iter)
	{
		delete (*iter).second;
	}
	tnameToTexCoordGroupMap::iterator iterTexCoord;
	for (iterTexCoord=m_nameToTexCoordGroupMap.begin();iterTexCoord != m_nameToTexCoordGroupMap.end();++iterTexCoord)
	{
		delete (*iterTexCoord).second;
	}

	m_nameToSmoothingGroupMap.clear();
	m_nameToTexCoordGroupMap.clear();

	// Default smoothing group:
	m_curSmoothingGroup = FindOrCreateSmoothingGroup("default");
	m_curTexCoordGroup = FindOrCreateTexCoordGroup("default");

	m_triInfos.clear();
}

const std::vector<const char*>& nvObjIO::GetFileExtensions() const 
{
	return m_fileExtensions;
}

int nvObjIO::ReadFile(const char* fileName)
{
	// Clear out info from previous file.
	Reset();
	// Open file to see if it actually exists:
	m_fp = fopen(fileName,"r");
	if (m_fp)
	{
		if (!FirstPass())
			return 0;
		rewind(m_fp);
		if (!SecondPass())
			return 0;
		m_fileRead = true;
		return 1;
	}
	else
		return 0;
}

size_t nvObjIO::GetNumTriangles()
{
	return m_numTriangles;
}

void nvObjIO::GetTriangleIndices(std::vector<int>& triangleIndices)
{
	triangleIndices = m_triangleIndices;
}

int nvObjIO::GetAttributes(std::vector<const char*>& attributes)
{
	if (!m_fp)
		return 0;
	if (!m_fileRead)
		return 0;
	
	for (unsigned int i=0;i<m_attributes.size();++i)
	{
		attributes.push_back(m_attributes[i]);
	}
	return 1;
}

int nvObjIO::GetAttributesAndValues(std::vector<const char*>& attributesRead, 
		std::vector<std::vector<float> >& attributeValueArrays, std::vector<size_t>& attributeSizes,
		std::vector<size_t>& attributeCounts)
{
	if (m_numVertices <= 0 || m_numTriangles <= 0)
		return 0;
	attributesRead.push_back(nvMeshAttribute::VERTEX_POSITION);
	attributeValueArrays.push_back(m_vertexPositions2);
	attributeSizes.push_back(sizeof(float)*3);
	attributeCounts.push_back(m_numVertices);

	if (m_numNormals > 0)
	{
		attributesRead.push_back(nvMeshAttribute::VERTEX_NORMAL);
		attributeValueArrays.push_back(m_vertexNormals2);
		attributeSizes.push_back(sizeof(float)*3);
		attributeCounts.push_back(m_numNormals);
	}
	if (m_numTexCoords > 0)
	{
		attributesRead.push_back(nvMeshAttribute::TEX_COORD0);
		attributeValueArrays.push_back(m_texCoords2);
		attributeSizes.push_back(sizeof(float)*m_texCoordSize);
		attributeCounts.push_back(m_numTexCoords);
	}
	
	return 1;
}

int nvObjIO::GetAttributeValues(const char* attribute, std::vector<float>& attributeValues, 
								size_t& attributeSize, size_t& attributeCount)
{
	if (!strcmp(attribute,nvMeshAttribute::VERTEX_POSITION))
	{
		attributeValues = m_vertexPositions2;
		return 1;
	}
	else if (!strcmp(attribute,nvMeshAttribute::VERTEX_NORMAL))
	{
		attributeValues = m_vertexNormals2;
		return 1;
	}
	else if (!strcmp(attribute,nvMeshAttribute::TEX_COORD0))
	{
		attributeValues = m_texCoords2;
		return 1;
	}
	return 0;
}

void nvObjIO::GetGroupTypes(std::vector<const char*>& groupTypes)
{
	if (m_nameToSmoothingGroupMap.size() > 0)
		groupTypes.push_back(nvMeshAttribute::SMOOTHING_GROUP);
	if (m_nameToTexCoordGroupMap.size() > 0)
		groupTypes.push_back(nvMeshAttribute::TEXCOORD_GROUP);
}

int nvObjIO::GetGroups(const char* groupType, std::vector<const char*>& groupNames)
{
	if (!strcmp(groupType,nvMeshAttribute::SMOOTHING_GROUP))
	{
		return GetSmoothingGroups(groupNames);
	}
	else if (!strcmp(groupType,nvMeshAttribute::TEXCOORD_GROUP))
	{
		// texcoord groups
		return GetTexCoordGroups(groupNames);
	}
	return 0;
}

int nvObjIO::GetGroupInfo(const char* groupType, const char* groupName, 
		std::vector<int>& triangleIndices, bool& attributePresent,
		std::vector<int>& triangleAttributeIndices, std::vector<int>& indexedTriangles)
{
	if (!strcmp(groupType,nvMeshAttribute::SMOOTHING_GROUP))
		return GetSmoothingGroupInfo(groupName,triangleIndices,attributePresent,triangleAttributeIndices,indexedTriangles);
	else if (!strcmp(groupType,nvMeshAttribute::TEXCOORD_GROUP))
		return GetTexCoordGroupInfo(groupName,triangleIndices,attributePresent,triangleAttributeIndices,indexedTriangles);
	else
		assert(false);
	return 0;
}

int nvObjIO::CheckAttributes(const std::vector<const char*>& attributes)
{
	// n^2 for now, but n should be small. (Else change to a map.)
	for (unsigned int i=0;i<attributes.size();++i)
	{
		bool goodAttribute = false;
		for (unsigned int j=0;j<m_attributes.size();++j)
		{
			if (!strcmp(attributes[i],m_attributes[j]))
			{
				goodAttribute = true;
				break;
			}
		}
		if (!goodAttribute)
			return 0;
	}
	return 1;
}

int nvObjIO::FirstPass()
{
	FILE* file = m_fp;
	
	char buf[BUF_SIZE];
	unsigned int n,v,t;
	if (!m_curSmoothingGroup)
	{
		assert(false);
		return 0;
	}
	while (fscanf(file,"%s",buf) != EOF) {
		switch (buf[0]) {
		case '#':
			fgets(buf,sizeof(buf),file);
			break;
		case 'v':
			switch (buf[1]) {
			case '\0':			/* vertex */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				m_numVertices++;
				break;
			case 'n':				/* normal */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				m_numNormals++;
				break;
			case 't':				/* texcoord */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				DetermineTexCoordSize(buf);
				m_numTexCoords++;
				break;
			default:
				return 0;
			}
			break;
		case 'm':
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			//model->mtllibname = strdup(buf);
			//ReadMaterial(buf,mesh);
			break;
		case 'u':
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'g':
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s", buf);
			break;
		case 's': // Smoothing group.
			fgets(buf,sizeof(buf),file);
			sscanf(buf,"%s",buf);
			ReadSmoothingGroup(buf);
			break;

		case 'f':				// face
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
					/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				m_curSmoothingGroup->m_numTriangles++;
				m_numTriangles++;
				while(fscanf(file, "%d//%d", &v, &n) > 0) {
					m_curSmoothingGroup->m_numTriangles++;
					m_numTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				m_curSmoothingGroup->m_numTriangles++;
				m_numTriangles++;
				while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					m_curSmoothingGroup->m_numTriangles++;
					m_numTriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				m_curSmoothingGroup->m_numTriangles++;
				m_numTriangles++;
				while(fscanf(file, "%d/%d", &v, &t) > 0) {
					m_curSmoothingGroup->m_numTriangles++;
					m_numTriangles++;
				}
			}
			else {
				/* v */
				sscanf(buf,"%d",&v);
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				m_curSmoothingGroup->m_numTriangles++;
				m_numTriangles++;
				while(fscanf(file, "%d", &v) > 0) {
					m_curSmoothingGroup->m_numTriangles++;
					m_numTriangles++;
				}
			}
			break;
		}
	}
	if (m_numVertices > 0)
		m_attributes.push_back(nvMeshAttribute::VERTEX_POSITION);
	if (m_numTriangles > 0)
		m_attributes.push_back(nvMeshAttribute::TRIANGLE_INDICES);
	if (m_numNormals > 0)
		m_attributes.push_back(nvMeshAttribute::VERTEX_NORMAL);
	if (m_numTexCoords > 0)
		m_attributes.push_back(nvMeshAttribute::TEX_COORD0);
	return 1;
}

/*
int nvObjIO::SecondPass() 
{

	if (!m_fp)
		return 0;
	FILE* file = m_fp;
	
	char buf[BUF_SIZE];
	unsigned int n,v,t;

	AllocateAttributes();

	float* vertices = m_vertexPositions;
	int* faceIndices = m_triangleIndices;

	float* normals = 0;
	if ((m_numNormals > 0) && !(m_vertexNormals == 0))
		normals = m_vertexNormals;

	float* texCoords = m_texCoords;

	int* fTexIndices = 0;
	int* fNormalIndices = 0;
	if (m_numTriangles > 0) {
		fTexIndices = (int*)malloc(m_numTriangles*sizeof(int)*3);
		fNormalIndices = (int*)malloc(m_numTriangles*sizeof(int)*3);
	}

	int numvertices, numnormals;
	numvertices = numnormals = 0;
	int numtexcoords = 0;

	int numTriangles = 0;
	//int groupId = FindGroup("default");
	//int materialId = 0;
	float dummyNormal[3];
	float dummyTexCoord[2];

	m_curSmoothingGroup = FindOrCreateSmoothingGroup("default");

	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
		case '#':				// comment 
		  // eat up rest of line
		  fgets(buf, sizeof(buf), file);
		  break;
		case 'v':				// v, vn, vt 
			switch(buf[1]) {
			case '\0':			// vertex 
				fscanf(file, "%f %f %f", 
					   &vertices[3*numvertices], 
					   &vertices[3*numvertices+1], 
					   &vertices[3*numvertices+2]);
				numvertices++;
				break;
			case 'n':				// normal
				if (normals != 0) {
					fscanf(file, "%f %f %f", 
					   &normals[3 * numnormals],
					   &normals[3 * numnormals + 1], 
					   &normals[3 * numnormals + 2]);
				}
				else 
					fscanf(file, "%f %f %f", &dummyNormal[0],&dummyNormal[1],&dummyNormal[2]);
				numnormals++;
				break;
			case 't':				// texcoord
				if (texCoords != 0)
					fscanf(file, "%f %f", 
					   &texCoords[2 * numtexcoords],
					   &texCoords[2 * numtexcoords + 1]);
				else
					fscanf(file, "%f %f",&dummyTexCoord[0],&dummyTexCoord[1]);
				numtexcoords++;
				break;
			} // end switch (buf[1])
			break; // end case 'v'.
		case 'u':	// usemtl tag
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			//materialId = FindMaterial(buf);
			//if (groupId != -1)
			//	m_groupToMaterialMap[groupId] = materialId;
			break;
		case 'g':				// group 
			// eat up rest of line 
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s", buf);
			//groupId = FindGroup(buf);
			//m_groupToMaterialMap[groupId] = materialId;
			break;
		case 'f':
			v = n = t = 0;
			if (!ParseFace(faceIndices,fTexIndices,fNormalIndices,&numTriangles))
				return false;
			break;
		case 's': // Smoothing group.
			fgets(buf,sizeof(buf),file);
			sscanf(buf,"%s",buf);
			ReadSmoothingGroup(buf);
			break;
		default:
			// eat up rest of line
			fgets(buf,sizeof(buf),file);
			break;

		} // switch (buf[0])
	}
	if (fTexIndices)
		delete[] fTexIndices;
	if (fNormalIndices)
		delete[] fNormalIndices;
	fclose(m_fp);
	return true;
}
*/

int nvObjIO::SecondPass() 
{

	if (!m_fp)
		return 0;
	FILE* file = m_fp;
	
	char buf[BUF_SIZE];
	unsigned int n,v,t;


	//float* vertices = m_vertexPositions;
	//int* faceIndices = m_triangleIndices;

	//float* normals = 0;
	//if ((m_numNormals > 0) && !(m_vertexNormals == 0))
	//	normals = m_vertexNormals;

	//float* texCoords = m_texCoords;

	int numvertices, numnormals;
	numvertices = numnormals = 0;
	int numtexcoords = 0;
	m_curNumTexCoords = 0;

	//int numTriangles = 0;
	//int groupId = FindGroup("default");
	//int materialId = 0;
	float dummyNormal[3];
	float dummyTexCoord[3];

	bool normals = false;
	if (m_numNormals > 0)
		normals = true;
	bool texCoords = false;
	if (m_numTexCoords > 0)
		texCoords = true;

	m_curSmoothingGroup = FindOrCreateSmoothingGroup("default");
	float vpos[3];
	float vnor[3];
	float vtex[3];
	unsigned int j;
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
		case '#':				/* comment */
		  // eat up rest of line
		  fgets(buf, sizeof(buf), file);
		  break;
		case 'v':				// v, vn, vt 
			switch(buf[1]) {
			case '\0':			// vertex 
				fscanf(file, "%f %f %f", 
					   &vpos[0], &vpos[1], &vpos[2]); 
				for (j=0;j<3;++j)
					m_vertexPositions2.push_back(vpos[j]);
				numvertices++;
				break;
			case 'n':				// normal
				if (normals) {
					fscanf(file, "%f %f %f", &vnor[0], &vnor[1], &vnor[2]);
				}
				else 
					fscanf(file, "%f %f %f", &dummyNormal[0],&dummyNormal[1],&dummyNormal[2]);
				numnormals++;
				for (j=0;j<3;++j)
					m_vertexNormals2.push_back(vnor[j]);
				break;
			case 't':				// texcoord
				if (texCoords) {
					for (j=0;j<m_texCoordSize;++j)
						fscanf(file,"%f",&vtex[j]);
				}
				else {
					for (j=0;j<m_texCoordSize;++j)
						fscanf(file,"%f",&dummyTexCoord[j]);
				}
				numtexcoords++;
				m_curNumTexCoords++;
				for (j=0;j<m_texCoordSize;++j)
					m_texCoords2.push_back(vtex[j]);
				break;
			} // end switch (buf[1])
			break; // end case 'v'.
		case 'u':	// usemtl tag
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			//materialId = FindMaterial(buf);
			//if (groupId != -1)
			//	m_groupToMaterialMap[groupId] = materialId;
			break;
		case 'g':				/* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s", buf);
			//groupId = FindGroup(buf);
			//m_groupToMaterialMap[groupId] = materialId;
			break;
		case 'f':
			v = n = t = 0;
			if (!ParseFace())
				return false;
			break;
		case 's': // Smoothing group.
			fgets(buf,sizeof(buf),file);
			sscanf(buf,"%s",buf);
			ReadSmoothingGroup(buf);
			break;
		default:
			// eat up rest of line
			fgets(buf,sizeof(buf),file);
			break;

		} // switch (buf[0])
	}
	
	fclose(m_fp);
	return true;
}

bool nvObjIO::ParseFace()
{
	if (!m_fp)
		return false;

	FILE* file = m_fp;
	
	char buf[BUF_SIZE];
	int v,n,t;
	v = n = t = 0;
	int numTriangles = m_triangleIndices.size()/3;
	fscanf(file,"%s",buf);
	nvTriInfo triInfo;
	std::vector<int> nindices;
	std::vector<int> tindices;
#ifdef _DEBUG
	int temp;
	if (numTriangles > 300)
		temp = 1;
#endif
	if (strstr(buf, "//")) {
		/* v//n */
		sscanf(buf, "%d//%d", &v, &n); // Indices start with 1 in obj, so:
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);
		UpdateGroups(v-1,-1,n-1);
		UpdateTriInfo(0,v-1,-1,n-1,triInfo);
		
		fscanf(file, "%d//%d", &v, &n);
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);		
		UpdateGroups(v-1,-1,n-1);
		UpdateTriInfo(1,v-1,-1,n-1,triInfo);
		
		
		fscanf(file, "%d//%d", &v, &n);
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);
		UpdateGroups(v-1,-1,n-1);
		UpdateTriInfo(2,v-1,-1,n-1,triInfo);

		UpdateGroups(numTriangles);

		triInfo.index = numTriangles;
		m_triInfos.push_back(triInfo);

		int prev = numTriangles;
		int prev2 = 0;
		numTriangles++;
		while (fscanf(file, "%d//%d", &v, &n) > 0) {
			m_triangleIndices.push_back(m_triangleIndices[3*prev]);
			nindices.push_back(nindices[3*prev2]);
			UpdateGroups(m_triangleIndices[3*prev],-1,nindices[3*prev2]);
			UpdateTriInfo(0,m_triangleIndices[3*prev],-1,nindices[3*prev2],triInfo);

			m_triangleIndices.push_back(m_triangleIndices[3*prev+2]);
			nindices.push_back(nindices[3*prev2+2]);
			UpdateGroups(m_triangleIndices[3*prev+2],-1,nindices[3*prev2+2]);
			UpdateTriInfo(1,m_triangleIndices[3*prev+2],-1,nindices[3*prev2+2],triInfo);
			
			m_triangleIndices.push_back(v-1);
			nindices.push_back(n-1);
			UpdateGroups(v-1,-1,n-1);
			UpdateTriInfo(2,v-1,-1,n-1,triInfo);

			UpdateGroups(numTriangles);
			prev = numTriangles;
			prev2++;
			triInfo.index = numTriangles;
			m_triInfos.push_back(triInfo);
			numTriangles++;
			
			
		}
	}
	else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
		/* v/t/n */
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,n-1);
		UpdateTriInfo(0,v-1,t-1,n-1,triInfo);

		fscanf(file, "%d/%d/%d", &v, &t, &n);
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,n-1);
		UpdateTriInfo(1,v-1,t-1,n-1,triInfo);

		fscanf(file, "%d/%d/%d", &v, &t, &n);
		m_triangleIndices.push_back(v-1);
		nindices.push_back(n-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,n-1);
		UpdateTriInfo(2,v-1,t-1,n-1,triInfo);

		UpdateGroups(numTriangles);
		triInfo.index = numTriangles;
		m_triInfos.push_back(triInfo);
		
		int prev = numTriangles;
		int prev2 = 0;
		numTriangles++;
		while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
			m_triangleIndices.push_back(m_triangleIndices[3*prev]);
			tindices.push_back(tindices[3*prev2]);
			nindices.push_back(nindices[3*prev2]);
			UpdateGroups(m_triangleIndices[3*prev],tindices[3*prev2],nindices[3*prev2]);
			UpdateTriInfo(0,m_triangleIndices[3*prev],tindices[3*prev2],nindices[3*prev2],triInfo);
			
			m_triangleIndices.push_back(m_triangleIndices[3*prev+2]);
			tindices.push_back(tindices[3*prev2+2]);
			nindices.push_back(nindices[3*prev2+2]);
			UpdateGroups(m_triangleIndices[3*prev+2],tindices[3*prev2+2],nindices[3*prev2+2]);
			UpdateTriInfo(1,m_triangleIndices[3*prev+2],tindices[3*prev2+2],nindices[3*prev2+2],triInfo);
			
			m_triangleIndices.push_back(v-1);
			if (t < 0)
				t = m_curNumTexCoords + t + 1;
			tindices.push_back(t-1);
			nindices.push_back(n-1);
			UpdateGroups(v-1,t-1,n-1);
			UpdateTriInfo(2,v-1,t-1,n-1,triInfo);
			
			UpdateGroups(numTriangles);
			triInfo.index = numTriangles;
			m_triInfos.push_back(triInfo);
			
			prev = numTriangles;
			prev2++;
			numTriangles++;
		}
	}
	else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
		/* v/t */
		m_triangleIndices.push_back(v-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,-1);
		UpdateTriInfo(0,v-1,t-1,-1,triInfo);
		
		fscanf(file, "%d/%d", &v, &t);
		m_triangleIndices.push_back(v-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,-1);
		UpdateTriInfo(1,v-1,t-1,-1,triInfo);

		fscanf(file, "%d/%d", &v, &t);
		m_triangleIndices.push_back(v-1);
		if (t < 0)
			t = m_curNumTexCoords + t + 1;
		tindices.push_back(t-1);
		UpdateGroups(v-1,t-1,-1);
		UpdateTriInfo(2,v-1,t-1,-1,triInfo);

		UpdateGroups(numTriangles);
		triInfo.index = numTriangles;
		m_triInfos.push_back(triInfo);
		
		int prev = numTriangles;
		int prev2 = 0;
		numTriangles++;
		while (fscanf(file, "%d/%d", &v, &t) > 0) {
			m_triangleIndices.push_back(m_triangleIndices[3*prev]);
			tindices.push_back(tindices[3*prev2]);
			UpdateGroups(m_triangleIndices[3*prev],tindices[3*prev2],-1);
			UpdateTriInfo(0,m_triangleIndices[3*prev],tindices[3*prev2],-1,triInfo);
			
			m_triangleIndices.push_back(m_triangleIndices[3*prev+2]);
			tindices.push_back(tindices[3*prev2+2]);
			UpdateGroups(m_triangleIndices[3*prev+2],tindices[3*prev2+2],-1);
			UpdateTriInfo(1,m_triangleIndices[3*prev+2],tindices[3*prev2+2],-1,triInfo);
			
			m_triangleIndices.push_back(v-1);
			if (t < 0)
				t = m_curNumTexCoords + t + 1;
			tindices.push_back(t-1);
			UpdateGroups(v-1,t-1,-1);
			UpdateTriInfo(2,v-1,t-1,-1,triInfo);

			UpdateGroups(numTriangles);
			triInfo.index = numTriangles;
			m_triInfos.push_back(triInfo);
			
			prev = numTriangles;
			prev2++;
			numTriangles++;
		}
	}
	else {
		/* v */
		sscanf(buf, "%d", &v);
		m_triangleIndices.push_back(v-1);
		UpdateGroups(v-1,-1,-1);
		UpdateTriInfo(0,v-1,-1,-1,triInfo);

		fscanf(file,"%d", &v);
		m_triangleIndices.push_back(v-1);
		UpdateGroups(v-1,-1,-1);
		UpdateTriInfo(1,v-1,-1,-1,triInfo);

		fscanf(file,"%d", &v);
		m_triangleIndices.push_back(v-1);
		UpdateGroups(v-1,-1,-1);
		UpdateTriInfo(2,v-1,-1,-1,triInfo);

		UpdateGroups(numTriangles);
		triInfo.index = numTriangles;
		m_triInfos.push_back(triInfo);
		
		int prev = numTriangles;
		numTriangles++;
		while (fscanf(file, "%d", &v) > 0) {
			m_triangleIndices.push_back(m_triangleIndices[3*prev]);
			UpdateGroups(m_triangleIndices[3*prev],-1,-1);
			UpdateTriInfo(0,m_triangleIndices[3*prev],-1,-1,triInfo);

			m_triangleIndices.push_back(m_triangleIndices[3*prev+2]);
			UpdateGroups(m_triangleIndices[3*prev+2],-1,-1);
			UpdateTriInfo(1,m_triangleIndices[3*prev+2],-1,-1,triInfo);

			m_triangleIndices.push_back(v-1);
			UpdateGroups(v-1,-1,-1);
			UpdateTriInfo(2,v-1,-1,-1,triInfo);

			UpdateGroups(numTriangles);
			triInfo.index = numTriangles;
			m_triInfos.push_back(triInfo);
			
			prev = numTriangles;
			numTriangles++;
		}
	}
	return true;
}

void nvObjIO::ReadSmoothingGroup(const char* buf)
{
	if (!strcmp(buf,"off"))
		m_curSmoothingGroup = FindOrCreateSmoothingGroup("default");
	else
	{
		m_curSmoothingGroup = FindOrCreateSmoothingGroup(buf);
	}
}

nvObjGroup* nvObjIO::FindOrCreateSmoothingGroup(const char* group)
{
	std::map<std::string, nvObjGroup*>::iterator iter = 
		m_nameToSmoothingGroupMap.find(std::string(group));
	if (iter == m_nameToSmoothingGroupMap.end())
	{
		// Not found, create:
		nvObjGroup* g = new nvObjGroup();
		g->m_name = std::string(group);
		g->m_id = m_nameToSmoothingGroupMap.size();
		m_nameToSmoothingGroupMap[std::string(group)] = g;
		return g;
	}
	else
		return (*iter).second;
}

nvObjGroup* nvObjIO::FindOrCreateTexCoordGroup(const char* group)
{
	std::map<std::string, nvObjGroup*>::iterator iter = 
		m_nameToTexCoordGroupMap.find(std::string(group));
	if (iter == m_nameToTexCoordGroupMap.end())
	{
		// Not found, create:
		nvObjGroup* g = new nvObjGroup();
		g->m_name = std::string(group);
		g->m_id = m_nameToTexCoordGroupMap.size();
		m_nameToTexCoordGroupMap[std::string(group)] = g;
		return g;
	}
	else
		return (*iter).second;
}



int nvObjIO::GetSmoothingGroups(std::vector<const char*>& groupNames)
{
	std::map<std::string, nvObjGroup*>::iterator iter;
	for (iter=m_nameToSmoothingGroupMap.begin();iter!=m_nameToSmoothingGroupMap.end();++iter)
	{
		groupNames.push_back(((*iter).first).c_str());
	}
	return 1;
}

int nvObjIO::GetSmoothingGroupInfo(const char* groupName, std::vector<int>& triangleIndices, 
								   bool& attributePresent, std::vector<int>& triangleAttributeIndices,
								   std::vector<int>& indexedTriangles)
{
	std::map<std::string, nvObjGroup*>::iterator iter;
	iter = m_nameToSmoothingGroupMap.find(std::string(groupName));
	if (iter == m_nameToSmoothingGroupMap.end())
		return 0;
	nvObjGroup* group = (*iter).second;
	if (group->m_dataIndices.size() == 0)
		attributePresent = false;
	else {
		attributePresent = true;
		triangleAttributeIndices = group->m_dataIndices;
	}
	triangleIndices = group->m_triangleIndices;
	indexedTriangles = group->m_indexedTriangles;
	return 1;
}

int nvObjIO::GetTexCoordGroups(std::vector<const char*>& groupNames)
{
	std::map<std::string, nvObjGroup*>::iterator iter;
	for (iter=m_nameToTexCoordGroupMap.begin();iter!=m_nameToTexCoordGroupMap.end();++iter)
	{
		groupNames.push_back(((*iter).first).c_str());
	}
	return 1;
}

int nvObjIO::GetTexCoordGroupInfo(const char* groupName, std::vector<int>& triangleIndices, 
								  bool& attributePresent, std::vector<int>& texCoordIndices,
								  std::vector<int>& indexedTriangles)
{
	std::map<std::string, nvObjGroup*>::iterator iter;
	iter = m_nameToTexCoordGroupMap.find(std::string(groupName));
	if (iter == m_nameToTexCoordGroupMap.end())
		return 0;
	nvObjGroup* group = (*iter).second;
	if (group->m_dataIndices.size() == 0)
		attributePresent = false;
	else
	{
		attributePresent = true;
		texCoordIndices = group->m_dataIndices;
	}
	triangleIndices = group->m_triangleIndices;
	indexedTriangles = group->m_indexedTriangles;
	return 1;
}

void nvObjIO::UpdateGroups(int vertexIndex, int texCoordIndex, int normalIndex)
{
	if (vertexIndex >= 0)
	{
		m_curSmoothingGroup->m_triangleIndices.push_back(vertexIndex);
		if (normalIndex >= 0)
			m_curSmoothingGroup->m_dataIndices.push_back(normalIndex);

		// For tex coord groups, check if the current group needs to be changed to a new
		// group. This happens if the new group of faces references texture vertices, where the
		// earlier one didn't or vice versa. Unfortunately such files are encountered in practice,
		// where a map has been applied only to a part of the geometry. Obj does not explicitly have
		// groups in this case, instead the face definitions change.
		if (texCoordIndex >= 0) {
			if (m_curTexCoordGroup->m_triangleIndices.size() > 0 && m_curTexCoordGroup->m_dataIndices.size() == 0) 
			{
				// need to create a new group. 
				std::string tempStr = "" + m_texCoordGroups.size();
				m_curTexCoordGroup = FindOrCreateTexCoordGroup(tempStr.c_str());
			}
			m_curTexCoordGroup->m_dataIndices.push_back(texCoordIndex);

		}
		else {
			if (m_curTexCoordGroup->m_triangleIndices.size() > 0 && m_curTexCoordGroup->m_dataIndices.size() > 0)
			{
				std::string tempStr = "" + m_texCoordGroups.size();
				m_curTexCoordGroup = FindOrCreateTexCoordGroup(tempStr.c_str());
			}
		}
		m_curTexCoordGroup->m_triangleIndices.push_back(vertexIndex);
	}

}

void nvObjIO::UpdateGroups(int numTriangles)
{
	if (numTriangles >= 0)
	{
		m_curSmoothingGroup->m_indexedTriangles.push_back(numTriangles);
		m_curTexCoordGroup->m_indexedTriangles.push_back(numTriangles);
	}
}

void nvObjIO::DetermineTexCoordSize(char* buf)
{
	unsigned int numCoords = 0;
	float fTemp;
	
	istringstream streambuffer_in(buf);
	while (streambuffer_in >> fTemp)
		numCoords++;

	if (numCoords > m_texCoordSize)
		m_texCoordSize = numCoords;
	if (m_texCoordSize > 3)
	{
		assert(false);
		m_texCoordSize = 3;
	}
}

void nvObjIO::UpdateTriInfo(int i, int vertexIndex, int texCoordIndex, int normalIndex, nvTriInfo& triInfo)
{
	triInfo.m_geometryGroupIndex = 0; // Change this to recognize geom groups
	triInfo.m_smoothingGroupIndex = m_curSmoothingGroup->m_id;
	triInfo.m_texCoordGroupIndex = m_curTexCoordGroup->m_id;
	triInfo.m_positionIndices[i] = vertexIndex;
	triInfo.m_normalIndices[i] = normalIndex;
	triInfo.m_textureIndices[i] = texCoordIndex;
	if (vertexIndex < 0)
		assert(false);
}

int nvObjIO::GetTriInfo(std::vector<nvTriInfo>& triInfo)
{
	triInfo.clear();
	triInfo = m_triInfos;
	return 1;
}

