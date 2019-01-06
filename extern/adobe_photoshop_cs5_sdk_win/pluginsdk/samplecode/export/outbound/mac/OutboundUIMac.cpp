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
//		OutboundUIMac.cpp
//
//	Copyright 1990-1992, Thomas Knoll.
//
//	Description:
//		This file contains the user interface source
//		for the Export module Outbound, a module that
//		creates a file and stores raw pixel data in it.
//
//	Use:
//		This module shows how to export raw data to a file.
//		It uses a simple "FileUtilities" library that comes
//		with the SDK.  You use it via File>>Export>>Outbound.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "Outbound.h"
#include <Navigation.h>

//-------------------------------------------------------------------------------
//	Definitions -- Dialog items
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//
//	DoAbout
//
//	Displays the About box.
//
//	Inputs:
//		AboutRecordPtr about	Is actually a pointer to some platform
//								specific data for Windows only.
//		AboutID					#define with ID of String resource to use for text
//
//	Outputs:
//		Displays About box (Dialog).  Press Escape, Enter, or Return to
//		dismiss, or click the mouse.
//
//-------------------------------------------------------------------------------

/*****************************************************************************/

static FSSpec reply;

/*****************************************************************************/

void DoAbout (AboutRecordPtr /*about*/)
{
	ShowAbout (AboutID);
} // end DoAbout

/*****************************************************************************/

Boolean DoUI (GPtr globals)
{

	if (!gQueryForParameters) return TRUE;

	Str255 prompt;
	OSErr err = noErr;
	NavDialogRef dialogRef;
	NavReplyRecord navReply;
	NavDialogCreationOptions navOptions;
	NavUserAction action;
	//int32 nameLength = 0;
	
	// gReply->validRecord = false;

	// this needs to happen first ( setup the dialog creation options )
	err = NavGetDefaultDialogCreationOptions( &navOptions );

	// setup the strings
	PIGetString(kPrompt, prompt);
	navOptions.windowTitle = CFStringCreateWithPascalString(NULL, prompt, kCFStringEncodingMacRoman);
	navOptions.message = CFStringCreateWithPascalString(NULL, "\pOutbound to file:", kCFStringEncodingMacRoman);
	navOptions.saveFileName = CFStringCreateWithPascalString(NULL, gStuff->filename, kCFStringEncodingMacRoman);

	// Setup flags for the default supports we want
	navOptions.optionFlags |= kNavNoTypePopup;
	navOptions.preferenceKey =  'p2il';
	navOptions.modality = kWindowModalityAppModal;

	err = NavCreatePutFileDialog( 	&navOptions,
									'????',
									'8BIM',
									NULL,
									(void *) globals,
									&dialogRef );
									
	err = NavDialogRun ( dialogRef );

	err = NavDialogGetReply( dialogRef, &navReply );

	// what did the user do?
	action = NavDialogGetUserAction( dialogRef );

	if ( navReply.validRecord && action == kNavUserActionSaveAs )
		{
		FSRef resultFSRef;
		AEKeyword dummyKeyword = 0L;
		DescType  dummyType;
		Size      dummySize;

		// move the data into a real FSSpec
		err = AEGetNthPtr ( &(navReply.selection),
							1,
							typeFSRef,
							&dummyKeyword, 
							&dummyType,	
							&resultFSRef, 
							sizeof(FSRef), 
							&dummySize);

		if ( noErr == err )
			{
			FSRef tempWorkingRef;
			CFRange range;
			CFIndex length;
			UniChar *nameString;
			FSCatalogInfo catInfo;
			FileInfo *info;
			
			memset( &catInfo, 0x00, sizeof(FSCatalogInfo));
			
			// setup the filetype and creator we'll be using for the FSCreate operation
			info = (FileInfo *)&(catInfo.finderInfo);
			info->fileType = '????';
			info->fileCreator = '8BIM';

			// setup the length and buffer for the unicode string
			length = CFStringGetLength( navReply.saveFileName );
			nameString = (UniChar *)NewPtr(length*sizeof(UniChar));
			
			// setup the unicode range of characters to use
			range.location = 0;
			range.length = length;
			
			// Get the unicode name of the file
			CFStringGetCharacters( navReply.saveFileName, range, nameString );
			
			// if we can find the object already, delete it, Nav has already asked
			// us if we really want to do that.
			if ( noErr == FSMakeFSRefUnicode( &resultFSRef, length, nameString,
				kTextEncodingUnknown, &tempWorkingRef ))
				{
				FSDeleteObject( &tempWorkingRef );
				}
			
			// create the file
			err = FSCreateFileUnicode( 	&resultFSRef,
										length,
										nameString,
										kFSCatInfoFinderInfo,
										&catInfo,
										NULL,
										&reply );
			
			if ( noErr == err )
				{
				//gReply->validRecord = true;
				//gReply->replacing = navReply.replacing;
				// gReply.sfType = info->fileType;
				// gReply.sfIsFolder = false;
				// gReply.sfIsVolume = false;
				//gReply->keyScript = navReply.keyScript; // need to change back to keyScript when checked in
				}
			// clean up the unicode name	
			DisposePtr( (Ptr)nameString);											
			}
		}


	CFRelease(navOptions.message);
	CFRelease(navOptions.windowTitle);
	CFRelease(navOptions.saveFileName);
	
	navOptions.message = NULL;
	navOptions.windowTitle = NULL;
	navOptions.saveFileName = NULL;
	
	// clean up the reply
	err = NavDisposeReply( &navReply );
	
	// clean up the nav dialog
	NavDialogDispose( dialogRef );
	
	return TRUE;
}
/*****************************************************************************/

Boolean CreateExportFile (GPtr globals)
{
	return PIOpenFile(reply, 
					  &gFRefNum,
					  &gResult);
}

/*****************************************************************************/

Boolean CloseExportFile (GPtr globals)
{
	return PICloseFileAndMakeAlias(reply,
					               gFRefNum,
					               gSameNames,
					               &gStuff->dirty,
					               &gAliasHandle,
					               &gResult);	
}
//-------------------------------------------------------------------------------
// end OutboundUIMac.cpp