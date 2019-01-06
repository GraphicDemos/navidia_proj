#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>
#include <math.h>
#include "PSScene.h"
#include "PITerminology.h"
#include "PIActions.h"
#if Macintosh
#include <wchar.h>
#endif

#define MAX_LINE	32768

bool ReadLine(FILE *f, int32 n, char *str)
{
	long	i;
	char	tempC;

	i = 0;
	tempC = fgetc(f);
	while (tempC != '\r' && tempC != '\n')
	{
		if (feof(f))
			break;
		str[i++] = tempC;
		if (i >= n - 1)
		{
			str[n - 1] = '\0';
			return false;
		}
		tempC = fgetc(f);
	}
	str[i] = '\0';
	PI3DTrimLeft(str);
	PI3DTrimRight(str);
	return true;
}

		
void LoadMaterialList(const wchar_t *path, PI3DImport *importer)
{
#if WIN32
	FILE	*f = _wfopen(path, L"r");
#else
	//¥¥¥Must fix this
	char macPath[2048];
	PI3DStringCopyWC(macPath,path);
	FILE	*f = fopen(macPath, "r");
#endif
	if (f == NULL)
		return;

	char			line[MAX_LINE];
	char			token[MAX_LINE];
	PI3DMaterial		*material = NULL;

	while (!feof(f))
	{
		ReadLine(f, MAX_LINE, line);
		if (!PI3DSplitString(token, line, true, '\0' ))
			continue;

		if (strcmp(token, "newmtl") == 0) // new material
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				material = PI3DUpdateMaterials(importer->scene, token, 0);
			}
		}
		else if (strcmp(token, "Kd") == 0 && material) // diffuse color
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(material->diffuse.red));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(material->diffuse.green));
					if (PI3DSplitString(token, line, true, '\0' ))
					{
						sscanf(token, "%lf", &(material->diffuse.blue));
					}
				}
			}
		}
		else if (strcmp(token, "Ka") == 0 && material) // ambient color
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(material->ambient.red));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(material->ambient.green));
					if (PI3DSplitString(token, line, true, '\0' ))
					{
						sscanf(token, "%lf", &(material->ambient.blue));
					}
				}
			}
		}
		else if (strcmp(token, "Ks") == 0 && material) // specular color
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(material->specular.red));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(material->specular.green));
					if (PI3DSplitString(token, line, true, '\0' ))
					{
						sscanf(token, "%lf", &(material->specular.blue));
					}
				}
			}
		}
		else if ((strcmp(token, "d") == 0 || strcmp(token, "Tr") == 0) && material) // transparency
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(material->transparency));
				material->transparency = 1.0 - material->transparency;
			}
		}
		else if (strcmp(token, "illum") == 0 && material) // illumination model (specular highlights or not)
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				int32	illum;
				sscanf(token, "%d", &illum);
				if (illum == 1)
				{
					material->shininess = 0.0;
					material->glossiness = 0.0;
				}
			}
		}
		else if (strcmp(token, "Ns") == 0 && material) // shininess
		{
			if (PI3DSplitString(token, line, true, '\0' ) && material->shininess > 0.001)
			{
				// It's really hard to know how to interpret this parameter.
				// Specular highlights are very dependent on the implementation of the specific renderer.
				sscanf(token, "%lf", &(material->glossiness));
				material->glossiness = log(fabs(material->glossiness)) / 10.0;
			}
		}
		else if (strcmp(token, "map_Kd") == 0 && material) // diffuse texture
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DDiffuseMap].map, token);
				material->maps[PI3DDiffuseMap].strength=1.0f;
				material->maps[PI3DDiffuseMap].flags=(PI3DTextureMapFlags)0;
			}
// The code below tries to tell if the texture map is absolute or relative.
// It does not work at all in this context, but is an example of what one might try in order to handle absolute paths.
#if 0
			FILE		*testF;

			lineStr = lineStr.RightAfterFirst(' ');
			testF = fopen(lineStr, "r");
			if (testF == NULL)
			{
				curMat->texturePath = path;
				curMat->texturePath = curMat->texturePath.Parent();
				curMat->texturePath.Append(lineStr);
			}
			else
			{
				fclose(testF);
				curMat->texturePath = lineStr;
			}
#endif
		}
#if 1
		else if (strcmp(token, "map_Bump") == 0 && material) // bump map
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DBumpMap].map, token);
				material->maps[PI3DBumpMap].strength=1.0f;
				material->maps[PI3DBumpMap].flags=(PI3DTextureMapFlags)0;
			}
		}
		else if (strcmp(token, "bump") == 0 && material) // bump map (alternate method)
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DBumpMap].map, token);
				material->maps[PI3DBumpMap].strength=1.0f;
				material->maps[PI3DBumpMap].flags=(PI3DTextureMapFlags)0;
			}
		}
#endif
		else if (strcmp(token, "map_D") == 0 && material) // transparency map
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DOpacityMap].map, token);
			}
		}
		else if (strcmp(token, "map_Ks") == 0 && material) // specular map
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DSpecularIntensityMap].map, token);
				material->maps[PI3DSpecularIntensityMap].strength=1.0f;
				material->maps[PI3DSpecularIntensityMap].flags=(PI3DTextureMapFlags)0;
			}
		}
		else if (strcmp(token, "map_Ka") == 0 && material) // ambient map
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				strcpy(material->maps[PI3DSelfIlluminationMap].map, token);
				material->maps[PI3DSelfIlluminationMap].strength=1.0f;
				material->maps[PI3DSelfIlluminationMap].flags=(PI3DTextureMapFlags)0;
			}
		}
	}

	fclose(f);
}

void PI3DAddInfiniteLight(PI3DLight **lightList,float dirx, float diry, float dirz, float red, float green, float blue)
{
	PI3DLight	*theNewLight=PI3DCreateLight ();
	int32 lightCount=1;
	PI3DLight *tempLight= *lightList;
	while(tempLight)
		{
		if(tempLight->type == PI3DInfiniteLight)
			lightCount++;
		tempLight = (PI3DLight*)tempLight->next;
		}

	
	char lightname[256];
	sprintf(lightname,"Infinite Light %d",lightCount);
	strcpy (theNewLight->name, lightname);

	theNewLight->attenuation = false;
	theNewLight->attenuationAbc.a = 0.0f;
	theNewLight->attenuationAbc.b = 0.0f;
	theNewLight->attenuationAbc.c = 0.0f;
	theNewLight->attenuationType = PI3DLinearAttenuation;
	theNewLight->col.red = red;
	theNewLight->col.green = green;
	theNewLight->col.blue = blue;
	theNewLight->falloff = 45.0f;
	theNewLight->hotspot = 0.7f*theNewLight->falloff;
	theNewLight->innerRadius = 0;
	theNewLight->multiple = 1.0f;
	theNewLight->outerRadius = 1.0;
	theNewLight->pos[0] = 0;
	theNewLight->pos[1] = 0;
	theNewLight->pos[2] = 0;
	theNewLight->target[0] = dirx;
	theNewLight->target[1] = diry;
	theNewLight->target[2] = dirz;
	theNewLight->type = PI3DInfiniteLight;
	PI3DListAdd ((PI3DList **)lightList, reinterpret_cast<PI3DList *>(theNewLight));
}
void PI3DDefaultLights(PI3DScene *scene)
{
	// default lights
	bool makeLights = true;
	PI3DLight* pLight = scene->lightList;
	while(pLight)
	{
		if (pLight->type != PI3DGlobalAmbientLight)
		{
			makeLights = false;
			break;
		}
		pLight = (PI3DLight*)pLight->next;
	}
	if (makeLights)
	{
		PI3DAddInfiniteLight(&scene->lightList, -2.0f, -1.5f, -0.5f,0.38f, 0.38f, 0.45f);
 		PI3DAddInfiniteLight(&scene->lightList,2.0f, 1.1f, -2.5f,0.6f, 0.6f, 0.67f);
		PI3DAddInfiniteLight(&scene->lightList, -0.5f, 0.0f, 2.0f,0.5f, 0.5f, 0.57f);
	}
}

void PI3DUpdateRenderState(RenderState *renderState)
{
	renderState->currentCameraPosition.x = 0;
	renderState->currentCameraPosition.y = 0;
	renderState->currentCameraPosition.z = 0;
	renderState->currentCameraPosition.yAngle = 180;
	renderState->currentCameraPosition.xAngle = 90;
	renderState->currentOrthoScale=2;
	renderState->currentOrthographic=false;
}

int PI3DParseFile(PI3DImport *importer)
{
	bool			readFaces = true;
	char			line[MAX_LINE];
	char			token[MAX_LINE];
	char			str[MAX_LINE];
	char			currentMaterial[MAX_LINE];
	PI3DMesh			*mesh;
	PI3DMaterial		*material;
	int32			currentMaterialID;
	int32			maxVertices = 8;
	int32			maxNormals = 8;
	int32			maxUVs = 8;
	int32			maxFaces = 8;

	// initialize the scene
	strcpy(currentMaterial, "__PS_CS3_3D_Default");
	currentMaterialID = 0;
	material = PI3DUpdateMaterials(importer->scene, currentMaterial, 0);
	material->diffuse.red = 1.0;
	material->diffuse.green = 1.0;
	material->diffuse.blue = 1.0;

	mesh = PI3DCreateMesh("", 0, 0, 0, 0, 0, 0);
	PI3DListAdd((PI3DList **)&importer->scene->meshList, reinterpret_cast<PI3DList *>(mesh));
	uint16 *objPathIn=(uint16*)importer->userData;
	
	#if WIN32
		wchar_t winPath[2048];
		PI3DStringCopy16W(winPath,objPathIn);
		FILE	*f = _wfopen(winPath, L"r");
	#else
		//¥¥¥Must fix this
		char macPath[2048];
		PI3DStringCopy16C(macPath,objPathIn);
		FILE	*f = fopen(macPath, "r");
	#endif
	if(!f)
		return 0;
		
	// load the mesh
	while (!feof(f))
	{
		ReadLine(f, MAX_LINE, line);
		PI3DTrimLeft(line);
		PI3DTrimRight(line);
		if (!PI3DSplitString(token, line, true, '\0' ))
			continue;

		if (strcmp(token, "mtllib") == 0) // material library
		{
			if (PI3DSplitString(token, line, true, '\0' ))
			{
				//const wchar_t	objPath[2048] = importer->bitStream->GetName();
				wchar_t			mtlPath[2048];
				PI3DStringCopy16W(mtlPath,objPathIn);
				//wcscpy(mtlPath, objPath);
				size_t i = wcslen(mtlPath);
				#if Macintosh
					wchar_t delimiter=L'/';
				#else
					wchar_t delimiter=L'\\';
					//wchar_t delimiter=(wchar_t)'\\';
				#endif	
				wchar_t currentCharacter=mtlPath[i - 1];
				while (currentCharacter != delimiter)
				{
					i--;
					if (i == 0)
						break;
					currentCharacter=mtlPath[i - 1];
				}
				mtlPath[i] = '\0';
				int32 j = 0;
				while (token[j] != '\0')
				{
					mtlPath[i++] = token[j++];
				}
				mtlPath[i] = '\0';
				LoadMaterialList(mtlPath, importer);
			}
		}
		else if (strcmp(token, "v") == 0) // vertex
		{
			PI3DVector	v = { 0.0, 0.0, 0.0 };

			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(v[0]));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(v[1]));
					if (PI3DSplitString(token, line, true, '\0' ))
					{
						sscanf(token, "%lf", &(v[2]));
					}
				}
			}
			if (!PI3DAddVertexToMesh(mesh, v, maxVertices))
				goto err;
		}
		else if (strcmp(token, "vn") == 0) // normal
		{
			PI3DVector	n = { 0.0, 0.0, 0.0 };

			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(n[0]));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(n[1]));
					if (PI3DSplitString(token, line, true, '\0' ))
					{
						sscanf(token, "%lf", &(n[2]));
					}
				}
			}
			if (!PI3DAddNormalToMesh(mesh, n, maxNormals))
				goto err;
		}
		else if (strcmp(token, "vt") == 0) // UV
		{
			PI3DPoint	uv = { 0.0, 0.0 };

			if (PI3DSplitString(token, line, true, '\0' ))
			{
				sscanf(token, "%lf", &(uv[0]));
				if (PI3DSplitString(token, line, true, '\0' ))
				{
					sscanf(token, "%lf", &(uv[1]));
				}
			}
			if (!PI3DAddUVToMesh(mesh, uv, maxUVs))
				goto err;
		}
		else if (strcmp(token, "f") == 0) // face
		{
			if (!readFaces)
				continue;

			int32		nPoints = 4;
			PI3DFace		f;

			f.flags = 0;
			f.numPoints = 0;
			f.points = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
			f.normals = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
			f.textures = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
			f.colors = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
			if (f.points == NULL || f.normals == NULL || f.textures == NULL || f.colors == NULL)
				goto err;
			f.smoothing = 1;
			f.material = currentMaterialID;
			while (PI3DSplitString(token, line, true, '\0' ))
			{
				if (f.numPoints == nPoints)
				{ // expand the list of points, normals, and UVs
					nPoints *= 2;
					int32	*newPoints = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
					int32	*newNormals = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
					int32	*newTextures = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
					int32	*newColors = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
					if (newPoints == NULL || newNormals == NULL || newTextures == NULL || newColors == NULL)
						goto err;
					for (int32 i = 0; i < f.numPoints; i++)
					{
						newPoints[i] = f.points[i];
						newNormals[i] = f.normals[i];
						newTextures[i] = f.textures[i];
						newColors[i] = f.colors[i];
					}
					PI3DMemoryFree(f.points);
					PI3DMemoryFree(f.normals);
					PI3DMemoryFree(f.textures);
					PI3DMemoryFree(f.colors);
					f.points = newPoints;
					f.normals = newNormals;
					f.textures = newTextures;
					f.colors = newColors;
				}
				f.points[f.numPoints] = 1;
				f.textures[f.numPoints] = 1;
				f.normals[f.numPoints] = 1;
				f.colors[f.numPoints] = 1;
				bool noUV = false;
				for (uint32 i = 0; i < strlen(token) - 1; i++)
				{
					if (token[i] == '/' && token[i + 1] == '/')
						noUV = true;
				}
				if (PI3DSplitString(str, token, false, '/'))
				{
					sscanf(str, "%d", &(f.points[f.numPoints]));
					if (PI3DSplitString(str, token, false, '/'))
					{
						if (noUV)
							sscanf(str, "%d", &(f.normals[f.numPoints]));
						else
							sscanf(str, "%d", &(f.textures[f.numPoints]));
						if (PI3DSplitString(str, token, false, '/'))
						{
							sscanf(str, "%d", &(f.normals[f.numPoints]));
						}
					}
				}
				// indices in OBJ are 1-based, but they are 0-based in Photoshop's scene structure
				f.points[f.numPoints]--;
				f.normals[f.numPoints]--;
				f.textures[f.numPoints]--;
				f.colors[f.numPoints]--;
				f.numPoints++;
			}
			if (!PI3DAddFaceToMesh(mesh, &f, maxFaces))
				goto err;
		}
		else if (strcmp(token, "o") == 0) // object
		{
			// not implemented (very optional)
		}
		else if (strcmp(token, "usemtl") == 0) // use a previously defined material
		{
			PI3DSplitString(currentMaterial, line, true, '\0' );
			PI3DMaterial	*mtl = PI3DUpdateMaterials(importer->scene, currentMaterial, 0);
			currentMaterialID = PI3DFindMaterial(importer->scene, mtl->name);
		}
		else if (strcmp(token, "s") == 0) // smoothing group
		{
			// not implemented
		}
		else if (strcmp(token, "g") == 0) // group
		{
			// not implemented
		}
#ifdef _DEBUG
		else if (strcmp(token, "faces_off") == 0) // ignore faces (not a real OBJ marker - I use it for debugging)
		{
			readFaces = false;
		}
		else if (strcmp(token, "faces_on") == 0) // read faces again (not a real OBJ marker - I use it for debugging)
		{
			readFaces = true;
		}
		else if (strcmp(token, "eof") == 0) // end-of-file (not a real OBJ marker - I use it for debugging)
		{
			break;
		}
#endif
	}

	if (mesh->normals == 0)
		mesh->smoothingGroupPresent = true;

	PI3DDefaultLights(importer->scene);

	fclose(f);
	
	return 1;
err:
	PI3DKillScene(importer->scene);
	fclose(f);
	return 0;
}
