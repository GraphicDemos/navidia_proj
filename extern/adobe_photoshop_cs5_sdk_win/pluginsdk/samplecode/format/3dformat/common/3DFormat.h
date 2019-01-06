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

#ifndef __3DFormat_H__	// Has this not been defined yet?
#define __3DFormat_H__	// Only include this once by predefining it

#include "PIDefines.h"
#include "PIFormat.h"					// Format Photoshop header file.
#include "PIUtilities.h"				// SDK Utility library.
#include "FileUtilities.h"				// File Utility library.
#include "3DFormatTerminology.h"	// Terminology for plug-in.
#include <string>
#include <vector>

using namespace std;
//-------------------------------------------------------------------------------
//	Structure -- FileHeader
//-------------------------------------------------------------------------------
typedef struct TRevertInfo {

    /* dialog options */
	Boolean     doOptions;
	
  /* stuff I need to know */	
	int32		fImageWidth;
	int32		fImageHeight;

} TRevertInfo, *PRevertInfo, **HRevertInfo;

//---------------------------------------------------------------
//	Data -- structures
//-------------------------------------------------------------------------------

typedef struct Data {
    Ptr             gFileBuffer;
    BufferID        idFileBuffer;
	short           gPassRows;
	//int32			gU3DDataSize;
} Data;
	
typedef struct ResourceInfo {
	uint32 totalSize;
	uint32 type;
	uint16 id;
	string name;
	uint32 size;
	bool keep;
} ResourceInfo;

extern SPPluginRef gPluginRef;
extern FormatRecord * gFormatRecord;
extern Data * gData;
extern int16 * gResult;

//-------------------------------------------------------------------------------
//	Prototypes
//-------------------------------------------------------------------------------

void DoAbout (AboutRecordPtr about); 	   		// Pop about box.

bool DoUI (vector<ResourceInfo *> & rInfos);

// During read phase:
Boolean ReadScriptParamsOnRead (void);	// Read any scripting params.
OSErr WriteScriptParamsOnRead (void);	// Write any scripting params.

// During write phase:
Boolean ReadScriptParamsOnWrite (void);	// Read any scripting params.
OSErr WriteScriptParamsOnWrite (void);	// Write any scripting params.

//-------------------------------------------------------------------------------

#endif // __3DFormat_H__
