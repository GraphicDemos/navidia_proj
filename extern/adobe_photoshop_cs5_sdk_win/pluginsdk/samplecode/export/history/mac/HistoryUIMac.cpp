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
//		HistoryUIMac.cpp
//
//	Copyright 1990-1992, Thomas Knoll.
//
//	Description:
//		This file contains the user interface routines
//		for the Export module History, a module that
//		displays and lets the user manipulate
//		pseudo-resources of type 'hist'.
//
//	Use:
//		This module shows how to examine, display, and work
//		with pseudo-resources.  An additional element might
//		be to have it export those remaining pseudo-resources
//		to a text file.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "History.h"

//-------------------------------------------------------------------------------
//	Definitions -- Dialog items
//-------------------------------------------------------------------------------

const uint8 kDTrimFirst = 3;
const uint8 kDTrimLast = kDTrimFirst+1;

const uint8 kDUpButton = 5;
const uint8 kDDownButton = kDUpButton+1;

const uint8 kDStatusText = 7;

const uint8 kDHistTotal = 7;
const uint8 kDHistItem1 = kDStatusText+1;
const uint8 kDHistItemEnd = kDHistItem1 + kDHistTotal;

//-------------------------------------------------------------------------------
//	Prototypes
//-------------------------------------------------------------------------------

void UpdateHistories (GPtr globals,
					  DialogPtr dp,
					  short count,
					  Str255 hS);

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

void DoAbout (AboutRecordPtr /*about*/)
{
	ShowAbout (AboutID);
} // end DoAbout

/*****************************************************************************/

#if !__LP64__

/*****************************************************************************/

/* Setup the parameters dialog.  Returns TRUE if it succeeds.				 */

Boolean DoUI (GPtr globals)
{		
	short item;
	Str255 hS = ""; //histstatus
	int16	currentResources = 0;

	DialogPtr dp;
	DialogTHndl dt;
	
	gStuff->theRect.top =
	gStuff->theRect.left =
	gStuff->theRect.bottom =
	gStuff->theRect.right = 0;
	
	dt = (DialogTHndl) GetResource ('DLOG', uiID);
	HNoPurge ((Handle) dt);
	
	CenterDialog (dt);

	dp = GetNewDialog (uiID, nil, (WindowPtr) -1);

	(void) SetDialogDefaultItem (dp, ok);
	(void) SetDialogCancelItem (dp, cancel);

	PIGetString(kHistStatus, hS); // get status string
	
	do
	{
		
		currentResources = CountPIResources(histResource);
		
		if (gCurrentHistory < 1)
			gCurrentHistory = 1;
		if (gCurrentHistory > currentResources)
			gCurrentHistory = currentResources;
		
		if (currentResources <= kDHistTotal || 
			gCurrentHistory+kDHistTotal > currentResources)
			PIDisableControl (dp, kDDownButton); // nothing extra to show
		else
			PIEnableControl (dp, kDDownButton);
		
		if (gCurrentHistory < 2)
			PIDisableControl (dp, kDUpButton);
		else
			PIEnableControl (dp, kDUpButton);
		
		if (currentResources >= gCurrentHistory && currentResources > 0)
			{
			PIEnableControl (dp, kDTrimFirst);
			PIEnableControl (dp, kDTrimLast);
			}
		else
		{
			PIDisableControl (dp, kDTrimFirst);
			PIDisableControl (dp, kDTrimLast);
		}
		
		UpdateHistories(globals, dp, currentResources, hS);

		MoveableModalDialog (dp, gStuff->processEvent, nil, &item);

		switch (item)
		{
			case cancel:
				gResult = userCanceledErr;
				// have to set this so we don't get recorded
				break;
			case kDTrimFirst:
				if (currentResources >= gCurrentHistory)
				{
					DeletePIResource (histResource, gCurrentHistory);
					gStuff->dirty = TRUE;
				}
				break;
			case kDTrimLast:
				if (currentResources >= gCurrentHistory + kDHistTotal-1)
				{
					DeletePIResource (histResource, gCurrentHistory+ kDHistTotal-1);
					gStuff->dirty = TRUE;
				}
				else if (currentResources > 0)
				{
					DeletePIResource (histResource, currentResources);
					gStuff->dirty = TRUE;
				}
				break;
			case kDUpButton:
				gCurrentHistory--;
				break;
			case kDDownButton:
				gCurrentHistory++;
				break;
		} // end switch (item)
	} while (item != ok && item != cancel);

	DisposeDialog (dp);
	HPurge ((Handle) dt);
	
	return (item == ok);
}

/*********************************************************************/

void UpdateHistories(GPtr globals, DialogPtr dp, short count, Str255 hS)
{
	Str255 	s = "";
	Str255  n1 = "";
	Str255	n2 = "";
	Str255	nT = "";
	Str255	ss = "";
	long	x = gCurrentHistory + (kDHistTotal - 1);
	short	loop;

	PICopy(ss, hS, hS[0]+1); // make a new copy

	for (loop = gCurrentHistory; loop < gCurrentHistory + kDHistTotal; loop++)
	{
		GetHistory (globals, loop, s);
		if (loop == gCurrentHistory && !s[0])
		{
			PIGetString(kNoHistories, s);
			PIResetString(ss);
		}
		StuffText (dp, kDHistItem1 + (loop - gCurrentHistory), s);
		if (s[0] < 1 && x > loop-1)
			x = loop-1;
	}

	if (ss[0] > 0)
	{ // got the display string.  Populate it.
		NumToString(gCurrentHistory, n1);
		NumToString(x, n2);
		NumToString(count, nT);
		PIParamText(ss, n1, n2, nT);
	}
	StuffText (dp, kDStatusText, ss);
	// even if nothing, stuff empty or string in field
}

//-------------------------------------------------------------------------------

#else // #if !__LP64__

//-------------------------------------------------------------------------------

Boolean DoUI (GPtr globals)
{
	return FALSE;
}

//-------------------------------------------------------------------------------

#endif // #if !__LP64__

//-------------------------------------------------------------------------------

// end HistoryUIMac.cpp
