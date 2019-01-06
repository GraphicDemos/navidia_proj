// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------------------
#include "PIDefines.h"
#include "PIUI.h"
#include "PIFilter.h"
#include "PITerminology.h"
#include "PIUtilities.h"
#include "FilterBigDocument.h"
#include "PS3DDescriptor.h"

FilterRecord * gFilterRecord;
intptr_t * gData;
int16 * gResult;
SPBasicSuite * sSPBasic = NULL;

#define MAX_LINE	32768

void DoAbout(void);
void DoParameters(void);
void DoPrepare(void);
void DoStart(void);
void DoContinue(void);
void DoFinish(void);

static uint32 StringLength3D(wchar_t *string)
	{
	if(!string)
		return 0;
		
	uint32 strLen=0;
	
	while(string[strLen] != 0) 
		strLen++;
	
	return strLen;
	}	
static uint32 StringLength3D(uint16 *string)
	{
	if(!string)
		return 0;
		
	uint32 strLen=0;
	
	while(string[strLen] != 0) 
		strLen++;
	
	return strLen;
	}	

static void StringCopy3D(uint16 *destString,wchar_t *sourceString)
	{
	if(!destString || !sourceString)
		return;
		
	//¥¥¥ÊWe hope dest has enough memory
	uint32 strLen=StringLength3D(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
	}

static void StringCopy3D(char *destString,uint16 *sourceString)
	{
	if(!destString || !sourceString)
		return;
		
	//¥¥¥ÊWe hope dest has enough memory
	uint32 strLen=StringLength3D(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
	}


static void StringAppend3D(uint16 *destString,char *sourceString)
	{
	if(!destString || !sourceString)
		return;
		
	//¥¥¥ÊWe hope dest has enough memory
	uint32 strLenToAppend=strlen(sourceString);
	uint32 strLenInitial=StringLength3D(destString);

	for(uint16 i=0;i<=strLenToAppend;i++)
		destString[strLenInitial+i]=sourceString[i];
	
	destString[strLenToAppend+strLenInitial]=0;
	}
static void GetTempFolderPath( uint16 *tempPath)
{
	//Find the Temp directory
	#if MSWindows
		wchar_t tempFolderPath[4096];
		GetTempPathW(4096,tempFolderPath);
		StringCopy3D(tempPath,tempFolderPath);
	#else
		FSRef folderRef;
		OSErr  err = FSFindFolder( kOnSystemDisk, kTemporaryFolderType, true, &folderRef );
		if ( err != noErr )
			{
			err = FSFindFolder( kOnAppropriateDisk, kTemporaryFolderType, true, &folderRef );
			}
		if(err != noErr)
			{
			wchar_t tempFolderPath[]=L"/tmp/";
//			strcpy(tempFolderPath,"/tmp/");
			StringCopy3D(tempPath,tempFolderPath);
			}
		else
			{
			CFURLRef url = CFURLCreateFromFSRef( kCFAllocatorDefault, &folderRef );
			CFStringRef cfString = NULL;
			if ( url != NULL )
				{
				cfString = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
				CFStringGetCString(cfString,(char*)tempPath,2048,kCFStringEncodingUnicode);
				int32 len=CFStringGetLength(cfString);
				CFRelease( url );
				
				tempPath[len]='/';
				tempPath[len+1]=0;
				}
			
			}
		
	#endif

}
static void PackPathIntoDescritpor( FilterRecord* fr,uint16 *tempPath)
{
	
	PSActionDescriptorProcs *actionDescriptorProcs=NULL;    
    int32 error = fr->sSPBasic->AcquireSuite(kPSActionDescriptorSuite,
                                  kPSActionDescriptorSuiteVersion, 
                                  (const void**)&actionDescriptorProcs);    
    ASZStringSuite *sPSZStringSuite=NULL;
    error = fr->sSPBasic->AcquireSuite(kASZStringSuite,
                                  kASZStringSuiteVersion1, 
                                  (const void**)&sPSZStringSuite);
                                  
    PIActionDescriptor actionDescriptor=NULL;
    actionDescriptorProcs->Make(&actionDescriptor);
    
    ASZString pathString;
    sPSZStringSuite->MakeFromUnicode(tempPath,StringLength3D(tempPath),&pathString);
    
    actionDescriptorProcs->PutZString(actionDescriptor,keyFile,pathString);
    
        
    actionDescriptorProcs->AsHandle(actionDescriptor, &fr->output3DScene->descriptor);
    
    actionDescriptorProcs->Free(actionDescriptor);
    
}
static void PackPlatformSpecIntoDescritpor( FilterRecord* fr,SPPlatformFileSpecificationW *spec)
{
	
	PSActionDescriptorProcs *actionDescriptorProcs=NULL;    
    int32 error = fr->sSPBasic->AcquireSuite(kPSActionDescriptorSuite,
                                  kPSActionDescriptorSuiteVersion, 
                                  (const void**)&actionDescriptorProcs);    
    PSAliasSuite *sPSAliasSuite=NULL;
    error = fr->sSPBasic->AcquireSuite(kPSAliasSuite,
                                  kPSAliasSuiteVersion1, 
                                  (const void**)&sPSAliasSuite);
                                  
    PIActionDescriptor actionDescriptor=NULL;
    actionDescriptorProcs->Make(&actionDescriptor);
    
    AliasHandle aliasHandle;
	
#ifdef WIN32
    error=sPSAliasSuite->WinNewAliasFromWidePath(spec->mReference,&aliasHandle);
#else
    error=sPSAliasSuite->MacNewAliasFromFSRef((void*)&spec->mReference,&aliasHandle);
#endif

    actionDescriptorProcs->PutAlias(actionDescriptor,keyFileReference,(Handle)aliasHandle);

    actionDescriptorProcs->AsHandle(actionDescriptor, &fr->output3DScene->descriptor);
    
    actionDescriptorProcs->Free(actionDescriptor);
    
}
DLLExport SPAPI void PluginMain(const int16 selector,
								FilterRecordPtr filterRecord,
								intptr_t * data,
								int16 * result)
{
	gData = data;
	gResult = result;
	
	if (selector == filterSelectorAbout)
	{
		sSPBasic = ((AboutRecordPtr)filterRecord)->sSPBasic;
	} 
	else 
	{
		gFilterRecord = filterRecord;
		sSPBasic = gFilterRecord->sSPBasic;

		if (gFilterRecord->bigDocumentData != NULL)
			gFilterRecord->bigDocumentData->PluginUsing32BitCoordinates = true;
	}

	switch (selector)
	{
		case filterSelectorAbout:
			DoAbout();
			break;
		case filterSelectorParameters:
			DoParameters();
			break;
		case filterSelectorPrepare:
			DoPrepare();
			break;
		case filterSelectorStart:
			DoStart();
			break;
		case filterSelectorContinue:
			DoContinue();
			break;
		case filterSelectorFinish:
			DoFinish();
			break;
		default:
			*gResult = filterBadParameters;
			break;
	}
}

void DoAbout(void)
{
	AboutRecord * aboutRecord = reinterpret_cast<AboutRecord *>(gFilterRecord);
	DoAbout(static_cast<SPPluginRef>(aboutRecord->plugInRef), AboutID);
}

void DoParameters(void)
{
}

void DoPrepare(void)
{
	gFilterRecord->bufferSpace = 0;
	gFilterRecord->maxSpace = 0;
}
static bool FaceExists(int32 x,int32 y, VRect &filterRect,PI3DVector *vertices)
	{
	int32 imageHor = filterRect.right - filterRect.left;
	int32 topLeft=x+y*imageHor;
	int32 topRight=x+1+y*imageHor;
	int32 bottomLeft=x+(y+1)*imageHor;
	int32 bottomRight=x+1+(y+1)*imageHor;
	if(vertices[topLeft][2] || vertices[topRight][2] || vertices[bottomLeft][2] || vertices[bottomRight][2])
		return true;
		
	return false;
	}


Malloc3D g3DMallocFunction=NULL;
Free3D g3DFreeFunction=NULL;

void DoStart(void)
{
	// initialize the scene
	PI3DScene *theScene=PI3DCreateScene(NULL);
			
	char			currentMaterial[MAX_LINE];
	PI3DMesh			*mesh;
	PI3DMaterial		*material;
	int32			currentMaterialID;

	//create the default material - required
	strcpy(currentMaterial, "__PS_CS3_3D_Default");
	currentMaterialID = 0;
	material = PI3DUpdateMaterials(theScene, currentMaterial, 0);
	material->diffuse.red = 1.0;
	material->diffuse.green = 1.0;
	material->diffuse.blue = 1.0;
	
	//Create a second material with the 2 maps attached
	strcpy(currentMaterial, "MyMaterial");
	currentMaterialID = 1;
	material = PI3DUpdateMaterials(theScene, currentMaterial, 0);
	material->diffuse.red = 0.8;
	material->diffuse.green = 0.8;
	material->diffuse.blue = 0.8;
	material->ambient.red = 0.8;
	material->ambient.green = 0.8;
	material->ambient.blue = 0.8;
	material->specular.red = 0.6;
	material->specular.green = 0.6;
	material->specular.blue = 0.6;
	strcpy(material->maps[PI3DDiffuseMap].map, "Diffuse.png");
	material->maps[PI3DDiffuseMap].strength=1.0f;
	material->maps[PI3DDiffuseMap].flags=(PI3DTextureMapFlags)0;
	
	strcpy(material->maps[PI3DBumpMap].map, "Bump.png");
	material->maps[PI3DBumpMap].strength=1.0f;
	material->maps[PI3DBumpMap].flags=(PI3DTextureMapFlags)0;
	
	//Create an empty mesh and add to list
	mesh = PI3DCreateMesh("", 0, 0, 0, 0, 0, 0);
	PI3DListAdd((PI3DList **)&theScene->meshList, reinterpret_cast<PI3DList *>(mesh));
	
	//Get things ready to cycle through the pixels
	int16 tileHeight = gFilterRecord->outTileHeight;
	int16 tileWidth = gFilterRecord->outTileWidth;

	if (tileWidth == 0 || tileHeight == 0 || gFilterRecord->advanceState == NULL)
	{
		*gResult = filterBadParameters;
		return;
	}
	
	VRect outRect = GetOutRect();
	VRect filterRect = GetFilterRect();
	
	int32 imageVert = filterRect.bottom - filterRect.top;
	int32 imageHor = filterRect.right - filterRect.left;

	uint32 tilesVert = (tileHeight - 1 + imageVert) / tileHeight;
	uint32 tilesHoriz = (tileWidth - 1 + imageHor) / tileWidth;

	int16 layerPlanes = 1;
	if (gFilterRecord->filterCase > 2)
		layerPlanes = gFilterRecord->outLayerPlanes;
	else
		layerPlanes = gFilterRecord->planes;

	
	int32 progress_total = layerPlanes * tilesVert;
	int32 progress_complete = 0;
	
	//Allocate the space for the vertices and uvs of the mesh
	mesh->vertices=imageVert*imageHor;
	mesh->vertex = (PI3DVector *)PI3DMemoryAlloc(sizeof(PI3DVector) * mesh->vertices);
	mesh->textures=imageVert*imageHor;
	mesh->texture = (PI3DPoint *)PI3DMemoryAlloc(sizeof(PI3DPoint) * mesh->textures);
	mesh->faces=(imageVert-1)*(imageHor-1);
	mesh->face = (PI3DFace *)PI3DMemoryAlloc(sizeof(PI3DFace) * mesh->faces);
	mesh->smoothingGroupPresent = true;
	
	//We'll just grab the red channel for now
	int16 planeCount = 0;
	//for(int16 planeCount = 0; planeCount < layerPlanes; planeCount++)
	{
		gFilterRecord->outLoPlane = planeCount;
		gFilterRecord->outHiPlane = planeCount;

		for(uint16 vertTile = 0; vertTile < tilesVert; vertTile++)
		{
			for(uint16 horizTile = 0; horizTile < tilesHoriz; horizTile++)
			{
				outRect.top = filterRect.top + ( vertTile * tileHeight );
				outRect.left = filterRect.left + ( horizTile * tileWidth );
				outRect.bottom = outRect.top + tileHeight;
				outRect.right = outRect.left + tileWidth;

				if (outRect.bottom > filterRect.bottom)
					outRect.bottom = filterRect.bottom;
				if (outRect.right > filterRect.right)
					outRect.right = filterRect.right;

				SetOutRect(outRect);

				*gResult = gFilterRecord->advanceState();
				if (*gResult != kNoErr) return;

				outRect = GetOutRect();
				
				uint8 * smallPixel = static_cast<uint8 *>(gFilterRecord->outData);
				uint16 * largePixel = static_cast<uint16 *>(gFilterRecord->outData);

				uint32 rectHeight = outRect.bottom - outRect.top;
				uint32 rectWidth = outRect.right - outRect.left;

				for(uint32 pixelY = 0; pixelY < rectHeight; pixelY++)
				{
					for(uint32 pixelX = 0; pixelX < rectWidth; pixelX++)
					{
						uint32 x	=	outRect.left	+	pixelX;
						uint32 y	=	outRect.top		+	pixelY;
						
						uint32 vertexIndex = x + imageHor*y;
						uint16 height=0;
						
						if (gFilterRecord->depth == 8 && *smallPixel)
							height=*smallPixel;
						if (gFilterRecord->depth == 16 && *largePixel)
							height=*largePixel;
						
						//Calculate each vertex based on the pixel value
						mesh->vertex[vertexIndex][0]=imageHor-x;
						mesh->vertex[vertexIndex][1]=height;
						mesh->vertex[vertexIndex][2]=imageVert-y;
						
						//Calculate each texture coordinate based on the pixel value
						mesh->texture[vertexIndex][0]=((nativeFloat)x)/((nativeFloat)imageHor);
						mesh->texture[vertexIndex][1]=((nativeFloat)(imageVert-y))/((nativeFloat)imageVert);
									
						largePixel++;
						smallPixel++;
					}
					smallPixel += gFilterRecord->outRowBytes - rectWidth;
					largePixel += gFilterRecord->outRowBytes / 2 - rectWidth;
				}

			}

			gFilterRecord->progressProc(++progress_complete, progress_total);

			if (gFilterRecord->abortProc()) 
			{
				*gResult = userCanceledErr;
				goto done;
			}
		}
	}
done:
	
	outRect.top = 0;
	outRect.left = 0;
	outRect.bottom = 0;
	outRect.right = 0;
	SetOutRect(outRect);
	
	//Cycle back through and create faces to map to the vertices and uvs we made
	int32 x1,y1,faceIndex=0;
	for(y1=filterRect.top; y1<filterRect.bottom-1;y1++)
		{
		
		int32 topLeft=y1*imageHor;
		int32 topRight=1+y1*imageHor;
		int32 bottomLeft=(y1+1)*imageHor;
		int32 bottomRight=1+(y1+1)*imageHor;
		
		for(x1=filterRect.left; x1<filterRect.right-1;x1++)
			{
			//Only add a face if there is a non-black value in one of the 4 corners - this is how holes are made
			if(mesh->vertex[topLeft][1] || mesh->vertex[topRight][1] || mesh->vertex[bottomLeft][1] || mesh->vertex[bottomRight][1])
				{
				int32		nPoints = 4;
				PI3DFace	f;
				f.points = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
				f.normals = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
				f.textures = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
				f.colors = (int32 *)PI3DMemoryAlloc(sizeof(int32) * nPoints);
				f.flags = 0x57;
				
				if (f.points == NULL || f.normals == NULL || f.textures == NULL || f.colors == NULL)
					break;
				f.smoothing = 1;
				f.material = currentMaterialID;
				f.numPoints = nPoints;
				
				f.points[0]=topLeft;
				f.normals[0]=topLeft;
				f.textures[0]=topLeft;
				f.colors[0]=topLeft;
				
				f.points[1]=topRight;
				f.normals[1]=topRight;
				f.textures[1]=topRight;
				f.colors[1]=topRight;
				
				f.points[2]=bottomRight;
				f.normals[2]=bottomRight;
				f.textures[2]=bottomRight;
				f.colors[2]=bottomRight;
				
				f.points[3]=bottomLeft;
				f.normals[3]=bottomLeft;
				f.textures[3]=bottomLeft;
				f.colors[3]=bottomLeft;
				
				mesh->face[faceIndex++]=f;
				}
			topLeft++;
			bottomRight++;
			bottomLeft++;
			topRight++;
			
			}
		}
	
	//Reset the # of faces to match how many we actually added
	mesh->faces=faceIndex;
			
	if(theScene)
		{
		//Fill in scene
		
		PI3DDescriptorProcs descriptorProcs;
		
		descriptorProcs.actionDescriptorProcs=NULL;
		descriptorProcs.actionListProcs=NULL;
		descriptorProcs.actionControlProcs=NULL;
		descriptorProcs.zStringProcs=NULL;
		
		int32 error = gFilterRecord->sSPBasic->AcquireSuite(kPSActionDescriptorSuite,
                                  kPSActionDescriptorSuiteVersion, 
                                  (const void**)&descriptorProcs.actionDescriptorProcs);    
		if(!error)
			error = gFilterRecord->sSPBasic->AcquireSuite(kPSActionListSuite,
                                  kPSActionListSuiteVersion,
                                  (const void**)&descriptorProcs.actionListProcs);
		if(!error)
			error = gFilterRecord->sSPBasic->AcquireSuite(kPSBasicActionControlSuite,
                                  kPSBasicActionControlSuitePrevVersion,
                                  (const void**)&descriptorProcs.actionControlProcs);
		if(!error)
			error = gFilterRecord->sSPBasic->AcquireSuite(kASZStringSuite,
                                  kASZStringSuiteVersion1, 
                                  (const void**)&descriptorProcs.zStringProcs);

		PIActionDescriptor theLayerDescriptor=NULL;
		if(!error)
			{
			
			descriptorProcs.actionDescriptorProcs->Make(&theLayerDescriptor);
			if(theLayerDescriptor)
				{
				PI3DLayer theLayer;
				VPoint docSize;
				docSize.h=gFilterRecord->imageSize.h;
				docSize.v=gFilterRecord->imageSize.v;
				
				//Get the 3D layer set up
				PI3DInitialize3DLayer(&theLayer,&docSize);
				
				//Hand it the scene we created
				theLayer.currentScene=theScene;
				
				//Fill in current camera and object position and the textures
				//Set up an orthographic camera to fit the object exactly in the view
				
				//This first section is for the View menu in the 3D tool
				theLayer.stateList.length=1;
				theLayer.stateList.positionNames=(uint16**)PI3DMemoryAlloc(theLayer.stateList.length * sizeof(uint16*)); 
				theLayer.stateList.positionNames[0] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				PI3DStringCopyC16(theLayer.stateList.positionNames[0],(const char *)"Test Camera");
				theLayer.stateList.viewStates=(RenderState*)PI3DMemoryAlloc(theLayer.stateList.length * sizeof(RenderState)); 
				memset(&theLayer.stateList.viewStates[0],0,sizeof(RenderState));
				
				if(gFilterRecord->depth == 8)
					theLayer.stateList.viewStates[0].currentCameraPosition.y = 400;
				else
					theLayer.stateList.viewStates[0].currentCameraPosition.y = 100000;
				theLayer.stateList.viewStates[0].currentOrthoScale=imageVert;
				theLayer.stateList.viewStates[0].currentOrthographic=true;
				theLayer.stateList.viewStates[0].currentCameraPosition.xAngle=-90;
				theLayer.stateList.viewStates[0].currentFieldOfView=42.0;
				theLayer.stateIndex=8;
				
				//This sets the current view to be this camera
				theLayer.currentRenderState.currentOrthoScale=theLayer.stateList.viewStates[0].currentOrthoScale;
				theLayer.currentRenderState.currentOrthographic=theLayer.stateList.viewStates[0].currentOrthographic;
				theLayer.currentRenderState.currentCameraPosition=theLayer.stateList.viewStates[0].currentCameraPosition;
				theLayer.currentRenderState.currentFieldOfView=theLayer.stateList.viewStates[0].currentFieldOfView;
				
				//Add in some textures
				theLayer.texturesExternal=true; //Textures are external to the file
				theLayer.texturesEnabled=true; //The master eyeball
				theLayer.textureList.length=2;
				
				//The individual eyyeballs
				theLayer.textureEnabled = (Boolean*)PI3DMemoryAlloc(theLayer.textureList.length * sizeof(Boolean));
				theLayer.textureEnabled[0]=true;
				theLayer.textureEnabled[1]=true;
				
				//The type of each texture - Note that bump maps disable the Cross Section tool
				theLayer.textureList.textureType = (uint32*)PI3DMemoryAlloc(theLayer.textureList.length * sizeof(uint32));
				theLayer.textureList.textureType[0]=k3DMapTypeTexture;
				theLayer.textureList.textureType[1]=k3DMapTypeBump;
				
				//We need to set a path for the scene so that PS knows where to look for textures
				theLayer.scenePath = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				GetTempFolderPath( theLayer.scenePath);
				StringAppend3D(theLayer.scenePath,"dummy.obj");
				
				//Give the paths to the textures
				//In this case we're just going to give paths to nothing
				//This will create empty textures that users can fill in later
				//Note that whatever path you put in for texturePaths, that file will get removed as
				//it is assumed to be a temp file.  So, you probably want point it at a bogus file.
				theLayer.textureList.textureNames = (uint16**)PI3DMemoryAlloc(theLayer.textureList.length * sizeof(uint16*));
				theLayer.textureList.texturePaths =(uint16**) PI3DMemoryAlloc(theLayer.textureList.length * sizeof(uint16*));
				
				theLayer.textureList.texturePaths[0] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				theLayer.textureList.textureNames[0] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				theLayer.textureList.texturePaths[0][0]=0;
				GetTempFolderPath( theLayer.textureList.textureNames[0]);
				StringAppend3D(theLayer.textureList.textureNames[0],"Diffuse.png");
				StringAppend3D(theLayer.textureList.texturePaths[0],"Diffuse.png");
				
				theLayer.textureList.texturePaths[1] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				theLayer.textureList.textureNames[1] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
				theLayer.textureList.texturePaths[1][0]=0;
				GetTempFolderPath( theLayer.textureList.textureNames[1]);
				StringAppend3D(theLayer.textureList.texturePaths[1],"Bump.png");
				StringAppend3D(theLayer.textureList.textureNames[1],"Bump.png");
				
				//Convert the entire layer data structure into a descriptor for Photoshop
				PI3DMakeLayerDescriptor(&descriptorProcs, &theLayer, theLayerDescriptor);
				
				//Clean up behind us
				theLayer.currentScene=NULL;
				PI3DKill3DLayer(&theLayer);
				}
			}
			
		//Clean up behind us	
		if(theScene)
			PI3DKillScene(theScene);
		
		//Convert theLayerDescriptor to a handle and free the descriptor
		//When PS sees this descriptor handle it creates a new 3D layer with that data in it
		if(theLayerDescriptor)
			{
			descriptorProcs.actionDescriptorProcs->AsHandle(theLayerDescriptor, &gFilterRecord->output3DScene->descriptor);
			descriptorProcs.actionDescriptorProcs->Free(theLayerDescriptor);
			}
		
		}
}

void DoContinue(void)
{
	VRect outRect = { 0, 0, 0, 0};
	SetOutRect(outRect);
}

void DoFinish(void)
{
}

//Need to stub this out...
int					PI3DParseFile (PI3DImport *importer) { return 1; }
// end 3dheightfield.cpp