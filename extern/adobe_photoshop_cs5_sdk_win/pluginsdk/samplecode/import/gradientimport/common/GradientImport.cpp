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
//		GradientImport.cpp
//
//	Copyright 1990, Thomas Knoll.
//	All Rights Reserved.
//
//	Description:
//		This file contains the source and functions for the
//		Import module GradientImport, a scriptable multiple-acquire
//		plug-in that creates 4 types of gradients in batches.
//
//	Use:
//		This import module is a great example of scripting
//		Import modules that use the old style Photoshop
//		multiple-acquire mechanism.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "GradientImport.h"


//-------------------------------------------------------------------------------
//	Globals -- Define global variables for plug-in scope.
//-------------------------------------------------------------------------------

SPBasicSuite * sSPBasic = NULL;

//-------------------------------------------------------------------------------
//	Globals -- Constants.
//-------------------------------------------------------------------------------

const uint8 kBitmapPlane = 1; // Bitmap cannot have alpha
const uint8 kSinglePlane = 2; // For Grayscale, Indexed, etc. = 1 plane + alpha
const uint8 kMultiPlane = 4; // For RGB = 3 planes + alpha

//-------------------------------------------------------------------------------
//	Prototypes.
//-------------------------------------------------------------------------------

void InitGlobals (Ptr globalPtr);
void ValidateParameters (GPtr globals);
void CreateDescriptor (GPtr globals);
void SwitchScriptInfo (GPtr globals, PIReadDescriptor readToken);
void DoPrepare (GPtr globals);
void DoStart (GPtr globals);
void DoContinue (GPtr globals);
void DoFinish (GPtr globals);
void DoFinalize (GPtr globals);

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
//		const int16 selector					Host provides selector indicating
//												what command to do.
//
//		AcquireRecord *acquireParamBlock		Host provides pointer to parameter
//												block containing pertinent data
//												and callbacks from the host.
//												See PIAcquire.h.
//
//	Outputs:
//		AcquireRecord *acquireParamBlock		Host provides pointer to parameter
//												block containing pertinent data
//												and callbacks from the host.
//												See PIAcquire.h.
//
//		void *data								Use this to store a handle or pointer to our
//												global parameters structure, which
//												is maintained by the host between
//												calls to the plug-in.
//
//		int16 *result							Return error result or noErr.  Some
//												errors are handled by the host, some
//												are silent, and some you must handle.
//												See PIGeneral.h.
//
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain (const int16 selector,
						             AcquireRecord *acquireParamBlock,
						             intptr_t *data,
						             int16 *result)
{
	//---------------------------------------------------------------------------
	//	(1) Check for about box request.
	//
	// 	The about box is a special request; the parameter block is not filled
	// 	out, none of the callbacks or standard data is available.  Instead,
	// 	the parameter block points to an AboutRecord, which is used
	// 	on Windows.
	//---------------------------------------------------------------------------

	if (selector == acquireSelectorAbout)
	{
		sSPBasic = ((AboutRecord*)acquireParamBlock)->sSPBasic;
		DoAbout((AboutRecordPtr)acquireParamBlock);
	}
	else
	{ // do the rest of the process as normal:

		sSPBasic = ((AcquireRecordPtr)acquireParamBlock)->sSPBasic;
		
		Ptr globalPtr = NULL;		// Pointer for global structure
		GPtr globals = NULL; 		// actual globals

		//-----------------------------------------------------------------------
		//	(2) Allocate and initalize globals.
		//
		// 	AllocateGlobals requires the pointer to result, the pointer to the
		// 	parameter block, a pointer to the handle procs, the size of our local
		// 	"Globals" structure, a pointer to the long *data, a Function
		// 	Proc (FProcP) to the InitGlobals routine.  It automatically sets-up,
		// 	initializes the globals (if necessary), results result to 0, and
		// 	returns with a valid pointer to the locked globals handle or NULL.
		//-----------------------------------------------------------------------
		
		globalPtr = AllocateGlobals (result,
									 acquireParamBlock,
									 acquireParamBlock->handleProcs,
									 sizeof(Globals),
						 			 data,
						 			 InitGlobals);
		
		if (globalPtr == NULL)
		{ // Something bad happened if we couldn't allocate our pointer.
		  // Fortunately, everything's already been cleaned up,
		  // so all we have to do is report an error.
		  
		  *result = memFullErr;
		  return;
		}
		
		// Get our "globals" variable assigned as a Global Pointer struct with the
		// data we've returned:
		globals = (GPtr)globalPtr;

		//-----------------------------------------------------------------------
		//	(5) Dispatch selector.
		//-----------------------------------------------------------------------

		switch (selector)
		{
			case acquireSelectorPrepare:
				DoPrepare(globals);
				break;
			case acquireSelectorStart:
				DoStart(globals);
				break;
			case acquireSelectorContinue:
				DoContinue(globals);
				break;
			case acquireSelectorFinish:
				DoFinish(globals);
				break;
			case acquireSelectorFinalize:
				DoFinalize(globals);
				break;
		}
				
			
		//-----------------------------------------------------------------------
		//	(6) Unlock data, and exit resource.
		//
		//	Result is automatically returned in *result, which is
		//	pointed to by gResult.
		//-----------------------------------------------------------------------	
		
		// unlock handle pointing to parameter block and data so it can move
		// if memory gets shuffled:
		if ((Handle)*data != NULL)
			PIUnlockHandle((Handle)*data);
	
	} // about selector special		
	
} // end PluginMain

//-------------------------------------------------------------------------------
//
//	InitGlobals
//	
//	Initalize any global values here.  Called only once when global
//	space is reserved for the first time.
//
//	Inputs:
//		Ptr globalPtr		Standard pointer to a global structure.
//
//	Outputs:
//		Initializes any global values with their defaults.
//
//-------------------------------------------------------------------------------

void InitGlobals (Ptr globalPtr)
{	
	// create "globals" as a our struct global pointer so that any
	// macros work:
	GPtr globals = (GPtr)globalPtr;
	
	// Initialize global variables:
	gQueryForParameters = true;
	gInitParameters = true;
	ValidateParameters (globals);
	
} // end InitGlobals

//-------------------------------------------------------------------------------
//
//	ValidateParameters
//
//	Initialize parameters to default values.
//
//	Inputs:
//		GPtr globals					Pointer to global structure.
//
//	Outputs:
//		gLastImages						Default: 0.
//
//		gLastRows						Default: 256.
//
//		gLastCols						Default: 256.
//
//		gLastMode						Default: plugInModeRGBColor.
//
//		gLastInvert						Default: false.
//
//		gContinueImport					Default: false.
//
//		gInitParameters					false.
//
//		gCount							Default: 0.
//
//		gToken, gDialog, gDialogHdl		NULL.
//
//		gArray []						NULL.
//
//-------------------------------------------------------------------------------

void ValidateParameters (GPtr globals)
{
	short loop;
		
	if (gInitParameters)
	{
		gDialog = NULL;

		#if __PIMac__
			gDialogHdl = NULL;	 
		#endif
	
		gLastImages = 0;
		gLastRows = 256;
		gLastCols = 256;
		gLastMode = plugInModeRGBColor;
		gLastInvert = false;
		gContinueImport = false;
		
		gInitParameters = false;
		
		gConvertAlpha = false;
		
		gHostIsPhotoshop401orLater = true;
		
		gCount = 0;
		gToken = NULL;
		
		for (loop = 0; loop < kMaxDescriptors; loop++)
			gArray [loop] = NULL;	
	}
	
} // end ValidateParameters

/*****************************************************************************/

/* Prepare to acquire an image.	If the plug-in module needs a large amount
   of buffer memory, this routine should set the maxData field to the
   number of bytes required.  Since we are going to use the bufferProcs,
   we simply set maxData to zero.  */

void DoPrepare (GPtr globals)
	{
	
	gStuff->maxData = 0;
	
	if (!WarnBufferProcsAvailable ())
		gResult = userCanceledErr; // exit gracefully
		
	/* If finalization is available, we will want it. */
	
	gStuff->wantFinalize = true;

	ValidateParameters (globals);
	/* if stuff hasn't been initialized that we need, do it,
		then go check if we've got scripting commands to
		override our settings */
	OpenScriptParams (globals); // update our parameters with the scripting parameters, if available

}


/*****************************************************************************/

void DoStart (GPtr globals)
{
	
	int16 j = 0;
	
	/* Insist on having the buffer procs. */
	
	if (!WarnBufferProcsAvailable ())
		{
		gResult = userCanceledErr; // exit gracefully
		return;
		}
		
	/* Assume we won't be coming back around for another pass unless explicitly so. */
		
	gStuff->acquireAgain = gContinueImport = false;
	
	/* If we need to use our dialog, or it's already up, do it. */
	
	ValidateParameters (globals);
	ReadScriptParams (globals);
	
	if (gQueryForParameters)
		{
		/* Open the dialog.  If it is already up, this does nothing.             */
	
		if (!OpenOurDialog (globals))
			{
			gQueryForParameters = false;
			CloseScriptParams(globals);
			gResult = memFullErr;
			return;
			}

		/* Run the parameters dialog.											 */
	
		if (!RunOurDialog (globals))
			{
			
			gQueryForParameters = false;
			
			CloseOurDialog (globals);
			CloseScriptParams(globals);
	
			gResult = userCanceledErr;
			return;
				
			}
		else
			{

			/* Still going! */
		
			gContinueImport = true;
		
			}
			
		/* If we cannot finalize, then we want to close the dialog. */
	
		if (!gStuff->canFinalize)
			{
			
			gQueryForParameters = false;
			CloseOurDialog (globals);
			CloseScriptParams(globals);
				
			}
			

		}
				
/* Fill in the parameters for the document. */
	
	gStuff->imageSize.v = gLastRows;
	gStuff->imageSize.h = gLastCols;
	gStuff->imageMode	= gLastMode;

	if (gStuff->imageMode == plugInModeBitmap)
	{
		gStuff->depth = 1;
		gStuff->imageHRes = FixRatio (300, 1);
		gStuff->imageVRes = FixRatio (300, 1);
	
		gStuff->planes = kBitmapPlane; // can't have an alpha!
	}
	else
	{
		gStuff->depth = 8;

		gStuff->imageHRes = FixRatio (72, 1);
		gStuff->imageVRes = FixRatio (72, 1);

		if (gStuff->imageMode == plugInModeRGBColor)
			gStuff->planes = kMultiPlane; // going to return an alpha (normally 3 planes)
		else
		{
			gStuff->planes = kSinglePlane; // return an alpha (normally 1 plane)
			if (gStuff->imageMode == plugInModeIndexedColor)
			{
				for (j = 0; j < 256; j++)
				{
					gStuff->redLUT	 [j] = (unsigned8) j;
					gStuff->greenLUT [j] = (unsigned8) (255 - j);
					gStuff->blueLUT  [j] = (unsigned8) j;
				}
			}
		}
	}
		
	if (gHostIsPhotoshop401orLater)
		gStuff->isFirstAlphaTransparency = gConvertAlpha;

	gStuff->data = nil;
	
}

/*****************************************************************************/

#define RANGE_ITER(lower,upper,first,last,step)								  \
	for (lower = (first);                         							  \
		 (upper = (((lower) + (short)(step) < (last)) ? (lower) + (short)(step) : (last))), \
		 lower < (last);													  \
		 lower = upper)

/*****************************************************************************/

void DoContinue (GPtr globals)
	{
	
	AcquireRegion region;
	AcquireDataLayout layout;
	Ptr data;
	int32 chunkRows;
	BufferID buffer = 0;
	int16 maxHeight;
	int16 actualHeight;
	
	long done = 0;
	long total = (long) gStuff->imageSize.v * (long) gStuff->planes;

	/* Scale total for post-processing. */
	
	if (gLastInvert && gStuff->canReadBack)
		total *= 3;
	
	/* Set up the layout. */
	
	if (gStuff->depth == 8)
		layout.rowBytes = gStuff->imageSize.h;
	else
		layout.rowBytes = (gStuff->imageSize.h + 7) >> 3;
	layout.colBytes = 1;
	layout.planeBytes = 0;	/* irrelevant -- we work one plane at a time */
	
	/* Allocate the buffer and start acquisition.   We limit the buffer
	   height to keep the space consumed by this plug-in down.   We base
	   this estimate on reserving enough space for one of row of tiles
	   with an assumed height of 300 pixels. */
	   
	maxHeight = (int16) ((BufferSpace () / layout.rowBytes) - 300);
	
	if (maxHeight > gStuff->imageSize.v)
		maxHeight = gStuff->imageSize.v;
	else if (maxHeight < 1)
		maxHeight = 1;
		
	gResult = AllocateStripBuffer (layout.rowBytes,
								   1,
								   maxHeight,
								   1,
								   &actualHeight,
								   &buffer);
	
	chunkRows = actualHeight;							   
	
	if (gResult != noErr)
		goto exit;
			
	data = LockBuffer (buffer, false);
	
	region.rect.left = 0;
	region.rect.right = gStuff->imageSize.h;
	
	for (region.loPlane = 0; region.loPlane < gStuff->planes; ++region.loPlane)
		{
		
		region.hiPlane = region.loPlane;
			
		RANGE_ITER (region.rect.top, region.rect.bottom,
					0, gStuff->imageSize.v, chunkRows)
			{
			
			Ptr p;
			int16 subRow;
			
			for (subRow = region.rect.top, p = data;
				 subRow < region.rect.bottom;
				 ++subRow, p += layout.rowBytes)
				{
				
				int16 col; 
				Ptr q;
	
				if (TestAbort ())
					{
					gResult = userCanceledErr;
					goto exit;
					}
					
				PIUpdateProgress (done++, total);
				
				for (col = 0, q = p; col < layout.rowBytes; ++col, ++q)
					{
					
					if (gStuff->depth == 1)
						*q = 0xFF << (subRow & 7);
					else if (gStuff->planes == kSinglePlane)
						*q = subRow + col;
					else if (region.loPlane == 0)
						*q = (char) col;
					else if (region.loPlane == 1)
						*q = (char ) subRow;
					else
						*q = subRow + col;
					
					}
					
				}
			
			gResult = StoreData (gStuff, data, &region, &layout);
			
			if (gResult != noErr)
				goto exit;
			
			}
			
		}
			
	if (gLastInvert && gStuff->canReadBack)
		{
		
		for (region.loPlane = 0; region.loPlane < gStuff->planes; ++region.loPlane)
			{
			
			region.hiPlane = region.loPlane;
				
			RANGE_ITER (region.rect.top, region.rect.bottom,
						0, gStuff->imageSize.v, chunkRows)
				{
				
				Ptr p;
				int16 subRow;
				
				gResult = FetchData (gStuff, data, &region, &layout);
				
				if (gResult != noErr)
					goto exit;
				
				done += region.rect.bottom - region.rect.top;
				
				for (subRow = region.rect.top, p = data;
					 subRow < region.rect.bottom;
					 ++subRow, p += layout.rowBytes)
					{
					
					int16 col; 
					Ptr q;
		
					if (TestAbort ())
						{
						gResult = userCanceledErr;
						goto exit;
						}
						
					PIUpdateProgress (done++, total);
					
					for (col = 0, q = p; col < layout.rowBytes; ++col, ++q)
						*q = ~*q;
					
					}
					
				gResult = StoreData (gStuff, data, &region, &layout);
				
				if (gResult != noErr)
					goto exit;
				
				}
				
			}
		
		}
		
	gStuff->data = nil;
	PISetRect (&gStuff->theRect, 0, 0, 0, 0);
	
	exit:
	
	if (buffer != 0)
		FreeBuffer (buffer);
		
	}

/*****************************************************************************/

/* This routine will always be called if DoStart does not return an error
   (even if DoContinue returns an error or the user aborts the operation).
   This allows the module to perform any needed cleanup. */

void DoFinish (GPtr globals)
	{

	gStuff->acquireAgain = gContinueImport;
	
	/* Store scripting parameters in object descriptor */
	CreateDescriptor(globals);
	
	/* Write our parameters if we can't finalize */
	
	if (!gStuff->canFinalize)
		CheckAndWriteScriptParams(globals); // writes script params
		
	}

/*****************************************************************************/

void DoFinalize (GPtr globals)
	{
	
	gQueryForParameters = false;
	CloseOurDialog (globals);
	
	/* Write our parameters */
	
	CheckAndWriteScriptParams(globals); // writes script params
	
	}

/*****************************************************************************/

OSErr StoreData (AcquireRecord *stuff,
				 void *data,
				 AcquireRegion *region,
				 AcquireDataLayout *layout)
	{
	
	OSErr result = noErr;
	
	if (!WarnHostAdvanceStateAvailable (stuff->advanceState))
		return userCanceledErr; // exit gracefully
		
	stuff->data = data;
	stuff->theRect = region->rect;
	stuff->loPlane = region->loPlane;
	stuff->hiPlane = region->hiPlane;
	stuff->rowBytes = layout->rowBytes;
	stuff->colBytes = (short)layout->colBytes;
	stuff->planeBytes = layout->planeBytes;
	
	stuff->wantReadBack = false;
	
	result = (*(stuff->advanceState)) ();
	
	stuff->data = NULL;
	
	return result;
	
	}
	
/*****************************************************************************/

OSErr FetchData (AcquireRecord *stuff,
				 void *data,
				 AcquireRegion *region,
				 AcquireDataLayout *layout)
	{
	
	OSErr result = noErr;
	
	if (!WarnHostAdvanceStateAvailable (stuff->advanceState))
		return userCanceledErr; // exit gracefully
		
	if (!stuff->canReadBack)
		return errPlugInHostInsufficient;
		
	stuff->data = data;
	stuff->theRect = region->rect;
	stuff->loPlane = region->loPlane;
	stuff->hiPlane = region->hiPlane;
	stuff->rowBytes = layout->rowBytes;
	stuff->colBytes = (short)layout->colBytes;
	stuff->planeBytes = layout->planeBytes;
	
	stuff->wantReadBack = true;
	
	result = (*(stuff->advanceState)) ();
	
	stuff->data = NULL;
	stuff->wantReadBack = false;
	
	return result;
	
	}

/*****************************************************************************/
// end GradientImport.cpp
