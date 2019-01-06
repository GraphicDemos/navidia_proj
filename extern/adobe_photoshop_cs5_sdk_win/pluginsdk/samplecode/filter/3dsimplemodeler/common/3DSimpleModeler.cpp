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
#include "PIUtilities.h"
#include "FilterBigDocument.h"
#include "PS3DDescriptor.h"

FilterRecord * gFilterRecord;
intptr_t * gData;
int16 * gResult;
SPBasicSuite * sSPBasic = NULL;

void DoAbout(void);
void DoParameters(void);
void DoPrepare(void);
void DoStart(void);
void DoContinue(void);
void DoFinish(void);

void InvertTile(void * tileData, VRect & tileRect, int32 rowBytes);

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
		gFilterRecord = (FilterRecordPtr)filterRecord;
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
	
	//••• We hope dest has enough memory
	uint32 strLen=StringLength3D(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
}

static void StringCopy3D(char *destString,uint16 *sourceString)
{
	if(!destString || !sourceString)
		return;
	
	//••• We hope dest has enough memory
	uint32 strLen=StringLength3D(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
}
static void StringCopy3D(uint16 *destString,uint16 *sourceString)
{
	if(!destString || !sourceString)
		return;
	
	//••• We hope dest has enough memory
	uint32 strLen=StringLength3D(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
}
static void StringCopy3D(uint16 *destString,char *sourceString)
{
	if(!destString || !sourceString)
		return;
	
	//••• We hope dest has enough memory
	uint32 strLen=strlen(sourceString);
	for(uint16 i=0;i<=strLen;i++)
		destString[i]=sourceString[i];
	
	destString[strLen]=0;
}
static void StringAppend3D(uint16 *destString,char *sourceString)
{
	if(!destString || !sourceString)
		return;
	
	//••• We hope dest has enough memory
	uint32 strLenToAppend=strlen(sourceString);
	uint32 strLenInitial=StringLength3D(destString);
	
	for(uint16 i=0;i<=strLenToAppend;i++)
		destString[strLenInitial+i]=sourceString[i];
	
	destString[strLenToAppend+strLenInitial]=0;
}	
void DoStart(void)
{
	//Convert to data structure
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
	
	descriptorProcs.handleProcs=gFilterRecord->handleProcs;
	
	if(error)
		{
		*gResult = filterBadParameters;
		return;
		}
	PIActionDescriptor inputDescriptor = NULL;
			
	//Turn the handle into a decriptor
	descriptorProcs.actionDescriptorProcs->HandleToDescriptor(gFilterRecord->input3DScene->descriptor,&inputDescriptor);
			
	PI3DLayer theLayer;
	VPoint docSize;
	docSize.h=gFilterRecord->imageSize.h;
	docSize.v=gFilterRecord->imageSize.v;
	
	//Initialize the 3D Layer data structure - See PI3DDescriptor.h
	PI3DInitialize3DLayer(&theLayer,&docSize);
	if(PI3DGetLayerFromDescriptor(&descriptorProcs, inputDescriptor, &theLayer))
		{
		//If this fails, we couldn't get the 3D scene data
		*gResult = filterBadParameters;
		return;
		}
	
	//Free up the input descriptor
	descriptorProcs.actionDescriptorProcs->Free(inputDescriptor);
	

	// get user defined data
	ResourceProcs* sResourceSuite = NULL;
	if (gFilterRecord->sSPBasic->AcquireSuite(kPIResourceSuite, kPIResourceSuiteVersion, (const void **)&sResourceSuite))
	{
		*gResult = filterBadParameters;
		return;
	}

#if  defined (__ppc__)
	const ResType userDefType = 'tRsU';	// this is the type reversed for big endian
#else
	const ResType userDefType = 'UsRt'; // this the type you define for your format
#endif
		
		uint16 num = sResourceSuite->countProc(userDefType);
		if (num == 1){
			if(Handle ourData = sResourceSuite->getProc(userDefType,num)){
				uint32 size = gFilterRecord->handleProcs->getSizeProc(ourData);
				gFilterRecord->handleProcs->lockProc(ourData, false);
				std::string temp(*ourData,size);
				// do something with user data here
				gFilterRecord->handleProcs->unlockProc(ourData);
			}
		}

	gFilterRecord->sSPBasic->ReleaseSuite(kPIResourceSuite, kPIResourceSuiteVersion);
	//end get user data

	//Modify the Scene - This is where the work happens
	//We're just going to add a single new light to the scene for this example for now.
	//However, here is where you would do whatever you are going to do to the scene
	//For more information about how to access what is already inside the scene, see the 3DSimpleRender example.
	//Also, if you look in PSScene, PI3DParseSceneIntoDescriptor traverses teh entire scene graph 
	PI3DLight	*theNewLight=PI3DCreateLight ();
	strcpy (theNewLight->name, "My New Light");
	theNewLight->attenuation = false;
	theNewLight->attenuationAbc.a = 0.0;
	theNewLight->attenuationAbc.b = 0.0;
	theNewLight->attenuationAbc.c = 0.0;
	theNewLight->attenuationType = PI3DLinearAttenuation;
	theNewLight->col.red = 1.0;
	theNewLight->col.green = 0.0;
	theNewLight->col.blue = 0.0;
	theNewLight->falloff = 180.0;
	theNewLight->hotspot = 0.7f*theNewLight->falloff;
	theNewLight->innerRadius = 0;
	theNewLight->multiple = 1.0;
	theNewLight->outerRadius = 0;
	theNewLight->pos[0] = theLayer.currentRenderState.currentCameraPosition.x;
	theNewLight->pos[1] = theLayer.currentRenderState.currentCameraPosition.y;
	theNewLight->pos[2] = theLayer.currentRenderState.currentCameraPosition.z;
	theNewLight->target[0] = theLayer.currentRenderState.currentObjectPosition.x;
	theNewLight->target[1] = theLayer.currentRenderState.currentObjectPosition.y;
	theNewLight->target[2] = theLayer.currentRenderState.currentObjectPosition.z;
	theNewLight->shadowFlag = false;
	theNewLight->type = PI3DInfiniteLight;
	PI3DListAdd ((PI3DList **)&theLayer.currentScene->lightList, reinterpret_cast<PI3DList *>(theNewLight));
	
	PI3DMaterial *theMaterial=theLayer.currentScene->matPropList;
	
	Boolean				*textureEnabled;	//The eyeball on each layer.  This array should be textures.length in size.
	U3DTextureArray		textureList;		//textureNames - a full path to the origin texture file.  Used to enable/disable Replace Textures among other things
	textureList.length=theLayer.textureList.length+1;

// Below here is some sample code for adding/removing/replacing textures in the scene
// Delete an image
	//This kills the diffuse texture on the first material of the scene.
	//theMaterial->maps[PI3DDiffuseMap].map[0]=0;
	
// Add an image
	//strcpy((char*)theMaterial->maps[PI3DSphereEnvironmentMap].map,"/Users/falco/test.jpg");
	strcpy((char*)theMaterial->maps[PI3DDiffuseMap].map,"/Users/falco/test.jpg");
	/*theMaterial->maps[PI3DDiffuseMap].photoshopTextureID = textureList.length-1;
	while(0)//theMaterial)
		{
		for(int i=0;i<kMax3DMapTypes;i++)
			theMaterial->maps[i].photoshopTextureID=-1;
		theMaterial = (PI3DMaterial*)theMaterial->next;
		}*/
	//Only do this on add
	//Give the paths to the textures
	//In this case we're just going to give paths to nothing
	//This will create empty textures that users can fill in later
	
	textureList.textureNames = (uint16**)PI3DMemoryAlloc(textureList.length * sizeof(uint16*));
	textureList.texturePaths = (uint16**)PI3DMemoryAlloc(textureList.length * sizeof(uint16*));
	textureEnabled  = (Boolean*) PI3DMemoryAlloc(textureList.length * sizeof(Boolean));
	
	for(int index=0;index < theLayer.textureList.length; index++)
		{
		textureList.texturePaths[index] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
		textureList.textureNames[index] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));	

		textureEnabled[index]=theLayer.textureEnabled[index];
		StringCopy3D(textureList.textureNames[index],theLayer.textureList.textureNames[index]);
		StringCopy3D(textureList.texturePaths[index],theLayer.textureList.texturePaths[index]);

		PI3DMemoryFree(theLayer.textureList.textureNames[index]);
		PI3DMemoryFree(theLayer.textureList.texturePaths[index]);
		}
	
	int32 newIndex=textureList.length-1;
	PI3DMemoryFree(theLayer.textureList.textureNames);
	PI3DMemoryFree(theLayer.textureList.texturePaths);
	PI3DMemoryFree(theLayer.textureEnabled);

	textureList.texturePaths[newIndex] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
	textureList.textureNames[newIndex] = (uint16*)PI3DMemoryAlloc(256 * sizeof(uint16));
	StringCopy3D(textureList.textureNames[newIndex],"/Users/falco/test.jpg");
	StringCopy3D(textureList.texturePaths[newIndex],"/Users/falco/test.jpg");
	textureEnabled[newIndex]=true;

	theLayer.textureEnabled = textureEnabled;
	theLayer.textureList.texturePaths=textureList.texturePaths;
	theLayer.textureList.textureNames=textureList.textureNames;

	theLayer.currentRenderState.currentObjectPosition.yAngle=90;
done:
	VRect outRect;
	outRect.top = 0;
	outRect.left = 0;
	outRect.bottom = 0;
	outRect.right = 0;
	SetOutRect(outRect);
	
	//Here, we take the scene and all of the associated data and repack it into a descriptor for Photoshop
	PIActionDescriptor outputDescriptor=NULL;
	descriptorProcs.actionDescriptorProcs->Make(&outputDescriptor);
	PI3DMakeLayerDescriptor(&descriptorProcs, &theLayer, outputDescriptor);
	
	//Free up the memory
	PI3DKill3DLayer(&theLayer);
	
	//Convert outputDescriptor to a handle and free the descriptor
	if(outputDescriptor)
		{
		descriptorProcs.actionDescriptorProcs->AsHandle(outputDescriptor, &gFilterRecord->output3DScene->descriptor);
		descriptorProcs.actionDescriptorProcs->Free(outputDescriptor);
		}
	
	// this code used to send block of user defined data
	if (gFilterRecord->sSPBasic->AcquireSuite(kPIResourceSuite, kPIResourceSuiteVersion, (const void **)&sResourceSuite))
	{
		*gResult = filterBadParameters;
		return;
	}		
		std::string data("block of data");
		int dataSZ = static_cast<int>(data.size());
		Handle ourData = NULL;
		num = sResourceSuite->countProc(userDefType);
		if (num <= 1){
			if (num == 1){		/// this had better be ours!
				sResourceSuite->deleteProc(userDefType,num);
			}
			ourData = gFilterRecord->handleProcs->newProc(dataSZ);
			gFilterRecord->handleProcs->lockProc(ourData, false);
			std::memcpy(*ourData,data.c_str(),dataSZ);
			gFilterRecord->handleProcs->unlockProc(ourData);
			sResourceSuite->addProc(userDefType,ourData);
			gFilterRecord->handleProcs->disposeProc(ourData);
		}

	gFilterRecord->sSPBasic->ReleaseSuite(kPIResourceSuite, kPIResourceSuiteVersion);
	// end send new user data

	gFilterRecord->createNewLayer=true;
			
}

void DoContinue(void)
{
	VRect outRect = { 0, 0, 0, 0};
	SetOutRect(outRect);
}

void DoFinish(void)
{
}

void InvertTile(void * tileData, VRect & tileRect, int32 rowBytes)
{
	uint8 * smallPixel = static_cast<uint8 *>(tileData);
	uint16 * largePixel = static_cast<uint16 *>(tileData);

	uint32 rectHeight = tileRect.bottom - tileRect.top;
	uint32 rectWidth = tileRect.right - tileRect.left;

	for(uint32 pixelY = 0; pixelY < rectHeight; pixelY++)
	{
		for(uint32 pixelX = 0; pixelX < rectWidth; pixelX++)
		{
			if (gFilterRecord->depth == 16)
			{
				*largePixel = 32768 - *largePixel;
				largePixel++;
			} 
			else
			{
				*smallPixel = 255 - *smallPixel;
				smallPixel++;
			}
		}
		smallPixel += rowBytes - rectWidth;
		largePixel += rowBytes / 2 - rectWidth;
	}
}

//Need to stub this out...
int					PI3DParseFile (PI3DImport *importer) { return 1; }
// end Invert.cpp