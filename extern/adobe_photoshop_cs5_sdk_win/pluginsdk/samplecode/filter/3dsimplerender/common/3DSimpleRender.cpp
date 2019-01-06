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
Malloc3D g3DMallocFunction=NULL;
Free3D g3DFreeFunction=NULL;	
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
	PI3DInitialize3DLayer(&theLayer,&docSize);
	
	if(PI3DGetLayerFromDescriptor(&descriptorProcs, inputDescriptor, &theLayer))
		{
		*gResult = filterBadParameters;
		return;
		}
	
	//Free up the input descriptor
	descriptorProcs.actionDescriptorProcs->Free(inputDescriptor);
	
	
	RenderDrawBuffer textureBuffer;
	textureBuffer.pixels=NULL;
/*		
	//Texture test
	if(theLayer.getTextureSizeFunction && theLayer.getTextureFunction)
		{
		int index=0;
		theLayer.getTextureSizeFunction(index,&textureBuffer);
		if(textureBuffer.sceneSize.h > 0)
			{
			//Allocate some memory
			textureBuffer.planeMap[0]=0;
			textureBuffer.planeMap[1]=1;
			textureBuffer.planeMap[2]=2;
			textureBuffer.planeMap[3]=3;
			
			textureBuffer.pixels = new char[textureBuffer.rowBytes*textureBuffer.sceneSize.h];
			theLayer.getTextureFunction(index,&textureBuffer);
			FILE *tempFile=fopen("/temp.raw","wb");
			if(tempFile)
				{
				fwrite(textureBuffer.pixels,1,textureBuffer.rowBytes*textureBuffer.sceneSize.h,tempFile);
				fflush(tempFile);
				fclose(tempFile);
				}
			}
		}
*/	
	
	int16 tileHeight = gFilterRecord->outTileHeight;
	int16 tileWidth = gFilterRecord->outTileWidth;

	if (tileWidth == 0 || tileHeight == 0 || gFilterRecord->advanceState == NULL)
	{
		*gResult = filterBadParameters;
		return;
	}
	
	//For now, all we're going to do is invert the pixels.
	//Furture drops of the SDK will include a simple renderer
	//The scene is contained in theLayer.currentScene and is defined in PSScene.h
	//To see how to traverse it, look in PSScene.cpp - PI3DParseSceneIntoDescriptor
	VRect outRect = GetOutRect();
	VRect filterRect = GetFilterRect();

	int32 imageVert = filterRect.bottom - filterRect.top;
	int32 imageHor = filterRect.right - filterRect.left;

	uint32 tilesVert = (tileHeight - 1 + imageVert) / tileHeight;
	uint32 tilesHoriz = (tileWidth - 1 + imageHor) / tileWidth;

	int16 layerPlanes = 0;
	if (gFilterRecord->filterCase > 2)
		layerPlanes = gFilterRecord->outLayerPlanes;
	else
		layerPlanes = gFilterRecord->planes;

	int32 progress_total = layerPlanes * tilesVert;
	int32 progress_complete = 0;

	for(int16 planeCount = 0; planeCount < layerPlanes; planeCount++)
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
				
								
				int32 rowBytes = gFilterRecord->outRowBytes;
				uint8 * smallPixel = static_cast<uint8 *>(gFilterRecord->outData);
				uint16 * largePixel = static_cast<uint16 *>(gFilterRecord->outData);
				float * reallyLargePixel = static_cast<float *>(gFilterRecord->outData);

				uint32 rectHeight = outRect.bottom - outRect.top;
				uint32 rectWidth = outRect.right - outRect.left;
				
				
				
				for(uint32 pixelY = 0; pixelY < rectHeight; pixelY++)
				{
					
					for(uint32 pixelX = 0; pixelX < rectWidth; pixelX++)
					{
						if (gFilterRecord->depth == 32)
						{
							*reallyLargePixel = (float)(1.0) - *reallyLargePixel;
							reallyLargePixel++;
						} 
						else if (gFilterRecord->depth == 16)
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
					largePixel += rowBytes/2 - rectWidth;
					reallyLargePixel += rowBytes/4 - rectWidth;
					
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
	
	//Kill the scene memory
	PI3DKill3DLayer(&theLayer);
	
	//Do we want the results in a new layer or should the old layer get rasterized
	//You probably want to ask the use what they would like
	//gFilterRecord->createNewLayer=true;
	
	if(textureBuffer.pixels)
		delete []((uint8*)textureBuffer.pixels);
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