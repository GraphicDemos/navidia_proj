// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		3DFormat.h
//
//	Description:
//		This file contains the header prototypes and macros for the
//		File Format module 3DFormat, 
//		which writes a flat file with merged document pixels.
//
//	Use:
//		Format modules are called from the Save, Save as,
//		and Save a copy dialogs.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include <vector>
#include <sstream>
#include <time.h>
#include "3DFormat.h"
#include "PIUI.h"
#include "PI3D.h"
#include "PSScene.h"
#include "PS3DDescriptor.h"

using namespace std;

//-------------------------------------------------------------------------------
//	Prototypes
//-------------------------------------------------------------------------------
static void DoReadPrepare (void);
static void DoReadStart (void);
static void DoReadContinue (void);
static void DoReadFinish (void);
static void DoOptionsPrepare (void);
static void DoOptionsStart (void);
static void DoOptionsContinue (void);
static void DoOptionsFinish (void);
static void DoEstimatePrepare (void);
static void DoEstimateStart (void);
static void DoEstimateContinue (void);
static void DoEstimateFinish (void);
static void DoWritePrepare (void);
static void DoWriteStart (void);
static void DoWriteContinue (void);
static void DoWriteFinish (void);
static void DoFilterFile (void);


const int32 HEADER_CANT_READ = 0;
const int32 HEADER_VER1 = 1;
const int32 HEADER_VER2 = 2;

// let's use the TIFF spec. to do cross platform files
const int16 BIGENDIAN = 0x4d4d;
const int16 LITTLEENDIAN = 0x4949;
const int16 TESTENDIAN = 0x002a;

const int32 DESIREDMATTING = 0;

static int CheckIdentifier (char identifier []);
static void SetIdentifier (char identifier []);
static int32 RowBytes (void);

static void ReadSome (int32 count, void * buffer);
static void WriteSome (int32 count, void * buffer);
static void ReadRow (Ptr pixelData, bool needsSwap);
static void WriteRow (Ptr pixelData);
static void DisposeImageResources (void);
static void SwapRow(int32 rowBytes, Ptr pixelData);

static void InitData(void);
static void CreateDataHandle(void);
static void LockHandles(void);
static void UnlockHandles(void);

static VPoint GetFormatImageSize(void);
static void SetFormatImageSize(VPoint inPoint);
static void SetFormatTheRect(VRect inRect);


template <typename Result>
inline Result GetBigEndian(uint8 *& source)
{
	Result result (0);
	
	for (int bytes (0); bytes < sizeof(Result); ++bytes) 
	{
		result = result << 8;
		result |= *source++;
	}
	
	return result;
}

//-------------------------------------------------------------------------------
//	Globals -- Define global variables for plug-in scope.
//-------------------------------------------------------------------------------

SPBasicSuite * sSPBasic = NULL;
SPPluginRef gPluginRef = NULL;

FormatRecord * gFormatRecord = NULL;
// intptr_t * gDataHandle = NULL;
Data * gData = NULL;
int16 * gResult = NULL;


#define gCountResources gFormatRecord->resourceProcs->countProc
#define gGetResources   gFormatRecord->resourceProcs->getProc
#define gAddResource	gFormatRecord->resourceProcs->addProc

//-------------------------------------------------------------------------------
//
//	PluginMain / main
//
//	All calls to the plug-in module come through this routine.
//	It must be placed first in the resource.  To achieve this,
//	most development systems require this be the first routine
//	in the source.
//
//	The entrypoint will be "pascal void" for Macintosh,
//	"void" for Windows.
//
//	Inputs:
//		const int16 selector						Host provides selector indicating
//													what command to do.
//
//		FormatRecord * formatParamBlock				Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PIFormat.h.
//
//	Outputs:
//		FormatRecord * formatParamBlock				Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PIFormat.h.
//
//		void * data									Use this to store a handle or pointer to our
//													global parameters structure, which
//													is maintained by the host between
//													calls to the plug-in.
//
//		int16 * result								Return error result or noErr.  Some
//													errors are handled by the host, some
//													are silent, and some you must handle.
//													See PIGeneral.h.
//
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain (const int16 selector,
						             FormatRecordPtr formatParamBlock,
						             intptr_t * data,
						             int16 * result)
{

	//---------------------------------------------------------------------------
	//	(1) Update our global parameters from the passed in values.
	// 
	//	We removed that nasty passing around of globals. It's global right! So
	//	why pass it around. This also removes the use of some those nasty #defines.
	//---------------------------------------------------------------------------
	gFormatRecord = reinterpret_cast<FormatRecordPtr>(formatParamBlock);
	gPluginRef = reinterpret_cast<SPPluginRef>(gFormatRecord->plugInRef);
	gResult = result;
	gData = (Data*)*data;

	//---------------------------------------------------------------------------
	//	(2) Check for about box request.
	//
	// 	The about box is a special request; the parameter block is not filled
	// 	out, none of the callbacks or standard data is available.  Instead,
	// 	the parameter block points to an AboutRecord, which is used
	// 	on Windows.
	//---------------------------------------------------------------------------
	if (selector == formatSelectorAbout)
	{
		AboutRecordPtr aboutRecord = reinterpret_cast<AboutRecordPtr>(formatParamBlock);
		sSPBasic = aboutRecord->sSPBasic;
		gPluginRef = reinterpret_cast<SPPluginRef>(aboutRecord->plugInRef);
		DoAbout(gPluginRef, AboutID);
	}
	else
	{ // do the rest of the process as normal:

		sSPBasic = ((FormatRecordPtr)formatParamBlock)->sSPBasic;

		if (gCountResources == NULL ||
            gGetResources == NULL ||
            gAddResource == NULL ||
			gFormatRecord->advanceState == NULL)
		{
			*gResult = errPlugInHostInsufficient;
			return;
		}

		// new for Photoshop 8, big documents, rows and columns are now > 30000 pixels
		if (gFormatRecord->HostSupports32BitCoordinates)
			gFormatRecord->PluginUsing32BitCoordinates = true;

		//-----------------------------------------------------------------------
		//	(3) Allocate and initalize globals.
		//
		//-----------------------------------------------------------------------

 		if (gData == NULL)
		{
			gData = (Data*)malloc(sizeof(Data)); 
			//CreateDataHandle();
			//if (*gResult != noErr) return;
			//LockHandles();
			//if (*gResult != noErr) return;
			InitData();
		}

		//if (*gResult == noErr)
		//{
		//	LockHandles();
		//	if (*gResult != noErr) return;
		//}

		//-----------------------------------------------------------------------
		//	(4) Dispatch selector.
		//-----------------------------------------------------------------------
		switch (selector)
		{
			case formatSelectorReadPrepare:
				DoReadPrepare();
				break;
			case formatSelectorReadStart:
				DoReadStart();
				break;
			case formatSelectorReadContinue:
				DoReadContinue();
				break;
			case formatSelectorReadFinish:
				DoReadFinish();
				break;

			case formatSelectorOptionsPrepare:
				DoOptionsPrepare();
				break;
			case formatSelectorOptionsStart:
				DoOptionsStart();
				break;
			case formatSelectorOptionsContinue:
				DoOptionsContinue();
				break;
			case formatSelectorOptionsFinish:
				DoOptionsFinish();
				break;

			case formatSelectorEstimatePrepare:
				DoEstimatePrepare();
				break;
			case formatSelectorEstimateStart:
				DoEstimateStart();
				break;
			case formatSelectorEstimateContinue:
				DoEstimateContinue();
				break;
			case formatSelectorEstimateFinish:
				DoEstimateFinish();
				break;

			case formatSelectorWritePrepare:
				DoWritePrepare();
				break;
			case formatSelectorWriteStart:
				DoWriteStart();
				break;
			case formatSelectorWriteContinue:
				DoWriteContinue();
				break;
			case formatSelectorWriteFinish:
				DoWriteFinish();
				break;

			case formatSelectorFilterFile:
				DoFilterFile();
				break;
		}
			
		//-----------------------------------------------------------------------
		//	(5) Unlock data, and exit resource.
		//
		//	Result is automatically returned in *result, which is
		//	pointed to by gResult.
		//-----------------------------------------------------------------------	
		
		// UnlockHandles();
	
	} // about selector special		

	// release any suites that we may have acquired
	if (selector == formatSelectorAbout ||
		selector == formatSelectorWriteFinish ||
		selector == formatSelectorReadFinish ||
		selector == formatSelectorOptionsFinish ||
		selector == formatSelectorEstimateFinish ||
		selector == formatSelectorFilterFile ||
		*gResult != noErr)
	{
		PIUSuitesRelease();
	}

} // end PluginMain


//-------------------------------------------------------------------------------
//
//	InitData
//	
//	Initalize any global values here.  Called only once when global
//	space is reserved for the first time.
//
//	Outputs:
//		Initializes any global values with their defaults.
//
//-------------------------------------------------------------------------------

static void InitData (void)
{	

} // end InitData

static void RenderThumbnail(Data *data,FormatRecordPtr fmtRec,PI3DScene *scene,RenderRecord &tempRenderRecord) 
{ 
	{
	unsigned long   *rowPtr;
	long 			y;
	
	tempRenderRecord.fileSpec2=fmtRec->fileSpec2;
	tempRenderRecord.renderState.quality = kRenderQualityHigh;
	tempRenderRecord.renderState.currentTime = 0.0f;
	tempRenderRecord.drawBuffer.sceneSize.h = fmtRec->imageSize.h;
	tempRenderRecord.drawBuffer.sceneSize.v = fmtRec->imageSize.v;
	tempRenderRecord.sSPBasic=fmtRec->sSPBasic;
	tempRenderRecord.drawBuffer.pixels = data->gFileBuffer;
	tempRenderRecord.drawBuffer.rowBytes = fmtRec->rowBytes;
	tempRenderRecord.drawBuffer.planeMap[0]=fmtRec->planeMap[0];
	tempRenderRecord.drawBuffer.planeMap[1]=fmtRec->planeMap[1];
	tempRenderRecord.drawBuffer.planeMap[2]=fmtRec->planeMap[2];
	tempRenderRecord.drawBuffer.planeMap[3]=fmtRec->planeMap[3];
		
	VRect theRect;
	theRect.left = 0;
	theRect.right = fmtRec->imageSize.h;
	
	int32 nextRowChunk=data->gPassRows;
	uint32 rowsDone=0;
	
	while(nextRowChunk)
		{
		nextRowChunk = fmtRec->imageSize.v - rowsDone;
		if (nextRowChunk > data->gPassRows)
			nextRowChunk = data->gPassRows;
		
		if( !nextRowChunk)
			break;
					
		theRect.top = rowsDone;
		theRect.bottom = rowsDone + nextRowChunk;
			
		tempRenderRecord.drawBuffer.tileRect = theRect;
		
		//еее Draw!!! Not implemented yet - but here is where you would render your thumbnail
		
		//Copy each row into the document buffer
		for(y=0;y<nextRowChunk;y++)
			{
			theRect.top = rowsDone + y;
			theRect.bottom = rowsDone + y + 1;
			
			SetFormatTheRect(theRect);
			
			rowPtr=(unsigned long *)(data->gFileBuffer + y*fmtRec->rowBytes);
			
			fmtRec->data = rowPtr;
			
			*gResult = (fmtRec->advanceState) ();
			}
			
		rowsDone +=  nextRowChunk;

		}
	
	}
}

//-------------------------------------------------------------------------------
//
//	CheckIdentifier
//	
//	Check the passed in character array for our identifier.
//
//  Inputs:
//		array of characters representing the identifier
//	Outputs:
//		HEADER_CANT_READ		= I have no idea what this file is
//		HEADER_VER1	= This is my old header, it has 16 bit rows and columns
//		HEADER_VER2	= This is my NEW header, it has 32 bit rows and columns
//
//-------------------------------------------------------------------------------

static int CheckIdentifier (char identifier [])
{
	if (identifier[3] != 'b' ||
		identifier[4] != 'r' ||
		identifier[5] != 'a' ||
		identifier[6] != 'i' ||
		identifier[7] != 'n')
		return HEADER_CANT_READ;

	if (identifier[0] == 'o' && 
		identifier[1] == 'n' && 
		identifier[2] == 'e')
		return HEADER_VER1;

	if (identifier[0] == 'b' && 
		identifier[1] == 'i' && 
		identifier[2] == 'g')
		return HEADER_VER2;

	return HEADER_CANT_READ;
}

//-------------------------------------------------------------------------------
//
//	SetIdentifier
//	
//	Set the passed in character array to our identifier.
//
//  Inputs:
//		array of characters representing the identifier
//	Outputs:
//		array of characters = "bigbrain"
//
//-------------------------------------------------------------------------------

static void SetIdentifier (char identifier [])
{
	
	identifier[0] = 'b';
    identifier[1] = 'i';
    identifier[2] = 'g';
    identifier[3] = 'b';
    identifier[4] = 'r';
    identifier[5] = 'a';
    identifier[6] = 'i';
    identifier[7] = 'n';
	
}

/*****************************************************************************/

static int32 RowBytes (void)
{
	VPoint imageSize = GetFormatImageSize();
	return (imageSize.h * gFormatRecord->depth + 7) >> 3;
	
}

/*****************************************************************************/

static void DoReadPrepare (void)
{
	gFormatRecord->maxData = 0;
}

/*****************************************************************************/

static void ReadSome (int32 count, void * buffer)
{
	
	int32 readCount = count;
	
	if (*gResult != noErr)
		return;
	
	*gResult = PSSDKRead (gFormatRecord->dataFork, &readCount, buffer);
	
	if (*gResult == noErr && readCount != count)
		*gResult = eofErr;
	
}

/*****************************************************************************/

static void WriteSome (int32 count, void * buffer)
{
	
	int32 writeCount = count;
	
	if (*gResult != noErr)
		return;
	
	*gResult = PSSDKWrite (gFormatRecord->dataFork, &writeCount, buffer);
	
	if (*gResult == noErr && writeCount != count)
		*gResult = dskFulErr;
	
}

/*****************************************************************************/

static void ReadRow (Ptr pixelData, bool needsSwap)
{
	ReadSome (RowBytes(), pixelData);
	if (gFormatRecord->depth == 16 && needsSwap)
		SwapRow(RowBytes(), pixelData);
}

static void SwapRow(int32 rowBytes, Ptr pixelData)
{
	uint16 * bigPixels = reinterpret_cast<uint16 *>(pixelData);
	for (int32 a = 0; a < rowBytes; a+=2, bigPixels++)
		Swap(*bigPixels);
}

/*****************************************************************************/

static void WriteRow (Ptr pixelData)
{
	WriteSome (RowBytes(), pixelData);
}

/*****************************************************************************/

static void DisposeImageResources (void)
{
	
	if (gFormatRecord->imageRsrcData)
	{
		
		sPSHandle->Dispose(gFormatRecord->imageRsrcData);
		
		gFormatRecord->imageRsrcData = NULL;
		
		gFormatRecord->imageRsrcSize = 0;
		
	}
	
}

/*****************************************************************************/

static void DoReadStart (void)
{
	FormatRecordPtr fmtRec = gFormatRecord;
	
	gData->gPassRows=256;
	
	int32 bytesPerPixel=4;
	
	fmtRec->imageMode = plugInModeRGBColor;
	fmtRec->depth = 8;
	fmtRec->planes = 4;
	fmtRec->planeBytes = 1;	

#ifdef WIN32
	fmtRec->planeMap[0] = 2;	// BGRA - non associated
	fmtRec->planeMap[1] = 1;
	fmtRec->planeMap[2] = 0;
	fmtRec->planeMap[3] = 3;
#else
	fmtRec->planeMap[0] = 3;	// BGRA - non associated
	fmtRec->planeMap[1] = 2;
	fmtRec->planeMap[2] = 1;
	fmtRec->planeMap[3] = 0;
#endif
	fmtRec->loPlane = 0;
	fmtRec->hiPlane = fmtRec->planes-1;
	fmtRec->colBytes = (int16)bytesPerPixel;
	fmtRec->imageHRes = 72*65536L;
	fmtRec->imageVRes = 72*65536L;	

	gData->gPassRows=1024;
	
	int32 fImageWidth=0;
	int32 fImageHeight=0;
	
	if(fmtRec->preferredSize.h>0)
		fImageWidth=fmtRec->preferredSize.h;
	if(fmtRec->preferredSize.v>0)
		fImageHeight=fmtRec->preferredSize.v;
	
	if (!fmtRec->openForPreview)
		{
		if(fImageWidth==0 || fImageHeight == 0)
			{
			//Should pop a dialog here to ask for the width and height
			fImageWidth=gData->gPassRows;
			fImageHeight=gData->gPassRows;
			}
			
		if(*gResult != noErr)
			return;	
			
		fmtRec->rowBytes = bytesPerPixel*fImageWidth;
		}
	/*else
		{
		fImageWidth=gData->gPassRows;
		fImageHeight=gData->gPassRows;
		fmtRec->rowBytes = bytesPerPixel*fImageHeight;
		*gResult = HostAllocateBuffer (fmtRec->bufferProcs, fmtRec->rowBytes * gData->gPassRows, &gData->idFileBuffer);
		}*/
	
	fmtRec->imageSize.h = (short)fImageWidth;
	fmtRec->imageSize.v = (short)fImageHeight;
	
	fmtRec->imageSize32.h = fImageWidth;
	fmtRec->imageSize32.v = fImageHeight;

    fmtRec->data = NULL;
    
   /*
    if (*gResult == noErr) 
    	{
        if (gData->idFileBuffer)
            gData->gFileBuffer = HostLockBuffer (fmtRec->bufferProcs, gData->idFileBuffer, TRUE);
    	}
    else 
    	{
        if (gData->idFileBuffer) 
        	{
            HostFreeBuffer (fmtRec->bufferProcs, gData->idFileBuffer);
            gData->idFileBuffer = 0;
        
        	}
        
    	}*/
	
	//Tells host that it is 3D data
	fmtRec->renderRecord=(RenderRecordPtr)1;
        
}

/*****************************************************************************/
Malloc3D g3DMallocFunction;
Free3D g3DFreeFunction;
static void DoReadContinue (void)
{
	FormatRecordPtr fmtRec	= gFormatRecord;
	RenderRecordPtr	renderRecord = (RenderRecordPtr)fmtRec->renderRecord;
	
	RenderRecord tempRenderRecord;
	memset(&tempRenderRecord,0,sizeof(RenderRecord));
	
	uint16 path[2048];
		
	PI3DGetPathFromSpec(fmtRec->fileSpec2,path);

	SPErr error=0;
	
	PI3DDescriptorProcs descriptorProcs;
	
	if(!error)
		error = fmtRec->sSPBasic->AcquireSuite(kPSActionDescriptorSuite,
							  kPSActionDescriptorSuiteVersion, 
							  (const void**)&descriptorProcs.actionDescriptorProcs);	
						
	if(!error)
		error = fmtRec->sSPBasic->AcquireSuite(kPSActionListSuite,
							  kPSActionListSuiteVersion, 
							  (const void**)&descriptorProcs.actionListProcs);	

	if(!error)
		error = fmtRec->sSPBasic->AcquireSuite(kPSBasicActionControlSuite,
													  kPSBasicActionControlSuitePrevVersion,
													  (const void**)&descriptorProcs.actionControlProcs);
	if(!error)
		error = fmtRec->sSPBasic->AcquireSuite(kASZStringSuite,
													  kASZStringSuiteVersion1, 
													  (const void**)&descriptorProcs.zStringProcs);
	
	g3DMallocFunction = renderRecord->mallocFunc;
	g3DFreeFunction = renderRecord->freeFunc;

	//PI3DScene **sceneToPass=(PI3DScene **)new long;
	PI3DScene *scene=PI3DCreateScene(path);

	//*sceneToPass = scene;
	
	short parseResult=PI3DParseSceneIntoDescriptor(&descriptorProcs, scene, &tempRenderRecord.sceneDescriptor);

	//State
	
	error = fmtRec->sSPBasic->AcquireSuite(kPSBasicActionControlSuite,
                              kPSBasicActionControlSuitePrevVersion,
                              (const void**)&descriptorProcs.actionControlProcs);
	if(!error)
		error = fmtRec->sSPBasic->AcquireSuite(kASZStringSuite,
                              kASZStringSuiteVersion1, 
                              (const void**)&descriptorProcs.zStringProcs);	
	
	DescriptorTypeID typeID;
	PIActionDescriptor stateDescriptor=NULL;
	PIActionDescriptor SceneDescriptor = NULL;
	descriptorProcs.actionDescriptorProcs->Make(&stateDescriptor);
	if(!stateDescriptor)
		return;
	
	RenderState newrenderState;
	newrenderState.currentFieldOfView=42.0;//42 degrees is vertical fov for a 18mm lens on a Rebel XT
	newrenderState.currentObjectXScale=1.0;
	newrenderState.currentObjectYScale=1.0;
	newrenderState.currentObjectZScale=1.0;
	PI3DInitializeRenderSettings(newrenderState.currentRenderSettings);

	//specific format render state updates
	PI3DUpdateRenderState(&newrenderState);

	PI3DMakeStateDescriptor(&descriptorProcs, &newrenderState,NULL,stateDescriptor);
	
	descriptorProcs.actionControlProcs->StringIDToTypeID(key3DState,&typeID);
	descriptorProcs.actionDescriptorProcs->HandleToDescriptor(tempRenderRecord.sceneDescriptor,&SceneDescriptor);	
	descriptorProcs.actionDescriptorProcs->PutObject(SceneDescriptor,typeID,typeID,stateDescriptor);
	descriptorProcs.actionDescriptorProcs->Free(stateDescriptor);
	if(tempRenderRecord.sceneDescriptor)
		fmtRec->handleProcs->disposeProc(tempRenderRecord.sceneDescriptor);
	descriptorProcs.actionDescriptorProcs->AsHandle(SceneDescriptor, &tempRenderRecord.sceneDescriptor);
	descriptorProcs.actionDescriptorProcs->Free(SceneDescriptor);
	//end Stste
		
	// this code used to send block of user defined data
	ResourceProcs* sResourceSuite = NULL;
	if (fmtRec->sSPBasic->AcquireSuite(kPIResourceSuite, kPIResourceSuiteVersion, (const void **)&sResourceSuite))
	{
		return;
	}	
#if  defined (__ppc__)
	const ResType userDefType = 'tRsU';	// this is the type reversed for big endian
#else
	const ResType userDefType = 'UsRt'; // this the type you define for your format
#endif
		std::string data("block of data");
		int dataSZ = static_cast<int>(data.size());
		Handle ourData = NULL;
		uint16 num = sResourceSuite->countProc(userDefType);
		if (num <= 1){
			if (num == 1){		/// this had better be ours!
				sResourceSuite->deleteProc(userDefType,num);
			}
			ourData = fmtRec->handleProcs->newProc(dataSZ);
			fmtRec->handleProcs->lockProc(ourData, false);
			std::memcpy(*ourData,data.c_str(),dataSZ);
			fmtRec->handleProcs->unlockProc(ourData);
			sResourceSuite->addProc(userDefType,ourData);
			fmtRec->handleProcs->disposeProc(ourData);
		}

	fmtRec->sSPBasic->ReleaseSuite(kPIResourceSuite, kPIResourceSuiteVersion);
	// end send new user data

	if(descriptorProcs.actionDescriptorProcs)
		fmtRec->sSPBasic->ReleaseSuite(kPSActionDescriptorSuite, kPSActionDescriptorSuiteVersion);	

	
	if(descriptorProcs.actionListProcs)
		fmtRec->sSPBasic->ReleaseSuite(kPSActionListSuite, kPSActionListSuiteVersion);	

	if(descriptorProcs.actionControlProcs)
		fmtRec->sSPBasic->ReleaseSuite(kPSBasicActionControlSuite, kPSBasicActionControlSuitePrevVersion);

	if(descriptorProcs.zStringProcs)
		fmtRec->sSPBasic->ReleaseSuite(kASZStringSuite, kASZStringSuiteVersion1);

	
	if(!parseResult)
		{
		*gResult=formatCannotRead;
		return;
		}
		
	if(renderRecord && renderRecord != (RenderRecordPtr) 1)
		{
		renderRecord->data = NULL;
		renderRecord->dataSize = 0;
		renderRecord->sceneDescriptor=tempRenderRecord.sceneDescriptor;
		renderRecord->texturesExternal=true;
		}
		
		
	PI3DKillScene(scene);
	//delete sceneToPass;
		
	//Only if this is for a preview do we render something 
	if(fmtRec->openForPreview || !renderRecord || renderRecord == (RenderRecordPtr)1)
		{
		RenderThumbnail(gData,fmtRec,scene,tempRenderRecord);
		
		if(tempRenderRecord.sceneDescriptor)
			fmtRec->handleProcs->disposeProc(tempRenderRecord.sceneDescriptor);
		}
		
	fmtRec->data = NULL; 	
}

/*****************************************************************************/

static void DoReadFinish (void)
{
	FormatRecordPtr fmtRec	= gFormatRecord;
    fmtRec->data = NULL;

    if (gData->idFileBuffer) 
    	{
        //HostUnlockBuffer (fmtRec->bufferProcs, gData->idFileBuffer);
        //HostFreeBuffer (fmtRec->bufferProcs,gData-> idFileBuffer);
        gData->idFileBuffer = 0;
        gData->gFileBuffer = NULL;
	    }
	
	RenderRecordPtr renderRecord = (RenderRecordPtr)fmtRec->renderRecord;
	if(!renderRecord || renderRecord == (RenderRecordPtr)1)
		return;
	
	if(renderRecord->sceneDescriptor)
		fmtRec->handleProcs->disposeProc(renderRecord->sceneDescriptor);
	renderRecord->sceneDescriptor=NULL;
	renderRecord->data=NULL;

	WriteScriptParamsOnRead (); // should be different for read/write

}

/*****************************************************************************/

static void DoOptionsPrepare (void)
{
	gFormatRecord->maxData = 0;
}

/*****************************************************************************/

static void DoOptionsStart (void)
{
	gFormatRecord->data = NULL;
}

/*****************************************************************************/

static void DoOptionsContinue (void)
{
}

/*****************************************************************************/

static void DoOptionsFinish (void)
{
}

/*****************************************************************************/

static void DoEstimatePrepare (void)
{
	gFormatRecord->maxData = 0;
}

/*****************************************************************************/

static void DoEstimateStart (void)
{
	
	gFormatRecord->minDataBytes = gFormatRecord->rowBytes * gFormatRecord->imageSize.v;
	gFormatRecord->maxDataBytes = gFormatRecord->rowBytes * gFormatRecord->imageSize.v;
	
	gFormatRecord->data = NULL;

}

/*****************************************************************************/

static void DoEstimateContinue (void)
{
}

/*****************************************************************************/

static void DoEstimateFinish (void)
{
}

/*****************************************************************************/

static void DoWritePrepare (void)
{
	gFormatRecord->maxData = 0;	
}

/*****************************************************************************/

static void DoWriteStart (void)
{
}

/*****************************************************************************/

static void DoWriteContinue (void)
{
}

/*****************************************************************************/

static void DoWriteFinish (void)
{
}

/*****************************************************************************/

static void DoFilterFile (void)
{	
}

static VPoint GetFormatImageSize(void)
{
	VPoint returnPoint = { 0, 0};
	if (gFormatRecord->HostSupports32BitCoordinates && gFormatRecord->PluginUsing32BitCoordinates)
	{
		returnPoint.v = gFormatRecord->imageSize32.v;
		returnPoint.h = gFormatRecord->imageSize32.h;
	}
	else
	{
		returnPoint.v = gFormatRecord->imageSize.v;
		returnPoint.h = gFormatRecord->imageSize.h;
	}
	return returnPoint;
}

static void SetFormatImageSize(VPoint inPoint)
{
	if (gFormatRecord->HostSupports32BitCoordinates && 
		gFormatRecord->PluginUsing32BitCoordinates)
	{
		gFormatRecord->imageSize32.v = inPoint.v;
		gFormatRecord->imageSize32.h = inPoint.h;
	}
	else
	{
		gFormatRecord->imageSize.v = static_cast<int16>(inPoint.v);
		gFormatRecord->imageSize.h = static_cast<int16>(inPoint.h);
	}
}
static void SetFormatTheRect(VRect inRect)
{
	if (gFormatRecord->HostSupports32BitCoordinates && 
		gFormatRecord->PluginUsing32BitCoordinates)
	{
		gFormatRecord->theRect32.top = inRect.top;
		gFormatRecord->theRect32.left = inRect.left;
		gFormatRecord->theRect32.bottom = inRect.bottom;
		gFormatRecord->theRect32.right = inRect.right;
	}
	else
	{
		gFormatRecord->theRect.top = static_cast<int16>(inRect.top);
		gFormatRecord->theRect.left = static_cast<int16>(inRect.left);
		gFormatRecord->theRect.bottom = static_cast<int16>(inRect.bottom);
		gFormatRecord->theRect.right = static_cast<int16>(inRect.right);
	}
}

//-------------------------------------------------------------------------------
//
// CreateDataHandle
//
// Create a handle to our Data structure. Photoshop will take ownership of this
// handle and delete it when necessary.
//-------------------------------------------------------------------------------
//static void CreateDataHandle(void)
//{
//	Handle h = sPSHandle->New(sizeof(Data));
//	if (h != NULL)
//		*gDataHandle = reinterpret_cast<intptr_t>(h);
//	else
//		*gResult = memFullErr;
//}

//-------------------------------------------------------------------------------
//
// LockHandles
//
// Lock the handles and get the pointers for gData and gParams
// Set the global error, *gResult, if there is trouble
//
//-------------------------------------------------------------------------------
//static void LockHandles(void)
//{
//	if ( ! (*gDataHandle) )
//	{
//		*gResult = formatBadParameters;
//		return;
//	}
//	
//	sPSHandle->SetLock(reinterpret_cast<Handle>(*gDataHandle), true, 
//		               reinterpret_cast<Ptr *>(&gData), NULL);
//	
//	if (gData == NULL)
//	{
//		*gResult = memFullErr;
//		return;
//	}
//}

//-------------------------------------------------------------------------------
//
// UnlockHandles
//
// Unlock the handles used by the data and params pointers
//
//-------------------------------------------------------------------------------
//static void UnlockHandles(void)
//{
//	if ((*gDataHandle))
//		sPSHandle->SetLock(reinterpret_cast<Handle>(*gDataHandle), false, 
//		                   reinterpret_cast<Ptr *>(&gData), FALSE);
//}

// end 3DFormat.cpp