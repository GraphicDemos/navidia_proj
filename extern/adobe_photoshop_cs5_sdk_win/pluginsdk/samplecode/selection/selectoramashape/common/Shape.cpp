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
//		Shape.cpp
//
//	Description:
//		This file contains the source and functions for the
//		Import module Shape, which returns a path, layer,
//		or selection with an interesting shape.
//
//	Use:
//		This module specifically works the path return
//		functionality of Photoshop.  The Paths are stored
//		by derezing a work path saved in Photoshop then
//		storing them in a "Path" resource, which is a
//		binary data resource of a "Path" as defined in the
//		"Path layout" section of the Photosop File Format.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "Shape.h"

//-------------------------------------------------------------------------------
//	Globals -- Define global variables for plug-in scope.
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain1 (const int16 selector,
						             PISelectionParams *selectionParamBlock,
						             intptr_t *data,
						             int16 *result);
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
//		const short selector						Host provides selector indicating
//													what command to do.
//
//		PISelectionParams *selectionParamBlock		Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PISelection.h.
//
//	Outputs:
//		PISelectionParams *selectionParamBlock		Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PISelection.h.
//
//		void *data									Use this to store a handle or pointer to our
//													global parameters structure, which
//													is maintained by the host between
//													calls to the plug-in.
//
//		short *result								Return error result or noErr.  Some
//													errors are handled by the host, some
//													are silent, and some you must handle.
//													See PIGeneral.h.
//
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain1 (const short selector,
						             PISelectionParams *selectionParamBlock,
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

	if (selector == selectionSelectorAbout)
	{
		// we dont have to because Selectorama did it already or will do it soon
		// sSPBasic = ((AboutRecordPtr)selectionParamBlock)->sSPBasic;
		// DoAbout((AboutRecordPtr)selectionParamBlock);
	}
	else
	{ // do the rest of the process as normal:

		sSPBasic = ((PISelectionParams*)selectionParamBlock)->sSPBasic;

		Ptr globalPtr = NULL;		// Pointer for global structure
		GPtr globals = NULL; 		// actual globals

		//-----------------------------------------------------------------------
		//	(2) Allocate and initalize globals.
		//
		// 	AllocateGlobals requires the pointer to result, the pointer to the
		// 	parameter block, a pointer to the handle procs, the size of our local
		// 	"Globals" structure, a pointer to the long *data, a Function
		// 	Proc (FProcP) to the InitGlobalsShape routine.  It automatically sets-up,
		// 	initializes the globals (if necessary), results result to 0, and
		// 	returns with a valid pointer to the locked globals handle or NULL.
		//-----------------------------------------------------------------------
		
		globalPtr = AllocateGlobals (result,
									 selectionParamBlock,
									 selectionParamBlock->handleProcs,
									 sizeof(Globals),
						 			 data,
						 			 InitGlobalsShape);
		
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
		//	(3) Dispatch selector.
		//-----------------------------------------------------------------------

		DoExecuteShape(globals);
			
		//-----------------------------------------------------------------------
		//	(4) Unlock data, and exit resource.
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
//	InitGlobalsShape
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

void InitGlobalsShape (Ptr globalPtr)
{	
	// create "globals" as a our struct global pointer so that any
	// macros work:
	GPtr globals = (GPtr)globalPtr;
	
	// Initialize global variables:
	gQueryForParameters = true;
	ValidateParameters (globals);
	
} // end InitGlobalsShape

//-------------------------------------------------------------------------------
//
//	ValidateParameters
//
//	Initialize parameters to default values.
//
//	Inputs:
//		GPtr globals		Pointer to global structure.
//
//	Outputs:
//		gWhatShape			Default: iShapeTriangle.
//
//		gCreate				Default: iCreateSelection.
//
//		gIdleAmount			Default: 2.0 seconds.
//
//-------------------------------------------------------------------------------

void ValidateParameters (GPtr globals)
{
	gWhatShape = iShapeTriangle;
	gCreate = iCreateSelection;

} // end ValidateParameters

//-------------------------------------------------------------------------------
//
//	DoExecuteShape
//
//	Main routine.  In this case, pop the UI then return the path.
//
//	Inputs:
//		GPtr globals		Pointer to global structure.
//
//	Outputs:
//		gResult				Returns noErr if completed without error.
//							Most probable error return, if any, is
//							not enough memory err.
//
//-------------------------------------------------------------------------------

void DoExecuteShape (GPtr globals)
{
	Boolean			doThis = true;

	gQueryForParameters = ReadScriptParamsShape (globals);


	if ( gQueryForParameters ) doThis = DoUIShape (globals);

	if ( doThis )
	{
		size_t size = 0;

		// PIGetResource returns a Handle to a cross-platform resource:		
		gStuff->newPath = PIGetResource (PathResource, (int32)(ResourceID+gWhatShape), &size);
		if (gStuff->newPath == NULL || size < 1)
		{
			gResult = memFullErr;
			return;
		}

		/* look in gStuff->supportedTreatments for support for this next thang */
		
		gStuff->treatment = KeyToEnumShape(EnumToKeyShape(gCreate,typeMyCreate),typeMyPISel);

		WriteScriptParamsShape (globals);
	} // user cancelled or dialog err or silent
}

//-------------------------------------------------------------------------------

// end Shape.cpp
